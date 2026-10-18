/* boot/prg.c — carga de un GEMDOS .PRG en la imagen plana del juego.
 *
 * Espeja `tools/recreate_kit/oracle/loader.py` y `tools/prg_dis.py`: cabecera de 28
 * bytes, text+data en la base de carga (0x10000), y la tabla DRI de reubicación de
 * longwords (primer offset + flujo de deltas de 1 byte; 0 = fin, 1 = salto de 254).
 * La BSS no viaja en el fichero: la imagen ya es 1 MiB a cero.
 *
 * El core es C nativo, pero sus TABLAS DE DATOS (gráficos, guiones, textos) viven en
 * el PRG original: por eso hay que cargarlo en la imagen, igual que el arnés.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PRG_HEADER_LEN 28
#define PRG_LOAD_BASE  0x10000u
#define PRG_IMAGE_SIZE 0x100000u

static uint32_t rd32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static void wr32be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

int kult_load_prg(uint8_t *img, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    uint8_t hdr[PRG_HEADER_LEN];
    if (fread(hdr, 1, PRG_HEADER_LEN, f) != PRG_HEADER_LEN) { fclose(f); return -1; }
    uint32_t tlen = rd32(hdr + 2);
    uint32_t dlen = rd32(hdr + 6);
    /* blen (hdr+10) y slen (hdr+14) */
    uint32_t slen = rd32(hdr + 14);
    uint32_t absf = (uint32_t)hdr[26] << 8 | hdr[27];
    uint32_t seg  = tlen + dlen;

    if (PRG_LOAD_BASE + seg > PRG_IMAGE_SIZE) { fclose(f); return -1; }
    if (fread(img + PRG_LOAD_BASE, 1, seg, f) != seg) { fclose(f); return -1; }

    if (absf == 0) {
        long reloc = PRG_HEADER_LEN + seg + slen;
        if (fseek(f, reloc, SEEK_SET) == 0) {
            uint8_t b4[4];
            if (fread(b4, 1, 4, f) == 4) {
                uint32_t cur = rd32(b4);
                if (cur != 0) {
                    /* El primer longword es el offset de la 1.a reloc. */
                    uint32_t a = PRG_LOAD_BASE + cur;
                    if (a + 4 <= PRG_IMAGE_SIZE)
                        wr32be(img + a, rd32(img + a) + PRG_LOAD_BASE);
                    /* Luego, un byte por paso: 0 = fin; 1 = SALTO de 254 SIN
                     * relocalizar; cualquier otro = delta a la siguiente. */
                    for (;;) {
                        int c = fgetc(f);
                        if (c == EOF || c == 0) break;
                        if (c == 1) { cur += 254; continue; }   /* skip: no aplica */
                        cur += (uint32_t)c;
                        a = PRG_LOAD_BASE + cur;
                        if (a + 4 <= PRG_IMAGE_SIZE)
                            wr32be(img + a, rd32(img + a) + PRG_LOAD_BASE);
                    }
                }
            }
        }
    }
    fclose(f);
    return 0;
}
