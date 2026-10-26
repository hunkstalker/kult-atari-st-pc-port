/* platform/input.c — entrada del port: teclado (GEMDOS) y paquete IKBD (ratón).
 *
 * M1 es headless: la entrada se inyecta por script (KULT_KEYS) o con una auto-tecla
 * (KULT_KEY, por defecto `E`, que es el idioma inglés en el menú del arranque). El
 * ratón entra por el paquete IKBD (`A_IKBD_PACKET`): sus bytes 1-4 SON A_CURSOR_X/Y
 * (ver docs/PORTING.md), así que escribirlos ahí actualiza el cursor del juego.
 *
 * La entrada real (SDL) llega en M3; aquí basta con que las esperas no cuelguen y
 * que el cursor/estado sean deterministas.
 *
 * Botones (modelo del original): el IKBD se interroga con el comando 0x0D
 * ("interrogate mouse position") en cada wait_frame. La respuesta del original
 * es un EVENTO: los bits de bajada/subida sólo aparecen desde la interroga-
 * ción anterior, no mientras el botón sigue pulsado. Por eso aquí se entrega
 * un flanco de bajada una sola vez por pulsación:
 *   - izquierdo: acción (bit0 del paquete -> bit2 de A_INPUT_BUTTONS), sin skip.
 *   - derecho: acción + skip (A_SKIP_TRANSITION = 0xff) en esa misma bajada.
 * Mantener cualquiera de los dos NO repite la acción. El core `wait_frame`
 * deriva acción y skip del bit0 del paquete IKBD; la HAL pisa `$1a570` al salir
 * de `wait_frame` (`platform_input_apply_skip`) para separar el skip del
 * derecho, igual que hacía el motor de referencia. Así `src/` queda intacto.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "os.h"
#include "addrs.h"
#include "platform.h"
#include "platform_internal.h"
#include "kult_port.h"

#define KEYQ 64

static uint32_t g_keys[KEYQ];
static int g_head, g_tail;
static uint32_t g_auto_key = 'E';
static uint16_t g_cursor_x = 160;
static uint16_t g_cursor_y = 100;
static uint16_t g_delivered_x = 160;
static uint16_t g_delivered_y = 100;
static uint16_t g_observed_x = 160;
static uint16_t g_observed_y = 100;
static uint8_t  g_game_cursor_override;
static uint8_t  g_left_down, g_right_down; /* estado físico de los botones */
static uint8_t  g_action_pending;  /* flanco de bajada aún no entregado */
static uint8_t  g_skip_pending;    /* bajada del derecho aún no entregada */
static uint8_t  g_skip_event;      /* skip del último paquete entregado */
static uint8_t  g_auto_prev;       /* estado sintético del clic headless */
static uint16_t g_mouse_max_x = 319;   /* rango absoluto configurado por el IKBD */
static uint16_t g_mouse_max_y = 199;
static uint32_t g_deliveries;
static int g_click_toggle = 1;     /* headless: alterna clic para liberar esperas */
static int g_auto = 1;             /* 1 = headless (auto-tecla); 0 = entrada real */
static void (*g_idle)(uint8_t *img);   /* bombeo durante una espera bloqueante */

/* ---- replay de entrada ---------------------------------------------------
 *
 * Se guardan las respuestas que ve el juego en cada sondeo de entrada, no los
 * eventos crudos de SDL:
 *   1 = paquete IKBD (boton, skip, cursor x/y)
 *   2 = Crawio (tecla o 0)
 *   3 = Cnecin (tecla)
 * Asi el replay conserva exactamente los clics, el cursor y las teclas que
 * consumio el juego, sin depender del reloj real ni del objeto inicial.
 */
#define REPLAY_MAGIC "KULTREP1"
#define REPLAY_VERSION 1u
#define REPLAY_IKBD 1u
#define REPLAY_CRAWIO 2u
#define REPLAY_CNECIN 3u

typedef struct {
    char magic[8];
    uint32_t version;
    uint32_t flags;
} replay_header;

typedef struct {
    uint8_t type;
    uint8_t button;
    uint8_t skip;
    uint8_t reserved;
    uint16_t x;
    uint16_t y;
    uint32_t key;
    uint32_t frame;
} replay_event;

_Static_assert(sizeof(replay_header) == 16, "cabecera de replay inesperada");
_Static_assert(sizeof(replay_event) == 16, "evento de replay inesperado");

static FILE *g_replay_file;
static int g_replay_mode;          /* 0 = off, 1 = grabar, 2 = reproducir */
static int g_replay_eof;

static void replay_write_event(uint8_t type, uint8_t button, uint8_t skip,
                               uint16_t x, uint16_t y, uint32_t key) {
    if (!g_replay_file || g_replay_mode != 1) return;
    replay_event ev = { type, button, skip, 0, x, y, key, platform_frame_count() };
    fwrite(&ev, sizeof ev, 1, g_replay_file);
    fflush(g_replay_file);
}

static int replay_read_event(replay_event *ev, uint8_t want) {
    if (!g_replay_file || g_replay_mode != 2) return 0;
    if (fread(ev, sizeof *ev, 1, g_replay_file) != 1) {
        if (!g_replay_eof) {
            g_replay_eof = 1;
            platform_log("replay: fin del fichero antes de terminar la partida");
        }
        g_replay_mode = 0;
        return 0;
    }
    if (ev->type != want) {
        platform_log("replay: tipo inesperado %u (esperaba %u) en frame %u",
                     (unsigned)ev->type, (unsigned)want, (unsigned)ev->frame);
        g_replay_mode = 0;
        return 0;
    }
    return 1;
}

void platform_replay_init(void) {
    const char *rec = getenv("KULT_REPLAY_REC");
    const char *play = getenv("KULT_REPLAY_PLAY");
    g_replay_file = NULL;
    g_replay_mode = 0;
    g_replay_eof = 0;
    if (rec && *rec && play && *play) {
        platform_log("replay: define solo KULT_REPLAY_REC o KULT_REPLAY_PLAY");
        return;
    }
    if (rec && *rec) {
        g_replay_file = fopen(rec, "wb");
        if (!g_replay_file) {
            platform_log("replay: no se pudo crear %s", rec);
            return;
        }
        replay_header h = { { 'K','U','L','T','R','E','P','1' }, REPLAY_VERSION, 0 };
        fwrite(&h, sizeof h, 1, g_replay_file);
        fflush(g_replay_file);
        g_replay_mode = 1;
        platform_log("replay: grabando en %s", rec);
    } else if (play && *play) {
        g_replay_file = fopen(play, "rb");
        if (!g_replay_file) {
            platform_log("replay: no se pudo abrir %s", play);
            return;
        }
        replay_header h;
        if (fread(&h, sizeof h, 1, g_replay_file) != 1 ||
            memcmp(h.magic, REPLAY_MAGIC, sizeof h.magic) != 0 ||
            h.version != REPLAY_VERSION) {
            platform_log("replay: fichero no valido %s", play);
            fclose(g_replay_file);
            g_replay_file = NULL;
            return;
        }
        g_replay_mode = 2;
        platform_log("replay: reproduciendo %s", play);
    }
}

int platform_replay_playing(void) { return g_replay_mode == 2; }
int platform_replay_recording(void) { return g_replay_mode == 1; }

void platform_replay_shutdown(void) {
    if (g_replay_file) {
        fflush(g_replay_file);
        fclose(g_replay_file);
        g_replay_file = NULL;
    }
    g_replay_mode = 0;
}

void platform_set_idle(void (*fn)(uint8_t *img)) { g_idle = fn; }

void platform_input_set_auto(int on) {
    on = on ? 1 : 0;
    if (on == g_auto) return;
    g_auto = on;
    /* No arrastrar botones/flancos entre modos (los tests y el arranque alternan
     * entre entrada real y auto). */
    g_left_down = g_right_down = 0;
    g_action_pending = g_skip_pending = g_skip_event = 0;
    g_auto_prev = 0;
    g_deliveries = 0;
}

void platform_mouse_set(int x, int y, int buttons) {
    if (g_replay_mode == 2) return;   /* la entrada viene del replay */
    if (x < 0) x = 0;
    if (x > g_mouse_max_x) x = g_mouse_max_x;
    if (y < 0) y = 0;
    if (y > g_mouse_max_y) y = g_mouse_max_y;
    if (g_cursor_x != (uint16_t)x || g_cursor_y != (uint16_t)y)
        g_game_cursor_override = 0;
    g_cursor_x = (uint16_t)x;
    g_cursor_y = (uint16_t)y;
    /* Máscara del puerto: bit0 = izquierdo, bit1 = derecho. Sólo las bajadas
     * generan evento; mantener el botón no repite la acción. */
    int left = (buttons & 0x01) != 0;
    int right = (buttons & 0x02) != 0;
    if (left && !g_left_down) {
#ifndef KULT_NO_CPUWORK
        platform_diag_input("mouse_down", NULL, g_cursor_x, g_cursor_y,
                            (uint32_t)buttons);
#endif
        g_action_pending = 1;
    }
    if (!left && g_left_down) {
#ifndef KULT_NO_CPUWORK
        platform_diag_input("mouse_up", NULL, g_cursor_x, g_cursor_y,
                            (uint32_t)buttons);
#endif
    }
    if (right && !g_right_down) {
#ifndef KULT_NO_CPUWORK
        platform_diag_input("mouse_down_r", NULL, g_cursor_x, g_cursor_y,
                            (uint32_t)buttons);
#endif
        g_action_pending = 1;
        g_skip_pending = 1;
    }
    if (!right && g_right_down) {
#ifndef KULT_NO_CPUWORK
        platform_diag_input("mouse_up_r", NULL, g_cursor_x, g_cursor_y,
                            (uint32_t)buttons);
#endif
    }
    g_left_down = (uint8_t)left;
    g_right_down = (uint8_t)right;
}

int platform_input_skip_state(void) { return g_right_down; }

/* Posición para el overlay: si el ratón se movió después de la última entrega
 * IKBD, usa la posición nueva; si no, respeta un cursor movido por el juego
 * (p. ej. la ficha de un objeto). */
void platform_input_cursor_for_overlay(const uint8_t *img, uint16_t *x,
                                       uint16_t *y) {
    if (!img || !x || !y) return;
    uint16_t ax = be16(img + A_CURSOR_X);
    uint16_t ay = be16(img + A_CURSOR_Y);
    if (ax != g_observed_x || ay != g_observed_y) {
        g_observed_x = ax;
        g_observed_y = ay;
        g_game_cursor_override = 1;   /* el juego recolocó el cursor */
    }
    if (g_game_cursor_override) {
        *x = ax;
        *y = ay;
    } else if (g_cursor_x != g_delivered_x || g_cursor_y != g_delivered_y) {
        *x = g_cursor_x;
        *y = g_cursor_y;
    } else {
        *x = ax;
        *y = ay;
    }
}

/* Emula el comando IKBD que el juego envía en crt0_init:
 *   Ikbdws(0x19, 0x1a459, 4) -> 5 bytes: 09 01 30 00 b8
 * El 0x09 configura el ratón absoluto con máximo X=0x0130 (304) e Y=0x00b8
 * (184), que es justo el área para un cursor de 16x16. Sin esto, el ratón del
 * port llega a 319/199 y el sprite se sale por el borde. */
void platform_ikbdws(const uint8_t *buf, uint32_t len) {
    if (!buf || len < 5 || buf[0] != 0x09) return;
    uint32_t xmax = ((uint32_t)buf[1] << 8) | buf[2];
    uint32_t ymax = ((uint32_t)buf[3] << 8) | buf[4];
    if (xmax <= 319) g_mouse_max_x = (uint16_t)xmax;
    if (ymax <= 199) g_mouse_max_y = (uint16_t)ymax;
    if (g_cursor_x > g_mouse_max_x) g_cursor_x = g_mouse_max_x;
    if (g_cursor_y > g_mouse_max_y) g_cursor_y = g_mouse_max_y;
}

/* La HAL pisa el flag de skip que acaba de escribir `wait_frame` (core) con el
 * flanco del derecho del paquete recién entregado. Es el equivalente port-side
 * del intercept de `$1a570` del motor de referencia, pero por evento (no por
 * nivel). */
void platform_input_apply_skip(uint8_t *img) {
    img[A_SKIP_TRANSITION] = g_skip_event ? 0xff : 0;
    g_skip_event = 0;
}

void platform_input_init(void) {
    platform_replay_init();
    atexit(platform_replay_shutdown);
    g_head = g_tail = 0;
    g_deliveries = 0;
    g_left_down = g_right_down = 0;
    g_action_pending = g_skip_pending = g_skip_event = 0;
    g_auto_prev = 0;
    g_mouse_max_x = 319;
    g_mouse_max_y = 199;
    const char *click = getenv("KULT_CLICK");
    if (click && click[0] == '0') g_click_toggle = 0;
    const char *autoenv = getenv("KULT_KEY");
    if (autoenv && *autoenv) g_auto_key = (uint8_t)autoenv[0];
    const char *script = getenv("KULT_KEYS");
    if (script) platform_key_push_str(script);
    const char *cx = getenv("KULT_CURSOR_X");
    const char *cy = getenv("KULT_CURSOR_Y");
    if (cx) g_cursor_x = (uint16_t)atoi(cx);
    if (cy) g_cursor_y = (uint16_t)atoi(cy);
}

void platform_key_push(uint32_t key) {
    if (g_replay_mode == 2) return;   /* la entrada viene del replay */
    int next = (g_tail + 1) % KEYQ;
    if (next == g_head) return;                 /* cola llena: se descarta */
    g_keys[g_tail] = key;
    g_tail = next;
}

void platform_key_push_str(const char *s) {
    for (; s && *s; s++)
        platform_key_push((uint8_t)*s);
}

/* GEMDOS Crawio(0xff): lectura no bloqueante; devuelve la tecla o 0. */
uint32_t pc_os_crawio(uint8_t *mem, uint16_t w) {
    (void)mem;
    if (w != 0x00ff) return 0;                  /* escritura a consola: no-op */
    if (g_replay_mode == 2) {
        replay_event ev;
        if (replay_read_event(&ev, REPLAY_CRAWIO))
            return ev.key;
        return 0;
    }
    uint32_t k = 0;
    if (g_head != g_tail) {
        k = g_keys[g_head];
        g_head = (g_head + 1) % KEYQ;
    }
    if (g_replay_mode == 1)
        replay_write_event(REPLAY_CRAWIO, 0, 0, 0, 0, k);
    return k;
}

/* GEMDOS Cnecin: lectura bloqueante. En headless se sirve la cola o la auto-tecla
 * (nunca cuelga). Con entrada real (`platform_input_set_auto(0)`) espera bombeando
 * `g_idle` (SDL) hasta que llegue una tecla. Devuelve ascii en el byte bajo. */
int pc_os_cnecin(uint8_t *mem, uint32_t *out) {
    if (g_replay_mode == 2) {
        replay_event ev;
        uint32_t k = replay_read_event(&ev, REPLAY_CNECIN) ? ev.key : 0;
        if (out) *out = k;
        return 1;
    }
    if (g_head == g_tail) {
        if (!g_auto) {
            while (g_head == g_tail && g_idle)
                g_idle(mem);
        }
        if (g_head == g_tail) {                   /* sin tecla y sin idle */
            if (out) *out = g_auto_key;
            if (g_replay_mode == 1)
                replay_write_event(REPLAY_CNECIN, 0, 0, 0, 0, g_auto_key);
            return 1;
        }
    }
    uint32_t k = g_keys[g_head];
    g_head = (g_head + 1) % KEYQ;
    if (out) *out = k;
    if (g_replay_mode == 1)
        replay_write_event(REPLAY_CNECIN, 0, 0, 0, 0, k);
    return 1;
}

/* Paquete IKBD: [botones][x_hi][x_lo][y_hi][y_lo] = A_IKBD_PACKET(+1..).
 * Se entrega un flanco de bajada (bit0) y se consume; si el botón sigue
 * pulsado, las siguientes entregas van a 0, como la respuesta 0x0D del IKBD.
 * Headless: el clic sintético alterna para liberar las esperas "soltar y
 * pulsar" (prompt_wait). Entrada real: se entrega el flanco pendiente. */
void platform_ikbd_deliver(uint8_t *img) {
    uint8_t b;
    uint8_t skip;
    if (g_replay_mode == 2) {
        replay_event ev;
        if (replay_read_event(&ev, REPLAY_IKBD)) {
            b = ev.button ? 1 : 0;
            skip = ev.skip;
            g_cursor_x = ev.x;
            g_cursor_y = ev.y;
        } else {
            b = 0;
            skip = 0;
        }
        g_left_down = b;
        g_right_down = 0;
        g_action_pending = 0;
        g_skip_pending = 0;
        g_skip_event = skip;
        g_deliveries++;
    } else {
        if (g_auto) {
            int synthetic = g_click_toggle && (g_deliveries & 1);
            if (synthetic && !g_auto_prev)
                g_action_pending = 1;
            g_auto_prev = (uint8_t)synthetic;
        }
        b = g_action_pending ? 1 : 0;
        g_action_pending = 0;
        g_skip_event = g_skip_pending;
        g_skip_pending = 0;
        g_deliveries++;
        skip = g_skip_event;
        if (g_replay_mode == 1)
            replay_write_event(REPLAY_IKBD, b, skip, g_cursor_x, g_cursor_y, 0);
    }
    img[A_IKBD_PACKET] = b;
    if (b & 1)
        img[A_INPUT_BUTTONS] |= 0x04;
    else
        img[A_INPUT_BUTTONS] &= (uint8_t)~0x04;
    wr16(img + A_CURSOR_X, g_cursor_x);
    wr16(img + A_CURSOR_Y, g_cursor_y);
    g_delivered_x = g_cursor_x;
    g_delivered_y = g_cursor_y;
    g_observed_x = g_cursor_x;
    g_observed_y = g_cursor_y;
    g_game_cursor_override = 0;
    if (b & 1)
        kult_diag_input("ikbd_click", img, g_cursor_x, g_cursor_y, b);
}
