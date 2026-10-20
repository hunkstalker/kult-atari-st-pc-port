/* platform/input.c — entrada del port: teclado (GEMDOS) y paquete IKBD (ratón).
 *
 * M1 es headless: la entrada se inyecta por script (KULT_KEYS) o con una auto-tecla
 * (KULT_KEY, por defecto `E`, que es el idioma inglés en el menú del arranque). El
 * ratón entra por el paquete IKBD (`A_IKBD_PACKET`): sus bytes 1-4 SON A_CURSOR_X/Y
 * (ver docs/PORTING.md), así que escribirlos ahí actualiza el cursor del juego.
 *
 * La entrada real (SDL) llega en M3; aquí basta con que las esperas no cuelguen y
 * que el cursor/estado sean deterministas.
 *
 * Botones (modelo del original):
 *   - izquierdo: acción (bit2 de A_INPUT_BUTTONS), sin skip.
 *   - derecho: acción + skip (A_SKIP_TRANSITION = 0xff).
 * El core `wait_frame` deriva acción y skip del bit0 del paquete IKBD; para que
 * el izquierdo no salte, la HAL pisa `$1a570` al salir de `wait_frame`
 * (`platform_input_apply_skip`), igual que hacía el motor legacy al interceptar
 * la escritura de esa dirección. Así `src/` queda intacto.
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
static uint8_t  g_buttons;         /* acción: izquierdo o derecho pulsado */
static uint8_t  g_mouse_skip;      /* derecho: pide saltar ($1a570) */
static uint16_t g_mouse_max_x = 319;   /* rango absoluto configurado por el IKBD */
static uint16_t g_mouse_max_y = 199;
static uint32_t g_deliveries;
static int g_click_toggle = 1;     /* headless: alterna clic para liberar esperas */
static int g_auto = 1;             /* 1 = headless (auto-tecla); 0 = entrada real */
static void (*g_idle)(uint8_t *img);   /* bombeo durante una espera bloqueante */

void platform_set_idle(void (*fn)(uint8_t *img)) { g_idle = fn; }
void platform_input_set_auto(int on) { g_auto = on ? 1 : 0; }

void platform_mouse_set(int x, int y, int buttons) {
    if (x < 0) x = 0;
    if (x > g_mouse_max_x) x = g_mouse_max_x;
    if (y < 0) y = 0;
    if (y > g_mouse_max_y) y = g_mouse_max_y;
    g_cursor_x = (uint16_t)x;
    g_cursor_y = (uint16_t)y;
    /* Máscara del puerto: bit0 = izquierdo, bit1 = derecho. El izquierdo y el
     * derecho ambos actúan; solo el derecho pide skip. */
    g_buttons = (uint8_t)((buttons & 0x03) ? 1 : 0);
    g_mouse_skip = (uint8_t)((buttons & 0x02) ? 1 : 0);
}

int platform_input_skip_state(void) { return g_mouse_skip; }

/* Emula el comando IKBD que el juego envía en crt0_init:
 *   Ikbdws(0x19, 0x1a459, 4) -> 5 bytes: 09 01 30 00 b8
 * El 0x09 configura el ratón absoluto con máximo X=0x0130 (304) e Y=0x00b8
 * (184), que es justo el área para un cursor de 16x16. Sin esto, el ratón del
 * port llega a 319/199 y el sprite se sale por el borde. */
void platform_ikbdws(const uint8_t *buf, uint32_t len) {
    if (!buf || len < 5 || buf[0] != 0x09) return;
    uint32_t xmax = ((uint32_t)buf[1] << 8) | buf[2];
    uint32_t ymax = ((uint32_t)buf[3] << 8) | buf[4];
    if (xmax <= 319) g_mouse_max_x = (uint16_t)xmax;
    if (ymax <= 199) g_mouse_max_y = (uint16_t)ymax;
    if (g_cursor_x > g_mouse_max_x) g_cursor_x = g_mouse_max_x;
    if (g_cursor_y > g_mouse_max_y) g_cursor_y = g_mouse_max_y;
}

/* La HAL pisa el flag de skip que acaba de escribir `wait_frame` (core). Es el
 * equivalente port-side del intercept de `$1a570` del motor legacy. */
void platform_input_apply_skip(uint8_t *img) {
    img[A_SKIP_TRANSITION] = g_mouse_skip ? 0xff : 0;
}

void platform_input_init(void) {
    g_head = g_tail = 0;
    g_deliveries = 0;
    g_buttons = 0;
    g_mouse_skip = 0;
    g_mouse_max_x = 319;
    g_mouse_max_y = 199;
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
    if (b & 1)
        img[A_INPUT_BUTTONS] |= 0x04;
    else
        img[A_INPUT_BUTTONS] &= (uint8_t)~0x04;
    wr16(img + A_CURSOR_X, g_cursor_x);
    wr16(img + A_CURSOR_Y, g_cursor_y);
}
