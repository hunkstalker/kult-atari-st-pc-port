/* boot/language.c — menú de idioma de `_start` (0x10000), reconstruido.
 *
 * Vive aparte de `boot/boot.c` para poder probarlo sin arrastrar todo el arranque
 * (solo depende de los servicios OS). Transcribe `_start` l_010088/l_01009e y la
 * escritura de la letra en las 4 entradas de la lista A.
 *
 *   l_010088: Cconis (trap #1/$0B) + Cnecin (trap #1/$08) hasta vaciar las teclas
 *             pendientes (una tecla pulsada durante el título no debe elegir
 *             idioma). El port no modela 0x0B; Crawio(0xff) es su equivalente no
 *             bloqueante.
 *   l_01009e: Cnecin bloqueante; se acepta D/E/F (tras pasar a mayúsculas) y se
 *             escribe la letra en 4 bytes de la lista A.
 */
#include <stdio.h>
#include <stdint.h>

#include "os.h"
#include "platform.h"
#include "boot.h"

uint8_t kult_boot_language(uint8_t *img) {
    /* Vacía las teclas ya encoladas (Cconis+Cnecin del original). */
    while (os_crawio(img, 0x00ff) != 0)
        ;

    for (;;) {
        uint32_t key = 0;
        os_cnecin(img, &key);                 /* trap #1/$8 (Cnecin) */
        key &= 0xff;
        if (key >= 'a' && key <= 'z')
            key -= 0x20;                      /* subi.b #$20 */
        if (key >= 'D' && key < 'G')
            return (uint8_t)key;
        if (key == 'S' || key == 'C') {
            char name[16];
            snprintf(name, sizeof name, "VERB%c.BIN", key);
            if (platform_file_exists(name))
                return (uint8_t)key;
            platform_log("idioma %c no instalado; se usa E", key);
            return 'E';
        }
    }
}

void kult_boot_apply_language(uint8_t *img, uint8_t lang) {
    img[0x1a3aa + 4]    = lang;
    img[0x1a3aa + 0x12] = lang;
    img[0x1a3aa + 0x20] = lang;
    img[0x1a3aa + 0x2e] = lang;
}
