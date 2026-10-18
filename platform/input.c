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
static uint8_t  g_buttons;
static uint32_t g_deliveries;
static int g_click_toggle = 1;   /* headless: alterna clic para liberar esperas de clic */

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

/* GEMDOS Cnecin: lectura bloqueante. En headless se sirve la cola o la auto-tecla,
 * de modo que nunca cuelga. Devuelve ascii en el byte bajo (como el original usa). */
int pc_os_cnecin(uint8_t *mem, uint32_t *out) {
    (void)mem;
    uint32_t k = g_auto_key;
    if (g_head != g_tail) {
        k = g_keys[g_head];
        g_head = (g_head + 1) % KEYQ;
    }
    if (out) *out = k;
    return 1;
}

/* Paquete IKBD: [botones][x_hi][x_lo][y_hi][y_lo] = A_IKBD_PACKET(+1..).
 * El primer paquete va con el botón SOLTADO (0) y luego se alterna, de modo que las
 * esperas "soltar y pulsar" del original (prompt_wait) terminan. */
void platform_ikbd_deliver(uint8_t *img) {
    g_buttons = (uint8_t)((g_click_toggle && (g_deliveries & 1)) ? 1 : 0);
    g_deliveries++;
    img[A_IKBD_PACKET] = g_buttons;
    wr16(img + A_CURSOR_X, g_cursor_x);
    wr16(img + A_CURSOR_Y, g_cursor_y);
}
