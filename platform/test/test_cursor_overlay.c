/* Regresión de T-138: composición del cursor en una copia de la imagen. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "machine.h"
#include "addrs.h"
#include "platform.h"

void cursor_draw(uint8_t *img);
void cursor_restore(uint8_t *img);
void platform_cursor_compose(uint8_t *dst, const uint8_t *src);
int platform_cursor_visible(void);
void kult_core_cursor_draw(uint8_t *img);
void kult_core_cursor_restore(uint8_t *img);

/* Stubs de instrumentación: este test enlaza solo cursor.o, no el core. */
void __cyg_profile_func_enter(void *a, void *b) { (void)a; (void)b; }
void __cyg_profile_func_exit(void *a, void *b) { (void)a; (void)b; }

uint32_t screen_address_raw(uint8_t *img, uint32_t x, uint32_t y,
                            uint32_t *phase) {
    x &= 0xffff; y &= 0xffff;
    *phase = x & 0xf;
    x = (x & 0xfff0) >> 1;
    return be32(img + A_SCREEN_PTR_A) + x + y * 0xa0;
}

static int cmp(const uint8_t *a, const uint8_t *b) {
    for (size_t i = 0; i < PLATFORM_IMAGE_SIZE; i++)
        if (a[i] != b[i]) {
            fprintf(stderr, "cursor overlay: diferencia en %zu (%02x != %02x)\n",
                    i, a[i], b[i]);
            return 1;
        }
    return 0;
}

int main(void) {
    static uint8_t src[PLATFORM_IMAGE_SIZE], dst[PLATFORM_IMAGE_SIZE],
                   ref[PLATFORM_IMAGE_SIZE];
    memset(src, 0, sizeof src);
    wr32(src + A_SCREEN_PTR_A, 0x80000);
    wr32(src + A_SCREEN_PTR_B, 0x90000);
    wr32(src + A_CURSOR_SPRITE_PTR, 0xA0000);
    wr16(src + A_CURSOR_X, 100);
    wr16(src + A_CURSOR_Y, 100);
    wr16(src + A_CLIP_X, 0);
    wr16(src + A_CLIP_Y, 0);

    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 160; x++)
            src[0x80000 + y * 160 + x] = (uint8_t)(x ^ y);
    for (int i = 0; i < 16 * 4; i += 4) {
        wr16(src + 0xA0000 + i, 0x0000);
        wr16(src + 0xA0000 + i + 2, 0x8000);
    }

    cursor_draw(src);
    if (!platform_cursor_visible()) {
        fprintf(stderr, "cursor overlay: no se marcó visible\n");
        return 1;
    }
    platform_cursor_compose(dst, src);
    if (cmp(src, dst)) return 1;

    wr16(src + A_CURSOR_X, 200);
    wr16(src + A_CURSOR_Y, 120);
    platform_cursor_compose(dst, src);
    memcpy(ref, src, sizeof ref);
    kult_core_cursor_restore(ref);
    kult_core_cursor_draw(ref);
    if (cmp(ref, dst)) return 1;

    cursor_restore(src);
    if (platform_cursor_visible()) {
        fprintf(stderr, "cursor overlay: sigue visible tras restore\n");
        return 1;
    }
    platform_cursor_compose(dst, src);
    if (cmp(src, dst)) return 1;

    puts("cursor overlay OK");
    return 0;
}
