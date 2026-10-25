/* platform/osd.h — aviso breve superpuesto por la capa de presentación. */
#ifndef KULT_PLATFORM_OSD_H
#define KULT_PLATFORM_OSD_H

#include <stdint.h>

void platform_osd_show(const char *text);
void platform_osd_show_frames(const char *text, uint32_t frames);
void platform_osd_clear(void);
int  platform_osd_active(void);
const char *platform_osd_text(void);

#endif /* KULT_PLATFORM_OSD_H */
