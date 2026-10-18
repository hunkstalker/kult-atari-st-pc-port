/* app/main.c — driver headless del port (M1).
 *
 * Carga el PRG del usuario en la imagen plana, inicializa la HAL de PC, ejecuta el
 * arranque reconstruido y avanza `KULT_FRAMES` pasos del bucle principal, y vuelca
 * el framebuffer visible a PPM (y, opcionalmente, el planar crudo).
 *
 * Variables de entorno:
 *   KULT_PRG       ruta del .PRG            (def. work/disk_a/KULT.PRG)
 *   KULT_DATA_DIR  directorio(s) de datos   (def. data/; admite varios con ':')
 *   KULT_FRAMES    frames a avanzar         (def. 300)
 *   KULT_DUMP      PPM de salida            (def. out/frame.ppm)
 *   KULT_PLANAR    raw planar de salida     (opcional)
 *   KULT_KEYS      teclas inyectadas        (def. auto 'E')
 *
 * La ventana SDL y la entrada real llegan en M3; esto es la base de validación.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "platform.h"
#include "boot.h"

#define IMAGE_SIZE 0x100000u

static const char *env_or(const char *name, const char *def) {
    const char *v = getenv(name);
    return (v && *v) ? v : def;
}

int main(void) {
    const char *prg    = env_or("KULT_PRG", "work/disk_a/KULT.PRG");
    const char *dump   = env_or("KULT_DUMP", "out/frame.ppm");
    const char *planar = getenv("KULT_PLANAR");
    uint32_t frames    = (uint32_t)strtoul(env_or("KULT_FRAMES", "300"), NULL, 0);

    uint8_t *img = calloc(1, IMAGE_SIZE);
    if (!img) { fprintf(stderr, "sin memoria para la imagen\n"); return 2; }

    if (kult_load_prg(img, prg) != 0) {
        fprintf(stderr, "no se pudo cargar el PRG: %s\n", prg);
        return 2;
    }

    platform_set_image(img);
    platform_init();

    if (kult_boot(img, frames) != 0) {
        fprintf(stderr, "el arranque falló\n");
        return 3;
    }

    if (platform_dump_ppm(dump) != 0)
        fprintf(stderr, "no se pudo escribir %s\n", dump);
    if (planar && platform_dump_planar(planar) != 0)
        fprintf(stderr, "no se pudo escribir %s\n", planar);

    printf("prg=%s frames=%u visible=0x%05x fb_nonzero=%u ppm=%s\n",
           prg, platform_frame_count(), platform_visible_base(),
           platform_fb_nonzero(), dump);
    free(img);
    return 0;
}
