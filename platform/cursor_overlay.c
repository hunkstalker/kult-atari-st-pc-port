/* platform/cursor_overlay.c — cursor del juego como capa de presentación.
 *
 * El core sigue dibujando el cursor software en la imagen (para conservar su
 * save/restore y el estado de las partidas). La presentación y las capturas
 * usan una copia de la imagen con el cursor recolocado a la posición más
 * reciente del ratón.
 */
#include <stdint.h>
#include <string.h>

#include "machine.h"
#include "addrs.h"
#include "platform.h"

void kult_core_cursor_draw(uint8_t *img);
void kult_core_cursor_restore(uint8_t *img);

static int g_visible;

void cursor_draw(uint8_t *img) {
    kult_core_cursor_draw(img);
    g_visible = 1;
}

void cursor_restore(uint8_t *img) {
    kult_core_cursor_restore(img);
    g_visible = 0;
}

int platform_cursor_visible(void) {
    return g_visible;
}

void platform_cursor_compose(uint8_t *dst, const uint8_t *src) {
    if (!dst || !src) return;
    memcpy(dst, src, PLATFORM_IMAGE_SIZE);
    if (!g_visible) return;

    /* Quita el cursor de la posición anterior y lo pinta en la posición de
     * presentación: el ratón más reciente o el cursor movido por el juego. */
    uint16_t x, y;
    platform_input_cursor_for_overlay(src, &x, &y);
    wr16(dst + A_CURSOR_X, x);
    wr16(dst + A_CURSOR_Y, y);
    kult_core_cursor_restore(dst);
    kult_core_cursor_draw(dst);
}
