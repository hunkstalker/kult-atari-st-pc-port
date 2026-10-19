/* platform/paths.c — rutas del ejecutable (M7/T-070).
 *
 * `platform_exe_dir` devuelve el directorio del ejecutable sin barra final. Lo
 * usan `platform/fs.c` (data/ junto al exe) y `platform/log.c` (kult.log). Se
 * implementa con la API del sistema, sin depender de SDL (el build headless no
 * enlaza SDL).
 */
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "platform_internal.h"

int platform_exe_dir(char *buf, size_t n) {
#ifdef _WIN32
    DWORD len = GetModuleFileNameA(NULL, buf, (DWORD)n);
    if (len == 0 || len >= n) return -1;
    char *s = strrchr(buf, '\\');
    if (!s) s = strrchr(buf, '/');
    if (!s) return -1;
    *s = '\0';
    return 0;
#else
    ssize_t len = readlink("/proc/self/exe", buf, n - 1);
    if (len <= 0) return -1;
    buf[len] = '\0';
    char *s = strrchr(buf, '/');
    if (!s) return -1;
    *s = '\0';
    return 0;
#endif
}
