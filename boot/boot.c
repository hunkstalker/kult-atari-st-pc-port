/* boot/boot.c — reconstrucción de `_start` (0x10000) para el port PC.
 *
 * Traduce `matching/src/start.s` a llamadas al C verificado. El único trozo sin
 * equivalente C es 0x135ba (XBIOS Setscreen(-1,-1,0)), que en PC no cambia la base
 * y por eso es un no-op. El bucle de espera de click del original se satisface
 * porque en headless no hay click (bit 2 a 0) y el bucle da una sola pasada.
 *
 * El bucle principal es infinito; se corta con el hook del motor + longjmp para
 * volcar el framebuffer tras N frames.
 */
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"
#include "addrs.h"
#include "kult_api.h"
#include "boot.h"

/* Hook del motor (definido en src/loop.c): se llama al inicio de cada iteración. */
extern void (*hook_loop_top)(uint8_t *img);

static jmp_buf g_jmp;
static uint32_t g_frame, g_max;

static int boot_dbg(void) {
    static int d = -1;
    if (d < 0) { const char *e = getenv("KULT_DEBUG"); d = (e && *e) ? 1 : 0; }
    return d;
}
#define BOOT_TRACE(step) do { if (boot_dbg()) fprintf(stderr, "[boot] %s\n", step); } while (0)

static void frame_hook(uint8_t *img) {
    (void)img;
    if (g_max && ++g_frame >= g_max)
        longjmp(g_jmp, 1);
}

/* Lee una tecla válida de idioma (D/E/F) del menú. */
static uint8_t boot_language(uint8_t *img) {
    for (;;) {
        uint32_t key = gemdos_cnecin(img) & 0xff;
        if (key >= 'a' && key <= 'z') key -= 0x20;
        if (key >= 'D' && key < 'G') return (uint8_t)key;
    }
}

int kult_boot(uint8_t *img, uint32_t max_frames) {
    /* Siembra del patrón del FDC (0x19316 se omite): el modo de escena (0x14496)
     * consulta $d8eb == 0x52 para salir de su bucle; el patrón vive en el PRG en
     * 0x10000+0x9484 y se copia a $d8e8 en bloques de 0x18 (0x46 veces). */
    for (int i = 0; i < 0x46; i++)
        memcpy(img + 0xd8e8 + i * 0x18, img + 0x10000 + 0x9484, 0x18);

    crt0_init(img);
    BOOT_TRACE("crt0");
    (void)init_hardware(img, 0);
    BOOT_TRACE("init_hardware");

    /* Camino "sin DRAP" (como el producto del predecesor): salta PRES/DRAP/
     * prompt_wait/idioma y va directo a fijar la letra en los nombres de recurso.
     * Útil para bring-up headless y para la versión de un solo idioma. */
    uint8_t lang;
    int skip_drap = getenv("KULT_SKIP_DRAP") != 0;
    if (!skip_drap) {
        /* PRES.PI1 -> pantalla B -> copia a A */
        wr32(img + A_CUR_FILENAME, 0x1a427);
        load_named_screen(img);
        /* 0x135ba: XBIOS Setscreen(-1,-1,0) — no cambia la base; no-op en PC. */
        copy_screen(img);
        BOOT_TRACE("pres");

        /* DRAP.PI1 -> pantalla B; espera; paleta; swap (queda delante) */
        wr32(img + A_CUR_FILENAME, 0x1a430);
        load_named_screen(img);
        /* Bring-up headless: prompt_wait espera un CLIC en la pantalla de título.
         * En M1 no hay entrada real; con mode_flag == 0x87 (sin overlay) prompt_wait
         * sale por Cnecin (auto-tecla) en vez de esperar el clic. M3 lo sustituye por
         * entrada real. */
        img[A_MODE_FLAG] = 0x87;
        prompt_wait(img);
        BOOT_TRACE("pres/drap/menu");
        set_palette(img);
        swap_screens(img);
        lang = boot_language(img);
    } else {
        lang = 'E';
    }

    /* Menú de idioma: escribe la letra en las 4 entradas de la lista A */
    img[0x1a3aa + 4]   = lang;
    img[0x1a3aa + 0x12] = lang;
    img[0x1a3aa + 0x20] = lang;
    img[0x1a3aa + 0x2e] = lang;
    copy_screen(img);
    BOOT_TRACE("lang");

    (void)load_resource_list_a(img);
    BOOT_TRACE("lista_a");
    wr16(img + A_THE_COMMAND, 0x918e);
    run_command_keep_sp(img, 0);

    /* Bucle de carga original (l_0100ec) */
    (void)load_fond(img);
    BOOT_TRACE("fond");
    for (int i = 0; i < 8; i++) {
        if (load_resource_list_b(img) >= 0)
            break;
        wr16(img + A_THE_COMMAND, 0x918e);
        run_command_keep_sp(img, 0);
    }

    /* Bancos ECHE 0..7 contiguos desde 0x64c8e (l_01011a): carga cada banco y
     * guarda su puntero en la tabla 0x1a4e8. */
    if (be32(img + A_SCREEN_PTR_A) >= 0x78001u) {
        uint32_t dst = 0x64c8eu;
        for (uint32_t d = 0; d <= 7; d++) {
            wr32(img + 0x1a4e8 + d * 4, dst);
            load_eche_bank(img, d, dst);
            dst += be32(img + A_LOAD_LENGTH);
        }
        img[0x1a57c] = 0xff;
    }
    BOOT_TRACE("eche");

    set_palette(img);
    copy_screen(img);
    save_clear_bin(img);
    music_stop(img, 0);
    sound_stop_supexec(img);
    BOOT_TRACE("clear/music");

    /* l_010158: espera de click; en headless el clic alterno la libera. */
    wait_frame(img);
    BOOT_TRACE("wait_frame l_010158");

    /* M1: el camino hasta aquí ya carga sala/fond/hints y deja un framebuffer
     * coherente. El comando 0xc001 y el bucle principal dependen del tick de VBL
     * 0x160c2 (M2: addq + rotación de paleta + blits MURSM/TEMPL), sin el cual la
     * plantilla despacha a un índice de glifo inválido. Con KULT_RUN_LOOP=1 se
     * intenta igualmente. */
    if (!getenv("KULT_RUN_LOOP")) {
        BOOT_TRACE("stop antes de c001 (M1; el tick 0x160c2 es M2)");
        return 0;
    }

    img[A_MODE_FLAG] = 7;
    img[A_DAT_1AD0D] = 0;
    wr16(img + A_THE_COMMAND, 0xc001);
    run_command_keep_sp(img, 0);
    BOOT_TRACE("run_command c001");
    draw_hint_icons(img);
    BOOT_TRACE("hint_icons");
    wait_frame_cursor(img);
    BOOT_TRACE("entrando game_loop");

    /* Bucle principal, con corte por número de frames. */
    g_frame = 0;
    g_max = max_frames;
    hook_loop_top = frame_hook;
    if (setjmp(g_jmp) == 0)
        game_loop(img, 0);
    hook_loop_top = 0;
    return 0;
}
