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
 */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <time.h>

#include "sched.h"
#include "addrs.h"
#include "platform.h"
#include "platform_internal.h"

#define SITE_CAP 4096
#define VBL_NS   20000000ull     /* 50 Hz (PAL): un VBL cada 20 ms */
#define VBL_US   20000u          /* el mismo VBL en microsegundos (audio) */

static uint32_t g_frames;
static void (*g_tick)(uint8_t *img);

/* Ritmo en tiempo real (sólo en modo ventana): cada VBL consume 20 ms de reloj. */
static int g_rt;
static uint64_t g_rt_base_ns;
static uint32_t g_rt_count;

void platform_time_init(void) { g_frames = 0; g_tick = 0; g_rt = 0; }

void platform_set_tick(void (*fn)(uint8_t *img)) { g_tick = fn; }

void platform_set_realtime(int on) {
    g_rt = on ? 1 : 0;
    if (g_rt) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        g_rt_base_ns = (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
        g_rt_count = 0;
    }
}

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

/* Reloj de pared para el audio: en modo tiempo real el Timer A del MFP avanza con
 * el tiempo transcurrido de verdad (también durante las esperas bloqueantes). */
int platform_time_realtime(void) { return g_rt; }
uint64_t platform_now_ns(void) { return now_ns(); }

uint32_t platform_frame_count(void) { return g_frames; }

void platform_tick(uint8_t *img) {
    if (g_tick)
        g_tick(img);                             /* tick del juego (0x160c2 en M2) */
    else
        img[A_DAT_1ACD4] = (uint8_t)(img[A_DAT_1ACD4] + 1);
    g_frames++;

    /* Audio emulado: un VBL son 20 ms del reloj del original. El Timer A del MFP
     * dispara sus ISR a su propia frecuencia (música ~289 Hz, PCM ~5 kHz). */
    platform_audio_tick(img, VBL_US);

    if (g_rt) {
        g_rt_count++;
        uint64_t target = g_rt_base_ns + (uint64_t)g_rt_count * VBL_NS;
        uint64_t now = now_ns();
        if (target > now) {
            uint64_t d = target - now;
            struct timespec sl = { (time_t)(d / 1000000000ull), (long)(d % 1000000000ull) };
            nanosleep(&sl, NULL);
        } else if (now - target > 500000000ull) {     /* muy por detrás: resincroniza */
            g_rt_base_ns = now;
            g_rt_count = 0;
        }
    }
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
