/* platform/platform_internal.h — inicialización y primitivas internas de la HAL.
 *
 * Sólo lo usan `platform_init` y los propios módulos de `platform/`; no forma
 * parte del contrato con el core ni de la API de `app/`/`boot/`.
 */
#ifndef KULT_PLATFORM_INTERNAL_H
#define KULT_PLATFORM_INTERNAL_H

#include <stdint.h>

void platform_fs_init(void);
void platform_video_init(void);
void platform_input_init(void);
void platform_time_init(void);
void platform_audio_init(void);

/* Escribe el paquete IKBD actual (botones + cursor) en A_IKBD_PACKET. */
void platform_ikbd_deliver(uint8_t *img);

/* Avanza el reloj un frame: incrementa el contador de VBL de la imagen. */
void platform_tick(uint8_t *img);

#endif /* KULT_PLATFORM_INTERNAL_H */
