/* boot/vbl.c — tick de VBL del original (0x160c2), reconstruido para el port.
 *
 * Es el trabajo por frame que el handler de la cola de VBL ejecuta en el ST:
 *   - incrementa el contador de VBL A_DAT_1ACD4;
 *   - según A_DAT_1AD0D: rotación de paleta (==5), o el trabajo normal (==0);
 *   - cada 64 VBLs: avanza A_DAT_1AB60 y estampa un strip de 16x30 desde GAUSS
 *     (0x268de) en las dos pantallas;
 *   - en los VBLs impares con A_FRAME_1AB4F: blit enmascarado de 3 planos desde la
 *     lista que empieza en 0x1a82a (MURSM 0x23e9c, y al cruzarla TEMPL 0x1d402).
 *
 * El core no lo tiene (es una de las piezas sin C del original); vive aquí (boot/,
 * ADR 0005). `pal_next_byte` sí está verificado en el core.
 */
#include <stdint.h>

#include "machine.h"
#include "addrs.h"
#include "kult_api.h"
#include "os.h"
#include "platform.h"
#include "boot.h"

#define A_DAT_1ACD5   0x1acd5u   /* último frame de GAUSS estampado */
#define A_PTR_1A82A   0x1a82au   /* puntero de avance del blit MURSM/TEMPL */
#define GAUSS_BASE    0x268deu   /* GAUSS.BIN en runtime (0x168DE + 0x10000) */

/* Rotación de los registros de color 12..15 ($ff8258..$ff825f): el port la aplica
 * a su paleta visible (los registros de hardware no se modelan; M4 los cubrirá). */
static void vbl_palette_rotate(void) {
    const uint16_t *pal = platform_palette();
    uint16_t w0 = pal[12], w1 = pal[13], w2 = pal[14], w3 = pal[15];
    os_setcolor(12, (int16_t)w1);
    os_setcolor(13, (int16_t)w2);
    os_setcolor(14, (int16_t)w3);
    os_setcolor(15, (int16_t)w0);
}

/* l_01610e: cada 64 VBLs, strip de 16x30 desde GAUSS a las dos pantallas. */
static void vbl_gauss_strip(uint8_t *img) {
    uint32_t d0 = pal_next_byte(img) & 3;
    if ((uint8_t)d0 == img[A_DAT_1ACD5])
        d0 = (d0 + 1) & 3;
    img[A_DAT_1ACD5] = (uint8_t)d0;

    uint32_t a1 = GAUSS_BASE + d0 * 0x1e0;
    uint32_t a0 = be32(img + A_SCREEN_PTR_A) + 0x140;
    uint32_t a2 = be32(img + A_SCREEN_PTR_B) + 0x140;
    for (uint32_t d1 = 0x1d;; d1 = (d1 - 1) & 0xffff) {
        for (int k = 0; k < 4; k++) {
            wr32(img + a2, be32(img + a1)); a2 += 4;      /* move.l (a1),(a2)+ */
            wr32(img + a0, be32(img + a1)); a1 += 4; a0 += 4;  /* move.l (a1)+,(a0)+ */
        }
        a0 += 0x90;
        a2 += 0x90;
        if (d1 == 0) break;
    }
}

/* l_01618a: blit enmascarado de 3 planos (wor0/+2/+6) con el patrón de 0x1a82a. */
static void vbl_mask_blit(uint8_t *img) {
    uint32_t a1 = be32(img + A_SCREEN_PTR_A) + 0x5f10;
    uint32_t a2 = be32(img + A_PTR_1A82A);
    for (uint32_t d5 = 0x1c;; d5 = (d5 - 1) & 0xffff) {
        uint32_t a0 = a1;
        for (uint32_t d4 = 0xf;; d4 = (d4 - 1) & 0xffff) {
            uint32_t wA = be16(img + a0);
            uint32_t wB = be16(img + a0 + 2);
            uint32_t wC = be16(img + a0 + 6);
            uint32_t d0 = (wA | wB) & 0xffff;
            uint32_t d1 = (~(wA & wB & wC)) & 0xffff;
            d0 &= d1;
            wr16(img + a0, (uint16_t)(wA & d0));
            wr16(img + a0 + 2, (uint16_t)(wB & d0));
            wr16(img + a0 + 6, (uint16_t)(wC & d0));
            d0 = (~d0) & 0xffff;
            d1 = (be16(img + a2) & d0) & 0xffff;
            wr16(img + a0, (uint16_t)(be16(img + a0) | d1));
            wr16(img + a0 + 2, (uint16_t)(be16(img + a0 + 2) | d1));
            wr16(img + a0 + 6, (uint16_t)(be16(img + a0 + 6) | d1));
            a0 += 8;
            a2 += 2;
            if (a2 == A_ELEM_TBL_23E9C)
                a2 = A_TEMPL_DATA;
            if (d4 == 0) break;
        }
        a1 += 0xa0;
        if (d5 == 0) break;
    }
    wr32(img + A_PTR_1A82A, a2);
}

void kult_vbl_tick(uint8_t *img) {
    img[A_DAT_1ACD4] = (uint8_t)(img[A_DAT_1ACD4] + 1);

    uint8_t f = img[A_DAT_1AD0D];
    if (f != 0) {
        if (f != 5) return;                      /* f_010768: rts */
        if (img[A_DAT_1ACD4] & 3) return;
        vbl_palette_rotate();
        return;
    }

    if (img[A_DAT_1ACD4] & 0x3f) {
        /* VBL impar con el flag de frame puesto: blit enmascarado. */
        if (!(img[A_DAT_1ACD4] & 1)) return;
        if (!img[A_FRAME_1AB4F]) return;
        vbl_mask_blit(img);
        return;
    }

    /* Múltiplo de 64: avanza el contador lento y estampa GAUSS. */
    wr16(img + A_DAT_1AB60, (uint16_t)(be16(img + A_DAT_1AB60) + 1));
    vbl_gauss_strip(img);
}
