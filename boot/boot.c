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
#include "os.h"
#include "platform.h"
#include "kult_api.h"
#include "boot.h"

/* Hook del motor (definido en src/loop.c): se llama al inicio de cada iteración. */
extern void (*hook_loop_top)(uint8_t *img);

static jmp_buf g_jmp;
static uint32_t g_frame, g_max;
static void (*g_frame_cb)(uint8_t *img);
static int g_stop;
static int g_restart_armed;    /* 1 = dentro de game_loop; reset_clear puede reiniciar */

void kult_set_frame_cb(void (*fn)(uint8_t *img)) { g_frame_cb = fn; }
void kult_request_stop(void) { g_stop = 1; }

static int boot_dbg(void) {
    static int d = -1;
    if (d < 0) { const char *e = getenv("KULT_DEBUG"); d = (e && *e) ? 1 : 0; }
    return d;
}
#define BOOT_TRACE(step) do { if (boot_dbg()) fprintf(stderr, "[boot] %s\n", step); } while (0)

static void frame_hook(uint8_t *img) {
    if (g_frame_cb)
        g_frame_cb(img);
    /* Diagnóstico: fuerza un reinicio desde 0x10158 en un VBL concreto. */
    { static long at = -2; static int done;
      if (at == -2) { const char *e = getenv("KULT_RESTART_AT");
                      at = (e && *e) ? strtol(e, NULL, 0) : -1; }
      if (!done && at >= 0 && (long)platform_frame_count() >= at) {
          done = 1;
          kult_request_restart();
      } }
    /* Diagnóstico: fuerza l_0101a8 (reinicio suave) en un VBL concreto. */
    { static long at = -2; static int done;
      if (at == -2) { const char *e = getenv("KULT_SOFT_RESTART_AT");
                      at = (e && *e) ? strtol(e, NULL, 0) : -1; }
      if (!done && at >= 0 && (long)platform_frame_count() >= at) {
          done = 1;
          platform_log("reinicio suave: KULT_SOFT_RESTART_AT=%ld", at);
          longjmp(g_jmp, 3);
      } }
    if (g_stop)
        longjmp(g_jmp, 1);
    if (g_max && ++g_frame >= g_max)
        longjmp(g_jmp, 1);
}

/* Equivalente al `bra 0x10158` que el original ejecuta al final de reset_clear
 * (0x1682a): vuelve a arrancar el bucle de juego desde el Maestro/objeto. */
void kult_request_restart(void) {
    if (!g_restart_armed)
        return;
    g_restart_armed = 0;
    platform_cpuwork_reset_stack();
    platform_log("reinicio: reset_clear -> 0x10158");
    longjmp(g_jmp, 2);
}

/* l_010158 del original: espera a soltar el clic, fija el modo 7 y lanza el
 * comando 0xc001 (Maestro/objeto) antes de entrar en game_loop. */
static void boot_enter_game(uint8_t *img) {
    do {
        wait_frame(img);
    } while (img[A_INPUT_BUTTONS] & 0x04);
    BOOT_TRACE("wait_frame l_010158");

    img[A_MODE_FLAG] = 7;
    img[A_DAT_1AD0D] = 0;
    wr16(img + A_THE_COMMAND, 0xc001);
    run_command_keep_sp(img, 0);
    BOOT_TRACE("run_command c001");
    draw_hint_icons(img);
    BOOT_TRACE("hint_icons");
    wait_frame_cursor(img);
    BOOT_TRACE("entrando game_loop");
}

/* l_0100ec del original: FOND, lista B, bancos ECHE, paleta, copia a pantalla A,
 * CLEAR y parada de música/sonido. El aviso de cambio al disco 2 (0x918e) se
 * omite: el port sirve ambos discos desde el mismo directorio y el preflight de
 * app/main.c ya lista los ficheros que faltan. Devuelve <0 si FOND o la lista B
 * no cargan (mismo error que el arranque en frío). */
static int boot_load_0100ec(uint8_t *img) {
    load_fond(img);
    if ((int16_t)be16(img + A_FILE_HANDLE) < 0) {
        BOOT_TRACE("fond falló");
        return -1;
    }
    BOOT_TRACE("fond");
    if (load_resource_list_b(img) < 0) {
        BOOT_TRACE("lista_b falló");
        return -1;
    }
    BOOT_TRACE("lista_b");

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

    platform_disk_timing(1);       /* desde aquí sí: salas, estrella, reinicio */
    set_palette(img);
    copy_screen(img);
    save_clear_bin(img);
    music_stop(img, 0);
    sound_stop_supexec(img);
    BOOT_TRACE("clear/music");
    return 0;
}

/* l_0101a8 del original: reinicio suave. Carga la lista C, rehace crt0, recarga
 * la lista A y salta a l_0100ec. Los fallos de las listas C/A se ignoran, como
 * en el original; el retorno de l_0100ec sí decide si se puede entrar al juego.
 * No se escribe A7 (STACK=0x1d1fe es la pila del 68000): la pila de
 * instrumentación se vacía aparte. No repite init_hardware, menú ni parches de
 * idioma/MOTS: la imagen ya los conserva. */
static int boot_l0101a8(uint8_t *img) {
    platform_cpuwork_reset_stack();
    platform_disk_timing(1);
    if (load_resource_list_c(img) < 0)
        platform_log("reinicio suave: lista C no disponible; se continúa");
    BOOT_TRACE("lista_c");
    crt0_init(img);
    platform_ikbdws(img + 0x1a459, 5);
    BOOT_TRACE("crt0 (reinicio)");
    if (load_resource_list_a(img) < 0)
        platform_log("reinicio suave: lista A no disponible; se continúa");
    BOOT_TRACE("lista_a (reinicio)");
    return boot_load_0100ec(img);
}

int kult_boot(uint8_t *img, uint32_t max_frames) {
    /* Siembra del patrón del FDC (0x19316 se omite): el modo de escena (0x14496)
     * consulta $d8eb == 0x52 para salir de su bucle; el patrón vive en el PRG en
     * 0x10000+0x9484 y se copia a $d8e8 en bloques de 0x18 (0x46 veces). */
    for (int i = 0; i < 0x46; i++)
        memcpy(img + 0xd8e8 + i * 0x18, img + 0x10000 + 0x9484, 0x18);

    crt0_init(img);
    /* crt0_init manda Ikbdws(0x19, 0x1a459, 4): 5 bytes (09 01 30 00 b8) que
     * configuran el ratón absoluto con máximo 304x184. La HAL lo emula para que
     * el cursor de 16x16 no se salga por el borde. */
    platform_ikbdws(img + 0x1a459, 5);
    BOOT_TRACE("crt0");
    (void)init_hardware(img, 0);
    BOOT_TRACE("init_hardware");

    /* Camino "sin DRAP" (como el producto del predecesor): salta PRES/DRAP/
     * prompt_wait/idioma y va directo a fijar la letra en los nombres de recurso.
     * Útil para bring-up headless y para la versión de un solo idioma. */
    /* Las cargas de PRES/DRAP ocurren antes de que el usuario pueda
     * interactuar. Si pagasen la espera de disquete, habría ~4 s de pantalla
     * negra/título y la canción empezaría ya avanzada. Se modelan (instantáneas)
     * y se reactiva el timing antes de las cargas posteriores al idioma. */
    platform_disk_timing(0);
    uint8_t lang;
    int skip_drap = getenv("KULT_SKIP_DRAP") != 0;
    if (!skip_drap) {
        /* PRES.PI1 -> pantalla B -> copia a A */
        wr32(img + A_CUR_FILENAME, 0x1a427);
        load_named_screen(img);
        /* _start f_0135ba: XBIOS Setscreen(-1,-1,0) — mantiene la base (solo
         * cambia resolución); en PC `os_setscreen` lo trata como no-op. */
        os_setscreen(0xffffffffu, 0xffffffffu, 0);
        copy_screen(img);
        /* El original solo llama a `set_palette` tras la espera del título (y en
         * el ST se hereda la paleta del escritorio). El HAL arranca en negro, así
         * que el título (PRES, ya copiado a A) se veía negro. Se aplica la paleta
         * del PI1 recién cargado (cabecera en screen_ptr_b-0x20 = A_PALETTE_PTR):
         * el título se ve con sus colores mientras espera tecla/clic. */
        set_palette(img);
        BOOT_TRACE("pres/palette");

        /* DRAP.PI1 -> pantalla B (su paleta es la misma que la de PRES) */
        wr32(img + A_CUR_FILENAME, 0x1a430);
        load_named_screen(img);
        /* Volcado opcional del título para validación visual (sin ventana). */
        if (getenv("KULT_DUMP_TITLE"))
            platform_dump_ppm(getenv("KULT_DUMP_TITLE"));
        /* _start f_012292: espera tecla o clic en la pantalla de título (bit 2 de
         * A_INPUT_BUTTONS; wait_frame trata una tecla de Crawio como clic). En
         * headless el clic alterno de la HAL lo libera; con entrada real lo
         * libera el botón izquierdo del ratón o una tecla. */
        prompt_wait(img);
        platform_log("titulo: clic");
        BOOT_TRACE("pres/drap/menu");
        set_palette(img);
        swap_screens(img);
        lang = kult_boot_language(img);
    } else {
        lang = 'E';
    }

    platform_log("idioma: %c", lang);
    /* Menú de idioma: escribe la letra en las 4 entradas de la lista A */
    kult_boot_apply_language(img, lang);
    if (lang == 'S' || lang == 'C') {
        char font_name[16];
        snprintf(font_name, sizeof font_name, "CARAC%c.BIN", lang);
        uint8_t *font = NULL; size_t font_len = 0;
        if (platform_file_read(font_name, &font, &font_len) == 0 && font_len >= 0x0b * 6 + 6) {
            memcpy(img + A_FONT_DATA + 0x0b * 6, font + 0x0b * 6, 6);
            platform_log("fuente %s aplicada", font_name);
        }
        free(font);
    }
    copy_screen(img);
    BOOT_TRACE("lang");

    /* La carga inicial completa (listas A/B, FOND, ECHE) sigue sin espera: el
     * usuario acaba de elegir idioma y no queremos una pausa de ~20 s. El timing
     * se reactiva tras ECHE, antes de entrar al juego; la estrella/salas sí
     * conservan su espera. */
    /* El banco MOTS de español/catalán puede superar los 1.078 B del hueco
     * original (0x41b76-0x41fac) y PUZZL lo pisaría. Se carga en la zona libre
     * 0x89000-0x90000 y se redirigen la tabla y la referencia de código. */
    wr32(img + 0x1a3de, 0x89000);   /* puntero MOTS en la tabla A */
    wr32(img + 0x10a46, 0x89000);   /* inmediato de la rutina de overlay A */
    (void)load_resource_list_a(img);
    BOOT_TRACE("lista_a");

    if (boot_load_0100ec(img) < 0)
        return -1;

    /* Bucle principal, con corte por número de frames. `setjmp` también recibe el
     * longjmp de `reset_clear` (valor 2): en ese caso se vuelve a 0x10158
     * (`boot_enter_game`) en vez de repetir el guion de muerte. El valor 3 es el
     * reinicio suave de diagnóstico (l_0101a8), que recarga las listas antes de
     * volver al mismo 0x10158. */
    g_frame = 0;
    g_max = max_frames;
    g_stop = 0;
    g_restart_armed = 0;
    hook_loop_top = frame_hook;
    /* Ya presentará el hook del bucle; el tick deja de presentar en arranque. */
    platform_log("game_loop: inicio");
    platform_set_loop(1);
    int rc = 0;
    for (;;) {
        int reason = setjmp(g_jmp);
        if (reason == 1)                 /* stop o límite de frames */
            break;
        g_restart_armed = 0;             /* no reiniciar durante 0x10158 */
        if (reason == 3 && boot_l0101a8(img) < 0) {
            platform_log("reinicio suave: fallo en l_0100ec");
            rc = -1;
            break;
        }
        boot_enter_game(img);
        g_restart_armed = 1;
        game_loop(img, 0);               /* solo sale por longjmp */
    }
    platform_set_loop(0);
    hook_loop_top = 0;
    return rc;
}
