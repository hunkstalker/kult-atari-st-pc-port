/* platform/paths.c — rutas del ejecutable y base de datos (M7/T-070).
 *
 * `platform_exe_dir` devuelve el directorio del ejecutable. `platform_base_dir`
 * usa la carpeta del ejecutable en Windows/Linux y Application Support en
 * macOS. No depende de SDL (el build headless no enlaza SDL).
 */
#if defined(__APPLE__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#endif

#include "platform_internal.h"

static char g_test_base[1024];

void platform_set_base_dir_for_tests(const char *dir) {
    if (!dir) { g_test_base[0] = '\0'; return; }
    snprintf(g_test_base, sizeof g_test_base, "%s", dir);
}

int platform_base_dir(char *buf, size_t n) {
#ifndef _WIN32
    const char *override = getenv("KULT_BASE_DIR");
    if (override && override[0]) {
        snprintf(buf, n, "%s", override);
        return 0;
    }
#endif
    if (g_test_base[0]) {
        snprintf(buf, n, "%s", g_test_base);
        return 0;
    }
#ifdef __APPLE__
    const char *home = getenv("HOME");
    if (home && home[0]) {
        snprintf(buf, n, "%s/Library/Application Support/Kult", home);
        return 0;
    }
#endif
    return platform_exe_dir(buf, n);
}

int platform_exe_dir(char *buf, size_t n) {
#ifdef _WIN32
    DWORD len = GetModuleFileNameA(NULL, buf, (DWORD)n);
    if (len == 0 || len >= n) return -1;
    char *s = strrchr(buf, '\\');
    if (!s) s = strrchr(buf, '/');
    if (!s) return -1;
    *s = '\0';
    return 0;
#elif defined(__APPLE__)
    char raw[PATH_MAX];
    uint32_t len = (uint32_t)sizeof raw;
    if (_NSGetExecutablePath(raw, &len) != 0) return -1;
    char resolved[PATH_MAX];
    const char *path = realpath(raw, resolved);
    if (!path) path = raw;
    if (strlen(path) >= n) return -1;
    snprintf(buf, n, "%s", path);
    char *s = strrchr(buf, '/');
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
