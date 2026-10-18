/* platform/psg.c — fichero de registros del PSG/YM2149 (M1: sin sonido).
 *
 * El core selecciona registro y escribe/lee por `psg_port_write/read`. En M1 sólo se
 * guarda el fichero de 16 registros (determinista para las RMW del mixer); M4
 * conectará la emulación del chip y la salida SDL.
 */
#include <stdint.h>
#include <string.h>

#include "psg.h"

static uint8_t g_regs[PSG_NREGS];

void psg_port_write(unsigned reg, uint8_t value) {
    if (reg < PSG_NREGS)
        g_regs[reg] = value;
}

uint8_t psg_port_read(unsigned reg) {
    if (reg < PSG_NREGS)
        return g_regs[reg];
    return 0xff;
}

const uint8_t *psg_regs(void) { return g_regs; }
