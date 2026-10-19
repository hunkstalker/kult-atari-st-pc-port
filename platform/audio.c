/* platform/audio.c — emulación del YM2149 y mezcla del port PC (M4/T-041).
 *
 * El juego no sintetiza sonido: escribe los 16 registros del PSG/YM2149 (por
 * `psg_port_write`, tanto desde las rutinas del core como desde las ISR del Timer
 * A) y este módulo los convierte en muestras. El modelo es el mínimo que reproduce
 * el juego:
 *   - 3 canales de onda cuadrada (periodos R0..R5),
 *   - ruido de 17 bits (R6),
 *   - mezclador (R7),
 *   - volúmenes de 4 bits con tabla logarítmica (R8..R10), con envolvente
 *     aproximada (R11..R13),
 *   - un filtro de continua a la salida: el PCM del juego es unipolar por niveles
 *     de volumen y hay que centrarlo.
 *
 * `platform_audio_advance(psg, us, gain)` genera las muestras de un intervalo de
 * tiempo *emulado* (lo llama el modelo del Timer A), las encola en un anillo
 * single-producer/single-consumer y acumula energía para la verificación objetiva
 * (docs/VERIFICACION.md: RMS/energía). La salida SDL (T-042) vive en
 * `platform/audio_sdl.c` y drena el anillo.
 *
 * No pretende ser un YM exacto (la escucha final la valida el usuario); sí ser
 * determinista y medible.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "psg.h"
#include "platform.h"
#include "platform_internal.h"

#define YM_CLOCK  2000000u      /* reloj del YM2149 en el ST */
#define OUT_RATE  48000         /* frecuencia de salida (mono S16) */

#define RING_BITS 15
#define RING_SIZE (1u << RING_BITS)
#define RING_MASK (RING_SIZE - 1u)

static int16_t  g_ring[RING_SIZE];
static volatile uint32_t g_head, g_tail;

static int      g_enabled = 1;
static int      g_out_rate = OUT_RATE;
static double   g_frac;
static int      g_gain = 100;           /* ganancia (%) del bloque en curso */
static int      g_music_gain = 25;      /* ganancia (%) del reproductor 0x3a872 */

/* Estado del generador. */
static uint32_t g_tone_acc[3], g_tone_bit[3];
static uint32_t g_noise_acc, g_noise_lfsr, g_noise_bit;
static int32_t  g_hp_x, g_hp_y;

/* Envolvente. */
static uint32_t g_env_acc;
static int      g_env_cnt;
static int      g_env_shape = -1;

/* Energía acumulada de lo renderizado (para la verificación objetiva). */
static uint64_t g_sum_sq;
static uint64_t g_n_samples;
static int32_t  g_peak;

static const uint8_t g_vol[16] = {
    0, 2, 3, 5, 8, 13, 20, 35, 40, 69, 95, 118, 146, 180, 215, 255
};

static int env_value(int shape, int cnt) {
    int t = cnt & 15, h = cnt & 16;
    switch (shape & 7) {
    case 0: return 15 - t;              /* \__ decae y mantiene 0 */
    case 1: return t;                   /* /__ ataca y mantiene 15 */
    case 2: return h ? 15 - t : t;      /* \/\/ */
    case 3: return h ? t : 15 - t;      /* /\/\ */
    case 4: return 15 - t;              /* triángulo */
    case 5: return t;
    case 6: return h ? 15 - t : t;
    default: return h ? t : 15 - t;
    }
}

static void ring_push(int16_t s) {
    uint32_t h = g_head, t = g_tail;
    if ((uint32_t)(h - t) >= RING_SIZE)
        return;                         /* lleno: se descarta (sin audio device) */
    g_ring[h & RING_MASK] = s;
    g_head = h + 1;
}

int platform_audio_pull(int16_t *out, int maxframes) {
    int n = 0;
    while (n < maxframes) {
        uint32_t t = g_tail, h = g_head;
        if (t == h)
            break;
        out[n++] = g_ring[t & RING_MASK];
        g_tail = t + 1;
    }
    return n;
}

static uint32_t isqrt64(uint64_t v) {
    uint64_t x = v, y = (x + 1) / 2;
    if (v == 0)
        return 0;
    while (y < x) {
        x = y;
        y = (x + v / x) / 2;
    }
    return (uint32_t)x;
}

int platform_audio_enabled(void) { return g_enabled; }

void platform_audio_set_enabled(int on) { g_enabled = on ? 1 : 0; }

void platform_audio_init(void) {
    const char *e = getenv("KULT_AUDIO");
    g_enabled = !(e && e[0] == '0');

    const char *mg = getenv("KULT_MUSIC_GAIN");
    if (mg && *mg) {
        int v = atoi(mg);
        g_music_gain = v < 0 ? 0 : (v > 200 ? 200 : v);
    }

    g_out_rate = OUT_RATE;
    g_frac = 0.0;
    g_head = g_tail = 0;
    memset(g_tone_acc, 0, sizeof g_tone_acc);
    memset(g_tone_bit, 0, sizeof g_tone_bit);
    g_noise_acc = 0;
    g_noise_lfsr = 0x1ffff;
    g_noise_bit = 1;
    g_hp_x = g_hp_y = 0;
    g_env_acc = 0;
    g_env_cnt = 0;
    g_env_shape = -1;
    g_sum_sq = 0;
    g_n_samples = 0;
    g_peak = 0;
}

void platform_audio_rms_reset(void) {
    g_sum_sq = 0;
    g_n_samples = 0;
    g_peak = 0;
}

uint32_t platform_audio_rms(void) {
    if (g_n_samples == 0)
        return 0;
    return isqrt64(g_sum_sq / g_n_samples);
}

uint32_t platform_audio_samples(void) { return (uint32_t)g_n_samples; }

uint32_t platform_audio_peak(void) {
    return g_peak < 0 ? (uint32_t)(-g_peak) : (uint32_t)g_peak;
}

/* Una muestra del YM a partir de los registros actuales. */
static int16_t ym_sample(const uint8_t *psg) {
    const int mixer = psg[7];
    int32_t sum = 0;

    /* Tono: f = YM_CLOCK/(16*period); el contador avanza 2f/Fs por muestra. */
    for (int c = 0; c < 3; c++) {
        uint32_t period = (uint32_t)psg[c * 2] | ((uint32_t)(psg[c * 2 + 1] & 0x0f) << 8);
        if (period == 0)
            period = 1;
        uint32_t lim = (uint32_t)g_out_rate * period;
        g_tone_acc[c] += YM_CLOCK / 8;
        while (g_tone_acc[c] >= lim) {
            g_tone_acc[c] -= lim;
            g_tone_bit[c] ^= 1u;
        }
    }

    /* Ruido: LFSR de 17 bits, f = YM_CLOCK/(16*period). */
    {
        uint32_t np = psg[6] & 0x1f;
        if (np == 0)
            np = 1;
        uint32_t lim = (uint32_t)g_out_rate * np;
        g_noise_acc += YM_CLOCK / 16;
        while (g_noise_acc >= lim) {
            g_noise_acc -= lim;
            uint32_t bit = (g_noise_lfsr ^ (g_noise_lfsr >> 3)) & 1u;
            g_noise_lfsr = (g_noise_lfsr >> 1) | (bit << 16);
        }
        g_noise_bit = g_noise_lfsr & 1u;
    }

    /* Envolvente: avanza si algún canal la usa (bit 4 del volumen). */
    int env_val = 0, use_env = 0;
    for (int c = 0; c < 3; c++)
        if (psg[8 + c] & 0x10)
            use_env = 1;
    if (use_env) {
        uint32_t P = (uint32_t)psg[11] | ((uint32_t)psg[12] << 8);
        if (P == 0)
            P = 65536;
        int shape = psg[13] & 0x0f;
        if (shape != g_env_shape) {
            g_env_shape = shape;
            g_env_cnt = 0;
            g_env_acc = 0;
        }
        g_env_acc += (uint32_t)((double)YM_CLOCK / 256.0 / (double)P
                                / (double)g_out_rate * 65536.0) + 1u;
        while (g_env_acc >= 65536u) {
            g_env_acc -= 65536u;
            g_env_cnt = (g_env_cnt + 1) & 31;
        }
        env_val = env_value(shape, g_env_cnt);
    }

    for (int c = 0; c < 3; c++) {
        int vol = (psg[8 + c] & 0x10) ? env_val : (psg[8 + c] & 0x0f);
        int amp = g_vol[vol];
        int tdis = (mixer >> c) & 1;          /* 0 = tono activo */
        int ndis = (mixer >> (c + 3)) & 1;    /* 0 = ruido activo */
        int bit = (tdis ? 1 : (int)g_tone_bit[c]) & (ndis ? 1 : (int)g_noise_bit);
        sum += bit ? amp : -amp;
    }

    int32_t s = sum * 42 * g_gain / 100;
    /* Filtro de continua: centra el PCM unipolar por niveles de volumen. */
    int32_t y = s - g_hp_x + (int32_t)(g_hp_y * 0.995f);
    g_hp_x = s;
    g_hp_y = y;
    if (y > 32767) y = 32767;
    if (y < -32768) y = -32768;

    /* Energía de la señal renderizada. */
    g_sum_sq += (uint64_t)((int64_t)y * (int64_t)y);
    g_n_samples++;
    if (y > g_peak) g_peak = y;
    if (y < -g_peak) g_peak = (int32_t)(-y);

    return (int16_t)y;
}

void platform_audio_advance(const uint8_t *psg, uint32_t us, int gain_pct) {
    if (!g_enabled || !psg || us == 0)
        return;
    g_gain = gain_pct < 0 ? 0 : (gain_pct > 200 ? 200 : gain_pct);
    g_frac += (double)g_out_rate * (double)us / 1000000.0;
    while (g_frac >= 1.0) {
        g_frac -= 1.0;
        ring_push(ym_sample(psg));
    }
}

int platform_audio_music_gain(void) { return g_music_gain; }
