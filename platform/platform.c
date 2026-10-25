/* platform/platform.c — arranque de la HAL del port PC.
 *
 * Reúne la inicialización de cada servicio. La imagen plana la instala `app/` con
 * `platform_set_image` antes de tocar el juego.
 */
#include "platform.h"
#include "platform_internal.h"

void platform_init(void) {
    platform_fs_init();
    platform_video_init();
    platform_input_init();
    platform_time_init();
    platform_audio_init();
}

/* Banco A (MOTS) del port: zona libre entre ECHE y las pantallas.
 * Evita que un banco de idioma >1.078 B se solape con PUZZL en 0x41fac. */
uint32_t platform_overlay_a_bank(void) { return 0x89000u; }
