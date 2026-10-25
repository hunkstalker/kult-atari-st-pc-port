/* platform/display.c — escala entera Auto/1x..6x, relleno y viewport. */
#include "display.h"

#define FB_W 320
#define FB_H 200

static int normaliza_modo(int mode) {
    if (mode < PLATFORM_SCALE_AUTO ||
        (mode > PLATFORM_SCALE_MAX && mode != PLATFORM_SCALE_FILL))
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
    if (mode == PLATFORM_SCALE_FILL)
        return 0;
    if (mode == PLATFORM_SCALE_AUTO)
        return max;
    return mode < max ? mode : max;
}

void platform_display_viewport(int mode, int out_w, int out_h,
                               platform_viewport *vp) {
    if (!vp)
        return;
    if (out_w < 1) out_w = FB_W;
    if (out_h < 1) out_h = FB_H;
    mode = normaliza_modo(mode);
    if (mode == PLATFORM_SCALE_FILL) {
        vp->x = 0;
        vp->y = 0;
        vp->w = out_w;
        vp->h = out_h;
        vp->scale = 0;
        vp->fill = 1;
        return;
    }
    vp->scale = platform_display_scale(mode, out_w, out_h);
    vp->w = FB_W * vp->scale;
    vp->h = FB_H * vp->scale;
    vp->x = (out_w - vp->w) / 2;
    vp->y = (out_h - vp->h) / 2;
    vp->fill = 0;
}

int platform_display_next(int mode) {
    mode = normaliza_modo(mode);
    if (mode == PLATFORM_SCALE_AUTO)
        return PLATFORM_SCALE_FILL;
    if (mode == PLATFORM_SCALE_FILL)
        return PLATFORM_SCALE_MIN;
    if (mode >= PLATFORM_SCALE_MAX)
        return PLATFORM_SCALE_AUTO;
    return mode + 1;
}

void platform_display_point(const platform_viewport *vp, float rx, float ry,
                            int *x, int *y) {
    int gx = 0, gy = 0;
    if (vp && vp->fill) {
        gx = (int)(rx * (float)FB_W / (float)vp->w);
        gy = (int)(ry * (float)FB_H / (float)vp->h);
    } else if (vp && vp->scale > 0) {
        gx = (int)((rx - (float)vp->x) / (float)vp->scale);
        gy = (int)((ry - (float)vp->y) / (float)vp->scale);
    }
    if (gx < 0) gx = 0;
    if (gx > FB_W - 1) gx = FB_W - 1;
    if (gy < 0) gy = 0;
    if (gy > FB_H - 1) gy = FB_H - 1;
    if (x) *x = gx;
    if (y) *y = gy;
}
