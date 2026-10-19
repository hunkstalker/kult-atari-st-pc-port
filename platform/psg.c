/* platform/psg.c — fichero de registros del PSG/YM2149 + ledger de accesos.
 *
 * El core selecciona registro y escribe/lee por `psg_port_write/read`. Aquí se
 * guarda el fichero de 16 registros (lo que alimenta la emulación YM de
 * `platform/audio.c`) y se registra cada acceso en un **ledger ordenado**, espejo
 * del que expone el oráculo del kit: es la superficie que compara la verificación
 * (docs/VERIFICACION.md) — una escritura perdida, extra o fuera de orden, o una
 * lectura del registro equivocado, solo se ve en el ledger (los registros que
 * quedan pueden ser idénticos).
 *
 * A diferencia del oráculo, el port no "refusa" un registro no declarado: sirve su
 * contenido (0 si nadie lo escribió). El port no es un oráculo; el ledger es para
 * cotejar escenas controladas.
 */
#include <stdint.h>
#include <string.h>

#include "psg.h"

static uint8_t  g_regs[PSG_NREGS];
static uint32_t g_known;                     /* bit R = contenido de R conocido */
static uint8_t  g_log_kind[PSG_LOG_MAX];
static uint8_t  g_log_reg[PSG_LOG_MAX];
static uint8_t  g_log_val[PSG_LOG_MAX];
static uint32_t g_log_n;

static void log_ev(uint8_t kind, unsigned reg, uint8_t value) {
    if (g_log_n >= PSG_LOG_MAX)
        return;
    g_log_kind[g_log_n] = kind;
    g_log_reg[g_log_n] = (uint8_t)reg;
    g_log_val[g_log_n] = value;
    g_log_n++;
}

void psg_log_reset(void) { g_log_n = 0; }

void psg_reset(const uint8_t *seed, uint32_t known) {
    g_log_n = 0;
    g_known = known & 0xffffu;
    for (unsigned r = 0; r < PSG_NREGS; r++)
        g_regs[r] = ((g_known >> r) & 1u) && seed ? seed[r] : 0;
}

void psg_seed_reg(unsigned reg, uint8_t value) {
    if (reg >= PSG_NREGS)
        return;
    g_regs[reg] = value;
    g_known |= (1u << reg);
}

void psg_port_write(unsigned reg, uint8_t value) {
    if (reg >= PSG_NREGS)
        return;
    g_regs[reg] = value;
    g_known |= (1u << reg);
    log_ev(0, reg, value);
}

uint8_t psg_port_read(unsigned reg) {
    if (reg >= PSG_NREGS)
        return 0xff;
    uint8_t v = g_regs[reg];
    log_ev(1, reg, v);
    return v;
}

const uint8_t  *psg_regs(void)      { return g_regs; }
uint32_t        psg_log_count(void) { return g_log_n; }
const uint8_t  *psg_log_kinds(void) { return g_log_kind; }
const uint8_t  *psg_log_regs(void)  { return g_log_reg; }
const uint8_t  *psg_log_vals(void)  { return g_log_val; }
