/* platform/video.c — vídeo del port: pantalla en memoria + paleta + volcado.
 *
 * El juego escribe un framebuffer PLANAR 320x200/16 colores (4 planos, 160 B/fila,
 * 32000 B) y una paleta de 16 words formato ST 0x0RGB. Aquí se guarda cuál es la
 * base visible (os_setscreen) y la paleta (os_setpalette/os_setcolor), y se ofrece
 * la conversión planar->RGB para volcar a PPM/raw.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "os.h"
#include "addrs.h"
#include "platform.h"

#define FB_ROW_BYTES 160
#define FB_W         320
#define FB_H         200
#define FB_BYTES     (FB_ROW_BYTES * FB_H)

static uint32_t g_visible = 0;      /* 0 = sin Setscreen todavía */
static uint16_t g_palette[16];

void platform_video_init(void) {
    g_visible = 0;
    memset(g_palette, 0, sizeof g_palette);
}

void pc_os_setscreen(uint32_t log_base, uint32_t phys_base, int16_t resolution) {
    (void)phys_base;
    (void)resolution;
    if (log_base != 0xffffffffu)
        g_visible = log_base;
}

void pc_os_setpalette(uint32_t table) {
    if (!g_kult_img || table == 0) return;
    for (int i = 0; i < 16; i++)
        g_palette[i] = be16(g_kult_img + table + i * 2);
}

void pc_os_setcolor(int16_t index, int16_t colour) {
    if (colour < 0) return;                    /* -1 = "no cambies" (consulta en TOS) */
    if (index < 0 || index > 15) return;
    g_palette[(unsigned)index] = (uint16_t)colour & 0x0fff;
}

uint32_t platform_visible_base(void) {
    if (g_visible) return g_visible;
    return g_kult_img ? be32(g_kult_img + A_SCREEN_PTR_A) : 0;
}

const uint16_t *platform_palette(void) { return g_palette; }

/* Color (0..15) del píxel (x,y) en el framebuffer planar `base`. */
static unsigned fb_pixel(uint32_t base, int x, int y) {
    const uint8_t *row = g_kult_img + base + (uint32_t)y * FB_ROW_BYTES;
    const uint8_t *grp = row + (x >> 4) * 8;         /* 4 planos x word = 8 B */
    unsigned bit = 15u - (unsigned)(x & 15);
    unsigned c = 0;
    for (unsigned p = 0; p < 4; p++) {
        uint16_t w = (uint16_t)((grp[p * 2] << 8) | grp[p * 2 + 1]);
        c |= ((w >> bit) & 1u) << p;
    }
    return c;
}

static unsigned chan8(unsigned v3) {               /* 3 bits -> 8 bits */
    return (v3 << 5) | (v3 << 2) | (v3 >> 1);
}

uint32_t platform_fb_nonzero(void) {
    if (!g_kult_img) return 0;
    uint32_t base = platform_visible_base();
    uint32_t n = 0;
    for (int y = 0; y < FB_H; y++)
        for (int x = 0; x < FB_W; x++)
            if (fb_pixel(base, x, y) != 0) n++;
    return n;
}

int platform_dump_ppm(const char *path) {
    if (!g_kult_img) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    uint32_t base = platform_visible_base();
    fprintf(f, "P6\n%d %d\n255\n", FB_W, FB_H);
    for (int y = 0; y < FB_H; y++) {
        for (int x = 0; x < FB_W; x++) {
            uint16_t w = g_palette[fb_pixel(base, x, y) & 15];
            uint8_t rgb[3] = {
                (uint8_t)chan8((w >> 8) & 7),
                (uint8_t)chan8((w >> 4) & 7),
                (uint8_t)chan8(w & 7),
            };
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return 0;
}

int platform_dump_planar(const char *path) {
    if (!g_kult_img) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(g_kult_img + platform_visible_base(), 1, FB_BYTES, f);
    fclose(f);
    return 0;
}
