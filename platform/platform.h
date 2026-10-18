/* platform/platform.h — API de la HAL del port PC (uso interno de app/boot).
 *
 * No forma parte del contrato con el core (ese vive en platform/include/os.h,
 * hw.h, psg.h, sched.h). Aquí está lo que necesitan `app/` y `boot/`: inicializar
 * la HAL, volcar el framebuffer y controlar el reloj/entrada.
 */
#ifndef KULT_PLATFORM_H
#define KULT_PLATFORM_H

#include <stdint.h>

/* Inicia la HAL: directorio de datos (KULT_DATA_DIR o `data/`), entrada
 * (KULT_KEYS o auto-tecla 'E') y estado de vídeo. `img` no se toca aquí. */
void platform_init(void);

/* Fija la imagen plana (1 MiB) que comparten los servicios. */
void platform_set_image(uint8_t *img);

/* Entrada: encola una tecla (formato GEMDOS: ascii en el byte bajo). */
void platform_key_push(uint32_t key);
void platform_key_push_str(const char *s);

/* Reloj: número de frames (VBL) entregados desde el arranque. */
uint32_t platform_frame_count(void);

/* Trabajo a ejecutar en cada frame (VBL). El driver registra aquí el tick del
 * juego (boot/vbl.c: 0x160c2); sin hook, el reloj sólo incrementa A_DAT_1ACD4. */
void platform_set_tick(void (*fn)(uint8_t *img));

/* Vídeo: base visible actual (la última que fijó os_setscreen, o la de
 * screen_ptr_a si el juego aún no ha llamado a Setscreen). */
uint32_t platform_visible_base(void);

/* Volcados a disco del framebuffer visible (320x200, 16 colores):
 *   - PPM (P6): imagen RGB legible.
 *   - RAW planar: 32000 B tal cual (formato ST).
 * Devuelven 0 si todo fue bien. */
int platform_dump_ppm(const char *path);
int platform_dump_planar(const char *path);

/* Paleta actual (16 words formato ST 0x0RGB), sólo lectura. */
const uint16_t *platform_palette(void);

/* Convierte el framebuffer visible (320x200) a RGBA8888 (256000 B). */
void platform_fb_to_rgba(uint8_t *dst);

/* Cuenta de píxeles no-negro del framebuffer visible (para comprobaciones). */
uint32_t platform_fb_nonzero(void);

/* Ventana SDL3 (sólo se enlaza en el build `make port-sdl`, -DHAVE_SDL3). */
int  platform_sdl_open(const char *title, int scale);
void platform_sdl_present(uint8_t *img);
int  platform_sdl_pump(void);      /* 1 = seguir, 0 = cerrar */
void platform_sdl_close(void);

#endif /* KULT_PLATFORM_H */
