/* platform/audio.c — audio del port (M1: stub sin salida).
 *
 * El sonido (MFP Timer A + PSG/YM2149 + SDL audio) es un hito propio (M4). Aquí
 * sólo queda el punto de inicialización para que la HAL arranque completa.
 */
#include "platform_internal.h"

void platform_audio_init(void) {
    /* M4: inicializar SDL audio y el modelo del MFP/PSG. */
}
