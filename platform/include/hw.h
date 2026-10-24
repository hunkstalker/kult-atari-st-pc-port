/* platform/include/hw.h — registros de hardware (MFP/shifter/ACIA) en el port PC.
 *
 * Sombrea `lib/kult-core/tools/recreate_kit/include/hw.h` en el build del ejecutable. En el
 * diferencial estas escrituras se comparan por un ledger; aquí son el modelo real
 * del port (M1: MFP mínimo; el audio llega en M4). La frontera con el core es la
 * misma: el juego escribe registros por su dirección y el port decide qué hacer.
 */
#ifndef KULT_PLATFORM_HW_H
#define KULT_PLATFORM_HW_H

#include <stdint.h>

void hw_write8(uint32_t addr, uint32_t value);
void hw_write16(uint32_t addr, uint32_t value);
void hw_write32(uint32_t addr, uint32_t value);

void hw_bset8(uint32_t addr, uint32_t bit);
void hw_bclr8(uint32_t addr, uint32_t bit);
void hw_and8(uint32_t addr, uint32_t mask);
void hw_sub8(uint32_t addr, uint32_t value);

/* Reajuste de un registro al patrón de reseteo (lo usa el modelo del MFP). */
uint8_t hw_read_model8(uint32_t addr);

#endif /* KULT_PLATFORM_HW_H */
