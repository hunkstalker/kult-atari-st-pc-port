/* platform/diag.c — diagnóstico opcional del port.
 *
 * Todo el diagnóstico activable por variables de entorno vive aquí, separado
 * del modelo de timing de platform/cpuwork.c:
 *   KULT_MARKS    marcas F/K/B/S/U/P, F12 y trazas Y/V/D de la build diag;
 *   KULT_RINGLOG  eventos R del anillo y T de texto (si no, usa KULT_MARKS);
 *   KULT_RANDLOG  lecturas D de la tabla ALEAT;
 *   KULT_INPUTLOG eventos I de entrada.
 * KULT_AUTOLOG/KULT_F12_AT solo deciden cuándo llamar a platform_marks_event
 * desde platform/time.c; el volcado se implementa aquí.
 *
 * Sin esas variables no se abre ningún fichero. El buffer circular de F12 se
 * reserva de forma perezosa al abrir KULT_MARKS: la build normal no reserva los
 * 2 MiB que antes quedaban en BSS aunque no se usara el diagnóstico.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"
#include "addrs.h"
#include "platform.h"
#include "diag.h"

extern uint8_t *g_kult_img;

/* Buffer circular de las ultimas entradas (funcion, VBL) para que F12 pueda
 * volcar los ~3 s ANTERIORES aunque se pulse despues de la animacion. */
#define MARK_RING (1u << 18)
typedef struct { uint32_t addr, vbl; } mark_entry;
static mark_entry *g_ring;
static uint32_t g_ring_pos, g_ring_count;
static uint32_t g_mark_until;
static int g_last_skip = -1;

static FILE *marks_file(void) {
    static FILE *f = (FILE *)-1;
    if (f == (FILE *)-1) {
        const char *e = getenv("KULT_MARKS");
        f = (e && *e) ? fopen(e, "a") : NULL;
        if (f) {
            /* Solo se reservan los 2 MiB del historial si hay diagnóstico. */
            g_ring = calloc(MARK_RING, sizeof *g_ring);
            fprintf(f, "# RUN\n");
            fflush(f);
        }
    }
    return f;
}

static int marks_recording(void) {
    static int on = -1;
    if (on < 0) { const char *e = getenv("KULT_MARKS"); on = (e && *e) ? 1 : 0; }
    return on;
}

/* Ultima linea de texto dibujada: se incluye en el volcado F12/auto. */
static struct {
    uint32_t vbl, str, end, width, index;
    char tag[16];
    uint8_t raw[80];
    uint32_t raw_len;
} g_last_text;

static void ring_push(uint32_t addr, uint32_t vbl) {
    if (!g_ring) return;
    g_ring[g_ring_pos].addr = addr;
    g_ring[g_ring_pos].vbl = vbl;
    g_ring_pos = (g_ring_pos + 1) & (MARK_RING - 1);
    if (g_ring_count < MARK_RING) g_ring_count++;
}

/* Direcciones cuyas entradas se registran con KULT_MARKS (personaje, reveal y
 * transición/lift). Se mantiene pequeño para no generar logs gigantes. */
static const uint32_t k_mark_addrs[] = {
    0x15da6, 0x12e2e, 0x12f28, 0x123c0, 0x12b40, 0x15a90, 0x1215e,
    0x15d7e, 0x159e2, 0x159d2, 0x15ac8, 0x138a6, 0x13740,
    0x12ce4, 0x12e08, 0x12ea2,
};

static int mark_wanted(uint32_t addr) {
    for (unsigned i = 0; i < sizeof k_mark_addrs / sizeof k_mark_addrs[0]; i++)
        if (k_mark_addrs[i] == addr) return 1;
    return 0;
}

static uint32_t fnv1a(const uint8_t *p, uint32_t n) {
    uint32_t h = 2166136261u;
    for (uint32_t i = 0; i < n; i++) { h ^= p[i]; h *= 16777619u; }
    return h;
}

void platform_diag_tick(void) {
    if (g_kult_img && marks_recording()) {
        int sk = g_kult_img[A_SKIP_TRANSITION];
        if (sk != g_last_skip) {
            FILE *mf = marks_file();
            if (mf) {
                fprintf(mf, "K %u %d\n", platform_frame_count(), sk);
                fflush(mf);
            }
            g_last_skip = sk;
        }
    }
}

void platform_diag_func_begin(uint32_t addr, uint32_t vbl, int first) {
    if (!addr) return;
    if (first && marks_recording())
        ring_push(addr, vbl);              /* una vez por funcion y VBL */
    int curated = mark_wanted(addr);
    int win = vbl <= g_mark_until;
    if (curated || (win && first)) {
        FILE *mf = marks_file();
        if (mf) {
            fprintf(mf, "F %x %u\n", addr, vbl);
            fflush(mf);
        }
    }
}

void platform_diag_func_end(uint32_t addr, uint32_t vbl) {
    if (!g_kult_img) return;

    /* Diagnóstico de blits de elemento: dimensiones/fuente que recibe el
     * dispatcher. Si son enormes, el bucle de 12b40 se dispara y la partida
     * parece colgarse. */
    if (addr == 0x11f2e || addr == 0x12b04) {
        FILE *mf = marks_file();
        if (mf) {
            uint32_t selp = be32(g_kult_img + A_SELECTED_OBJECT);
            uint32_t clsp = be32(g_kult_img + A_ELEM_CLASS);
            fprintf(mf, "B %u fn=%x sel=%x cls=%x cmd=%04x "
                        "src=%x w=%04x h=%04x x=%04x y=%04x selb=",
                    vbl, addr, selp, clsp,
                    (unsigned)be16(g_kult_img + A_THE_COMMAND),
                    (unsigned)be32(g_kult_img + A_ELEM_1A528),
                    (unsigned)be16(g_kult_img + A_ELEM_1A52E),
                    (unsigned)be16(g_kult_img + A_ELEM_1A52C),
                    (unsigned)be16(g_kult_img + A_ELEM_1A532),
                    (unsigned)be16(g_kult_img + A_ELEM_1A530));
            if (selp >= 0x10000u && selp + 8u <= 0x100000u)
                for (uint32_t k = 0; k < 8u; k++)
                    fprintf(mf, "%02x", g_kult_img[selp + k]);
            else
                fprintf(mf, "-");
            fprintf(mf, " clsb=");
            if (clsp >= 0x10000u && clsp + 8u <= 0x100000u)
                for (uint32_t k = 0; k < 8u; k++)
                    fprintf(mf, "%02x", g_kult_img[clsp + k]);
            else
                fprintf(mf, "-");
            fprintf(mf, "\n");
            fflush(mf);
        }
    }

    /* Diagnóstico de acciones de objeto: puntero y bytes del objeto
     * seleccionado/clase. Sirve para saber por qué un objeto incorrecto llega
     * a un script de scale_prep. */
    if (addr == 0x17ca4 || addr == 0x13702) {
        FILE *mf = marks_file();
        if (mf) {
            uint32_t selp = be32(g_kult_img + A_SELECTED_OBJECT);
            uint32_t clsp = be32(g_kult_img + A_ELEM_CLASS);
            fprintf(mf, "%c %u sel=%x cls=%x cmd=%04x acc7=%02x "
                        "acd2=%02x a57a=%02x a57b=%02x ad26=%02x "
                        "acf7=%02x ad09=%02x",
                    addr == 0x17ca4 ? 'U' : 'P', vbl, selp, clsp,
                    (unsigned)be16(g_kult_img + A_THE_COMMAND),
                    g_kult_img[0x1acc7], g_kult_img[0x1acd2],
                    g_kult_img[0x1a57a], g_kult_img[0x1a57b],
                    g_kult_img[0x1ad26], g_kult_img[0x1acf7],
                    g_kult_img[0x1ad09]);
            if (selp >= 0x10000u && selp + 8u <= 0x100000u) {
                fprintf(mf, " selb=");
                for (uint32_t k = 0; k < 8u; k++)
                    fprintf(mf, "%02x", g_kult_img[selp + k]);
            }
            if (clsp >= 0x10000u && clsp + 8u <= 0x100000u) {
                fprintf(mf, " clsb=");
                for (uint32_t k = 0; k < 8u; k++)
                    fprintf(mf, "%02x", g_kult_img[clsp + k]);
            }
            fprintf(mf, "\n");
            fflush(mf);
        }
    }

    if (addr == 0x15da6) {
        uint16_t delay = (uint16_t)be16(g_kult_img + A_ANIM_1A470);
        uint32_t w = be16(g_kult_img + A_DAT_1A49E);
        uint32_t ht = be16(g_kult_img + A_ANIM_1A4A0);
        FILE *mf = marks_file();
        if (mf) {
            fprintf(mf, "S %u %u %u %u %u\n", vbl,
                    delay, w, ht, g_kult_img[A_ANIM_1A578]);
            fflush(mf);
        }
    }
}

void platform_diag_bank(uint32_t addr, uint32_t caller_addr) {
    FILE *mf = marks_file();
    if (!mf || !g_kult_img) return;
    uint32_t ptr = be32(g_kult_img + A_ANIM_SPR_PTR);
    uint32_t w = 0, h = 0, hash = 0;
    if (ptr && ptr + 0x640u <= 0x100000u) {
        w = g_kult_img[ptr];
        h = g_kult_img[ptr + 1];
        hash = fnv1a(g_kult_img + ptr, 0x640);
    }
    uint32_t cnt = (uint32_t)g_kult_img[0x1a57d] |
                   ((uint32_t)g_kult_img[0x1a57e] << 8) |
                   ((uint32_t)g_kult_img[0x1a57f] << 16) |
                   ((uint32_t)g_kult_img[0x1a580] << 24);
    fprintf(mf, "B %u %x %x %u %u %x %x %u\n",
            platform_frame_count(), addr, caller_addr,
            w, h, hash, cnt, g_kult_img[A_SKIP_TRANSITION]);
    fflush(mf);
}

/* Fichero de la traza R: KULT_RINGLOG si está definido; si no, el de KULT_MARKS.
 * Así se puede capturar el anillo sin generar el log masivo de F/Y/V. */
static FILE *g_ring_file;
static int g_ring_tried;

static FILE *ring_file(void) {
    if (!g_ring_tried) {
        g_ring_tried = 1;
        const char *e = getenv("KULT_RINGLOG");
        if (e && *e)
            g_ring_file = fopen(e, "a");
    }
    return g_ring_file ? g_ring_file : marks_file();
}

void platform_text_ev(const char *tag, uint8_t *img, uint32_t str,
                      uint32_t end, uint32_t width, uint32_t index) {
    FILE *mf = ring_file();
    if (!mf || !img) return;
    uint32_t vbl = platform_frame_count();
    g_last_text.vbl = vbl;
    g_last_text.str = str;
    g_last_text.end = end;
    g_last_text.width = width;
    g_last_text.index = index;
    snprintf(g_last_text.tag, sizeof g_last_text.tag, "%s", tag ? tag : "?");
    g_last_text.raw_len = 0;
    if (str < 0x100000u && end <= 0x100000u && end > str) {
        uint32_t n = end - str;
        if (n > sizeof g_last_text.raw) n = sizeof g_last_text.raw;
        memcpy(g_last_text.raw, img + str, n);
        g_last_text.raw_len = n;
    }
    fprintf(mf, "T %u %s idx=%04x w=%u str=%05x end=%05x cx=%04x cy=%04x "
                "raw=",
            vbl, tag ? tag : "?", (unsigned)index, (unsigned)width,
            (unsigned)str, (unsigned)end,
            (unsigned)be16(img + A_CURSOR_TEXT_X),
            (unsigned)be16(img + A_CURSOR_TEXT_Y));
    for (uint32_t i = 0; i < g_last_text.raw_len; i++)
        fprintf(mf, "%02x", g_last_text.raw[i]);
    fputc('\n', mf);
    fflush(mf);
}

/* Traza del anillo de acciones. Escribe si KULT_RINGLOG o KULT_MARKS están activos. */
void platform_diag_ev(const char *tag, uint8_t *img,
                      uint32_t a, uint32_t b, uint32_t c) {
    FILE *mf = ring_file();
    if (!mf || !img) return;
    uint32_t sel = be32(img + A_SELECTED_OBJECT);
    uint32_t cls = be32(img + A_ELEM_CLASS);
    fprintf(mf,
            "R %u %s a=%05x b=%05x c=%05x cr2=%05x abf8=%04x "
            "ace7=%02x acf2=%02x ace8=%02x ad09=%02x w19316=%04x "
            "a826=%05x hp=%02x ad03=%02x ad02=%02x rnd=%02x/%02x "
            "sel=%06x cls=%06x",
            platform_frame_count(), tag ? tag : "?", (unsigned)a, (unsigned)b,
            (unsigned)c, (unsigned)be32(img + 0x1a4c2),
            (unsigned)be16(img + A_DAT_1ABF8), (unsigned)img[A_DAT_1ACE7],
            (unsigned)img[A_DAT_1ACF2], (unsigned)img[A_DAT_1ACE8],
            (unsigned)img[A_DAT_1AD09], (unsigned)be16(img + 0x19316),
            (unsigned)be32(img + A_RUN_STACK), (unsigned)img[0x1ad20],
            (unsigned)img[0x1ad03], (unsigned)img[0x1ad02],
            (unsigned)img[A_DAT_1A579], (unsigned)img[A_DAT_1ACD7],
            (unsigned)sel, (unsigned)cls);
    if (sel >= 0x10000u && sel + 8u <= 0x100000u) {
        fprintf(mf, " selb=");
        for (uint32_t k = 0; k < 8u; k++)
            fprintf(mf, "%02x", img[sel + k]);
    }
    if (cls >= 0x10000u && cls + 8u <= 0x100000u) {
        fprintf(mf, " clsb=");
        for (uint32_t k = 0; k < 8u; k++)
            fprintf(mf, "%02x", img[cls + k]);
    }
    fputc('\n', mf);
    fflush(mf);
}

/* Traza de lecturas de ALEAT (0x2741e). Solo escribe con KULT_RANDLOG. */
static FILE *g_rand_file;
static int g_rand_tried;

static FILE *rand_file(void) {
    if (!g_rand_tried) {
        g_rand_tried = 1;
        const char *e = getenv("KULT_RANDLOG");
        if (e && *e)
            g_rand_file = fopen(e, "a");
    }
    return g_rand_file;
}

void platform_diag_rnd(const char *tag, uint8_t *img,
                       uint32_t a, uint32_t b, uint32_t c) {
    FILE *mf = rand_file();
    if (!mf || !img) return;
    uint32_t sel = be32(img + A_SELECTED_OBJECT);
    uint32_t cls = be32(img + A_ELEM_CLASS);
    fprintf(mf,
            "D %u %s a=%02x b=%04x c=%04x idx=%02x val=%02x "
            "hp=%02x ad03=%02x ad02=%02x ad09=%02x cmd=%04x "
            "sel=%06x cls=%06x",
            platform_frame_count(), tag ? tag : "?", (unsigned)a, (unsigned)b,
            (unsigned)c, (unsigned)img[A_DAT_1A579], (unsigned)img[A_DAT_1ACD7],
            (unsigned)img[0x1ad20], (unsigned)img[0x1ad03],
            (unsigned)img[0x1ad02], (unsigned)img[A_DAT_1AD09],
            (unsigned)be16(img + A_THE_COMMAND), (unsigned)sel, (unsigned)cls);
    if (sel >= 0x10000u && sel + 8u <= 0x100000u) {
        fprintf(mf, " selb=");
        for (uint32_t k = 0; k < 8u; k++)
            fprintf(mf, "%02x", img[sel + k]);
    }
    if (cls >= 0x10000u && cls + 8u <= 0x100000u) {
        fprintf(mf, " clsb=");
        for (uint32_t k = 0; k < 8u; k++)
            fprintf(mf, "%02x", img[cls + k]);
    }
    fputc('\n', mf);
    fflush(mf);
}

/* Traza de entrada (clics). Solo escribe con KULT_INPUTLOG. */
static FILE *g_input_file;
static int g_input_tried;

static FILE *input_file(void) {
    if (!g_input_tried) {
        g_input_tried = 1;
        const char *e = getenv("KULT_INPUTLOG");
        if (e && *e)
            g_input_file = fopen(e, "a");
    }
    return g_input_file;
}

void platform_diag_input(const char *tag, uint8_t *img,
                         uint32_t x, uint32_t y, uint32_t buttons) {
    FILE *mf = input_file();
    if (!mf) return;
    fprintf(mf, "I %u %s x=%04x y=%04x btn=%02x",
            platform_frame_count(), tag ? tag : "?", (unsigned)x,
            (unsigned)y, (unsigned)buttons);
    if (img) {
        uint32_t sel = be32(img + A_SELECTED_OBJECT);
        uint32_t cls = be32(img + A_ELEM_CLASS);
        fprintf(mf,
                " hp=%02x ad03=%02x ad02=%02x ad09=%02x cmd=%04x "
                "sel=%06x cls=%06x",
                (unsigned)img[0x1ad20], (unsigned)img[0x1ad03],
                (unsigned)img[0x1ad02], (unsigned)img[A_DAT_1AD09],
                (unsigned)be16(img + A_THE_COMMAND), (unsigned)sel, (unsigned)cls);
        if (sel >= 0x10000u && sel + 8u <= 0x100000u) {
            fprintf(mf, " selb=");
            for (uint32_t k = 0; k < 8u; k++)
                fprintf(mf, "%02x", img[sel + k]);
        }
        if (cls >= 0x10000u && cls + 8u <= 0x100000u) {
            fprintf(mf, " clsb=");
            for (uint32_t k = 0; k < 8u; k++)
                fprintf(mf, "%02x", img[cls + k]);
        }
    }
    fputc('\n', mf);
    fflush(mf);
}

void platform_marks_line(const char *fmt, ...) {
    FILE *mf = marks_file();
    if (!mf) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(mf, fmt, ap);
    va_end(ap);
    fputc('\n', mf);
    fflush(mf);
}

void platform_marks_byte(uint32_t vbl, uint32_t b, uint32_t script) {
    FILE *mf = marks_file();
    if (!mf) return;
    fprintf(mf, "Y %u %02x %05x\n", vbl, (unsigned)(b & 0xff),
            (unsigned)script);
    fflush(mf);
}

void platform_marks_state(uint32_t vbl, uint32_t b, uint32_t script,
                          uint32_t acc9, uint32_t acd2, uint32_t aaf9,
                          uint32_t ad1d, uint32_t acc6, uint32_t skip,
                          uint32_t a572, uint32_t a0f, uint32_t cx,
                          uint32_t cy, uint32_t btn, uint32_t cmd,
                          uint32_t selobj) {
    FILE *mf = marks_file();
    if (!mf) return;
    fprintf(mf, "V %u %02x %05x %02x %02x %02x %02x %02x %02x %02x %02x "
                "%04x %04x %02x %04x %05x\n",
            vbl, (unsigned)(b & 0xff), (unsigned)script,
            (unsigned)(acc9 & 0xff), (unsigned)(acd2 & 0xff),
            (unsigned)(aaf9 & 0xff), (unsigned)(ad1d & 0xff),
            (unsigned)(acc6 & 0xff), (unsigned)(skip & 0xff),
            (unsigned)(a572 & 0xff), (unsigned)(a0f & 0xff),
            (unsigned)(cx & 0xffff), (unsigned)(cy & 0xffff),
            (unsigned)(btn & 0xff), (unsigned)(cmd & 0xffff),
            (unsigned)(selobj & 0xffffff));
    fflush(mf);
}

void platform_marks_event(const char *tag, int a, int b) {
    FILE *mf = marks_file();
    if (!mf) return;
    uint32_t vbl = platform_frame_count();
    /* Volcar los ~150 VBLs anteriores del buffer (3 s) ANTES de la marca:
     * asi F12 puede pulsarse despues de una animacion corta. */
    if (tag && (strcmp(tag, "f12") == 0 || strcmp(tag, "auto") == 0) &&
        g_ring && g_ring_count) {
        uint32_t cutoff = (vbl > 150u) ? vbl - 150u : 0u;
        uint32_t start = (g_ring_count == MARK_RING) ? g_ring_pos : 0u;
        for (uint32_t i = 0; i < g_ring_count; i++) {
            uint32_t k = (start + i) & (MARK_RING - 1);
            if (g_ring[k].vbl >= cutoff)
                fprintf(mf, "F %x %u\n", g_ring[k].addr, g_ring[k].vbl);
        }
    }
    if (tag && (strcmp(tag, "f12") == 0 || strcmp(tag, "auto") == 0) &&
        g_last_text.raw_len) {
        fprintf(mf, "X %s %u idx=%04x w=%u str=%05x end=%05x raw=",
                g_last_text.tag, g_last_text.vbl, (unsigned)g_last_text.index,
                (unsigned)g_last_text.width, (unsigned)g_last_text.str,
                (unsigned)g_last_text.end);
        for (uint32_t i = 0; i < g_last_text.raw_len; i++)
            fprintf(mf, "%02x", g_last_text.raw[i]);
        fputc('\n', mf);
    }
    fprintf(mf, "M %s %u %d %d\n", tag ? tag : "?", vbl, a, b);
    fflush(mf);
    /* F12: y durante ~1,5 s se sigue trazando todo (una vez por funcion/VBL). */
    if (tag && strcmp(tag, "f12") == 0)
        g_mark_until = vbl + 75;
}
