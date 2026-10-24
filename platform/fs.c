/* platform/fs.c — servicio de ficheros del port (GEMDOS sobre el SO real).
 *
 * Release portable: la base es siempre la carpeta del ejecutable (o la raíz de
 * test). Los assets del juego se leen de `<base>/data/`; las escrituras del
 * juego (CLEAR.BIN, SCENA.BIN...) van a `<base>/saves/`. No hay variables de
 * entorno ni fallback a appdata/tmp.
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

#define FS_MAX_HANDLES 32
#define FS_FIRST_HANDLE 6          /* GEMDOS reserva 0..5 */

static FILE *g_files[FS_MAX_HANDLES];
static char g_base_dir[1024];
static char g_data_dir[1200];
static char g_saves_dir[1200];
static int  g_storage_ok;

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
    char probe[1200];
    snprintf(probe, sizeof probe, "%s/.write-test", path);
    FILE *f = fopen(probe, "wb");
    if (!f) return 0;
    fclose(f);
    remove(probe);
    return 1;
}

void platform_fs_init(void) {
    if (platform_base_dir(g_base_dir, sizeof g_base_dir) != 0) {
        g_storage_ok = 0;
        platform_log("ERROR: no se pudo resolver la carpeta del ejecutable");
        return;
    }
    snprintf(g_data_dir, sizeof g_data_dir, "%s/data", g_base_dir);
    snprintf(g_saves_dir, sizeof g_saves_dir, "%s/saves", g_base_dir);

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
}

int platform_storage_ready(void) { return g_storage_ok; }
const char *platform_storage_path(void) { return g_base_dir; }

/* Basura de unidad/ruta: de `A:CLEAR.BIN` deja `CLEAR.BIN`. */
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

/* Lectura: primero saves/, después data/. Así el juego lee sus escrituras. */
static int resolve_read(const char *name, char *out, size_t out_size) {
    if (path_in(g_saves_dir, name, out, out_size) == 0) return 0;
    return path_in(g_data_dir, name, out, out_size);
}

static FILE *open_read(const char *name, char *path, size_t path_size) {
    if (resolve_read(name, path, path_size) != 0) return NULL;
    return fopen(path, "rb");
}

static FILE *open_write(const char *name, char *path, size_t path_size) {
    if (!g_storage_ok || !g_saves_dir[0]) return NULL;
    const char *base = base_name(name);
    size_t n = strlen(g_saves_dir);
    size_t m = strlen(base);
    if (n + 1 + m + 1 > path_size) return NULL;
    memcpy(path, g_saves_dir, n);
    path[n] = '/';
    memcpy(path + n + 1, base, m + 1);
    return fopen(path, "wb");
}

/* Resuelve un asset del juego solo en data/. Lo usa app/ para KULT.PRG. */
int platform_data_resolve(const char *name, char *out, size_t out_size) {
    return path_in(g_data_dir, name, out, out_size);
}

/* Tamaño de un fichero visible para el juego (saves/ o data/). */
static long data_size(const char *name) {
    char path[1200];
    FILE *f = open_read(name, path, sizeof path);
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long n = ftell(f);
    fclose(f);
    return n;
}

static int alloc_handle(FILE *f) {
    for (int i = 0; i < FS_MAX_HANDLES; i++) {
        if (!g_files[i]) { g_files[i] = f; return FS_FIRST_HANDLE + i; }
    }
    return -1;
}

static FILE *handle_file(uint16_t handle) {
    int i = (int)handle - FS_FIRST_HANDLE;
    if (i < 0 || i >= FS_MAX_HANDLES) return NULL;
    return g_files[i];
}

int32_t pc_os_fopen(uint8_t *mem, uint32_t name_ptr) {
    char path[1200];
    FILE *f = open_read((const char *)(mem + name_ptr), path, sizeof path);
    if (!f) return -1;
    int h = alloc_handle(f);
    if (h < 0) { fclose(f); return -1; }
    return h;
}

int32_t pc_os_fcreate(uint8_t *mem, uint32_t name_ptr) {
    char path[1200];
    FILE *f = open_write((const char *)(mem + name_ptr), path, sizeof path);
    if (!f) return -1;
    int h = alloc_handle(f);
    if (h < 0) { fclose(f); return -1; }
    return h;
}

int32_t pc_os_fread(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf) {
    FILE *f = handle_file(handle);
    if (!f) return -1;
    size_t n = fread(mem + buf, 1, count, f);
    /* Tiempo de lectura de disquete del ST (el original se queda bloqueado en
     * Fread; la imagen visible no cambia). No-op en headless/diferencial. */
    platform_disk_delay((uint32_t)n);
    return (int32_t)n;
}

int32_t pc_os_fwrite(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf) {
    FILE *f = handle_file(handle);
    if (!f) return -1;
    size_t n = fwrite(mem + buf, 1, count, f);
    return (int32_t)n;
}

int32_t pc_os_fclose(uint8_t *mem, uint16_t handle) {
    (void)mem;
    int i = (int)handle - FS_FIRST_HANDLE;
    if (i < 0 || i >= FS_MAX_HANDLES || !g_files[i]) return -1;
    fclose(g_files[i]);
    g_files[i] = NULL;
    return 0;
}

/* Fsfirst: existe el fichero -> vuelca nombre (13 B) y tamaño en el DTA y da 0. */
int32_t pc_os_fsfirst(uint8_t *mem, uint32_t name_ptr) {
    const char *name = (const char *)(mem + name_ptr);
    const char *base = base_name(name);
    long size = data_size(name);
    if (size < 0) return -1;
    memset(mem + g_os_dta + 0x1e, 0, 14);
    size_t n = strlen(base);
    if (n > 13) n = 13;
    memcpy(mem + g_os_dta + 0x1e, base, n);
    wr32(mem + g_os_dta + 0x1a, (uint32_t)size);
    return 0;
}
