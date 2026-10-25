/* platform/display.h — cálculo puro de escala y viewport (sin SDL). */
#ifndef KULT_PLATFORM_DISPLAY_H
#define KULT_PLATFORM_DISPLAY_H

#define PLATFORM_SCALE_AUTO 0
#define PLATFORM_SCALE_MIN  1
#define PLATFORM_SCALE_MAX  6

typedef struct {
    int x, y;
    int w, h;
    int scale;
} platform_viewport;

int  platform_display_scale(int mode, int out_w, int out_h);
void platform_display_viewport(int mode, int out_w, int out_h,
                               platform_viewport *vp);
int  platform_display_next(int mode);

#endif /* KULT_PLATFORM_DISPLAY_H */
