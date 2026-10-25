/* platform/video_sdl.c — ventana, escala entera y presentación con SDL3.
 *
 * Sólo se compila en el target `make port-sdl` (necesita SDL3). Lee el
 * framebuffer planar del juego + la paleta y lo presenta como textura RGBA con
 * escalado entero y nearest-neighbour. El viewport se calcula aparte
 * (platform/display.c) para poder cambiar la escala sin depender del tamaño de
 * la ventana. La conversión vive en platform/video.c (compartida con el volcado
 * PPM), de modo que render y golden usan la misma lectura.
 */
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "display.h"
#include "osd.h"
#include "kult_icon_rgba.h"      /* icono de la app (assets/, generado) */

#define FB_W 320
#define FB_H 200

static SDL_Window   *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture  *g_tex;
static uint8_t       g_rgba[FB_W * FB_H * 4];

static int g_scale_mode = PLATFORM_SCALE_AUTO;
static int g_fullscreen = 1;
static int g_last_window_scale = 3;
static platform_viewport g_vp = { 0, 0, FB_W, FB_H, 1, 0 };

static void update_viewport(void) {
    int ow = 0, oh = 0;
    if (g_ren)
        SDL_GetRenderOutputSize(g_ren, &ow, &oh);
    if (ow <= 0 || oh <= 0) {
        ow = FB_W * g_last_window_scale;
        oh = FB_H * g_last_window_scale;
    }
    platform_display_viewport(g_scale_mode, ow, oh, &g_vp);
}

static int max_window_scale(void) {
    int max = PLATFORM_SCALE_MAX;
    if (g_win) {
        SDL_DisplayID display = SDL_GetDisplayForWindow(g_win);
        const SDL_DisplayMode *dm = display ? SDL_GetDesktopDisplayMode(display) : NULL;
        if (dm && dm->w > 0 && dm->h > 0) {
            int sx = dm->w / FB_W;
            int sy = dm->h / FB_H;
            max = sx < sy ? sx : sy;
        }
    }
    if (max < PLATFORM_SCALE_MIN) max = PLATFORM_SCALE_MIN;
    if (max > PLATFORM_SCALE_MAX) max = PLATFORM_SCALE_MAX;
    return max;
}

static void apply_window_scale(void) {
    if (!g_win || g_fullscreen)
        return;
    int s = g_last_window_scale;
    int max = max_window_scale();
    if (s < PLATFORM_SCALE_MIN) s = PLATFORM_SCALE_MIN;
    if (s > max) s = max;
    SDL_SetWindowSize(g_win, FB_W * s, FB_H * s);
    SDL_SetWindowPosition(g_win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    update_viewport();
}

static void toggle_fullscreen(void) {
    if (!g_win)
        return;
    if (g_fullscreen) {
        if (SDL_SetWindowFullscreen(g_win, false)) {
            g_fullscreen = 0;
            apply_window_scale();
            platform_config_set_int("fullscreen", 0);
            platform_osd_show("VENTANA");
            platform_log("F1: ventana");
        } else {
            platform_log("F1: no se pudo salir de pantalla completa: %s",
                         SDL_GetError());
        }
    } else {
        if (SDL_SetWindowFullscreen(g_win, true)) {
            g_fullscreen = 1;
            update_viewport();
            platform_config_set_int("fullscreen", 1);
            platform_osd_show("PANTALLA COMPLETA");
            platform_log("F1: pantalla completa");
        } else {
            platform_log("F1: no se pudo entrar en pantalla completa: %s",
                         SDL_GetError());
        }
    }
}

static void cycle_scale(void) {
    g_scale_mode = platform_display_next(g_scale_mode);
    if (g_scale_mode >= PLATFORM_SCALE_MIN &&
        g_scale_mode <= PLATFORM_SCALE_MAX)
        g_last_window_scale = g_scale_mode;
    if (g_fullscreen || g_scale_mode == PLATFORM_SCALE_FILL)
        update_viewport();
    else
        apply_window_scale();

    platform_config_set_int("scale", g_scale_mode);

    char msg[32];
    if (g_scale_mode == PLATFORM_SCALE_AUTO)
        snprintf(msg, sizeof msg, "ESCALA AUTO");
    else if (g_scale_mode == PLATFORM_SCALE_FILL)
        snprintf(msg, sizeof msg, "ESTIRAR A PANTALLA");
    else
        snprintf(msg, sizeof msg, "ESCALA x%d", g_vp.scale);
    platform_osd_show(msg);
    platform_log("F2: modo=%d efectiva=%s", g_scale_mode,
                 g_vp.fill ? "estirar" : "entera");
}

static void show_audio_osd(void) {
    platform_config_set_int("volume", platform_audio_volume_saved());
    platform_config_set_int("muted", platform_audio_muted());
    char msg[32];
    if (platform_audio_muted())
        snprintf(msg, sizeof msg, "SILENCIO");
    else
        snprintf(msg, sizeof msg, "VOLUMEN %d", platform_audio_volume());
    platform_osd_show(msg);
}

static void mouse_to_game(float wx, float wy, int *x, int *y) {
    float rx = 0, ry = 0;
    if (g_ren && SDL_RenderCoordinatesFromWindow(g_ren, wx, wy, &rx, &ry))
        platform_display_point(&g_vp, rx, ry, x, y);
    else {
        *x = 160;
        *y = 100;
    }
}

int platform_sdl_open(const char *title, int scale_mode, int fullscreen) {
    if (scale_mode < PLATFORM_SCALE_AUTO ||
        (scale_mode > PLATFORM_SCALE_MAX && scale_mode != PLATFORM_SCALE_FILL))
        scale_mode = PLATFORM_SCALE_AUTO;
    g_scale_mode = scale_mode;
    g_last_window_scale = (scale_mode >= PLATFORM_SCALE_MIN &&
                           scale_mode <= PLATFORM_SCALE_MAX) ? scale_mode : 3;
    g_fullscreen = fullscreen ? 1 : 0;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        platform_log("SDL_Init(VIDEO): %s", SDL_GetError());
        return -1;
    }
    g_win = SDL_CreateWindow(title, FB_W * g_last_window_scale,
                             FB_H * g_last_window_scale, SDL_WINDOW_RESIZABLE);
    if (!g_win) {
        SDL_Log("CreateWindow: %s", SDL_GetError());
        platform_log("CreateWindow: %s", SDL_GetError());
        return -1;
    }
    /* El juego dibuja su propio cursor en el framebuffer. El cursor del sistema
     * (Windows) no debe verse encima; se oculta mientras la ventana tenga foco. */
    SDL_HideCursor();
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
    SDL_SetRenderVSync(g_ren, 1);
    update_viewport();

    if (g_fullscreen) {
        if (!SDL_SetWindowFullscreen(g_win, true)) {
            platform_log("fullscreen: no se pudo activar: %s", SDL_GetError());
            g_fullscreen = 0;
        } else {
            update_viewport();
        }
    } else {
        SDL_SetWindowPosition(g_win, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
    }
    return 0;
}

void platform_sdl_present(uint8_t *img) {
    if (!g_tex || !g_ren) return;
    update_viewport();
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
    if (platform_osd_active())
        platform_osd_draw_rgba(g_rgba, img, platform_osd_text());
    SDL_UpdateTexture(g_tex, NULL, g_rgba, FB_W * 4);
    SDL_SetRenderViewport(g_ren, NULL);
    SDL_SetRenderScale(g_ren, 1.0f, 1.0f);
    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);
    SDL_FRect dst = { (float)g_vp.x, (float)g_vp.y,
                      (float)g_vp.w, (float)g_vp.h };
    SDL_RenderTexture(g_ren, g_tex, NULL, &dst);
    SDL_RenderPresent(g_ren);
}

static int g_quit;
static SDL_MouseButtonFlags g_mouse_mask;   /* máscara SDL (LMASK/RMASK) */
static int g_mouse_gx = 160, g_mouse_gy = 100;   /* última posición en espacio de juego */

/* Traduce la máscara SDL a la del puerto: bit0 = izquierdo, bit1 = derecho.
 * OJO: en SDL3 RIGHT no es 2 sino 3, y RMASK = 1<<2 = 4; no pasar la máscara SDL
 * tal cual a platform_mouse_set. */
static int mouse_buttons(void) {
    return ((g_mouse_mask & SDL_BUTTON_LMASK) ? 1 : 0) |
           ((g_mouse_mask & SDL_BUTTON_RMASK) ? 2 : 0);
}

int platform_sdl_should_quit(void) { return g_quit; }

/* Bombea eventos: teclado -> cola GEMDOS; ratón -> cursor/botones (coords de
 * juego 320x200, vía viewport entero). Devuelve 1 para seguir, 0 si se cerró la
 * ventana. F1/F2 son del frontend y no llegan al juego. */
int platform_sdl_pump(void) {
    if (g_ren)
        update_viewport();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            g_quit = 1;
        } else if (e.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
            SDL_HideCursor();          /* solo el cursor del juego */
        } else if (e.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            /* Fuera de la ventana el usuario necesita el cursor del sistema. */
            g_mouse_mask = 0;
            SDL_ShowCursor();
        } else if (e.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN) {
            g_fullscreen = 1;
            update_viewport();
        } else if (e.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN) {
            g_fullscreen = 0;
            update_viewport();
        } else if (e.type == SDL_EVENT_WINDOW_RESIZED ||
                   e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
                   e.type == SDL_EVENT_WINDOW_DISPLAY_CHANGED) {
            update_viewport();
        } else if (e.type == SDL_EVENT_KEY_DOWN) {
            int special = e.key.key == SDLK_ESCAPE ||
                          e.key.key == SDLK_F1 || e.key.key == SDLK_F2 ||
                          e.key.key == SDLK_F5 || e.key.key == SDLK_F6 ||
                          e.key.key == SDLK_F7 || e.key.key == SDLK_F9 ||
                          e.key.key == SDLK_F10 || e.key.key == SDLK_F11 ||
                          e.key.key == SDLK_F12;
            if (e.key.repeat && special)
                continue;
            if (e.key.key == SDLK_ESCAPE) {
                g_quit = 1;
            } else if (e.key.key == SDLK_F1) {
                toggle_fullscreen();
            } else if (e.key.key == SDLK_F2) {
                cycle_scale();
            } else if (e.key.key == SDLK_F5) {
                platform_slots_select_next();
            } else if (e.key.key == SDLK_F6) {
                platform_slots_request_save();
            } else if (e.key.key == SDLK_F7) {
                platform_slots_request_load();
            } else if (e.key.key == SDLK_F9) {
                platform_audio_volume_step(-10);
                show_audio_osd();
            } else if (e.key.key == SDLK_F10) {
                platform_audio_toggle_mute();
                show_audio_osd();
            } else if (e.key.key == SDLK_F11) {
                platform_audio_volume_step(10);
                show_audio_osd();
            } else if (e.key.key == SDLK_F12) {
                /* Marca manual para T-073c (no va al juego). */
                platform_marks_event("f12", 0, 0);
                platform_log("F12: inicio de animacion");
            } else {
                platform_key_push((uint32_t)e.key.key & 0xff);
            }
        } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
            mouse_to_game(e.motion.x, e.motion.y, &g_mouse_gx, &g_mouse_gy);
            platform_mouse_set(g_mouse_gx, g_mouse_gy, mouse_buttons());
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                   e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            SDL_MouseButtonFlags bit = 0;
            if (e.button.button == SDL_BUTTON_LEFT)
                bit = SDL_BUTTON_LMASK;
            else if (e.button.button == SDL_BUTTON_RIGHT)
                bit = SDL_BUTTON_RMASK;
            if (bit) {
                if (e.button.down) g_mouse_mask |= bit;
                else               g_mouse_mask &= ~bit;
            }
            mouse_to_game(e.button.x, e.button.y, &g_mouse_gx, &g_mouse_gy);
            platform_mouse_set(g_mouse_gx, g_mouse_gy, mouse_buttons());
        }
    }
    {
        SDL_MouseButtonFlags mb = SDL_GetMouseState(NULL, NULL);
        g_mouse_mask = mb & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK);
        /* Sincroniza también si el botón cambió sin evento (p. ej. pérdida de
         * foco): el skip y la acción no se quedan pegados. */
        platform_mouse_set(g_mouse_gx, g_mouse_gy, mouse_buttons());
    }
    return !g_quit;
}

void platform_sdl_close(void) {
    SDL_ShowCursor();               /* restaurar el cursor del sistema al salir */
    if (g_tex) SDL_DestroyTexture(g_tex);
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    g_tex = NULL;
    g_ren = NULL;
    g_win = NULL;
    platform_osd_clear();
    SDL_Quit();
}
