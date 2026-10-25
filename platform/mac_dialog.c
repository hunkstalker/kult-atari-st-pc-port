/* platform/mac_dialog.c — primer arranque y errores visibles en macOS.
 *
 * El .app no puede escribir junto al ejecutable. Si falta kult.data, se pide
 * al usuario que seleccione los dos .STX y se copian a Application Support.
 */
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

typedef struct {
    SDL_AtomicInt done;
    char *paths[8];
    int count;
    int error;
} stx_pick;

static char *dup_path(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static void stx_pick_cb(void *userdata, const char * const *filelist, int filter) {
    stx_pick *p = userdata;
    (void)filter;
    if (!filelist) {
        p->error = 1;
    } else {
        for (int i = 0; filelist[i] && p->count < 8; i++) {
            p->paths[p->count] = dup_path(filelist[i]);
            if (p->paths[p->count]) p->count++;
        }
    }
    SDL_SetAtomicInt(&p->done, 1);
}

static int copy_to_data(const char *src) {
    const char *base = strrchr(src, '/');
    base = base ? base + 1 : src;
    char dst[1400];
    snprintf(dst, sizeof dst, "%s/%s", platform_data_dir(), base);
    if (strcmp(src, dst) == 0) return 0;

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
    if (ok) platform_log("STX importado: %s", base);
    return ok ? 0 : -1;
}

int platform_mac_select_stx(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        platform_log("ERROR: SDL_Init(VIDEO) para el selector STX: %s",
                     SDL_GetError());
        return 0;
    }

    static const SDL_DialogFileFilter filters[] = {
        { "Discos STX", "stx" },
        { "Todos los archivos", "*" },
    };
    stx_pick p = {0};
    SDL_SetAtomicInt(&p.done, 0);
    SDL_ShowOpenFileDialog(stx_pick_cb, &p, NULL, filters, 2, NULL, true);
    while (!SDL_GetAtomicInt(&p.done)) {
        SDL_PumpEvents();
        SDL_Delay(10);
    }
    SDL_Quit();

    int copied = 0;
    for (int i = 0; i < p.count; i++) {
        if (copy_to_data(p.paths[i]) == 0) copied++;
        free(p.paths[i]);
    }
    if (p.error) platform_log("ERROR: el selector de STX falló");
    if (p.count == 0) platform_log("selector STX: cancelado");
    return copied;
}

void platform_mac_show_error(const char *msg) {
    int was_init = SDL_WasInit(SDL_INIT_VIDEO) != 0;
    if (!was_init && !SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "[kult] %s\n", msg);
        return;
    }
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Kult", msg, NULL);
    if (!was_init) SDL_Quit();
}
