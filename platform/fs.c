/* platform/fs.c — servicio de ficheros del port (GEMDOS sobre el SO real).
 *
 * Mapea los nombres Atari del juego (con o sin prefijo de unidad `A:`) a ficheros
 * de un directorio de datos del PC. El directorio se toma de `KULT_DATA_DIR`
 * (admite varios separados por `;` en Windows y `:` en POSIX); si no, `data/`
 * junto al ejecutable (M7/T-070) y, en su defecto, `data/` del directorio actual.
 *
 * Es el equivalente PC de los `os_fopen/os_fread/...` del kit, pero sobre el
 * sistema de ficheros real: el core no cambia, sólo cambia quién responde.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "os.h"
#include "platform.h"
#include "platform_internal.h"

#define FS_MAX_HANDLES 32
#define FS_FIRST_HANDLE 6          /* GEMDOS reserva 0..5 */

#ifdef _WIN32
#define DATA_SEP ';'
#else
#define DATA_SEP ':'
#endif

static FILE *g_files[FS_MAX_HANDLES];
static char *g_dirstr;            /* copia de KULT_DATA_DIR (o del directorio por defecto) */
static const char *g_dirs[16];
static int g_ndirs;

void platform_fs_init(void) {
    const char *env = getenv("KULT_DATA_DIR");
    char def[1024];
    if (!env || !*env) {
        /* Por defecto, `data/` junto al ejecutable (Windows/Linux); si la ruta del
         * ejecutable no se puede resolver, `data/` relativo al directorio actual. */
        if (platform_exe_dir(def, sizeof def) == 0) {
            size_t l = strlen(def);
            snprintf(def + l, sizeof def - l, "/data");
        } else {
            snprintf(def, sizeof def, "data");
        }
        env = def;
    }
    g_dirstr = malloc(strlen(env) + 1);
    if (g_dirstr) strcpy(g_dirstr, env);
    g_ndirs = 0;
    if (!g_dirstr) return;
    char *p = g_dirstr;
    while (p && *p && g_ndirs < 16) {
        g_dirs[g_ndirs++] = p;
        char *sep = strchr(p, DATA_SEP);
        if (!sep) break;
        *sep = '\0';
        p = sep + 1;
    }

    for (int i = 0; i < g_ndirs; i++)
        platform_log("datos[%d]: %s", i, g_dirs[i]);
}

/* Basura de unidad/ruta: de `A:CLEAR.BIN` deja `CLEAR.BIN`. */
static const char *base_name(const char *name) {
    const char *b = name;
    for (const char *q = name; *q; q++)
        if (*q == ':' || *q == '/' || *q == '\\') b = q + 1;
    return b;
}

/* Busca `name` en el/los directorio(s) de datos y lo abre como `mode`. */
static FILE *open_data(const char *name, const char *mode) {
    const char *base = base_name(name);
    char path[1024];
    for (int i = 0; i < g_ndirs; i++) {
        snprintf(path, sizeof path, "%s/%s", g_dirs[i], base);
        FILE *f = fopen(path, mode);
        if (f) return f;
    }
    return NULL;
}

/* Resuelve `name` en los directorios de datos y copia su ruta a `out`; 0 si
 * existe. Lo usa `app/` (p. ej. para localizar KULT.PRG en `data/`). */
int platform_data_resolve(const char *name, char *out, size_t out_size) {
    const char *base = base_name(name);
    for (int i = 0; i < g_ndirs; i++) {
        snprintf(out, out_size, "%s/%s", g_dirs[i], base);
        FILE *f = fopen(out, "rb");
        if (f) { fclose(f); return 0; }
    }
    return -1;
}

/* Tamaño de un fichero de datos, o -1 si no existe. */
static long data_size(const char *name) {
    FILE *f = open_data(name, "rb");
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
    FILE *f = open_data((const char *)(mem + name_ptr), "rb");
    if (!f) return -1;
    int h = alloc_handle(f);
    if (h < 0) { fclose(f); return -1; }
    return h;
}

int32_t pc_os_fcreate(uint8_t *mem, uint32_t name_ptr) {
    FILE *f = open_data((const char *)(mem + name_ptr), "wb");
    if (!f) return -1;
    int h = alloc_handle(f);
    if (h < 0) { fclose(f); return -1; }
    return h;
}

int32_t pc_os_fread(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf) {
    FILE *f = handle_file(handle);
    if (!f) return -1;
    size_t n = fread(mem + buf, 1, count, f);
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
    const char *base = base_name((const char *)(mem + name_ptr));
    long size = data_size((const char *)(mem + name_ptr));
    if (size < 0) return -1;
    memset(mem + g_os_dta + 0x1e, 0, 14);
    size_t n = strlen(base);
    if (n > 13) n = 13;
    memcpy(mem + g_os_dta + 0x1e, base, n);
    wr32(mem + g_os_dta + 0x1a, (uint32_t)size);
    return 0;
}
