/* platform/include/psg.h — chip PSG/YM2149 en el port PC.
 *
 * Sombrea `lib/kult-core/tools/recreate_kit/include/psg.h` en el build del ejecutable. Mantiene
 * el fichero de 16 registros (determinista para las RMW del mixer) que leen las
 * reconstrucciones del core (`ym_write`, `psg_trap3`, la ISR PCM) y, desde M4,
 * además:
 *   - un **ledger ordenado** de cada acceso directo (escritura Y lectura), que es
 *     lo que se compara con el oráculo (docs/VERIFICACION.md);
 *   - un **seed** de los registros que la HAL declara conocidos en la entrada de
 *     una escena (fiel al modelo del kit: un registro no declarado se sirve 0).
 *
 * La emulación sonora del YM (ondas + envolvente + mezcla) vive en
 * `platform/audio.c` y lee el fichero con `psg_regs()`.
 */
#ifndef KULT_PLATFORM_PSG_H
#define KULT_PLATFORM_PSG_H

#include <stdint.h>

#define PSG_NREGS  16
#define PSG_LOG_MAX 1024

void    psg_port_write(unsigned reg, uint8_t value);
uint8_t psg_port_read(unsigned reg);

/* Fichero de registros vivo (para volcados/depuración y la emulación YM). */
const uint8_t *psg_regs(void);

/* ---- ledger de accesos directos (verificación contra el oráculo) ---------- */
/* Cada entrada es (kind, reg, val): kind = 0 escritura, 1 lectura. El valor de
 * una lectura es el que se sirvió. Limpia el ledger. */
void psg_log_reset(void);
uint32_t       psg_log_count(void);
const uint8_t *psg_log_kinds(void);
const uint8_t *psg_log_regs(void);
const uint8_t *psg_log_vals(void);

/* Instala el estado de entrada del chip: `known` es una máscara de bits (registro
 * R conocido si el bit R está puesto) y `seed` sus contenidos. Limpia el ledger
 * (como el arranque de una corrida del oráculo) y el resto de registros a 0. */
void psg_reset(const uint8_t *seed, uint32_t known);

/* Azúcar para escenas: declara un registro conocido con su contenido. */
void psg_seed_reg(unsigned reg, uint8_t value);

#endif /* KULT_PLATFORM_PSG_H */
