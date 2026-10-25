/* platform/diag.h — puente interno entre la instrumentación y el diagnóstico.
 *
 * Solo lo usa platform/cpuwork.c. La API pública de diagnóstico (eventos de
 * texto, anillo, marcas, entrada y RNG) sigue declarada en platform.h.
 */
#ifndef KULT_DIAG_H
#define KULT_DIAG_H

#include <stdint.h>

/* Cambios del byte de skip, una vez por entrada de función instrumentada. */
void platform_diag_tick(void);

/* Entrada de función: marcas F y buffer circular F12 (si procede). */
void platform_diag_func_begin(uint32_t addr, uint32_t vbl, int first);

/* Entrada de función: trazas específicas B/U/P/S (si procede). */
void platform_diag_func_end(uint32_t addr, uint32_t vbl);

/* Salida de anim_bank/anim_bank_build: línea B con el banco activo. */
void platform_diag_bank(uint32_t addr, uint32_t caller_addr);

#endif /* KULT_DIAG_H */
