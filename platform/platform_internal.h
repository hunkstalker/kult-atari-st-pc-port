/* platform/platform_internal.h — inicialización y primitivas internas de la HAL.
 *
 * Sólo lo usan `platform_init` y los propios módulos de `platform/`; no forma
 * parte del contrato con el core ni de la API de `app/`/`boot/`.
 */
#ifndef KULT_PLATFORM_INTERNAL_H
#define KULT_PLATFORM_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

/* Directorio del ejecutable (sin barra final); 0 si no se puede averiguar. Lo
 * usan fs.c (data/ junto al exe) y log.c (kult.log). */
int platform_exe_dir(char *buf, size_t n);

void platform_fs_init(void);
void platform_video_init(void);
void platform_input_init(void);
void platform_time_init(void);
void platform_audio_init(void);

/* Escribe el paquete IKBD actual (botones + cursor) en A_IKBD_PACKET. */
void platform_ikbd_deliver(uint8_t *img);

/* Avanza el reloj un frame: incrementa el contador de VBL de la imagen. */
void platform_tick(uint8_t *img);

/* ---- audio (M4) ---------------------------------------------------------- */
/* Avanza `us` microsegundos emulados: dispara el Timer A del MFP (platform/mfp.c)
 * y renderiza el YM (platform/audio.c). Lo llama platform_tick una vez por VBL. */
void platform_audio_tick(uint8_t *img, uint32_t us);

/* Renderiza `us` microsegundos con los registros PSG dados (salida al anillo). */
void platform_audio_advance(const uint8_t *psg, uint32_t us, int gain_pct);

/* Ganancia (%) aplicada al reproductor de música 0x3a872 (KULT_MUSIC_GAIN). */
int platform_audio_music_gain(void);

/* Reinicia el acumulador de tiempo del Timer A (para pruebas deterministas). */
void platform_mfp_audio_reset(void);

/* Reloj de pared y modo tiempo real (los usa el audio para las esperas). */
int      platform_time_realtime(void);
uint64_t platform_now_ns(void);

#endif /* KULT_PLATFORM_INTERNAL_H */
