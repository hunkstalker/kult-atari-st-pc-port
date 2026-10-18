/* platform/hw.c — registros de hardware del port (M1: MFP/ACIA mínimo).
 *
 * El juego escribe registros del MFP (Timer A, interrupciones) y del shifter por su
 * dirección. En M1 basta con guardarlos en un modelo pequeño para que las lecturas
 * de-modificación-escritura (`hw_sub8`, `hw_bclr8`, …) sean deterministas y no
 * ensucien memoria del host. El modelo funcional del MFP (que dispara las ISR) y el
 * PSG audible llegan en M4.
 */
#include <stdint.h>
#include <string.h>

#include "hw.h"

#define MFP_BASE   0xfffa00u
#define MFP_END    0xfffaffu
#define ACIA_BASE  0xfffc00u
#define ACIA_END   0xfffcffu
#define MFP_BYTES  0x100
#define ACIA_BYTES 0x100

static uint8_t g_mfp[MFP_BYTES];
static uint8_t g_acia[ACIA_BYTES];

static uint8_t *reg_slot(uint32_t addr) {
    if (addr >= MFP_BASE && addr <= MFP_END)   return &g_mfp[addr - MFP_BASE];
    if (addr >= ACIA_BASE && addr <= ACIA_END) return &g_acia[addr - ACIA_BASE];
    return NULL;                                /* shifter/otros: no modelado en M1 */
}

uint8_t hw_read_model8(uint32_t addr) {
    uint8_t *s = reg_slot(addr);
    return s ? *s : 0;
}

void hw_write8(uint32_t addr, uint32_t value) {
    uint8_t *s = reg_slot(addr);
    if (s) *s = (uint8_t)value;
}

void hw_write16(uint32_t addr, uint32_t value) {
    hw_write8(addr, value >> 8);
    hw_write8(addr + 1, value);
}

void hw_write32(uint32_t addr, uint32_t value) {
    hw_write8(addr, value >> 24);
    hw_write8(addr + 1, value >> 16);
    hw_write8(addr + 2, value >> 8);
    hw_write8(addr + 3, value);
}

void hw_bset8(uint32_t addr, uint32_t bit) {
    hw_write8(addr, hw_read_model8(addr) | (1u << (bit & 7)));
}

void hw_bclr8(uint32_t addr, uint32_t bit) {
    hw_write8(addr, hw_read_model8(addr) & ~(1u << (bit & 7)));
}

void hw_and8(uint32_t addr, uint32_t mask) {
    hw_write8(addr, hw_read_model8(addr) & (mask & 0xff));
}

void hw_sub8(uint32_t addr, uint32_t value) {
    hw_write8(addr, (uint8_t)(hw_read_model8(addr) - (value & 0xff)));
}
