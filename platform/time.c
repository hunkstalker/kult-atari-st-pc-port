/* platform/time.c — el reloj del port: VBL + entrega del paquete IKBD.
 *
 * El juego avanza "un paso por frame" y sus esperas (`wait_vbl`, `wait_frame`) son
 * bucles que releen un byte por `sched_poll8`. En el port ese byte lo mueve este
 * módulo:
 *   - un sondeo del contador de VBL (`A_DAT_1ACD4`) avanza un frame;
 *   - un sondeo del paquete IKBD, con el byte a 0xff (lo que `wait_frame` deja),
 *     entrega el paquete (botones + cursor) y avanza un frame.
 * Así ninguna espera se cuelga y el número de frames es determinista.
 *
 * M3 sustituirá el avance "un frame por sondeo" por un reloj a 50 Hz con
 * presentación desacoplada (docs/PLAN.md, T-032); la frontera no cambia.
 *
 * Portabilidad (M7/T-070): en POSIX el reloj es `clock_gettime`/`nanosleep`; en
 * Windows (`_WIN32`) se usa `QueryPerformanceCounter` + `Sleep`, sin depender de
 * SDL (el build headless no enlaza SDL).
 */
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include "sched.h"
#include "addrs.h"
#include "platform.h"
#include "platform_internal.h"

#define SITE_CAP 4096
#define VBL_NS   20000000ull     /* 50 Hz (PAL): un VBL cada 20 ms */
#define VBL_US   20000u          /* el mismo VBL en microsegundos (audio) */
#define CPU_HZ_DEFAULT 8000000u  /* CPU efectiva del ST para los retardos dbf */

extern uint8_t *g_kult_img;      /* imagen del juego (platform/include/os.h) */

/* Reloj monótono en nanosegundos. */
static uint64_t now_ns(void) {
#ifdef _WIN32
    static double freq;          /* Hz (fijo durante la sesión) */
    LARGE_INTEGER c;
    if (freq == 0.0) {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        freq = (double)f.QuadPart;
        if (freq == 0.0) freq = 1.0;
    }
    QueryPerformanceCounter(&c);
    return (uint64_t)((double)c.QuadPart * 1e9 / freq);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
#endif
}

static void (*g_render_pump)(void); /* presentación de alta frecuencia */

/* Duerme al menos `ns` nanosegundos (en Windows, resolución de ms). */
static void sleep_ns(uint64_t ns) {
#ifdef _WIN32
    DWORD ms = (DWORD)((ns + 999999ull) / 1000000ull);
    Sleep(ms ? ms : 1);
#else
    struct timespec sl = { (time_t)(ns / 1000000000ull),
                           (long)(ns % 1000000000ull) };
    nanosleep(&sl, NULL);
#endif
}

/* Duerme hasta `target`, pero da oportunidades al pump de presentación para
 * refrescar el cursor mientras el juego espera. El juego sigue a 50 Hz. */
#define RENDER_NS 8333333ull   /* ~120 Hz como máximo */
static void sleep_with_render(uint64_t target) {
    if (!g_render_pump) {
        uint64_t now = now_ns();
        if (now < target) sleep_ns(target - now);
        return;
    }
    for (;;) {
        uint64_t now = now_ns();
        if (now >= target) break;
        if (g_render_pump) g_render_pump();
        uint64_t left = target - now;
        if (left > RENDER_NS) left = RENDER_NS;
        sleep_ns(left);
    }
}

static uint32_t g_frames;
static void (*g_tick)(uint8_t *img);
static void (*g_present)(uint8_t *img);   /* presentación por VBL (arranque y bucle) */
static int g_in_loop;                     /* histórico: ya en game_loop (compat.) */

/* Ritmo en tiempo real (sólo en modo ventana): cada VBL consume 20 ms de reloj. */
static int g_rt;
static uint64_t g_rt_base_ns;
static uint32_t g_rt_count;

static uint64_t g_cpu_acc_us;    /* acumulador de retardos dbf (< 1 ms) */
static uint32_t g_cpu_hz;        /* Hz de la CPU efectiva (lazy, KULT_CPU_HZ) */
static int g_cpu_busy;           /* evita reentrada tick -> cpu_delay */
static int g_disk_timing = 1;    /* 0 = no modelar esperas de disco (arranque) */

void platform_time_init(void) {
    g_frames = 0; g_tick = 0; g_rt = 0;
    g_present = 0; g_in_loop = 0;
    g_cpu_acc_us = 0; g_cpu_hz = 0; g_cpu_busy = 0; g_disk_timing = 1;
}

void platform_set_tick(void (*fn)(uint8_t *img)) { g_tick = fn; }
void platform_set_present(void (*fn)(uint8_t *img)) { g_present = fn; }
void platform_set_render_pump(void (*fn)(void)) { g_render_pump = fn; }
void platform_set_loop(int on) { g_in_loop = on ? 1 : 0; }

void platform_set_realtime(int on) {
    g_rt = on ? 1 : 0;
    if (g_rt) {
        g_rt_base_ns = now_ns();
        g_rt_count = 0;
    }
}

/* Reloj de pared para el audio: en modo tiempo real el Timer A del MFP avanza con
 * el tiempo transcurrido de verdad (también durante las esperas bloqueantes). */
int platform_time_realtime(void) { return g_rt; }
uint64_t platform_now_ns(void) { return now_ns(); }

uint32_t platform_frame_count(void) { return g_frames; }
int platform_in_loop(void) { return g_in_loop; }

void platform_tick(uint8_t *img) {
    if (g_tick)
        g_tick(img);                             /* tick del juego (0x160c2 en M2) */
    else
        img[A_DAT_1ACD4] = (uint8_t)(img[A_DAT_1ACD4] + 1);
    g_frames++;

#ifndef KULT_NO_CPUWORK
    {   /* Volcado periodico del anillo: permite recuperar el estado si el juego
         * se queda bloqueado y no puede procesar F12. Solo con KULT_AUTOLOG. */
        static int autolog = -1;
        if (autolog < 0) {
            const char *e = getenv("KULT_AUTOLOG");
            autolog = (e && *e) ? 1 : 0;
        }
        if (autolog && (g_frames % 150u) == 0)
            platform_marks_event("auto", 0, 0);
    }
    {   /* Diagnostico: simular F12 en un VBL (pruebas del buffer de marcas). */
        static long at = -2;
        static int done;
        if (at == -2) { const char *e = getenv("KULT_F12_AT"); at = (e && *e) ? strtol(e, NULL, 0) : -1; }
        if (!done && at >= 0 && (long)g_frames == at) {
            platform_marks_event("f12", 0, 0);
            done = 1;
        }
    }
#endif

    /* Audio emulado: un VBL son 20 ms del reloj del original. El Timer A del MFP
     * dispara sus ISR a su propia frecuencia (música ~289 Hz, PCM ~5 kHz). */
    platform_audio_tick(img, VBL_US);

    /* Presentar y bombear SDL en CADA VBL, también dentro del bucle. Las
     * animaciones de los comandos (puertas, wipes) se quedan en `wait_vbl` y no
     * pasan por el frame hook; sin esto se oyen pero no se ven. */
    if (g_present)
        g_present(img);

    if (g_rt) {
        g_rt_count++;
        uint64_t target = g_rt_base_ns + (uint64_t)g_rt_count * VBL_NS;
        uint64_t now = now_ns();
        if (target > now) {
            sleep_with_render(target);
        } else if (now - target > 500000000ull) {     /* muy por detrás: resincroniza */
            g_rt_base_ns = now;
            g_rt_count = 0;
        }
    }
}

/* Reloj de CPU del original: cobra tiempo de CPU (ciclos) y lo duerme cediendo
 * VBLs, como haría el ISR durante los bucles de CPU del ST. `us` ya convertido. */
static int cpu_enabled(void) {
    static int en = -1;
    if (en < 0) {
        const char *e = getenv("KULT_CPU");
        en = !(e && e[0] == '0' && e[1] == '\0');
    }
    return en;
}

static uint32_t cpu_hz(void) {
    if (g_cpu_hz == 0) {
        const char *e = getenv("KULT_CPU_HZ");
        long v = (e && *e) ? strtol(e, NULL, 0) : 0;
        if (v > 0) {
            g_cpu_hz = (uint32_t)v;
        } else {
            /* El shifter roba ciclos de bus: KULT_ST_CONTEND = % de CPU
             * disponible (def. 100 = 8 MHz nominales). Calibrar contra el ST
             * real/Hatari con KULT_FPSLOG. */
            long st = 100;
            const char *s = getenv("KULT_ST_CONTEND");
            if (s && *s) st = strtol(s, NULL, 0);
            if (st <= 0 || st > 100) st = 100;
            g_cpu_hz = (uint32_t)((uint64_t)CPU_HZ_DEFAULT * (uint32_t)st / 100u);
        }
    }
    return g_cpu_hz;
}

static void cpu_charge_us(uint64_t us) {
    if (!g_rt || !g_kult_img || g_cpu_busy || !cpu_enabled()) return;

    g_cpu_acc_us += us;
    if (g_cpu_acc_us < 1000ull) return;           /* agrupa (< 1 ms) */

    uint64_t end = now_ns() + g_cpu_acc_us * 1000ull;
    g_cpu_acc_us = 0;

    g_cpu_busy = 1;
    for (;;) {
        uint64_t now = now_ns();
        uint64_t next_vbl = g_rt_base_ns + (uint64_t)(g_rt_count + 1) * VBL_NS;
        /* Entrega primero los VBLs cuyo instante ya pasó, aunque el reloj haya
         * rebasado `end` por la granularidad de nanosleep. */
        if (next_vbl <= end && now >= next_vbl) {
            platform_tick(g_kult_img);
            continue;
        }
        if (now >= end) break;
        uint64_t wake = next_vbl < end ? next_vbl : end;
        sleep_with_render(wake);
    }
    g_cpu_busy = 0;
}

/* Espera de lectura de disquete del ST. El original bloquea la CPU en Fread
 * mientras la estrella/el fondo siguen en pantalla; la HAL duerme el mismo
 * tiempo cediendo VBLs y presentando. Estimación DD: 9x512 B/pista a 300 rpm
 * = 1,4 s para 0x7d22 B; + búsqueda/encendido -> ~2 s (62,5 us/byte).
 * Se aplica a cada lectura de recurso (notes/0026). No depende de
 * KULT_CPU_HZ: no es trabajo de CPU, es E/S. */
void platform_disk_delay(uint32_t bytes) {
    if (!g_rt || !g_kult_img || g_cpu_busy || !g_disk_timing || bytes == 0)
        return;
    uint64_t us = ((uint64_t)bytes * 125ull) / 2ull;   /* 62,5 us/byte */
    uint64_t end = now_ns() + us * 1000ull;
    g_cpu_busy = 1;
    for (;;) {
        uint64_t now = now_ns();
        uint64_t next_vbl = g_rt_base_ns + (uint64_t)(g_rt_count + 1) * VBL_NS;
        if (next_vbl <= end && now >= next_vbl) {
            platform_tick(g_kult_img);
            continue;
        }
        if (now >= end) break;
        uint64_t wake = next_vbl < end ? next_vbl : end;
        sleep_with_render(wake);
    }
    g_cpu_busy = 0;
}

/* Activa/desactiva el modelo de espera de disco. El arranque lo apaga mientras
 * carga PRES/DRAP (pantalla negra/título) y lo reactiva tras elegir idioma:
 * así la canción no empieza avanzada y se conservan las esperas posteriores. */
void platform_disk_timing(int on) {
    g_disk_timing = on ? 1 : 0;
}

/* Bucles `dbf` transcritos por el core (`cpu_delay(unidades)`, ~10 ciclos por
 * unidad): `platform/include/kult_port.h` los enruta aquí. */
void platform_cpu_delay(uint32_t units) {
    if (!g_rt || !g_kult_img) return;
    uint64_t hz = cpu_hz();
    cpu_charge_us(((uint64_t)units * 10ull * 1000000ull + hz - 1) / hz);
}

/* Coste medido por función (T-073b): `platform/cpuwork.c` cobra ciclos. */
void platform_cpu_charge_cycles(uint64_t cycles) {
    if (!g_rt || !g_kult_img) return;
    uint64_t hz = cpu_hz();
    cpu_charge_us((cycles * 1000000ull + hz - 1) / hz);
}

uint8_t sched_poll8(uint8_t *image, uint32_t addr, uint32_t site_pc) {
    (void)site_pc;
    if (addr == A_DAT_1ACD4) {
        platform_tick(image);
    } else if (addr == A_IKBD_PACKET && image[addr] == 0xff) {
        platform_ikbd_deliver(image);
        platform_tick(image);
    }
    return image[addr];
}

int sched_wait8(uint8_t *image, uint32_t addr, uint8_t until, uint32_t site_pc) {
    for (uint32_t i = 0; i < SITE_CAP; i++)
        if (sched_poll8(image, addr, site_pc) == until)
            return 1;
    return 0;
}
