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
 *
 * El diagnóstico activable por entorno vive en platform/diag.c. Aquí solo se
 * conserva la resolución función -> dirección ST y los puntos de entrada/salida
 * de la instrumentación; el timing y los hooks de HAL no salen de este módulo.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "machine.h"
#include "addrs.h"
#include "platform.h"
#include "boot.h"
#include "diag.h"

extern uint8_t *g_kult_img;
extern void wait_frame(void);   /* lib/kult-core/src/input.c: escribe $1a570 (skip) */
extern void reset_clear(void);  /* lib/kult-core/src/fileio.c: 0x1682a (bra 0x10158) */
extern void reveal_line(void);  /* lib/kult-core/src/line.c: 0x13d0e (coste por longitud) */
extern void anim_engine_entry(void); /* 0x15da6: ajuste opcional KULT_15DA6_BLIT */

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
    uint32_t last;     /* ultimo VBL: una marca F12 por funcion y VBL */
} g_hash[CPUWORK_HASH];
static int g_ready;

#define CALLSTACK 128
static const void *g_stack[CALLSTACK];
static int g_depth;

__attribute__((no_instrument_function)) static uint32_t hash_of(const void *p);
__attribute__((no_instrument_function)) static void build(void);

/* Funciones sin coste (las excluye la tabla) pero que queremos mapear para las
 * marcas de animación de platform/diag.c. */
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
static uint32_t hash_of(const void *p) {
    uintptr_t v = (uintptr_t)p >> 4;
    return (uint32_t)((v ^ (v >> 17)) & (CPUWORK_HASH - 1));
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
    /* Todas las funciones mapeadas: tambien sirven a la ventana F12. */
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
    platform_diag_tick();
    uint32_t h = hash_of(this_fn);
    for (;;) {
        if (!g_hash[h].fn) return;
        if (g_hash[h].fn == this_fn) {
            uint32_t vbl = platform_frame_count();
            uint32_t addr = g_hash[h].addr;
            int first = (g_hash[h].last != vbl);
            if (addr && first)
                g_hash[h].last = vbl;
            platform_diag_func_begin(addr, vbl, first);
            if (g_hash[h].cost)
                platform_cpu_charge_cycles(g_hash[h].cost);
            /* 0x13d0e (`reveal_line`) no tiene coste fijo: recorre la recta
             * (0x1a516,0x1a51a)->(0x1a518,0x1a51c). Medido con el oráculo en
             * seis rectas y en el conjunto de `room_reset` (304 líneas,
             * 1392 pasos, 596.918 ciclos): ajuste ponderado
             * ciclos ≈ 1063 + 198 * pasos, con pasos = max(|dx|,|dy|)+1.
             * Cobrar aquí (no en `room_reset`, que solo envuelve 16 llamadas
             * por pasada) reparte el retardo entre las líneas, que es como lo
             * gasta el original. */
            if (this_fn == (const void *)reveal_line && g_kult_img) {
                int32_t x0 = (int16_t)be16(g_kult_img + 0x1a516);
                int32_t y0 = (int16_t)be16(g_kult_img + 0x1a51a);
                int32_t x1 = (int16_t)be16(g_kult_img + 0x1a518);
                int32_t y1 = (int16_t)be16(g_kult_img + 0x1a51c);
                int32_t dx = x1 - x0; if (dx < 0) dx = -dx;
                int32_t dy = y1 - y0; if (dy < 0) dy = -dy;
                uint32_t steps = (uint32_t)((dx > dy ? dx : dy) + 1);
                platform_cpu_charge_cycles(1063u + 198u * steps);
            }
            platform_diag_func_end(addr, vbl);
            /* Maestro/retrato con retardo 0: el `dbf` de 15da6 se salta, pero
             * el blit interno sigue costando CPU en el ST. Se cobra aparte
             * (KULT_15DA6_BLIT ciclos; **def. 0 = desactivado**). Las demas llamadas
             * con retardo > 0 ya van paced por cpu_delay. */
            if (this_fn == (const void *)anim_engine_entry && g_kult_img &&
                be16(g_kult_img + A_ANIM_1A470) == 0) {
                static long extra = -1;
                if (extra < 0) {
                    const char *e = getenv("KULT_15DA6_BLIT");
                    extra = (e && *e) ? strtol(e, NULL, 0) : 0;
                }
                if (extra > 0)
                    platform_cpu_charge_cycles((uint64_t)extra);
            }
            return;
        }
        h = (h + 1) & (CPUWORK_HASH - 1);
    }
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
            platform_diag_bank(lookup_addr(this_fn), lookup_addr(caller));
        /* HAL: el core `wait_frame` acaba de escribir $1a570 desde el bit0 del
         * IKBD. Lo pisamos con el flanco del botón derecho del paquete (el
         * izquierdo es acción sin skip), como el intercept de $1a570 de la
         * referencia. */
        if (this_fn == (void *)wait_frame && g_kult_img)
            platform_input_apply_skip(g_kult_img);
        /* El original termina reset_clear con `bra 0x10158` (inicio del bucle).
         * En C nativo no hay salto: lo emulamos pidiendo el reinicio al boot. */
        if (this_fn == (void *)reset_clear)
            kult_request_restart();
        g_depth--;
    }
}
