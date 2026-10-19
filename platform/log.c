/* platform/log.c — log de diagnóstico del port (M7).
 *
 * El ejecutable de Windows es de subsistema GUI (sin consola): cualquier error
 * (no encontrar KULT.PRG, fallo de SDL, fallo del arranque) sería invisible. Por
 * eso todo se escribe en `kult.log` (junto al .exe) y también a stderr (build
 * headless). `platform_fatal` además muestra un MessageBox en la build GUI.
 *
 * Variables:
 *   KULT_LOG=<ruta>  ruta alternativa del log
 *   KULT_LOG=0       apaga el fichero (queda solo stderr)
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "platform.h"
#include "platform_internal.h"

#ifndef KULT_VERSION
#define KULT_VERSION "dev"
#endif

static FILE *g_log;
static char  g_log_path[2048];

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

    const char *env = getenv("KULT_LOG");
    if (env && env[0] == '0' && env[1] == '\0') return;   /* KULT_LOG=0 lo apaga */

    if (env && *env && !(env[0] == '1' && env[1] == '\0')) {
        g_log = open_log_at(env);                         /* ruta explícita */
    } else {
        char dir[1024];
        if (platform_exe_dir(dir, sizeof dir) == 0) {
            char path[1200];
            snprintf(path, sizeof path, "%s/kult.log", dir);
            g_log = open_log_at(path);
        }
        if (!g_log) {
#ifdef _WIN32
            const char *la = getenv("LOCALAPPDATA");
            if (la && *la) {
                char d[1024], path[1200];
                snprintf(d, sizeof d, "%s/Kult", la);
                _mkdir(d);
                snprintf(path, sizeof path, "%s/kult.log", d);
                g_log = open_log_at(path);
            }
            if (!g_log) {
                const char *tmp = getenv("TEMP");
                if (tmp && *tmp) {
                    char path[1200];
                    snprintf(path, sizeof path, "%s/kult.log", tmp);
                    g_log = open_log_at(path);
                }
            }
#else
            g_log = open_log_at("/tmp/kult.log");
#endif
        }
    }

    char ts[32];
    log_stamp(ts, sizeof ts);
    if (g_log) {
        fprintf(g_log, "[%s] Kult %s — log en %s\n", ts, KULT_VERSION, g_log_path);
        fflush(g_log);
    } else {
        fprintf(stderr, "[kult] Kult %s — sin fichero de log (solo stderr)\n",
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

void platform_fatal(const char *fmt, ...) {
    char msg[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);

    platform_log("ERROR: %s", msg);
#ifdef _WIN32
    /* Sin consola (doble clic / subsistema GUI) el error sería invisible: se
     * muestra un diálogo. Si hay consola (kult-headless.exe desde cmd) no se
     * interpone: el mensaje ya está en stderr y en el log. */
    if (GetConsoleWindow() == NULL)
        MessageBoxA(NULL, msg, "Kult — error", MB_OK | MB_ICONERROR);
#endif
}

void platform_log_close(void) {
    if (g_log) {
        fclose(g_log);
        g_log = NULL;
    }
}
