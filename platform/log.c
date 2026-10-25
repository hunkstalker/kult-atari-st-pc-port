/* platform/log.c — log de diagnóstico del port.
 *
 * `kult.log` vive en la base de datos: junto al ejecutable en Windows/Linux y
 * en ~/Library/Application Support/Kult en macOS. No hay KULT_LOG ni /tmp.
 * Política B: los errores se registran con timestamp; en macOS se muestra un
 * diálogo fatal visible.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/stat.h>
#endif

#include "platform.h"
#include "platform_internal.h"

#ifndef KULT_VERSION
#define KULT_VERSION "dev"
#endif

static FILE *g_log;
static char  g_log_path[2048];
static void (*g_error_dialog)(const char *msg);

static void log_stamp(char *out, size_t n) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    if (tm) strftime(out, n, "%Y-%m-%d %H:%M:%S", tm);
    else snprintf(out, n, "-");
}

static FILE *open_log_at(const char *path) {
    FILE *f = fopen(path, "a");    /* append: cada run conserva las anteriores */
    if (f) snprintf(g_log_path, sizeof g_log_path, "%s", path);
    return f;
}

void platform_log_init(void) {
    if (g_log) return;

    char base[1024];
    if (platform_base_dir(base, sizeof base) == 0) {
#ifdef __APPLE__
        (void)mkdir(base, 0755);
#endif
        char path[1200];
        snprintf(path, sizeof path, "%s/kult.log", base);
        g_log = open_log_at(path);
    }

    char ts[32];
    log_stamp(ts, sizeof ts);
    if (g_log) {
        fprintf(g_log, "[%s] Kult %s — log en %s\n", ts, KULT_VERSION, g_log_path);
        fflush(g_log);
    } else {
        fprintf(stderr, "[kult] Kult %s — no se pudo escribir kult.log\n",
                KULT_VERSION);
    }
}

void platform_log(const char *fmt, ...) {
    char msg[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);

    if (g_log) {
        char ts[32];
        log_stamp(ts, sizeof ts);
        fprintf(g_log, "[%s] %s\n", ts, msg);
        fflush(g_log);                 /* el .exe puede morir en cualquier punto */
    }
    fprintf(stderr, "[kult] %s\n", msg);
}

void platform_set_error_dialog(void (*fn)(const char *msg)) {
    g_error_dialog = fn;
}

void platform_fatal(const char *fmt, ...) {
    char msg[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);

    platform_log("ERROR: %s", msg);
#ifdef _WIN32
    /* Política B: sin log no hay forma de diagnosticar; este es el único
     * diálogo permitido. Si el log existe, el error solo va a kult.log. */
    if (!g_log && GetConsoleWindow() == NULL)
        MessageBoxA(NULL, msg, "Kult — error", MB_OK | MB_ICONERROR);
#elif defined(__APPLE__)
    /* En un .app lanzado desde Finder no hay stderr visible: muestra el error. */
    if (g_error_dialog)
        g_error_dialog(msg);
#endif
}

void platform_log_close(void) {
    if (g_log) {
        fclose(g_log);
        g_log = NULL;
    }
}
