/* platform/os.c — símbolos del kit que el port debe satisfacer, adaptados a PC.
 *
 * En el diferencial, `os_refused` lleva una cuenta de negativas y `g_os_event` un
 * ledger de sucesos off-image; ambos sirven a la comparación con el oráculo. En el
 * port no hay oráculo: `os_refused` devuelve el centinela sin contar y el ledger de
 * eventos es un no-op (M1). Se declaran aquí para que el enlazado cierre.
 */
#include <stdint.h>

#include "os.h"

uint8_t *g_kult_img = NULL;
uint32_t g_os_dta = 0;

int32_t os_refused(int32_t sentinel) {
    return sentinel;
}

void g_os_event(uint16_t kind, uint32_t value) {
    (void)kind;
    (void)value;
}

void platform_set_image(uint8_t *img) {
    g_kult_img = img;
}
