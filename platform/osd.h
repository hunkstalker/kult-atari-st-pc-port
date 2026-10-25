/* platform/osd.h — aviso breve superpuesto por la capa de presentación. */
#ifndef KULT_PLATFORM_OSD_H
#define KULT_PLATFORM_OSD_H

#include <stdint.h>

void platform_osd_show(const char *text);
void platform_osd_show_frames(const char *text, uint32_t frames);
void platform_osd_clear(void);
int  platform_osd_active(void);
const char *platform_osd_text(void);
void platform_osd_draw_rgba(uint8_t *rgba, const uint8_t *img, const char *text);

#endif /* KULT_PLATFORM_OSD_H */
