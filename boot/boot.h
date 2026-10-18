/* boot/boot.h — arranque del port (reconstrucción de `_start` y carga del PRG).
 *
 * ADR 0005: el arranque vive en `boot/`, fuera de `src/`. La pieza `_start`
 * (0x10000) NO está en el C verificado (único nombre sin definición C); aquí se
 * reconstruye a partir de `matching/src/start.s`, encadenando las funciones que el
 * core ya aporta. En M1 es el puente para poder ejecutar headless; M2 lo valida y
 * completa (menú fiel, tick 0x160c2).
 */
#ifndef KULT_BOOT_H
#define KULT_BOOT_H

#include <stdint.h>

/* Carga un .PRG en la imagen (base 0x10000, 1 MiB). 0 = ok. */
int kult_load_prg(uint8_t *img, const char *path);

/* Ejecuta el arranque y el bucle principal, cortando tras `max_frames` pasos del
 * bucle (longjmp desde el hook del motor). Devuelve 0 si llegó al bucle; <0 si algo
 * falló antes. */
int kult_boot(uint8_t *img, uint32_t max_frames);

#endif /* KULT_BOOT_H */
