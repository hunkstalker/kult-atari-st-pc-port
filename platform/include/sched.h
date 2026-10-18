/* platform/include/sched.h — el reloj/planificador del port PC.
 *
 * Sombrea `tools/recreate_kit/include/sched.h`. En el diferencial `sched_poll8`
 * cuenta sondeos y aplica escrituras programadas declaradas por el test; aquí es
 * el RELOJ de la máquina: cada sondeo sobre el contador de VBL o sobre el paquete
 * IKBD avanza un frame y entrega la entrada pendiente. `src/delay.c` e `src/input.c`
 * lo llaman en sus bucles de espera, así que con esto las esperas retornan.
 */
#ifndef KULT_PLATFORM_SCHED_H
#define KULT_PLATFORM_SCHED_H

#include <stdint.h>

/* Un sondeo de la espera en `addr`, re-leído en el sitio `site_pc`. Devuelve el
 * byte actual de la imagen tras aplicar el modelo de reloj. */
uint8_t sched_poll8(uint8_t *image, uint32_t addr, uint32_t site_pc);

/* Espera activa hasta que `addr` valga `until` (para completitud; el core usa
 * sched_poll8 directamente). */
int sched_wait8(uint8_t *image, uint32_t addr, uint8_t until, uint32_t site_pc);

#endif /* KULT_PLATFORM_SCHED_H */
