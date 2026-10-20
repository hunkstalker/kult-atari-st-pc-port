/* platform/cpuwork.c — coste de CPU medido por función (T-073b).
 *
 * El core ejecuta el mismo grafo de llamadas que el original, pero en C. Donde el
 * 68000 tardaba ciclos, aquí se tarda microsegundos. Este módulo cobra a cada
 * llamada el coste medido del original (ciclos/llamada, del oráculo) y lo pasa a
 * `platform_cpu_charge_cycles`, que lo duerme contra el reloj de CPU de la HAL y
 * cede VBLs (misma puerta que `cpu_delay`).
 *
 * Solo se instrumentan las fuentes del core (src) en el build del port con
 * `-finstrument-functions`; el diferencial y `make test` no. Sin tiempo real /
 * headless, `platform_cpu_charge_cycles` no hace nada: la imagen no cambia.
 *
 * Tabla: `platform/cpuwork_costs.inc`, generada por tools/gen_cpuwork_costs.py a
 * partir de la medida del oráculo (referencia del predecesor).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"
#include "addrs.h"
#include "platform.h"
#include "boot.h"

extern uint8_t *g_kult_img;
extern void wait_frame(void);   /* src/input.c: escribe $1a570 (skip) */
extern void reset_clear(void);  /* src/fileio.c: 0x1682a (bra 0x10158) */

typedef struct {
    void *fn;
    uint32_t addr;   /* dirección ST (diagnóstico) */
    uint32_t cost;   /* ciclos del original por llamada */
} cpuwork_sym;

#include "cpuwork_costs.inc"

#define CPUWORK_HASH 2048

static struct {
    const void *fn;
    uint32_t addr;
    uint32_t cost;
    uint32_t last;     /* ultimo VBL registrado en la ventana F12 */
} g_hash[CPUWORK_HASH];
static int g_ready;
static uint32_t g_mark_until;

#define CALLSTACK 128
static const void *g_stack[CALLSTACK];
static int g_depth;
static int g_last_skip = -1;

__attribute__((no_instrument_function)) static uint32_t hash_of(const void *p);
__attribute__((no_instrument_function)) static void build(void);    /* ventana de traza completa tras F12 */

/* Buffer circular de las ultimas entradas (funcion, VBL) para que F12 pueda
 * volcar los ~3 s ANTERIORES aunque se pulse despues de la animacion. */
#define MARK_RING (1u << 18)
static struct { uint32_t addr, vbl; } g_ring[MARK_RING];
static uint32_t g_ring_pos, g_ring_count;

static int marks_recording(void) {
    static int on = -1;
    if (on < 0) { const char *e = getenv("KULT_MARKS"); on = (e && *e) ? 1 : 0; }
    return on;
}

static void ring_push(uint32_t addr, uint32_t vbl) {
    g_ring[g_ring_pos].addr = addr;
    g_ring[g_ring_pos].vbl = vbl;
    g_ring_pos = (g_ring_pos + 1) & (MARK_RING - 1);
    if (g_ring_count < MARK_RING) g_ring_count++;
}

/* Funciones sin coste (las excluye la tabla) pero que queremos marcar para
 * medir duraciones de animación (T-073c). */
extern void anim_engine_entry(void);
extern void anim_bank(void);
extern void anim_bank_build(void);
extern void reveal_region(void);
extern void reveal_pixel(void);
extern void scale_anim(void);
extern void scale_loop(void);
extern void view_mode_cycle(void);
extern void reveal_box_a(void);
extern void reveal_box_b(void);

static const cpuwork_sym k_mark_syms[] = {
    { (void *)anim_engine_entry, 0x15da6, 0 },
    { (void *)reveal_region,      0x12e2e, 0 },
    { (void *)reveal_pixel,       0x12f28, 0 },
    { (void *)scale_anim,         0x138a6, 0 },
    { (void *)scale_loop,         0x13740, 0 },
    { (void *)view_mode_cycle,    0x12ce4, 0 },
    { (void *)reveal_box_a,       0x12e08, 0 },
    { (void *)reveal_box_b,       0x12ea2, 0 },
};

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

__attribute__((no_instrument_function))
static uint32_t lookup_addr(const void *fn) {
    if (!fn) return 0;
    if (!g_ready) build();
    uint32_t h = hash_of(fn);
    while (g_hash[h].fn) {
        if (g_hash[h].fn == fn) return g_hash[h].addr;
        h = (h + 1) & (CPUWORK_HASH - 1);
    }
    return 0;
}

__attribute__((no_instrument_function))
static uint32_t fnv1a(const uint8_t *p, uint32_t n) {
    uint32_t h = 2166136261u;
    for (uint32_t i = 0; i < n; i++) { h ^= p[i]; h *= 16777619u; }
    return h;
}

static FILE *marks_file(void) {
    static FILE *f = (FILE *)-1;
    if (f == (FILE *)-1) {
        const char *e = getenv("KULT_MARKS");
        f = (e && *e) ? fopen(e, "a") : NULL;
        if (f) {
            fprintf(f, "# RUN\n");
            fflush(f);
        }
    }
    return f;
}

__attribute__((no_instrument_function))
static uint32_t hash_of(const void *p) {
    uintptr_t v = (uintptr_t)p >> 4;
    return (uint32_t)((v ^ (v >> 17)) & (CPUWORK_HASH - 1));
}

__attribute__((no_instrument_function))
__attribute__((no_instrument_function))
static void bank_mark(void *fn, const void *caller) {
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
            platform_frame_count(), lookup_addr(fn), lookup_addr(caller),
            w, h, hash, cnt, g_kult_img[A_SKIP_TRANSITION]);
    fflush(mf);
}

__attribute__((no_instrument_function))
static void hash_put(const void *fn, uint32_t addr, uint32_t cost) {
    if (!fn) return;
    uint32_t h = hash_of(fn);
    while (g_hash[h].fn) {
        if (g_hash[h].fn == fn) {              /* no pisar el coste medido */
            if (!g_hash[h].addr) g_hash[h].addr = addr;
            return;
        }
        h = (h + 1) & (CPUWORK_HASH - 1);
    }
    g_hash[h].fn = fn;
    g_hash[h].addr = addr;
    g_hash[h].cost = cost;
    g_hash[h].last = 0xffffffffu;
}

__attribute__((no_instrument_function))
static void build(void) {
    for (unsigned i = 0; i < k_cpuwork_ncosts; i++)
        hash_put(k_cpuwork_costs[i].fn, k_cpuwork_costs[i].addr,
                 k_cpuwork_costs[i].cost);
    for (unsigned i = 0; i < sizeof k_mark_syms / sizeof k_mark_syms[0]; i++)
        hash_put(k_mark_syms[i].fn, k_mark_syms[i].addr, 0);
    /* Todas las funciones mapeadas: solo para trazar en la ventana F12. */
    for (unsigned i = 0; i < k_cpuwork_nmarks; i++)
        hash_put(k_cpuwork_marks[i].fn, k_cpuwork_marks[i].addr, 0);
    g_ready = 1;
}

__attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    (void)call_site;
    if (g_depth < CALLSTACK) g_stack[g_depth] = this_fn;
    g_depth++;
    if (!g_ready) build();
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
    uint32_t h = hash_of(this_fn);
    for (;;) {
        if (!g_hash[h].fn) return;
        if (g_hash[h].fn == this_fn) {
            uint32_t vbl = platform_frame_count();
            uint32_t addr = g_hash[h].addr;
            int first = (g_hash[h].last != vbl);
            if (addr && first) {
                g_hash[h].last = vbl;
                if (marks_recording())
                    ring_push(addr, vbl);      /* una vez por funcion y VBL */
            }
            int curated = addr && mark_wanted(addr);
            int win = vbl <= g_mark_until;
            if (addr && (curated || (win && first))) {
                FILE *mf = marks_file();
                if (mf) {
                    fprintf(mf, "F %x %u\n", addr, vbl);
                    fflush(mf);
                }
            }
            if (g_hash[h].cost)
                platform_cpu_charge_cycles(g_hash[h].cost);
            /* Maestro/retrato con retardo 0: el `dbf` de 15da6 se salta, pero
             * el blit interno sigue costando CPU en el ST. Se cobra aparte
             * (KULT_15DA6_BLIT ciclos; **def. 0 = desactivado**). Las demas llamadas
             * con retardo > 0 ya van paced por cpu_delay. */
            if (g_hash[h].fn == (const void *)anim_engine_entry && g_kult_img) {
                uint16_t delay = (uint16_t)be16(g_kult_img + A_ANIM_1A470);
                uint32_t w = be16(g_kult_img + A_DAT_1A49E);
                uint32_t ht = be16(g_kult_img + A_ANIM_1A4A0);
                FILE *mf = marks_file();
                if (mf) {
                    fprintf(mf, "S %u %u %u %u %u\n", platform_frame_count(),
                            delay, w, ht, g_kult_img[A_ANIM_1A578]);
                    fflush(mf);
                }
                if (delay == 0) {
                    static long extra = -1;
                    if (extra < 0) {
                        const char *e = getenv("KULT_15DA6_BLIT");
                        extra = (e && *e) ? strtol(e, NULL, 0) : 0;
                    }
                    if (extra > 0)
                        platform_cpu_charge_cycles((uint64_t)extra);
                }
            }
            return;
        }
        h = (h + 1) & (CPUWORK_HASH - 1);
    }
}

__attribute__((no_instrument_function))
void platform_marks_byte(uint32_t vbl, uint32_t b, uint32_t script) {
    FILE *mf = marks_file();
    if (!mf) return;
    fprintf(mf, "Y %u %02x %05x\n", vbl, (unsigned)(b & 0xff),
            (unsigned)script);
    fflush(mf);
}

__attribute__((no_instrument_function))
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
    if (tag && strcmp(tag, "f12") == 0 && g_ring_count) {
        uint32_t cutoff = (vbl > 150u) ? vbl - 150u : 0u;
        uint32_t start = (g_ring_count == MARK_RING) ? g_ring_pos : 0u;
        for (uint32_t i = 0; i < g_ring_count; i++) {
            uint32_t k = (start + i) & (MARK_RING - 1);
            if (g_ring[k].vbl >= cutoff)
                fprintf(mf, "F %x %u\n", g_ring[k].addr, g_ring[k].vbl);
        }
    }
    fprintf(mf, "M %s %u %d %d\n", tag ? tag : "?", vbl, a, b);
    fflush(mf);
    /* F12: y durante ~1,5 s se sigue trazando todo (una vez por funcion/VBL). */
    if (tag && strcmp(tag, "f12") == 0)
        g_mark_until = vbl + 75;
}

__attribute__((no_instrument_function))
/* El reinicio de `reset_clear` sale por longjmp; la pila de instrumentación no
 * se desenrolla. El boot la vacía antes del salto. */
void platform_cpuwork_reset_stack(void) {
    g_depth = 0;
}

void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    (void)call_site;
    if (g_depth > 0) {
        const void *caller = (g_depth >= 2 && g_depth - 2 < CALLSTACK)
                             ? g_stack[g_depth - 2] : 0;
        if (this_fn == (void *)anim_bank || this_fn == (void *)anim_bank_build)
            bank_mark(this_fn, caller);
        /* HAL: el core `wait_frame` acaba de escribir $1a570 desde el bit0 del
         * IKBD. Lo pisamos con el botón derecho (izquierdo = acción sin skip),
         * igual que el intercept de $1a570 del motor legacy. */
        if (this_fn == (void *)wait_frame && g_kult_img)
            platform_input_apply_skip(g_kult_img);
        /* El original termina reset_clear con `bra 0x10158` (inicio del bucle).
         * En C nativo no hay salto: lo emulamos pidiendo el reinicio al boot. */
        if (this_fn == (void *)reset_clear)
            kult_request_restart();
        g_depth--;
    }
}
