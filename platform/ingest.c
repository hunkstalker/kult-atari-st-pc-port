/* platform/ingest.c — detección de STX por SHA-256, extracción Pasti/FAT12 y
 * generación atómica de data/kult.data.
 *
 * La parte pura (STX->raw, FAT12, validación y discos conocidos) está en
 * diskimage.c para poder probarla con imágenes sintéticas (T-095).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#include "platform.h"
#include "platform_internal.h"
#include "container.h"
#include "diskimage.h"
#include "sha256.h"

#define RAW_SIZE (2u * 80u * 9u * 512u)

static void sha_file(const char *path, uint8_t out[32], int *ok) {
    *ok = 0;
    FILE *f = fopen(path, "rb");
    if (!f) return;
    kult_sha256 s; uint8_t buf[8192];
    kult_sha256_init(&s);
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) kult_sha256_update(&s, buf, n);
    if (ferror(f)) { fclose(f); return; }
    fclose(f);
    kult_sha256_final(&s, out);
    *ok = 1;
}

/* Devuelve candidatos .STX en dir (no recursivo). */
static int scan_stx(const char *dir, char **out, int max) {
    int n = 0;
#ifdef _WIN32
    char pat[1200];
    snprintf(pat, sizeof pat, "%s/*", dir);
    WIN32_FIND_DATAA d;
    HANDLE h = FindFirstFileA(pat, &d);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (kult_stx_name(d.cFileName)) {
            char *p = malloc(1024);
            if (p) { snprintf(p, 1024, "%s/%s", dir, d.cFileName); out[n++] = p; }
        }
    } while (n < max && FindNextFileA(h, &d));
    FindClose(h);
#else
    DIR *d = opendir(dir);
    if (!d) return 0;
    struct dirent *e;
    while (n < max && (e = readdir(d)) != NULL) {
        if (!kult_stx_name(e->d_name)) continue;
        char *p = malloc(1024);
        if (p) { snprintf(p, 1024, "%s/%s", dir, e->d_name); out[n++] = p; }
    }
    closedir(d);
#endif
    return n;
}

static int find_disks(const char *base, const char *data_dir, char *pa, char *pb) {
    char *cand[16]; int n = 0;
    n += scan_stx(data_dir, cand + n, 16 - n);
    n += scan_stx(base, cand + n, 16 - n);
    int found = 0;
    for (int i = 0; i < n; i++) {
        uint8_t h[32]; int ok = 0, role = -1;
        sha_file(cand[i], h, &ok);
        if (ok) {
            if (kult_ingest_known_disk(h, &role)) {
                if (role == 0 && !(found & 1)) { snprintf(pa, 1024, "%s", cand[i]); found |= 1; }
                else if (role == 1 && !(found & 2)) { snprintf(pb, 1024, "%s", cand[i]); found |= 2; }
                else platform_log("ERROR: disco duplicado: %s", cand[i]);
            } else {
                platform_log("ERROR: STX desconocido: %s", cand[i]);
            }
        }
        free(cand[i]);
    }
    return (found == 3) ? 0 : -1;
}

int platform_ingest_ensure(void) {
    char base[1024], data_dir[1200], game[1300];
    if (platform_base_dir(base, sizeof base) != 0) return -1;
    snprintf(data_dir, sizeof data_dir, "%s/data", base);
    snprintf(game, sizeof game, "%s/kult.data", data_dir);

    FILE *gf = fopen(game, "rb");
    if (gf) {
        fclose(gf);
        kult_container c;
        if (kult_container_load(game, KULT_CONTAINER_GAME, &c) == 0) {
            kult_container_free(&c);
            return 0;
        }
        platform_log("kult.data inválido; se regenera desde los STX");
    }

    char pa[1024] = {0}, pb[1024] = {0};
    if (find_disks(base, data_dir, pa, pb) != 0) {
        platform_log("ERROR: faltan los dos discos STX conocidos");
        return -1;
    }
    platform_log("STX A: %s", pa);
    platform_log("STX B: %s", pb);

    uint8_t *rawa = calloc(1, RAW_SIZE);
    uint8_t *rawb = calloc(1, RAW_SIZE);
    if (!rawa || !rawb) { free(rawa); free(rawb); return -1; }
    if (kult_stx_to_raw(pa, rawa, RAW_SIZE) != 0 ||
        kult_stx_to_raw(pb, rawb, RAW_SIZE) != 0) {
        free(rawa); free(rawb); return -1;
    }

    kult_filelist fl = {0};
    if (kult_fat12_extract(rawa, RAW_SIZE, &fl) != 0 ||
        kult_fat12_extract(rawb, RAW_SIZE, &fl) != 0 ||
        kult_ingest_validate(&fl) != 0) {
        kult_filelist_free(&fl);
        free(rawa); free(rawb); return -1;
    }

    const char *names[KULT_MAX_FILES];
    const uint8_t *blobs[KULT_MAX_FILES];
    uint32_t sizes[KULT_MAX_FILES];
    for (int i = 0; i < fl.n; i++) {
        names[i] = fl.v[i].name; blobs[i] = fl.v[i].data; sizes[i] = fl.v[i].size;
    }
    int rc = kult_container_write(game, KULT_CONTAINER_GAME, names, blobs, sizes,
                                  (uint32_t)fl.n);
    platform_log(rc == 0 ? "kult.data generado (%d ficheros)" : "ERROR: no se pudo generar kult.data", fl.n);
    kult_filelist_free(&fl);
    free(rawa); free(rawb);
    return rc;
}
