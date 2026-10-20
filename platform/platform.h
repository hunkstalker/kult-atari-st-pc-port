/* platform/platform.h — API de la HAL del port PC (uso interno de app/boot).
 *
 * No forma parte del contrato con el core (ese vive en platform/include/os.h,
 * hw.h, psg.h, sched.h). Aquí está lo que necesitan `app/` y `boot/`: inicializar
 * la HAL, volcar el framebuffer y controlar el reloj/entrada.
 */
#ifndef KULT_PLATFORM_H
#define KULT_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

/* Inicia la HAL: directorio de datos (KULT_DATA_DIR o `data/` junto al
 * ejecutable), entrada (KULT_KEYS o auto-tecla 'E') y estado de vídeo. `img` no
 * se toca aquí. */
void platform_init(void);

/* Busca un fichero de datos (p. ej. "KULT.PRG") en los directorios de datos y
 * copia su ruta a `out`. Devuelve 0 si existe. (M7/T-070) */
int platform_data_resolve(const char *name, char *out, size_t out_size);

/* ---- diagnóstico (M7) ---------------------------------------------------- */
/* Log a fichero y a stderr. Por defecto escribe `kult.log` junto al ejecutable
 * (fallback a %LOCALAPPDATA% en Windows o /tmp en POSIX). KULT_LOG=<ruta> lo
 * redirige y KULT_LOG=0 lo apaga. `platform_fatal` además muestra un MessageBox
 * en la build GUI de Windows (KULT_GUI). Iniciar antes de resolver los datos. */
void platform_log_init(void);
void platform_log(const char *fmt, ...);
void platform_fatal(const char *fmt, ...);
void platform_log_close(void);

/* Fija la imagen plana (1 MiB) que comparten los servicios. */
void platform_set_image(uint8_t *img);

/* Entrada: encola una tecla (formato GEMDOS: ascii en el byte bajo). */
void platform_key_push(uint32_t key);
void platform_key_push_str(const char *s);

/* Entrada real (M3): posición/botones del ratón (coords de juego 320x200) y
 * conmutador del modo auto (headless). `buttons` usa la máscara del puerto:
 * bit0 = izquierdo (acción), bit1 = derecho (acción + skip). El paquete IKBD
 * lleva la acción en bit0; el skip se pisa en `$1a570` desde la HAL. */
void platform_mouse_set(int x, int y, int buttons);
/* Derecho pulsado: la HAL debe pisar `$1a570` tras `wait_frame` (legacy). */
int  platform_input_skip_state(void);
void platform_input_apply_skip(uint8_t *img);
/* Emula Ikbdws: 0x09 fija el máximo absoluto del ratón (xmax/ymax). */
void platform_ikbdws(const uint8_t *buf, uint32_t len);
void platform_input_set_auto(int on);
void platform_set_idle(void (*fn)(uint8_t *img));

/* Reloj: número de frames (VBL) entregados desde el arranque. */
uint32_t platform_frame_count(void);

/* Reloj monótono en ns (para diagnósticos, p. ej. FPS del bucle). */
uint64_t platform_now_ns(void);

/* Trabajo a ejecutar en cada frame (VBL). El driver registra aquí el tick del
 * juego (boot/vbl.c: 0x160c2); sin hook, el reloj sólo incrementa A_DAT_1ACD4. */
void platform_set_tick(void (*fn)(uint8_t *img));

/* Presentación por VBL (modo ventana): si se registra, `platform_tick` llama a
 * `fn` en cada VBL, tanto en el arranque como dentro del bucle. Así se ven las
 * animaciones que corren en `wait_vbl` dentro de un comando (puertas, wipes) y
 * las esperas de `wait_frame` siguen bombeando SDL. `platform_set_loop` se
 * conserva por compatibilidad, pero ya no desactiva la presentación. */
void platform_set_present(void (*fn)(uint8_t *img));
void platform_set_loop(int on);

/* Retardo de CPU del original (`dbf` que solo consumen tiempo: motor de
 * elementos, reveal/retrato...). Convierte "unidades" (~ciclos/10) a tiempo con
 * KULT_CPU_HZ (def. 8 MHz) y, en modo tiempo real, duerme cediendo VBLs y
 * presentando. En headless es un no-op. KULT_CPU=0 lo desactiva. */
void platform_cpu_delay(uint32_t units);

/* Cobra `cycles` ciclos de CPU del original (modelo medido por función,
 * `-finstrument-functions`). Misma puerta/reloj que `platform_cpu_delay`. */
void platform_cpu_charge_cycles(uint64_t cycles);

/* Vacía la pila de instrumentación tras un longjmp de reinicio (reset_clear). */
void platform_cpuwork_reset_stack(void);

/* Marca manual en el log de KULT_MARKS (p. ej. F12 en el momento exacto). */
void platform_marks_event(const char *tag, int a, int b);

/* Traza de bytes de guion de la VM (build de diagnostico T-073c). */
void platform_marks_byte(uint32_t vbl, uint32_t b, uint32_t script);

/* Estado de seleccion/animacion en cada byte de guion (T-073c). */
void platform_marks_state(uint32_t vbl, uint32_t b, uint32_t script,
                          uint32_t acc9, uint32_t acd2, uint32_t aaf9,
                          uint32_t ad1d, uint32_t acc6, uint32_t skip,
                          uint32_t a572, uint32_t a0f, uint32_t cx,
                          uint32_t cy, uint32_t btn, uint32_t cmd,
                          uint32_t selobj);

/* Ritmo en tiempo real (M3): cada VBL consume 20 ms de reloj (50 Hz). Se activa
 * en modo ventana; apagado (por defecto) el port va a la máxima velocidad. */
void platform_set_realtime(int on);

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
int  platform_sdl_should_quit(void);
void platform_sdl_close(void);

/* ---- audio (M4) ---------------------------------------------------------- */
/* La emulación YM + Timer A siempre corre (headless incluido) salvo KULT_AUDIO=0.
 * La salida a dispositivo SDL es opcional (sólo build port-sdl). */
int  platform_audio_enabled(void);
void platform_audio_set_enabled(int on);

/* Drena el anillo de muestras mono S16 (lo usa el callback de SDL y las pruebas). */
int  platform_audio_pull(int16_t *out, int maxframes);

/* Avanza el audio desde una espera bloqueante (bombeo de idle en modo ventana):
 * en tiempo real hace sonar la música mientras `Cnecin` espera. */
void platform_audio_pump(uint8_t *img);

/* Verificación objetiva: energía/RMS de todo lo renderizado desde el reset. */
void     platform_audio_rms_reset(void);
uint32_t platform_audio_rms(void);       /* RMS entero de la señal renderizada */
uint32_t platform_audio_peak(void);      /* pico absoluto */
uint32_t platform_audio_samples(void);

/* Modelo del MFP Timer A (T-040), expuesto para pruebas y diagnóstico. */
uint32_t platform_mfp_timer_a_period_us(void);
int      platform_mfp_timer_a_enabled(void);
uint32_t platform_mfp_isr_count(void);
uint32_t platform_mfp_music_isr_count(void);

/* Salida de audio SDL3 (sólo `make port-sdl`, -DHAVE_SDL3). */
void platform_audio_sdl_open(void);
void platform_audio_sdl_close(void);

#endif /* KULT_PLATFORM_H */
