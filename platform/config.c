/* platform/config.c — kult.ini junto al ejecutable (clave = valor). */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

static void config_path(char *out, size_t out_size) {
    snprintf(out, out_size, "%s/kult.ini", platform_storage_path());
}

static int key_line(const char *line, const char *key) {
    while (isspace((unsigned char)*line)) line++;
    size_t n = strlen(key);
    if (strncmp(line, key, n) != 0)
        return 0;
    char c = line[n];
    return c == '=' || c == ' ' || c == '\t' || c == '\0';
}

int platform_config_get_int(const char *key, int def) {
    if (!key || !*key)
        return def;
    char path[1300], line[256];
    config_path(path, sizeof path);
    FILE *f = fopen(path, "r");
    if (!f)
        return def;
    int value = def;
    while (fgets(line, sizeof line, f)) {
        if (!key_line(line, key))
            continue;
        char *eq = strchr(line, '=');
        if (eq)
            value = (int)strtol(eq + 1, NULL, 10);
    }
    fclose(f);
    return value;
}

void platform_config_set_int(const char *key, int value) {
    if (!key || !*key)
        return;
    char path[1300], tmp[1310], line[256];
    config_path(path, sizeof path);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);

    FILE *in = fopen(path, "r");
    FILE *out = fopen(tmp, "w");
    if (!out) {
        platform_log("config: no se pudo escribir %s", tmp);
        if (in) fclose(in);
        return;
    }
    int written = 0;
    if (in) {
        while (fgets(line, sizeof line, in)) {
            if (key_line(line, key)) {
                if (!written) {
                    fprintf(out, "%s = %d\n", key, value);
                    written = 1;
                }
            } else {
                fputs(line, out);
            }
        }
        fclose(in);
    }
    if (!written)
        fprintf(out, "%s = %d\n", key, value);
    if (fclose(out) != 0) {
        remove(tmp);
        platform_log("config: no se pudo cerrar %s", tmp);
        return;
    }
    remove(path);
    if (rename(tmp, path) != 0) {
        remove(tmp);
        platform_log("config: no se pudo reemplazar %s", path);
    }
}
