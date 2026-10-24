/* boot/prg.c — carga de un GEMDOS .PRG en la imagen plana del juego. */
#include <stdio.h>
#include <stdlib.h>
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
    p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8); p[3] = (uint8_t)v;
}

int kult_load_prg_mem(uint8_t *img, const uint8_t *data, size_t len) {
    if (!img || !data || len < PRG_HEADER_LEN) return -1;
    const uint8_t *hdr = data;
    uint32_t tlen = rd32(hdr + 2);
    uint32_t dlen = rd32(hdr + 6);
    uint32_t slen = rd32(hdr + 14);
    uint32_t absf = (uint32_t)hdr[26] << 8 | hdr[27];
    uint32_t seg = tlen + dlen;
    if (PRG_LOAD_BASE + seg > PRG_IMAGE_SIZE) return -1;
    if ((size_t)PRG_HEADER_LEN + seg > len) return -1;
    memcpy(img + PRG_LOAD_BASE, data + PRG_HEADER_LEN, seg);
    if (absf == 0) {
        size_t reloc = PRG_HEADER_LEN + seg + slen;
        if (reloc + 4 <= len) {
            uint32_t cur = rd32(data + reloc);
            if (cur != 0) {
                uint32_t a = PRG_LOAD_BASE + cur;
                if (a + 4 <= PRG_IMAGE_SIZE)
                    wr32be(img + a, rd32(img + a) + PRG_LOAD_BASE);
                for (size_t p = reloc + 4; p < len; p++) {
                    uint8_t c = data[p];
                    if (c == 0) break;
                    if (c == 1) { cur += 254; continue; }
                    cur += c;
                    a = PRG_LOAD_BASE + cur;
                    if (a + 4 <= PRG_IMAGE_SIZE)
                        wr32be(img + a, rd32(img + a) + PRG_LOAD_BASE);
                }
            }
        }
    }
    return 0;
}

int kult_load_prg(uint8_t *img, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return -1; }
    uint8_t *b = malloc((size_t)n ? (size_t)n : 1);
    if (!b) { fclose(f); return -1; }
    if (n && fread(b, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(b); return -1; }
    fclose(f);
    int rc = kult_load_prg_mem(img, b, (size_t)n);
    free(b);
    return rc;
}
