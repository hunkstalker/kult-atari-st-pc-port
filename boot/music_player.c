/* boot/music_player.c — reproductor del Timer A de música (0x3a872 + envolvente
 * 0x3a936), reconstruido para el port PC.
 *
 * `mfp_setup` (0x3a604, ya en el core) instala `0x3a872` en el vector $134: el MFP
 * Timer A lo interrumpe cada ~3,5 ms (TACR=4 /50, TADR=0xaa) y este ISR hace un
 * tick del tracker de 3 canales + canales enlazados, vuelca la tabla sombra al
 * PSG y avanza la envolvente de ruido. Es la otra mitad del audio de M4: la ISR
 * PCM (`0x190f6`/`0x1910e`) ya está en el core (`src/sound.c`).
 *
 * Por qué vive aquí: `0x3a872` NO está en el índice de 406 funciones (Ghidra solo
 * lo ve como el puntero de dato del vector $134), así que no tiene reconstrucción
 * C. Es una pieza del original que falta, como `_start` o el tick de VBL; ADR 0005
 * las sitúa en `boot/`.
 *
 * Transcripción literal de `matching/out/kult.s` (0x3a872..0x3a934 y 0x3a936):
 *   - `trap #3` -> `psg_trap3` (handler 0x3aa38, core) y `trap #4` -> `music_tick`
 *     (handler 0x3aab6, core), exactamente como el oráculo llega a ellos por
 *     vector;
 *   - la lectura del flag de rearme en estado[0x1a] (0 = tick saltado), el
 *     contador [0x1b]/[0x1c] y el puntero de envolvente [0x14];
 *   - el epílogo limpia ISRA bit 5 (`$fffa0f`) por `hw_bclr8`.
 *
 * El caso de verificación reproduce una escena controlada y compara el ledger PSG
 * del port con el del oráculo (23 eventos; ver `platform/test/test_platform.c`).
 */
#include <stdint.h>

#include "machine.h"
#include "hw.h"
#include "kult_api.h"
#include "boot.h"

#define MUSIC_STATE  0x3ae8cu   /* estado global del tracker */
#define MUSIC_SHADOW 0x3ae7cu   /* tabla sombra: 3 words de periodo */
#define MUSIC_VOL    0x3ae84u   /* tabla sombra: 3 bytes de volumen */
#define MUSIC_NOISE  0x3b19cu   /* estructura de la envolvente de ruido */
#define MUSIC_MASK   0x3b1a2u   /* máscara de bits del mixer */
#define CH0          0x3afbcu   /* canal 0; los canales van de 0x50 en 0x50 */
#define CH_STEP      0x50u

/* l_03a936: avanza la envolvente de ruido y vuelca sus dos registros.
 *   a0 = estado[0x14]; [0]=valor, [1]=contador, [2]=recarga, [3]/[4]=límites,
 *   [5]=paso (rebota con eor #$fe), [6]=máscara. Escribe R6 (periodo de ruido) y
 *   R7 (mixer, con el RMW de `psg_trap3`). */
static void music_noise(uint8_t *img, uint32_t state) {
    uint32_t a0 = be32(img + state + 0x14);

    if (img[a0 + 1] != 0) {
        img[a0 + 1] = (uint8_t)(img[a0 + 1] - 1);
        if (img[a0 + 1] == 0) {
            img[a0 + 1] = img[a0 + 2];
            uint8_t v = (uint8_t)(img[a0] + img[a0 + 5]);
            img[a0] = v;
            if (v == img[a0 + 3] || v == img[a0 + 4])
                img[a0 + 5] ^= 0xfeu;
        }
    }

    psg_trap3(img, img[a0], 6, 0, 0);
    psg_trap3(img, (uint32_t)(img[a0 + 6] & img[MUSIC_MASK]), 7, 0, 0);
}

/* 0x3a872: ISR del Timer A (termina en rte en 0x3a934). */
void kult_music_player(uint8_t *img) {
    const uint32_t state = MUSIC_STATE;

    /* tst.b 0x1a(a6) / beq al epílogo: un rearme a 0 salta el tick. */
    if (img[state + 0x1a] != 0) {
        img[state + 0x1a] = 0;

        /* subi.b #1,0x1b(a6) */
        img[state + 0x1b] = (uint8_t)(img[state + 0x1b] - 1);
        /* lea s_03b19c / move.l a0,0x14(a6): valor por defecto de la envolvente */
        wr32(img + state + 0x14, MUSIC_NOISE);

        /* trap #4 sobre los 3 canales principales. */
        for (uint32_t d7 = 0; d7 < 3; d7++)
            music_tick(img, CH0 + d7 * CH_STEP, state, d7);

        /* trap #4 sobre el canal enlazado de cada canal (si bit 1 de [0x27]). */
        for (uint32_t d7 = 0; d7 < 3; d7++) {
            uint32_t a3 = CH0 + d7 * CH_STEP;
            if (img[a3 + 0x27] & 0x02u)
                music_tick(img, be32(img + a3 + 0x1c), state, d7);
        }

        /* Tabla sombra de periodos (words) -> pares de registros (D2=0x8000). */
        {
            uint32_t a0 = MUSIC_SHADOW;
            for (uint32_t d7 = 0; d7 < 3; d7++) {
                uint32_t d0 = be16(img + a0);
                a0 += 2;
                psg_trap3(img, d0, 0x8000u, d7, 0);
            }
        }

        /* Tabla sombra de volúmenes (bytes) -> registros 8/9/10 (D2=8). */
        {
            uint32_t a0 = MUSIC_VOL;
            for (uint32_t d7 = 0; d7 < 3; d7++) {
                uint32_t d0 = img[a0];
                a0 += 1;
                psg_trap3(img, d0, 8, d7, 0);
            }
        }

        /* bsr l_03a936 (envolvente de ruido). */
        music_noise(img, state);

        /* tst.b 0x1b(a6) / bne; si llegó a 0, recarga desde 0x1c. */
        if (img[state + 0x1b] == 0)
            img[state + 0x1b] = img[state + 0x1c];

        /* st.b 0x1a(a6): rearma para el siguiente tick. */
        img[state + 0x1a] = 0xff;
    }

    /* Epílogo: bclr.b #5,$fffa0f (ISRA: Timer A fuera de servicio). */
    hw_bclr8(0xfffa0f, 5);
}
