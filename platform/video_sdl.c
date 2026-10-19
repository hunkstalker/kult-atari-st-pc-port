/* platform/video_sdl.c — ventana y presentación con SDL3 (M3/T-030).
 *
 * Sólo se compila en el target `make port-sdl` (necesita SDL3). Lee el framebuffer
 * planar del juego + la paleta y lo presenta como textura RGBA con escalado entero
 * y nearest-neighbour. La conversión vive en platform/video.c (compartida con el
 * volcado PPM), de modo que render y golden usan la misma lectura.
 */
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "kult_icon_rgba.h"      /* icono de la app (assets/, generado) */

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
        platform_log("SDL_Init(VIDEO): %s", SDL_GetError());
        return -1;
    }
    g_win = SDL_CreateWindow(title, FB_W * scale, FB_H * scale,
                             SDL_WINDOW_RESIZABLE);
    if (!g_win) {
        SDL_Log("CreateWindow: %s", SDL_GetError());
        platform_log("CreateWindow: %s", SDL_GetError());
        return -1;
    }
    /* Icono de la ventana (el recurso del .exe lo pone windres en Windows). */
    SDL_Surface *icon = SDL_CreateSurfaceFrom(KULT_ICON_W, KULT_ICON_H,
                                              SDL_PIXELFORMAT_RGBA32,
                                              (void *)kult_icon_rgba,
                                              KULT_ICON_W * 4);
    if (icon) {
        SDL_SetWindowIcon(g_win, icon);
        SDL_DestroySurface(icon);
    }
    g_ren = SDL_CreateRenderer(g_win, NULL);
    if (!g_ren) {
        SDL_Log("CreateRenderer: %s", SDL_GetError());
        platform_log("CreateRenderer: %s", SDL_GetError());
        return -1;
    }
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGBA32,
                              SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
    if (!g_tex) {
        SDL_Log("CreateTexture: %s", SDL_GetError());
        platform_log("CreateTexture: %s", SDL_GetError());
        return -1;
    }
    SDL_SetTextureScaleMode(g_tex, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderLogicalPresentation(g_ren, FB_W, FB_H,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_SetRenderVSync(g_ren, 1);
    return 0;
}

void platform_sdl_present(uint8_t *img) {
    (void)img;
    if (!g_tex) return;
    /* KULT_VBL_TITLE=1: muestra el VBL actual en la barra de titulo (marcas). */
    {
        static int en = -1;
        static uint32_t last;
        if (en < 0) en = getenv("KULT_VBL_TITLE") != NULL;
        uint32_t v = platform_frame_count();
        if (en && (v - last >= 10 || v < last)) {
            char t[64];
            snprintf(t, sizeof t, "Kult — VBL %u", v);
            SDL_SetWindowTitle(g_win, t);
            last = v;
        }
    }
    platform_fb_to_rgba(g_rgba);
    SDL_UpdateTexture(g_tex, NULL, g_rgba, FB_W * 4);
    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);
    SDL_RenderTexture(g_ren, g_tex, NULL, NULL);
    SDL_RenderPresent(g_ren);
}

static int g_quit;
static int g_mouse_btn;

int platform_sdl_should_quit(void) { return g_quit; }

/* Bombea eventos: teclado -> cola GEMDOS; ratón -> cursor/botones (coords de
 * juego 320x200, vía la presentación lógica). Devuelve 1 para seguir, 0 si se
 * cerró la ventana. */
int platform_sdl_pump(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            g_quit = 1;
        } else if (e.type == SDL_EVENT_KEY_DOWN) {
            if (e.key.key == SDLK_ESCAPE) {
                g_quit = 1;
            } else if (e.key.key == SDLK_F12) {
                /* Marca manual para T-073c (no va al juego). */
                platform_marks_event("f12", 0, 0);
                platform_log("F12: inicio de animacion");
            } else if (e.key.key == SDLK_F11) {
                platform_marks_event("f11", 0, 0);
                platform_log("F11: fin de animacion");
            } else {
                platform_key_push((uint32_t)e.key.key & 0xff);
            }
        } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
            float gx = 0, gy = 0;
            SDL_RenderCoordinatesFromWindow(g_ren, e.motion.x, e.motion.y, &gx, &gy);
            platform_mouse_set((int)gx, (int)gy, g_mouse_btn);
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                   e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (e.button.button == SDL_BUTTON_LEFT)
                g_mouse_btn = e.button.down ? 1 : 0;
            float gx = 0, gy = 0;
            SDL_RenderCoordinatesFromWindow(g_ren, e.button.x, e.button.y, &gx, &gy);
            platform_mouse_set((int)gx, (int)gy, g_mouse_btn);
        }
    }
    {
        SDL_MouseButtonFlags mb = SDL_GetMouseState(NULL, NULL);
        g_mouse_btn = (mb & SDL_BUTTON_LMASK) ? 1 : 0;
    }
    return !g_quit;
}

void platform_sdl_close(void) {
    if (g_tex) SDL_DestroyTexture(g_tex);
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}
