/* Contenedor KULTDTA1: cabecera + índice + blobs (little-endian). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "container.h"

static const uint8_t MAGIC[8] = {'K','U','L','T','D','T','A','1'};
#define HEADER 20u
#define ENTRY  72u

static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void wr32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}
static int name_eq(const char *a, const char *b) { return strcasecmp(a, b) == 0; }

int kult_container_load(const char *path, uint32_t expected_type, kult_container *out) {
    memset(out, 0, sizeof *out);
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long n = ftell(f);
    if (n < (long)HEADER || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return -1; }
    uint8_t *buf = malloc((size_t)n);
    if (!buf) { fclose(f); return -1; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(buf); return -1; }
    fclose(f);
    if (memcmp(buf, MAGIC, 8) != 0 || rd32(buf + 8) != 1u ||
        rd32(buf + 12) != expected_type) { free(buf); return -1; }
    uint32_t count = rd32(buf + 16);
    if (count > 4096u || HEADER + (size_t)count * ENTRY > (size_t)n) {
        free(buf); return -1;
    }
    kult_container_entry *e = calloc(count ? count : 1, sizeof *e);
    if (!e) { free(buf); return -1; }
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = buf + HEADER + (size_t)i * ENTRY;
        memcpy(e[i].name, p, 64);
        e[i].name[63] = '\0';
        e[i].offset = rd32(p + 64);
        e[i].size = rd32(p + 68);
        if (e[i].offset < HEADER || (size_t)e[i].offset + e[i].size > (size_t)n) {
            free(e); free(buf); return -1;
        }
    }
    out->data = buf; out->len = (size_t)n; out->type = expected_type;
    out->count = count; out->entries = e;
    return 0;
}

void kult_container_free(kult_container *c) {
    if (!c) return;
    free(c->entries);
    free(c->data);
    memset(c, 0, sizeof *c);
}

const uint8_t *kult_container_get(const kult_container *c, const char *name, uint32_t *size) {
    if (!c || !c->data || !c->entries) return NULL;
    for (uint32_t i = 0; i < c->count; i++) {
        if (name_eq(c->entries[i].name, name)) {
            if (size) *size = c->entries[i].size;
            return c->data + c->entries[i].offset;
        }
    }
    return NULL;
}

int kult_container_write(const char *path, uint32_t type,
                         const char *const *names, const uint8_t *const *blobs,
                         const uint32_t *sizes, uint32_t count) {
    if (count > 4096u) return -1;
    char tmp[2048];
    if (strlen(path) + 5 >= sizeof tmp) return -1;
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) return -1;
    uint8_t h[HEADER] = {0};
    memcpy(h, MAGIC, 8);
    wr32(h + 8, 1); wr32(h + 12, type); wr32(h + 16, count);
    if (fwrite(h, 1, sizeof h, f) != sizeof h) { fclose(f); remove(tmp); return -1; }
    uint32_t off = HEADER + count * ENTRY;
    uint8_t *idx = calloc(count ? count : 1, ENTRY);
    if (!idx) { fclose(f); remove(tmp); return -1; }
    for (uint32_t i = 0; i < count; i++) {
        uint8_t *p = idx + (size_t)i * ENTRY;
        size_t nl = strlen(names[i]);
        if (nl > 63) nl = 63;
        memcpy(p, names[i], nl);
        wr32(p + 64, off); wr32(p + 68, sizes[i]);
        off += sizes[i];
    }
    if (fwrite(idx, 1, (size_t)count * ENTRY, f) != (size_t)count * ENTRY) {
        free(idx); fclose(f); remove(tmp); return -1;
    }
    free(idx);
    for (uint32_t i = 0; i < count; i++) {
        if (sizes[i] && fwrite(blobs[i], 1, sizes[i], f) != sizes[i]) {
            fclose(f); remove(tmp); return -1;
        }
    }
    if (fflush(f) != 0 || fclose(f) != 0) { remove(tmp); return -1; }
    remove(path);
    if (rename(tmp, path) != 0) { remove(tmp); return -1; }
    return 0;
}
