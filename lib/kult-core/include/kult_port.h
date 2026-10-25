/* kult_port.h — ayudas del port que no son piezas del original.
 *
 * `cpu_delay`: sustituye los bucles `dbf` que solo consumen tiempo (reveal,
 * animacion, ...). En el diferencial es un no-op (lo que se compara es la
 * imagen, no el tiempo); el build nativo lo reemplaza para ceder frames.
 */
#ifndef KULT_PORT_H
#define KULT_PORT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void wait_vbl(uint8_t *img);

/* KULT_FRAME_GATE: retardo (VBLs) por construccion de banco de frames. */
static inline void frame_gate_delay(uint8_t *img) {
    static int fg = -1;
    if (fg < 0) { const char *e = getenv("KULT_FRAME_GATE"); fg = e ? atoi(e) : 0; }
    for (int i = 0; i < fg; i++) wait_vbl(img);
}

static inline void cpu_delay(uint32_t units) { (void)units; }

/* Retardo de CPU por ciclos exactos (bucles que no son `dbf`). En el
 * diferencial es un no-op: solo cuenta el tiempo en el ejecutable. */
static inline void cpu_delay_cycles(uint64_t cycles) { (void)cycles; }

/* Logger de diagnostico del motor nativo: no-op en el diferencial. */
static inline FILE *kult_dbgfile(void) { return NULL; }

/* Traza de diagnostico del anillo de acciones: no-op en el diferencial. */
static inline void kult_diag_ev(const char *tag, uint8_t *img,
                                uint32_t a, uint32_t b, uint32_t c) {
    (void)tag; (void)img; (void)a; (void)b; (void)c;
}

static inline void kult_diag_rnd(const char *tag, uint8_t *img,
                                 uint32_t a, uint32_t b, uint32_t c) {
    (void)tag; (void)img; (void)a; (void)b; (void)c;
}

static inline void kult_diag_input(const char *tag, uint8_t *img,
                                   uint32_t x, uint32_t y, uint32_t buttons) {
    (void)tag; (void)img; (void)x; (void)y; (void)buttons;
}

/* Instantanea de una linea de texto dibujada (solo diagnostico del port). */
static inline void kult_text_ev(const char *tag, uint8_t *img, uint32_t str,
                                uint32_t end, uint32_t width, uint32_t index) {
    (void)tag; (void)img; (void)str; (void)end; (void)width; (void)index;
}

/* Banco del overlay A (MOTS en el juego original). El port lo mueve para que
 * los packs grandes no se solapen con PUZZL; en el diferencial sigue fijo. */
static inline uint32_t kult_overlay_a_bank(void) { return 0x41b76u; }

#endif
