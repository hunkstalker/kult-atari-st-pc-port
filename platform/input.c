/* platform/input.c — entrada del port: teclado (GEMDOS) y paquete IKBD (ratón).
 *
 * M1 es headless: la entrada se inyecta por script (KULT_KEYS) o con una auto-tecla
 * (KULT_KEY, por defecto `E`, que es el idioma inglés en el menú del arranque). El
 * ratón entra por el paquete IKBD (`A_IKBD_PACKET`): sus bytes 1-4 SON A_CURSOR_X/Y
 * (ver docs/PORTING.md), así que escribirlos ahí actualiza el cursor del juego.
 *
 * La entrada real (SDL) llega en M3; aquí basta con que las esperas no cuelguen y
 * que el cursor/estado sean deterministas.
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "os.h"
#include "addrs.h"
#include "platform.h"
#include "platform_internal.h"

#define KEYQ 64

static uint32_t g_keys[KEYQ];
static int g_head, g_tail;
static uint32_t g_auto_key = 'E';
static uint16_t g_cursor_x = 160;
static uint16_t g_cursor_y = 100;
static uint8_t  g_buttons;         /* botón real del ratón (bit0 = izquierdo) */
static uint32_t g_deliveries;
static int g_click_toggle = 1;     /* headless: alterna clic para liberar esperas */
static int g_auto = 1;             /* 1 = headless (auto-tecla); 0 = entrada real */
static void (*g_idle)(uint8_t *img);   /* bombeo durante una espera bloqueante */

void platform_set_idle(void (*fn)(uint8_t *img)) { g_idle = fn; }
void platform_input_set_auto(int on) { g_auto = on ? 1 : 0; }

void platform_mouse_set(int x, int y, int buttons) {
    if (x < 0) x = 0;
    if (x > 319) x = 319;
    if (y < 0) y = 0;
    if (y > 199) y = 199;
    g_cursor_x = (uint16_t)x;
    g_cursor_y = (uint16_t)y;
    g_buttons = (uint8_t)(buttons & 1);
}

void platform_input_init(void) {
    g_head = g_tail = 0;
    g_deliveries = 0;
    const char *click = getenv("KULT_CLICK");
    if (click && click[0] == '0') g_click_toggle = 0;
    const char *autoenv = getenv("KULT_KEY");
    if (autoenv && *autoenv) g_auto_key = (uint8_t)autoenv[0];
    const char *script = getenv("KULT_KEYS");
    if (script) platform_key_push_str(script);
    const char *cx = getenv("KULT_CURSOR_X");
    const char *cy = getenv("KULT_CURSOR_Y");
    if (cx) g_cursor_x = (uint16_t)atoi(cx);
    if (cy) g_cursor_y = (uint16_t)atoi(cy);
}

void platform_key_push(uint32_t key) {
    int next = (g_tail + 1) % KEYQ;
    if (next == g_head) return;                 /* cola llena: se descarta */
    g_keys[g_tail] = key;
    g_tail = next;
}

void platform_key_push_str(const char *s) {
    for (; s && *s; s++)
        platform_key_push((uint8_t)*s);
}

/* GEMDOS Crawio(0xff): lectura no bloqueante; devuelve la tecla o 0. */
uint32_t pc_os_crawio(uint8_t *mem, uint16_t w) {
    (void)mem;
    if (w != 0x00ff) return 0;                  /* escritura a consola: no-op */
    if (g_head == g_tail) return 0;
    uint32_t k = g_keys[g_head];
    g_head = (g_head + 1) % KEYQ;
    return k;
}

/* GEMDOS Cnecin: lectura bloqueante. En headless se sirve la cola o la auto-tecla
 * (nunca cuelga). Con entrada real (`platform_input_set_auto(0)`) espera bombeando
 * `g_idle` (SDL) hasta que llegue una tecla. Devuelve ascii en el byte bajo. */
int pc_os_cnecin(uint8_t *mem, uint32_t *out) {
    if (g_head == g_tail) {
        if (!g_auto) {
            while (g_head == g_tail && g_idle)
                g_idle(mem);
        }
        if (g_head == g_tail) {                   /* sin tecla y sin idle */
            if (out) *out = g_auto_key;
            return 1;
        }
    }
    uint32_t k = g_keys[g_head];
    g_head = (g_head + 1) % KEYQ;
    if (out) *out = k;
    return 1;
}

/* Paquete IKBD: [botones][x_hi][x_lo][y_hi][y_lo] = A_IKBD_PACKET(+1..).
 * Headless: el botón va SOLTADO y luego alterna, para que las esperas "soltar y
 * pulsar" (prompt_wait) terminen. Entrada real: se entrega el ratón tal cual. */
void platform_ikbd_deliver(uint8_t *img) {
    uint8_t b = g_auto
        ? (uint8_t)((g_click_toggle && (g_deliveries & 1)) ? 1 : 0)
        : g_buttons;
    g_deliveries++;
    img[A_IKBD_PACKET] = b;
    wr16(img + A_CURSOR_X, g_cursor_x);
    wr16(img + A_CURSOR_Y, g_cursor_y);
}
