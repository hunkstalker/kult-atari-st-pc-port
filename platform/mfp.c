/* platform/mfp.c — modelo del MFP 68901: Timer A y despacho de su interrupción.
 *
 * El juego configura el Timer A por `hw_write8` y publica en el vector $134 la
 * rutina que debe correr al expirar:
 *   - `0x1910e` / `0x190f6` — ISR PCM (ya en el core, `src/sound.c`);
 *   - `0x3a872`             — reproductor de música (reconstruido en
 *                             `boot/music_player.c`; emite `trap #3`/`trap #4`,
 *                             cuyos handlers sí están en el core).
 *
 * En el ST el MFP interrumpe la CPU; en el port no hay CPU emulada, así que el
 * modelo resuelve el vector a la función C correspondiente y la llama. El tiempo
 * es el **emulado** del juego (1 VBL = 20 ms), no el de pared: así el audio va al
 * ritmo del original también en headless.
 *
 * Frecuencia (validada contra el original en el predecesor): reloj 2,4576 MHz,
 * prescaler de TACR[0..2] = {parado, 4, 10, 16, 50, 64, 100, 200} y TADR
 * ($fffa1f; 0 = 256). `f = 2.4576 MHz / (prescaler * TADR)`. La música usa
 * TACR=4/TADR=0xaa (~289 Hz) y el PCM TACR=2 con el TADR del sample (~5-6 kHz).
 *
 * Antes de llamar a la ISR se marca ISRA bit 5 (Timer A en servicio); todas las
 * ISR lo limpian al salir (`hw_bclr8($fffa0f,5)`), como el original.
 */
#include <stdint.h>

#include "machine.h"
#include "hw.h"
#include "psg.h"
#include "kult_api.h"
#include "boot.h"
#include "platform.h"
#include "platform_internal.h"

#define MFP_TACR   0xfffa19u
#define MFP_TADR   0xfffa1fu
#define MFP_IERA   0xfffa07u
#define MFP_IMRA   0xfffa13u
#define MFP_ISRA   0xfffa0fu

#define TIMER_A_VECTOR 0x134u

static uint64_t g_acc_us;       /* tiempo emulado pendiente de consumir */
static uint32_t g_isr_count;
static uint32_t g_music_isr_count;
static uint64_t g_last_ns;      /* última vez que se avanzó el reloj de pared */

static const uint32_t g_presc[8] = { 0, 4, 10, 16, 50, 64, 100, 200 };

int platform_mfp_timer_a_enabled(void) {
    return (hw_read_model8(MFP_TACR) & 0x07u) != 0
        && (hw_read_model8(MFP_IERA) & 0x20u) != 0
        && (hw_read_model8(MFP_IMRA) & 0x20u) != 0;
}

uint32_t platform_mfp_timer_a_period_us(void) {
    uint32_t ps = hw_read_model8(MFP_TACR) & 0x07u;
    if (ps == 0)
        return 0;                          /* Timer A parado */
    uint32_t d = hw_read_model8(MFP_TADR);
    uint64_t div = (uint64_t)g_presc[ps] * (d ? d : 256u);
    uint64_t us = (uint64_t)1000000u * div / 2457600u;
    return us ? (uint32_t)us : 1;
}

uint32_t platform_mfp_isr_count(void) { return g_isr_count; }
uint32_t platform_mfp_music_isr_count(void) { return g_music_isr_count; }

void platform_mfp_audio_reset(void) {
    g_acc_us = 0;
    g_isr_count = 0;
    g_music_isr_count = 0;
    g_last_ns = 0;
}

/* Resuelve el vector $134 y ejecuta la reconstrucción correspondiente. */
static void mfp_dispatch(uint8_t *img) {
    uint32_t handler = be32(img + TIMER_A_VECTOR);

    hw_bset8(MFP_ISRA, 5);                 /* Timer A en servicio hasta el rte */

    switch (handler) {
    case 0x1910eu: sound_isr_fwd(img);  break;
    case 0x190f6u: sound_isr_back(img); break;
    case 0x3a872u: kult_music_player(img); g_music_isr_count++; break;
    default:       break;                  /* vector no reconocido */
    }

    g_isr_count++;
}

/* Avanza el Timer A `dt` microsegundos y renderiza el YM. */
static void audio_advance(uint8_t *img, uint32_t dt) {
    g_acc_us += dt;

    /* Tope defensivo: con TADR mínimo el periodo es ~2 us; 20 ms -> ~10k ISR. */
    for (uint32_t guard = 0; guard < 200000u; guard++) {
        if (!platform_mfp_timer_a_enabled()) {
            g_acc_us = 0;
            break;
        }
        uint32_t period = platform_mfp_timer_a_period_us();
        if (period == 0 || g_acc_us < period)
            break;
        g_acc_us -= period;

        mfp_dispatch(img);

        /* La música de intro (0x3a872) suena más alta que voz/efectos; se escala. */
        int gain = (be32(img + TIMER_A_VECTOR) == 0x3a872u)
                 ? platform_audio_music_gain() : 100;
        platform_audio_advance(psg_regs(), period, gain);
    }
}

/* Delta de reloj de pared desde la llamada anterior, en microsegundos (0 la
 * primera vez). Acota a 250 ms para no dar una avalancha tras un parón. */
static uint32_t wall_delta_us(void) {
    uint64_t now = platform_now_ns();
    uint32_t dt = 0;
    if (g_last_ns != 0) {
        uint64_t d = now - g_last_ns;
        dt = d > 250000000ull ? 250000u : (uint32_t)(d / 1000ull);
    }
    g_last_ns = now;
    return dt;
}

/* Lo llama `platform_tick` una vez por VBL. En modo tiempo real toma el intervalo
 * del reloj de pared (la música sigue durante las esperas bloqueantes); si no, usa
 * `us` (bucle headless: 1 VBL = 20 ms emulados). */
void platform_audio_tick(uint8_t *img, uint32_t us) {
    if (!platform_audio_enabled()) {
        g_acc_us = 0;
        return;
    }
    audio_advance(img, platform_time_realtime() ? wall_delta_us() : us);
}

/* Bombeo desde una espera bloqueante (Cnecin en menús/prompts): avanza el audio
 * con el reloj de pared. Lo registra `app/` como callback de idle. */
void platform_audio_pump(uint8_t *img) {
    if (!platform_audio_enabled() || !platform_time_realtime())
        return;
    audio_advance(img, wall_delta_us());
}

