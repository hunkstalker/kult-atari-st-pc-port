/* platform/fs.c — servicio de ficheros portable (GEMDOS sobre el SO real).
 *
 * Orden de lectura: saves/ > language.data > kult.data > data/ sueltos.
 * Escrituras: siempre saves/. En Windows/Linux la base es la carpeta del
 * ejecutable; en macOS es ~/Library/Application Support/Kult. KULT_BASE_DIR
 * permite sobrescribirla.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "os.h"
#include "platform.h"
#include "platform_internal.h"
#include "container.h"

#define FS_MAX_HANDLES 32
#define FS_FIRST_HANDLE 6

typedef struct {
    FILE *f;
    const uint8_t *mem;
    size_t size, pos;
} file_handle;

static file_handle g_files[FS_MAX_HANDLES];
static char g_base_dir[1024];
static char g_data_dir[1200];
static char g_saves_dir[1200];
static int  g_storage_ok;
static kult_container g_lang, g_game;

static int ensure_dir(const char *path) {
#ifdef _WIN32
    if (_mkdir(path) == 0) return 0;
    struct _stat st;
    if (_stat(path, &st) == 0) return 0;
#else
    if (mkdir(path, 0755) == 0) return 0;
    struct stat st;
    if (stat(path, &st) == 0) return 0;
#endif
    return -1;
}

static int dir_writable(const char *path) {
    char probe[1300];
    snprintf(probe, sizeof probe, "%s/.write-test", path);
    FILE *f = fopen(probe, "wb");
    if (!f) return 0;
    fclose(f);
    remove(probe);
    return 1;
}

static const char *base_name(const char *name) {
    const char *b = name;
    for (const char *q = name; *q; q++)
        if (*q == ':' || *q == '/' || *q == '\\') b = q + 1;
    return b;
}

static int path_in(const char *dir, const char *name, char *out, size_t out_size) {
    if (!dir[0]) return -1;
    snprintf(out, out_size, "%s/%s", dir, base_name(name));
    FILE *f = fopen(out, "rb");
    if (!f) return -1;
    fclose(f);
    return 0;
}

#ifdef __APPLE__
static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }
    uint8_t buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return -1; }
    }
    int ok = !ferror(in);
    fclose(in);
    if (fclose(out) != 0) ok = 0;
    return ok ? 0 : -1;
}

/* El .app es de solo lectura. Copia el language.data empaquetado a la carpeta
 * escribible de Application Support antes de cargar los contenedores. */
static void seed_bundled_language(void) {
    char exe[1024], src[1200], dst[1300];
    if (platform_exe_dir(exe, sizeof exe) != 0) return;
    snprintf(src, sizeof src, "%s/../Resources/data/language.data", exe);
    snprintf(dst, sizeof dst, "%s/language.data", g_data_dir);
    FILE *f = fopen(src, "rb");
    if (!f) return;
    fclose(f);
    if (copy_file(src, dst) == 0)
        platform_log("language.data: copiado desde el bundle");
    else
        platform_log("ERROR: no se pudo copiar language.data del bundle");
}
#endif

static void load_containers(void) {
    kult_container_free(&g_lang);
    kult_container_free(&g_game);
    char path[1300];
    snprintf(path, sizeof path, "%s/language.data", g_data_dir);
    if (kult_container_load(path, KULT_CONTAINER_LANG, &g_lang) != 0) {
        FILE *f = fopen(path, "rb");
        if (f) { fclose(f); platform_log("ERROR: language.data inválido"); }
    } else {
        platform_log("language.data: %d entradas", (int)g_lang.count);
    }
    snprintf(path, sizeof path, "%s/kult.data", g_data_dir);
    if (kult_container_load(path, KULT_CONTAINER_GAME, &g_game) != 0) {
        FILE *f = fopen(path, "rb");
        if (f) { fclose(f); platform_log("ERROR: kult.data inválido"); }
    } else {
        platform_log("kult.data: %d entradas", (int)g_game.count);
    }
}

void platform_fs_init(void) {
    if (platform_base_dir(g_base_dir, sizeof g_base_dir) != 0) {
        g_storage_ok = 0;
        platform_log("ERROR: no se pudo resolver la carpeta del ejecutable");
        return;
    }
    snprintf(g_data_dir, sizeof g_data_dir, "%s/data", g_base_dir);
    snprintf(g_saves_dir, sizeof g_saves_dir, "%s/saves", g_base_dir);

    ensure_dir(g_base_dir);
    int ok_data = ensure_dir(g_data_dir) == 0;
    int ok_saves = ensure_dir(g_saves_dir) == 0;
    int wr_data = ok_data && dir_writable(g_data_dir);
    int wr_saves = ok_saves && dir_writable(g_saves_dir);
    g_storage_ok = wr_data && wr_saves;

    platform_log("base: %s", g_base_dir);
    platform_log("datos: %s", g_data_dir);
    platform_log("saves: %s", g_saves_dir);
    if (!g_storage_ok)
        platform_log("ERROR: la carpeta del juego no es escribible: %s", g_base_dir);
#ifdef __APPLE__
    if (ok_data)
        seed_bundled_language();
#endif
    load_containers();
}

void platform_fs_reload_containers(void) { load_containers(); }
int platform_storage_ready(void) { return g_storage_ok; }
const char *platform_storage_path(void) { return g_base_dir; }
const char *platform_data_dir(void) { return g_data_dir; }
const char *platform_saves_dir(void) { return g_saves_dir; }

static const uint8_t *mem_get(const char *name, uint32_t *size) {
    const uint8_t *p = kult_container_get(&g_lang, name, size);
    if (!p) p = kult_container_get(&g_game, name, size);
    return p;
}

int platform_file_exists(const char *name) {
    char path[1300];
    if (path_in(g_saves_dir, name, path, sizeof path) == 0) return 1;
    uint32_t sz = 0;
    if (mem_get(name, &sz)) return 1;
    return path_in(g_data_dir, name, path, sizeof path) == 0;
}

int platform_file_read(const char *name, uint8_t **out, size_t *out_size) {
    *out = NULL; *out_size = 0;
    char path[1300];
    const char *base = base_name(name);
    FILE *f = NULL;
    if (path_in(g_saves_dir, name, path, sizeof path) == 0) f = fopen(path, "rb");
    if (!f) {
        uint32_t sz = 0;
        const uint8_t *p = mem_get(name, &sz);
        if (p) {
            uint8_t *copy = malloc(sz ? sz : 1);
            if (!copy) return -1;
            memcpy(copy, p, sz); *out = copy; *out_size = sz;
            return 0;
        }
    }
    if (!f && path_in(g_data_dir, name, path, sizeof path) == 0) f = fopen(path, "rb");
    (void)base;
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return -1; }
    uint8_t *b = malloc((size_t)n ? (size_t)n : 1);
    if (!b) { fclose(f); return -1; }
    if (n && fread(b, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(b); return -1; }
    fclose(f); *out = b; *out_size = (size_t)n;
    return 0;
}

/* Resuelve un asset suelto solo en data/. */
int platform_data_resolve(const char *name, char *out, size_t out_size) {
    return path_in(g_data_dir, name, out, out_size);
}

static long file_size_any(const char *name) {
    char path[1300];
    if (path_in(g_saves_dir, name, path, sizeof path) == 0) {
        FILE *f = fopen(path, "rb");
        if (f) { fseek(f, 0, SEEK_END); long n = ftell(f); fclose(f); return n; }
    }
    uint32_t sz = 0;
    if (mem_get(name, &sz)) return (long)sz;
    if (path_in(g_data_dir, name, path, sizeof path) == 0) {
        FILE *f = fopen(path, "rb");
        if (f) { fseek(f, 0, SEEK_END); long n = ftell(f); fclose(f); return n; }
    }
    return -1;
}

static int alloc_handle(FILE *f, const uint8_t *mem, size_t size) {
    for (int i = 0; i < FS_MAX_HANDLES; i++) {
        if (!g_files[i].f && !g_files[i].mem) {
            g_files[i].f = f; g_files[i].mem = mem; g_files[i].size = size; g_files[i].pos = 0;
            return FS_FIRST_HANDLE + i;
        }
    }
    return -1;
}

static file_handle *handle_file(uint16_t handle) {
    int i = (int)handle - FS_FIRST_HANDLE;
    if (i < 0 || i >= FS_MAX_HANDLES) return NULL;
    return &g_files[i];
}

int32_t pc_os_fopen(uint8_t *mem, uint32_t name_ptr) {
    const char *name = (const char *)(mem + name_ptr);
    char path[1300];
    FILE *f = NULL;
    if (path_in(g_saves_dir, name, path, sizeof path) == 0) f = fopen(path, "rb");
    if (!f) {
        uint32_t sz = 0;
        const uint8_t *p = mem_get(name, &sz);
        if (p) {
            int h = alloc_handle(NULL, p, sz);
            return h < 0 ? -1 : h;
        }
    }
    if (!f && path_in(g_data_dir, name, path, sizeof path) == 0) f = fopen(path, "rb");
    if (!f) return -1;
    int h = alloc_handle(f, NULL, 0);
    if (h < 0) { fclose(f); return -1; }
    return h;
}

int32_t pc_os_fcreate(uint8_t *mem, uint32_t name_ptr) {
    if (!g_storage_ok || !g_saves_dir[0]) return -1;
    const char *name = (const char *)(mem + name_ptr);
    const char *base = base_name(name);
    size_t n = strlen(g_saves_dir), m = strlen(base);
    char path[1300];
    if (n + 1 + m + 1 > sizeof path) return -1;
    memcpy(path, g_saves_dir, n); path[n] = '/';
    memcpy(path + n + 1, base, m + 1);
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    int h = alloc_handle(f, NULL, 0);
    if (h < 0) { fclose(f); return -1; }
    return h;
}

int32_t pc_os_fread(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf) {
    file_handle *h = handle_file(handle);
    if (!h || (!h->f && !h->mem)) return -1;
    size_t n;
    if (h->mem) {
        size_t left = h->size - h->pos;
        n = count < left ? count : left;
        memcpy(mem + buf, h->mem + h->pos, n);
        h->pos += n;
    } else {
        n = fread(mem + buf, 1, count, h->f);
    }
    platform_disk_delay((uint32_t)n);
    return (int32_t)n;
}

int32_t pc_os_fwrite(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf) {
    file_handle *h = handle_file(handle);
    if (!h || !h->f) return -1;
    return (int32_t)fwrite(mem + buf, 1, count, h->f);
}

int32_t pc_os_fclose(uint8_t *mem, uint16_t handle) {
    (void)mem;
    file_handle *h = handle_file(handle);
    if (!h || (!h->f && !h->mem)) return -1;
    if (h->f) fclose(h->f);
    memset(h, 0, sizeof *h);
    return 0;
}

int32_t pc_os_fsfirst(uint8_t *mem, uint32_t name_ptr) {
    const char *name = (const char *)(mem + name_ptr);
    const char *base = base_name(name);
    long size = file_size_any(name);
    if (size < 0) return -1;
    memset(mem + g_os_dta + 0x1e, 0, 14);
    size_t n = strlen(base);
    if (n > 13) n = 13;
    memcpy(mem + g_os_dta + 0x1e, base, n);
    wr32(mem + g_os_dta + 0x1a, (uint32_t)size);
    return 0;
}
