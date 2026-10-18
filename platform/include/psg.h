/* platform/include/psg.h — chip PSG/YM2149 en el port PC.
 *
 * Sombrea `tools/recreate_kit/include/psg.h`. En M1 sólo se mantiene el fichero de
 * registros (para que las RMW del core sean deterministas); la emulación de sonido
 * y la salida SDL llegan en M4.
 */
#ifndef KULT_PLATFORM_PSG_H
#define KULT_PLATFORM_PSG_H

#include <stdint.h>

#define PSG_NREGS 16

void    psg_port_write(unsigned reg, uint8_t value);
uint8_t psg_port_read(unsigned reg);

/* Fichero de registros vivo (para volcados/depuración y tests). */
const uint8_t *psg_regs(void);

#endif /* KULT_PLATFORM_PSG_H */
