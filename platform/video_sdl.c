/* platform/video_sdl.c — ventana y presentación con SDL3 (M3/T-030).
 *
 * Sólo se compila en el target `make port-sdl` (necesita SDL3). Lee el framebuffer
 * planar del juego + la paleta y lo presenta como textura RGBA con escalado entero
 * y nearest-neighbour. La conversión vive en platform/video.c (compartida con el
 * volcado PPM), de modo que render y golden usan la misma lectura.
 */
#include <SDL3/SDL.h>
#include <stdint.h>

#include "platform.h"

#define FB_W 320
#define FB_H 200

static SDL_Window   *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture  *g_tex;
static uint8_t       g_rgba[FB_W * FB_H * 4];

int platform_sdl_open(const char *title, int scale) {
    if (scale < 1) scale = 1;
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return -1;
    }
    g_win = SDL_CreateWindow(title, FB_W * scale, FB_H * scale,
                             SDL_WINDOW_RESIZABLE);
    if (!g_win) { SDL_Log("CreateWindow: %s", SDL_GetError()); return -1; }
    g_ren = SDL_CreateRenderer(g_win, NULL);
    if (!g_ren) { SDL_Log("CreateRenderer: %s", SDL_GetError()); return -1; }
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGBA32,
                              SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
    if (!g_tex) { SDL_Log("CreateTexture: %s", SDL_GetError()); return -1; }
    SDL_SetTextureScaleMode(g_tex, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderLogicalPresentation(g_ren, FB_W, FB_H,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_SetRenderVSync(g_ren, 1);
    return 0;
}

void platform_sdl_present(uint8_t *img) {
    (void)img;
    if (!g_tex) return;
    platform_fb_to_rgba(g_rgba);
    SDL_UpdateTexture(g_tex, NULL, g_rgba, FB_W * 4);
    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);
    SDL_RenderTexture(g_ren, g_tex, NULL, NULL);
    SDL_RenderPresent(g_ren);
}

/* Devuelve 1 para seguir, 0 para salir. */
int platform_sdl_pump(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) return 0;
        if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) return 0;
    }
    return 1;
}

void platform_sdl_close(void) {
    if (g_tex) SDL_DestroyTexture(g_tex);
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}
