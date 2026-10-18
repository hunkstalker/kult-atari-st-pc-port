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
#include <stdint.h>

#include "sched.h"
#include "addrs.h"
#include "platform.h"
#include "platform_internal.h"

#define SITE_CAP 4096

static uint32_t g_frames;
static void (*g_tick)(uint8_t *img);

void platform_time_init(void) { g_frames = 0; g_tick = 0; }

void platform_set_tick(void (*fn)(uint8_t *img)) { g_tick = fn; }

uint32_t platform_frame_count(void) { return g_frames; }

void platform_tick(uint8_t *img) {
    if (g_tick)
        g_tick(img);                             /* tick del juego (0x160c2 en M2) */
    else
        img[A_DAT_1ACD4] = (uint8_t)(img[A_DAT_1ACD4] + 1);
    g_frames++;
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
