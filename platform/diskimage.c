/* platform/diskimage.c — Pasti/STX -> raw y extracción FAT12.
 *
 * Parte pura de la ingesta (sin rutas del ejecutable), con API propia para las
 * pruebas sintéticas de T-095.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "platform.h"
#include "diskimage.h"
#include "sha256.h"

static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint8_t *read_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    uint8_t *b = malloc((size_t)n ? (size_t)n : 1);
    if (!b) { fclose(f); return NULL; }
    if (n && fread(b, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(b); return NULL; }
    fclose(f);
    *len = (size_t)n;
    return b;
}

int kult_stx_name(const char *name) {
    size_t n = strlen(name);
    return n > 4 && (name[n-4] == '.') &&
           (name[n-3] == 's' || name[n-3] == 'S') &&
           (name[n-2] == 't' || name[n-2] == 'T') &&
           (name[n-1] == 'x' || name[n-1] == 'X');
}

/* --- STX Pasti -> raw sector image --- */
int kult_stx_to_raw(const char *path, uint8_t *raw, size_t raw_size) {
    size_t len = 0;
    uint8_t *d = read_file(path, &len);
    if (!d || len < 16 || memcmp(d, "RSY", 3) != 0) { free(d); return -1; }
    size_t pos = 16;
    while (pos + 16 <= len) {
        uint32_t length = rd32(d + pos);
        if (length < 16 || pos + length > len) break;
        uint32_t fuzzy = rd32(d + pos + 4);
        uint16_t nsec = rd16(d + pos + 8);
        uint16_t flags = rd16(d + pos + 10);
        uint8_t tracknum = d[pos + 14];
        const uint8_t *blk = d + pos;
        if (!(flags & 1)) {
            for (uint16_t i = 0; i < nsec; i++) {
                size_t off = 16u + (size_t)i * 512u;
                if (off + 512u > length) break;
                uint32_t lba = (((tracknum >> 7) & 1u) * 80u + (tracknum & 0x7fu)) * 9u + i;
                if ((size_t)lba * 512u + 512u <= raw_size)
                    memcpy(raw + (size_t)lba * 512u, blk + off, 512u);
            }
        } else {
            size_t p = 16;
            struct { uint32_t so; uint8_t stn, sid, snum, scode; } hs[64];
            uint16_t nh = 0;
            for (uint16_t i = 0; i < nsec && i < 64; i++) {
                if (p + 16 > length) break;
                hs[nh].so = rd32(blk + p);
                hs[nh].stn = blk[p + 8];
                hs[nh].sid = blk[p + 9];
                hs[nh].snum = blk[p + 10];
                hs[nh].scode = blk[p + 11];
                nh++; p += 16;
            }
            size_t da = p + fuzzy;
            for (uint16_t i = 0; i < nh; i++) {
                uint32_t size = 128u << (hs[i].scode & 3u);
                if (size > 512u) size = 512u;
                size_t off = da + hs[i].so;
                if (off + size > length) continue;
                uint32_t lba = (((hs[i].sid & 1u) * 80u + (hs[i].stn & 0x7fu)) * 9u) +
                               (hs[i].snum ? hs[i].snum - 1u : 0u);
                if ((size_t)lba * 512u + size <= raw_size)
                    memcpy(raw + (size_t)lba * 512u, blk + off, size);
            }
        }
        pos += length;
    }
    free(d);
    return 0;
}

/* --- FAT12 --- */
typedef struct {
    const uint8_t *img; size_t len;
    uint16_t bps, reserved, root_entries, spf;
    uint8_t spc, fats;
    uint32_t fat_start, root_start, root_bytes, data_start, cluster_bytes;
} fat12;

static int fat_init(fat12 *f, const uint8_t *img, size_t len) {
    if (len < 512) return -1;
    f->img = img; f->len = len;
    f->bps = rd16(img + 0x0b);
    f->spc = img[0x0d];
    f->reserved = rd16(img + 0x0e);
    f->fats = img[0x10];
    f->root_entries = rd16(img + 0x11);
    f->spf = rd16(img + 0x16);
    if (f->bps != 512 || !f->spc || !f->root_entries || !f->spf) return -1;
    f->fat_start = (uint32_t)f->reserved * f->bps;
    f->root_start = (uint32_t)(f->reserved + f->fats * f->spf) * f->bps;
    f->root_bytes = (uint32_t)f->root_entries * 32u;
    f->data_start = f->root_start + ((f->root_bytes + f->bps - 1) / f->bps) * f->bps;
    f->cluster_bytes = (uint32_t)f->spc * f->bps;
    return 0;
}

static uint16_t fat12_entry(const fat12 *f, uint16_t n) {
    size_t off = f->fat_start + (size_t)(n * 3u) / 2u;
    if (off + 1 >= f->len) return 0xfff;
    uint16_t v = rd16(f->img + off);
    return (n & 1u) ? (uint16_t)((v >> 4) & 0xfff) : (uint16_t)(v & 0xfff);
}

static uint8_t *fat_read(const fat12 *f, uint16_t start, uint32_t size,
                         uint32_t *out_len) {
    uint8_t *out = malloc(size ? size : 1);
    if (!out) return NULL;
    uint32_t got = 0;
    uint16_t c = start;
    for (int guard = 0; guard < 0x1000 && c >= 2 && c < 0xff8 && got < size; guard++) {
        size_t off = f->data_start + (size_t)(c - 2u) * f->cluster_bytes;
        if (off >= f->len) break;
        uint32_t take = f->cluster_bytes;
        if (take > size - got) take = size - got;
        if (off + take > f->len) take = (uint32_t)(f->len - off);
        memcpy(out + got, f->img + off, take);
        got += take;
        c = fat12_entry(f, c);
    }
    *out_len = got;
    return out;
}

static void trim_name(char *s, size_t n) {
    while (n && s[n-1] == ' ') s[--n] = '\0';
}

static int add_memfile(kult_filelist *fl, const char *name, uint8_t *data,
                       uint32_t size) {
    if (fl->n >= KULT_MAX_FILES) { free(data); return -1; }
    for (int i = 0; i < fl->n; i++)
        if (strcasecmp(fl->v[i].name, name) == 0) { free(data); return 0; }
    snprintf(fl->v[fl->n].name, sizeof fl->v[fl->n].name, "%s", name);
    fl->v[fl->n].data = data;
    fl->v[fl->n].size = size;
    fl->n++;
    return 0;
}

int kult_fat12_extract(const uint8_t *raw, size_t raw_len, kult_filelist *out) {
    fat12 f;
    if (fat_init(&f, raw, raw_len) != 0) return -1;
    for (uint16_t i = 0; i < f.root_entries; i++) {
        size_t off = f.root_start + (size_t)i * 32u;
        if (off + 32u > f.len) break;
        const uint8_t *e = f.img + off;
        if (e[0] == 0x00) break;
        if (e[0] == 0xe5 || (e[11] & 0x10) || e[11] == 0x0f) continue;
        char name[9] = {0}, ext[4] = {0}, full[16];
        memcpy(name, e, 8); memcpy(ext, e + 8, 3);
        trim_name(name, 8); trim_name(ext, 3);
        if (ext[0]) snprintf(full, sizeof full, "%s.%s", name, ext);
        else snprintf(full, sizeof full, "%s", name);
        uint16_t start = rd16(e + 26);
        uint32_t size = rd32(e + 28);
        if (!name[0] || start < 2) continue;
        uint32_t got = 0;
        uint8_t *data = fat_read(&f, start, size, &got);
        if (!data) return -1;
        if (add_memfile(out, full, data, got) != 0) return -1;
    }
    return 0;
}

const kult_memfile *kult_filelist_find(const kult_filelist *fl, const char *name) {
    for (int i = 0; i < fl->n; i++)
        if (strcasecmp(fl->v[i].name, name) == 0) return &fl->v[i];
    return NULL;
}

void kult_filelist_free(kult_filelist *fl) {
    if (!fl) return;
    for (int i = 0; i < fl->n; i++) free(fl->v[i].data);
    memset(fl, 0, sizeof *fl);
}

int kult_ingest_validate(const kult_filelist *fl) {
    static const char *req[] = {
        "KULT.PRG","PRES.PI1","DRAP.PI1","FOND.PI1","BRUIT.BIN","PUZZL.BIN","CLEAR.BIN",
        "ECHE0.BIN","ECHE1.BIN","ECHE2.BIN","ECHE3.BIN","ECHE4.BIN","ECHE5.BIN","ECHE6.BIN","ECHE7.BIN",
        "DESCD.BIN","DESCE.BIN","DESCF.BIN","DIALD.BIN","DIALE.BIN","DIALF.BIN",
        "MOTSD.BIN","MOTSE.BIN","MOTSF.BIN","VERBD.BIN","VERBE.BIN","VERBF.BIN"
    };
    for (size_t i = 0; i < sizeof req / sizeof req[0]; i++)
        if (!kult_filelist_find(fl, req[i])) {
            platform_log("ERROR: falta %s en los discos", req[i]);
            return -1;
        }
    const kult_memfile *prg = kult_filelist_find(fl, "KULT.PRG");
    if (!prg) return -1;
    kult_sha256 s;
    uint8_t h[32];
    kult_sha256_init(&s);
    kult_sha256_update(&s, prg->data, prg->size);
    kult_sha256_final(&s, h);
    static const uint8_t want[32] = {
        0x2e,0x85,0xf2,0x23,0x16,0x74,0x39,0x42,0x5a,0xb2,0x09,0x6b,0xd2,0x17,0x16,0xde,
        0x13,0x5f,0x9b,0x47,0x2b,0xde,0x90,0xf4,0xd4,0x0e,0x8b,0x0e,0x03,0x13,0x2f,0xe6
    };
    if (memcmp(h, want, 32) != 0) {
        platform_log("ERROR: hash de KULT.PRG no reconocido");
        return -1;
    }
    return 0;
}

int kult_ingest_known_disk(const uint8_t h[32], int *role) {
    static const uint8_t a[32] = {
        0xd3,0x4e,0x1e,0xfa,0x34,0xc2,0xf4,0xd3,0x07,0x3c,0xa6,0x12,0x21,0xd5,0x2f,0x4e,
        0xed,0xd1,0x90,0xfc,0x2d,0x2a,0x07,0xf5,0xcb,0xde,0xf5,0x25,0xea,0xb0,0x17,0xc1
    };
    static const uint8_t b[32] = {
        0xf4,0xef,0x13,0xda,0x50,0x39,0x82,0x1b,0xae,0xdf,0x83,0x78,0xb4,0x73,0x58,0xcf,
        0x35,0x4d,0x47,0x1a,0xfc,0x7f,0x64,0x7b,0x35,0xde,0xd1,0x98,0x8b,0x8a,0x06,0x4e
    };
    if (memcmp(h, a, 32) == 0) { *role = 0; return 1; }
    if (memcmp(h, b, 32) == 0) { *role = 1; return 1; }
    return 0;
}
