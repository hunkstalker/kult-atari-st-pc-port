/* platform/include/kult_port.h — sombra de `include/kult_port.h` en el build del
 * port (se pasa `-Iplatform/include` antes que `-Iinclude`).
 *
 * El core transcribe los bucles `dbf` que solo consumen tiempo como
 * `cpu_delay(unidades)` (motor de elementos, reveal pixelado/retrato, etc.). En
 * el diferencial (`make test`) es un no-op: se compara la imagen, no el tiempo.
 * En el **ejecutable** debe costar tiempo real: aquí se enlaza con
 * `platform_cpu_delay`, que convierte unidades (~ciclos) a tiempo de CPU del ST
 * y duerme cediendo VBLs/presentación. `src/` no se toca.
 */
#ifndef KULT_PORT_H
#define KULT_PORT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void wait_vbl(uint8_t *img);

/* Retardo de CPU del original (implementado en platform/time.c). */
void platform_cpu_delay(uint32_t units);

/* KULT_FRAME_GATE: retardo (VBLs) por construccion de banco de frames. */
static inline void frame_gate_delay(uint8_t *img) {
    static int fg = -1;
    if (fg < 0) { const char *e = getenv("KULT_FRAME_GATE"); fg = e ? atoi(e) : 0; }
    for (int i = 0; i < fg; i++) wait_vbl(img);
}

static inline void cpu_delay(uint32_t units) { platform_cpu_delay(units); }

/* Logger de diagnostico del motor nativo: no-op en el port. */
static inline FILE *kult_dbgfile(void) { return NULL; }

#endif
