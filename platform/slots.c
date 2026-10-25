/* platform/slots.c — savestates por ranura (T-103).
 *
 * Cada ranura guarda la imagen completa de 1 MiB del motor, la paleta y la base
 * visible. No incluye el estado del MFP/PSG: al cargar se restaura el estado
 * lógico/visual y el juego decide cuándo vuelve a sonar la música.
 */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "osd.h"
#include "kult_api.h"

#define SLOT_MAGIC "KSLOTP01"
#define SLOT_VERSION 1u
#define SLOT_HDR_SIZE 64u
#define SLOT_ROW_SIZE (SLOT_HDR_SIZE + PLATFORM_IMAGE_SIZE)

static int g_slot;
static int g_pending_save;
static int g_pending_load;

static void slot_path(int slot, char *out, size_t out_size) {
    snprintf(out, out_size, "%s/slot-%d.save", platform_saves_dir(), slot);
}

static void ini_path(char *out, size_t out_size) {
    snprintf(out, out_size, "%s/kult.ini", platform_storage_path());
}

static void put_be16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}

static void put_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static uint16_t get_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t get_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static int normaliza_slot(int slot) {
    if (slot < 0 || slot >= PLATFORM_SLOT_COUNT)
        return -1;
    return slot;
}

static int ini_leer_slot(void) {
    char path[1300], line[256];
    ini_path(path, sizeof path);
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    int slot = 0;
    while (fgets(line, sizeof line, f)) {
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (strncmp(p, "save_slot", 9) == 0) {
            char *eq = strchr(p, '=');
            if (eq) {
                int v = atoi(eq + 1);
                slot = normaliza_slot(v);
                if (slot < 0) slot = 0;
            }
        }
    }
    fclose(f);
    return slot;
}

static int ini_guardar_slot(int slot) {
    char path[1300], tmp[1310], line[256];
    ini_path(path, sizeof path);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE *in = fopen(path, "r");
    FILE *out = fopen(tmp, "w");
    if (!out) {
        platform_log("slot: no se pudo escribir %s", tmp);
        if (in) fclose(in);
        return -1;
    }
    int written = 0;
    if (in) {
        while (fgets(line, sizeof line, in)) {
            char *p = line;
            while (isspace((unsigned char)*p)) p++;
            if (strncmp(p, "save_slot", 9) == 0) {
                if (!written) {
                    fprintf(out, "save_slot = %d\n", slot);
                    written = 1;
                }
            } else {
                fputs(line, out);
            }
        }
        fclose(in);
    }
    if (!written)
        fprintf(out, "save_slot = %d\n", slot);
    if (fclose(out) != 0) {
        remove(tmp);
        return -1;
    }
    remove(path);
    if (rename(tmp, path) != 0) {
        remove(tmp);
        platform_log("slot: no se pudo reemplazar %s", path);
        return -1;
    }
    return 0;
}

void platform_slots_init(void) {
    g_slot = ini_leer_slot();
    platform_log("slot activo: %d", g_slot);
}

int platform_slots_current(void) {
    return g_slot;
}

void platform_slots_select_next(void) {
    g_slot = (g_slot + 1) % PLATFORM_SLOT_COUNT;
    (void)ini_guardar_slot(g_slot);
    char msg[32];
    snprintf(msg, sizeof msg, "RANURA %d", g_slot);
    platform_osd_show(msg);
    platform_log("slot activo: %d", g_slot);
}

void platform_slots_request_save(void) {
    if (!platform_in_loop()) {
        platform_osd_show("SOLO EN PARTIDA");
        return;
    }
    g_pending_save = 1;
    g_pending_load = 0;
}

void platform_slots_request_load(void) {
    if (!platform_in_loop()) {
        platform_osd_show("SOLO EN PARTIDA");
        return;
    }
    g_pending_load = 1;
    g_pending_save = 0;
}

void platform_slots_apply(uint8_t *img) {
    int do_load = g_pending_load;
    int do_save = g_pending_save;
    g_pending_load = 0;
    g_pending_save = 0;
    if (!img || (!do_load && !do_save))
        return;

    char msg[32];
    if (do_load) {
        sound_stop(img);
        if (platform_slots_load(img, g_slot) == 0) {
            cursor_draw(img);
            snprintf(msg, sizeof msg, "PARTIDA %d CARGADA", g_slot);
        } else {
            snprintf(msg, sizeof msg, "ERROR AL CARGAR");
        }
        platform_osd_show(msg);
        return;
    }

    cursor_restore(img);
    if (platform_slots_save(img, g_slot) == 0)
        snprintf(msg, sizeof msg, "PARTIDA %d GUARDADA", g_slot);
    else
        snprintf(msg, sizeof msg, "ERROR AL GUARDAR");
    platform_osd_show(msg);
}

int platform_slots_save(const uint8_t *img, int slot) {
    slot = normaliza_slot(slot);
    if (!img || slot < 0)
        return -1;

    char path[1300], tmp[1310];
    slot_path(slot, path, sizeof path);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);

    uint8_t hdr[SLOT_HDR_SIZE];
    memset(hdr, 0, sizeof hdr);
    memcpy(hdr, SLOT_MAGIC, 8);
    put_be32(hdr + 8, SLOT_VERSION);
    put_be32(hdr + 12, (uint32_t)slot);
    const uint16_t *pal = platform_palette();
    for (int i = 0; i < 16; i++)
        put_be16(hdr + 16 + i * 2, pal[i]);
    put_be32(hdr + 48, platform_visible_base());
    put_be32(hdr + 52, platform_frame_count());
    put_be32(hdr + 56, PLATFORM_IMAGE_SIZE);

    FILE *f = fopen(tmp, "wb");
    if (!f) {
        platform_log("slot %d: no se pudo crear %s", slot, tmp);
        return -1;
    }
    int ok = fwrite(hdr, 1, sizeof hdr, f) == sizeof hdr &&
             fwrite(img, 1, PLATFORM_IMAGE_SIZE, f) == PLATFORM_IMAGE_SIZE;
    if (fclose(f) != 0)
        ok = 0;
    if (!ok) {
        remove(tmp);
        platform_log("slot %d: fallo al escribir %s", slot, tmp);
        return -1;
    }
    remove(path);
    if (rename(tmp, path) != 0) {
        remove(tmp);
        platform_log("slot %d: no se pudo reemplazar %s", slot, path);
        return -1;
    }
    platform_log("slot %d: guardado (%u B)", slot, (unsigned)SLOT_ROW_SIZE);
    return 0;
}

int platform_slots_load(uint8_t *img, int slot) {
    slot = normaliza_slot(slot);
    if (!img || slot < 0)
        return -1;

    char path[1300];
    slot_path(slot, path, sizeof path);
    FILE *f = fopen(path, "rb");
    if (!f) {
        platform_log("slot %d: no existe %s", slot, path);
        return -1;
    }

    uint8_t hdr[SLOT_HDR_SIZE];
    int ok = fread(hdr, 1, sizeof hdr, f) == sizeof hdr;
    if (ok) {
        ok = memcmp(hdr, SLOT_MAGIC, 8) == 0 &&
             get_be32(hdr + 8) == SLOT_VERSION &&
             get_be32(hdr + 12) == (uint32_t)slot &&
             get_be32(hdr + 56) == PLATFORM_IMAGE_SIZE;
    }
    if (ok && fseek(f, 0, SEEK_END) == 0) {
        long sz = ftell(f);
        ok = sz == (long)SLOT_ROW_SIZE;
    } else {
        ok = 0;
    }
    if (ok && fseek(f, (long)SLOT_HDR_SIZE, SEEK_SET) != 0)
        ok = 0;

    uint8_t *tmp = ok ? malloc(PLATFORM_IMAGE_SIZE) : NULL;
    if (ok && !tmp)
        ok = 0;
    if (tmp && fread(tmp, 1, PLATFORM_IMAGE_SIZE, f) != PLATFORM_IMAGE_SIZE)
        ok = 0;
    fclose(f);
    if (!ok) {
        free(tmp);
        platform_log("slot %d: fichero invalido %s", slot, path);
        return -1;
    }

    uint16_t pal[16];
    for (int i = 0; i < 16; i++)
        pal[i] = get_be16(hdr + 16 + i * 2);
    memcpy(img, tmp, PLATFORM_IMAGE_SIZE);
    platform_video_restore(get_be32(hdr + 48), pal);
    free(tmp);
    platform_log("slot %d: cargado", slot);
    return 0;
}
