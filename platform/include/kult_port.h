/* platform/include/kult_port.h — sombra de `include/kult_port.h` en el build del
 * port (se pasa `-Iplatform/include` antes que `-Iinclude`).
 *
 * El core transcribe los bucles `dbf` que solo consumen tiempo como
 * `cpu_delay(unidades)` (motor de elementos, reveal pixelado/retrato, etc.). En
 * el diferencial (`make test`) es un no-op: se compara la imagen, no el tiempo.
 * En el **ejecutable** debe costar tiempo real: aquí se enlaza con
 * `platform_cpu_delay`, que convierte unidades (~ciclos) a tiempo de CPU del ST
 * y duerme cediendo VBLs/presentación. El core solo usa estos hooks; la lógica
 * del juego no cambia.
 */
#ifndef KULT_PORT_H
#define KULT_PORT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void wait_vbl(uint8_t *img);

/* Retardo de CPU del original (implementado en platform/time.c). */
void platform_cpu_delay(uint32_t units);

/* Coste de CPU en ciclos exactos (implementado en platform/time.c). */
void platform_cpu_charge_cycles(uint64_t cycles);

/* KULT_FRAME_GATE: retardo (VBLs) por construccion de banco de frames. */
static inline void frame_gate_delay(uint8_t *img) {
    static int fg = -1;
    if (fg < 0) { const char *e = getenv("KULT_FRAME_GATE"); fg = e ? atoi(e) : 0; }
    for (int i = 0; i < fg; i++) wait_vbl(img);
}

static inline void cpu_delay(uint32_t units) { platform_cpu_delay(units); }

/* Retardo de CPU por ciclos exactos (bucles que no son `dbf`). */
static inline void cpu_delay_cycles(uint64_t cycles) {
    platform_cpu_charge_cycles(cycles);
}

/* Logger de diagnostico del motor nativo: no-op en el port. */
static inline FILE *kult_dbgfile(void) { return NULL; }

/* Traza de diagnostico del anillo de acciones (platform/cpuwork.c). */
void platform_diag_ev(const char *tag, uint8_t *img,
                      uint32_t a, uint32_t b, uint32_t c);
static inline void kult_diag_ev(const char *tag, uint8_t *img,
                                uint32_t a, uint32_t b, uint32_t c) {
    platform_diag_ev(tag, img, a, b, c);
}

/* Traza de lecturas de la tabla ALEAT (0x2741e, pal_next_byte/word). */
void platform_diag_rnd(const char *tag, uint8_t *img,
                       uint32_t a, uint32_t b, uint32_t c);
static inline void kult_diag_rnd(const char *tag, uint8_t *img,
                                 uint32_t a, uint32_t b, uint32_t c) {
    platform_diag_rnd(tag, img, a, b, c);
}

/* Traza de entrada: clics crudos de SDL y clics procesados por wait_frame. */
void platform_diag_input(const char *tag, uint8_t *img,
                         uint32_t x, uint32_t y, uint32_t buttons);
#ifdef KULT_NO_CPUWORK
static inline void kult_diag_input(const char *tag, uint8_t *img,
                                   uint32_t x, uint32_t y, uint32_t buttons) {
    (void)tag; (void)img; (void)x; (void)y; (void)buttons;
}
#else
static inline void kult_diag_input(const char *tag, uint8_t *img,
                                   uint32_t x, uint32_t y, uint32_t buttons) {
    platform_diag_input(tag, img, x, y, buttons);
}
#endif

#endif
