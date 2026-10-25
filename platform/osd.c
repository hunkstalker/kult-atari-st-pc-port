/* platform/osd.c — texto breve y caducidad por tiempo real (sin SDL). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "addrs.h"
#include "osd.h"
#include "platform.h"

#define OSD_DEFAULT_MS 2500u   /* 2,5 s reales */

static char g_text[64];
static uint64_t g_until_ns;

void platform_osd_show_ms(const char *text, uint32_t ms) {
    snprintf(g_text, sizeof g_text, "%s", text ? text : "");
    g_until_ns = platform_now_ns() + (uint64_t)ms * 1000000ull;
}

void platform_osd_show(const char *text) {
    platform_osd_show_ms(text, OSD_DEFAULT_MS);
}

void platform_osd_clear(void) {
    g_text[0] = '\0';
    g_until_ns = 0;
}

int platform_osd_active(void) {
    return g_text[0] != '\0' && platform_now_ns() < g_until_ns;
}

const char *platform_osd_text(void) {
    return g_text;
}

/* La fuente del juego son 6 B por glifo (4x6, bits 4..1) en A_FONT_DATA. Los
 * códigos son 0x10-0x19 = dígitos, 0x20 = espacio y 0x21-0x3A = A-Z. */
static int osd_code(char ch) {
    if (ch >= 'a' && ch <= 'z')
        ch = (char)(ch - 'a' + 'A');
    if (ch >= 'A' && ch <= 'Z')
        return 0x21 + (ch - 'A');
    if (ch >= '0' && ch <= '9')
        return 0x10 + (ch - '0');
    return 0x20;
}

static int osd_font_ready(const uint8_t *img) {
    const uint8_t *a = img + A_FONT_DATA + 0x21 * 6;
    for (int i = 0; i < 6; i++)
        if (a[i])
            return 1;
    return 0;
}

void platform_osd_draw_rgba(uint8_t *rgba, const uint8_t *img, const char *text) {
    if (!rgba || !img || !text || !text[0] || !osd_font_ready(img))
        return;

    int len = (int)strlen(text);
    if (len > 78)
        len = 78;
    const int x0 = 4;
    const int y0 = 200 - 6 - 4;
    const int left = x0 - 1;
    const int top = y0 - 1;
    const int width = len * 4 + 2;
    const int height = 8;

    for (int y = top; y < top + height; y++) {
        if (y < 0 || y >= 200)
            continue;
        for (int x = left; x < left + width; x++) {
            if (x < 0 || x >= 320)
                continue;
            uint8_t *p = rgba + ((size_t)y * 320u + (size_t)x) * 4u;
            p[0] = p[1] = p[2] = 0;
            p[3] = 255;
        }
    }

    for (int i = 0; i < len; i++) {
        const uint8_t *gl = img + A_FONT_DATA + osd_code(text[i]) * 6;
        for (int row = 0; row < 6; row++) {
            uint8_t bits = gl[row];
            for (int col = 0; col < 4; col++) {
                if (!(bits & (1u << (4 - col))))
                    continue;
                int x = x0 + i * 4 + col;
                int y = y0 + row;
                if (x < 0 || x >= 320 || y < 0 || y >= 200)
                    continue;
                uint8_t *p = rgba + ((size_t)y * 320u + (size_t)x) * 4u;
                p[0] = p[1] = p[2] = 255;
                p[3] = 255;
            }
        }
    }
}
