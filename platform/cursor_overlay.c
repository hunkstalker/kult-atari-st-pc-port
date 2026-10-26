/* platform/cursor_overlay.c — cursor del juego como capa de presentación.
 *
 * El core sigue dibujando el cursor software en la imagen (para conservar su
 * save/restore y el estado de las partidas). La presentación y las capturas
 * usan una copia de la imagen con el cursor recolocado a la posición más
 * reciente del ratón.
 */
#include <stdint.h>
#include <string.h>

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

    /* Quita el cursor de la posición anterior y lo pinta en A_CURSOR_X/Y,
     * que la HAL actualiza con cada evento de ratón. */
    kult_core_cursor_restore(dst);
    kult_core_cursor_draw(dst);
}
