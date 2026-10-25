/* platform/osd.c — texto breve y caducidad por VBL (sin SDL). */
#include <stdio.h>
#include <stdint.h>

#include "osd.h"
#include "platform.h"

#define OSD_DEFAULT_FRAMES 125u   /* 2,5 s a 50 Hz */

static char g_text[64];
static uint32_t g_until;

void platform_osd_show_frames(const char *text, uint32_t frames) {
    snprintf(g_text, sizeof g_text, "%s", text ? text : "");
    g_until = platform_frame_count() + frames;
}

void platform_osd_show(const char *text) {
    platform_osd_show_frames(text, OSD_DEFAULT_FRAMES);
}

void platform_osd_clear(void) {
    g_text[0] = '\0';
    g_until = 0;
}

int platform_osd_active(void) {
    return g_text[0] != '\0' &&
           (int32_t)(g_until - platform_frame_count()) > 0;
}

const char *platform_osd_text(void) {
    return g_text;
}
