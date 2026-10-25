/* platform/display.c — escala entera Auto/1x..6x y viewport centrado. */
#include "display.h"

#define FB_W 320
#define FB_H 200

static int normaliza_modo(int mode) {
    if (mode < PLATFORM_SCALE_AUTO || mode > PLATFORM_SCALE_MAX)
        return PLATFORM_SCALE_AUTO;
    return mode;
}

static int escala_maxima(int out_w, int out_h) {
    int sx = out_w / FB_W;
    int sy = out_h / FB_H;
    int s = sx < sy ? sx : sy;
    return s < PLATFORM_SCALE_MIN ? PLATFORM_SCALE_MIN : s;
}

int platform_display_scale(int mode, int out_w, int out_h) {
    int max = escala_maxima(out_w, out_h);
    mode = normaliza_modo(mode);
    if (mode == PLATFORM_SCALE_AUTO)
        return max;
    return mode < max ? mode : max;
}

void platform_display_viewport(int mode, int out_w, int out_h,
                               platform_viewport *vp) {
    if (!vp)
        return;
    vp->scale = platform_display_scale(mode, out_w, out_h);
    vp->w = FB_W * vp->scale;
    vp->h = FB_H * vp->scale;
    vp->x = (out_w - vp->w) / 2;
    vp->y = (out_h - vp->h) / 2;
}

int platform_display_next(int mode) {
    mode = normaliza_modo(mode);
    if (mode == PLATFORM_SCALE_AUTO)
        return PLATFORM_SCALE_MIN;
    if (mode >= PLATFORM_SCALE_MAX)
        return PLATFORM_SCALE_AUTO;
    return mode + 1;
}
