/* app/main.c — driver headless del port (M1).
 *
 * Carga el PRG del usuario en la imagen plana, inicializa la HAL de PC, ejecuta el
 * arranque reconstruido y avanza `KULT_FRAMES` pasos del bucle principal, y vuelca
 * el framebuffer visible a PPM (y, opcionalmente, el planar crudo).
 *
 * Variables de entorno:
 *   KULT_PRG       ruta del .PRG            (def. KULT.PRG de los dirs de datos)
 *   KULT_DATA_DIR  directorio(s) de datos   (def. data/ junto al exe; en Windows
 *                                           varios con ';', en POSIX con ':')
 *   KULT_FRAMES    frames a avanzar         (def. 300; 0 = sin límite)
 *   KULT_DUMP      PPM de salida            (def. out/frame.ppm headless; en
 *                                           ventana no se vuelca si no se pide)
 *   KULT_PLANAR    raw planar de salida     (opcional)
 *   KULT_KEYS      teclas inyectadas        (def. auto 'E')
 *   KULT_WINDOW    ventana SDL              (build SDL: 1 por defecto; 0 =
 *                                           headless; build sin SDL: se ignora)
 *
 * La ventana SDL y la entrada real llegan en M3; esto es la base de validación.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "platform.h"
#include "boot.h"

#ifdef HAVE_SDL3
#include <SDL3/SDL_main.h>     /* entry point de SDL3 (Windows: WinMain -> main) */
#endif

#define IMAGE_SIZE 0x100000u

#ifndef KULT_VERSION
#define KULT_VERSION "dev"
#endif

#ifdef HAVE_SDL3
static const char *env_or(const char *name, const char *def) {
    const char *v = getenv(name);
    return (v && *v) ? v : def;
}
#endif

/* PRG del juego: KULT_PRG si está definido; si no, se busca KULT.PRG en los
 * directorios de datos (el paquete de Windows lo lleva en `data/`); y como
 * último recurso, la ruta del oráculo en el árbol de desarrollo. */
static const char *find_prg(char *buf, size_t n) {
    static const char *names[] = { "KULT.PRG", "kult.prg" };
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
        if (platform_data_resolve(names[i], buf, n) == 0) return buf;
    return "work/disk_a/KULT.PRG";
}

/* Ficheros que el juego necesita en el directorio de datos (los de disk_a +
 * disk_b, ya extraídos). Comprobarlos antes de arrancar evita un cuelgue/crash
 * opaco si falta alguno (p. ej. si solo hay imágenes .STX o subcarpetas). */
static const char *const k_game_files[] = {
    "PRES.PI1", "DRAP.PI1", "FOND.PI1", "PUZZL.BIN", "BRUIT.BIN", "CLEAR.BIN",
    "ECHE0.BIN", "ECHE1.BIN", "ECHE2.BIN", "ECHE3.BIN",
    "ECHE4.BIN", "ECHE5.BIN", "ECHE6.BIN", "ECHE7.BIN",
    "DESCD.BIN", "DESCE.BIN", "DESCF.BIN",
    "DIALD.BIN", "DIALE.BIN", "DIALF.BIN",
    "MOTSD.BIN", "MOTSE.BIN", "MOTSF.BIN",
    "VERBD.BIN", "VERBE.BIN", "VERBF.BIN",
};

static int check_game_files(void) {
    char path[1024], missing[1024] = "";
    int nmiss = 0;
    for (size_t i = 0; i < sizeof k_game_files / sizeof k_game_files[0]; i++) {
        if (platform_data_resolve(k_game_files[i], path, sizeof path) != 0) {
            nmiss++;
            if (strlen(missing) + strlen(k_game_files[i]) + 4 < sizeof missing) {
                strcat(missing, "  ");
                strcat(missing, k_game_files[i]);
                strcat(missing, "\n");
            }
        }
    }
    if (nmiss == 0) return 0;
    platform_fatal(
        "Faltan %d fichero(s) del juego en la carpeta data/:\n%s\n"
        "Copia los ficheros EXTRAIDOS de disk_a y disk_b directamente en data/ "
        "(sin subcarpetas). Las imágenes .STX/.MSA todavía no se leen.\n\n"
        "Directorios de datos y detalle en kult.log.", nmiss, missing);
    return -1;
}

#ifdef HAVE_SDL3
static int g_sdl;

/* KULT_FPSLOG=1: FPS del bucle del juego (una iteración = un wait_frame, igual
 * que el contador Ikbdws del arnés Hatari) cada ~3 s. Sirve para comparar la
 * misma escena con el ST/Hatari y calibrar KULT_CPU_HZ/KULT_ST_CONTEND. */
static void fps_note(void) {
    static int en = -1;
    static uint32_t n;
    static uint64_t last;
    if (en < 0) {
        const char *e = getenv("KULT_FPSLOG");
        en = e ? (atoi(e) != 0) : 1;        /* activo por defecto; =0 lo apaga */
    }
    if (!en) return;
    uint64_t now = platform_now_ns();
    if (!last) { last = now; n = 0; return; }
    n++;
    uint64_t dt = now - last;
    if (dt >= 3000000000ull) {
        platform_log("fps juego: %.1f", (double)n * 1e9 / (double)dt);
        last = now;
        n = 0;
    }
}

static void present_cb(uint8_t *img) {
    if (!platform_sdl_pump()) { kult_request_stop(); return; }
    fps_note();
    platform_sdl_present(img);
}
/* Bombeo durante una espera bloqueante (Cnecin en menús/prompts): presenta y, si
 * se cierra la ventana, sale del programa. */
static void idle_cb(uint8_t *img) {
    if (!platform_sdl_pump()) { platform_sdl_close(); exit(0); }
    /* El Timer A del MFP sigue interrumpiendo mientras se espera una tecla: la
     * música de intro/efectos no se detiene en los menús. */
    platform_audio_pump(img);
    platform_sdl_present(img);
}
/* Presentación durante las esperas de arranque (`wait_frame`): bombea SDL (para
 * que el ratón/teclado entren en esas esperas) y presenta. El audio ya lo avanza
 * `platform_tick` un VBL cada vez; aquí no se avanza por reloj. */
static void tick_cb(uint8_t *img) {
    if (!platform_sdl_pump()) { platform_sdl_close(); exit(0); }
    platform_sdl_present(img);
}
#endif

int main(int argc, char **argv) {
    (void)argc; (void)argv;    /* SDL3 (Windows) exige la firma con argumentos */
    platform_log_init();
    platform_log("Kult %s — inicio", KULT_VERSION);

    const char *dumped = getenv("KULT_DUMP");
    const char *planar = getenv("KULT_PLANAR");
    const char *fenv   = getenv("KULT_FRAMES");
    const char *wenv   = getenv("KULT_WINDOW");
#ifdef HAVE_SDL3
    /* Build con SDL: ventana por defecto (doble clic = juego). KULT_WINDOW=0
     * fuerza modo headless para diagnóstico/pruebas. */
    int window = wenv ? (wenv[0] != '0') : 1;
#else
    /* Build sin SDL: siempre headless; KULT_WINDOW se ignora. */
    int window = 0;
    if (wenv && wenv[0] != '0') {
        platform_log("build sin SDL3; ignoro KULT_WINDOW");
        fprintf(stderr, "build sin SDL3; ignoro KULT_WINDOW\n");
    }
#endif
    uint32_t frames    = fenv ? (uint32_t)strtoul(fenv, NULL, 0)
                              : (window ? 0u : 300u);   /* 0 = sin límite */
    /* En ventana no se vuelca PPM salvo que se pida (KULT_DUMP): en Windows el
     * directorio `out/` del árbol de desarrollo no existe. */
    const char *dump   = dumped ? dumped : (window ? NULL : "out/frame.ppm");

    uint8_t *img = calloc(1, IMAGE_SIZE);
    if (!img) {
        platform_fatal("Sin memoria para la imagen del juego.");
        platform_log_close();
        return 2;
    }

    platform_set_image(img);
    platform_init();
    platform_set_tick(kult_vbl_tick);

    /* El PRG del juego se busca en los directorios de datos (paquete Windows:
     * `data/KULT.PRG`) si KULT_PRG no lo fija. */
    char prgbuf[1024];
    const char *prgenv = getenv("KULT_PRG");
    const char *prg;
    if (prgenv && *prgenv) {
        prg = prgenv;
        platform_log("PRG (KULT_PRG): %s", prg);
    } else {
        prg = find_prg(prgbuf, sizeof prgbuf);
        platform_log("PRG: %s", prg);
    }
    if (kult_load_prg(img, prg) != 0) {
        platform_fatal(
            "No se pudo cargar KULT.PRG:\n%s\n\n"
            "Copia en la carpeta data/ (junto a kult.exe) los ficheros "
            "EXTRAIDOS de los disquetes del juego (KULT.PRG, *.PI1, *.BIN...), "
            "no las imágenes .STX/.MSA.\n\n"
            "Directorio(s) de datos y detalle en kult.log.", prg);
        platform_log_close();
        free(img);
        return 2;
    }
    platform_log("PRG cargado (%s)", prg);
    if (check_game_files() != 0) {
        platform_log_close();
        free(img);
        return 2;
    }

#ifdef HAVE_SDL3
    if (window) {
        int scale = (int)strtoul(env_or("KULT_SCALE", "3"), NULL, 0);
        if (platform_sdl_open("Kult: The Temple of Flying Saucers", scale) == 0) {
            g_sdl = 1;
            platform_log("ventana SDL: abierta (%dx%d)", 320 * scale, 200 * scale);
            kult_set_frame_cb(present_cb);
            platform_set_idle(idle_cb);
            platform_set_present(tick_cb);   /* presenta/bombea en el arranque */
            platform_log("FPS log: cada ~3 s de bucle (KULT_FPSLOG=0 lo apaga)");
            {
                const char *cpu = getenv("KULT_CPU");
                const char *hz = getenv("KULT_CPU_HZ");
                const char *st = getenv("KULT_ST_CONTEND");
                platform_log("ritmo: CPU=%s CPU_HZ=%s ST_CONTEND=%s",
                             (cpu && *cpu) ? cpu : "on",
                             (hz && *hz) ? hz : "def(8MHz)",
                             (st && *st) ? st : "def(100)");
            }
            /* Ritmo del original (50 Hz); KULT_REALTIME=0 lo desactiva. */
            const char *rt = getenv("KULT_REALTIME");
            platform_set_realtime(!(rt && rt[0] == '0'));
            /* Entrada real por defecto; KULT_AUTOINPUT=1 fuerza el modo auto
             * (útil sin display/para pruebas). */
            platform_input_set_auto(getenv("KULT_AUTOINPUT") ? 1 : 0);
            /* Audio SDL3 (KULT_AUDIO=0 lo desactiva). */
            platform_audio_sdl_open();
        } else {
            /* En GUI no hay consola: si la ventana falla hay que avisar y salir
             * (antes se seguía en headless con frames=0, invisible). */
            platform_fatal("No se pudo abrir la ventana SDL.\n"
                           "Detalle en kult.log (junto a kult.exe).");
            platform_log_close();
            free(img);
            return 4;
        }
    }
#endif

    int rc = 0;
    if (kult_boot(img, frames) != 0) {
        platform_fatal("El arranque del juego falló.\nDetalle en kult.log.");
        rc = 3;
    }
    platform_log("fin: frames=%u visible=0x%05x fb_nonzero=%u exit=%d",
                 platform_frame_count(), platform_visible_base(),
                 platform_fb_nonzero(), rc);

#ifdef HAVE_SDL3
    if (g_sdl) { platform_audio_sdl_close(); platform_sdl_close(); }
#endif

    if (dump) {
        if (platform_dump_ppm(dump) != 0) {
            platform_log("no se pudo escribir %s", dump);
            fprintf(stderr, "no se pudo escribir %s\n", dump);
        } else {
            platform_log("PPM: %s", dump);
        }
    }
    if (planar) {
        if (platform_dump_planar(planar) != 0) {
            platform_log("no se pudo escribir %s", planar);
            fprintf(stderr, "no se pudo escribir %s\n", planar);
        } else {
            platform_log("planar: %s", planar);
        }
    }

    printf("prg=%s frames=%u visible=0x%05x fb_nonzero=%u ppm=%s\n",
           prg, platform_frame_count(), platform_visible_base(),
           platform_fb_nonzero(), dump ? dump : "-");
    printf("audio: rms=%u peak=%u samples=%u isr=%u music_isr=%u\n",
           platform_audio_rms(), platform_audio_peak(),
           platform_audio_samples(), platform_mfp_isr_count(),
           platform_mfp_music_isr_count());
    if (getenv("KULT_DEBUG")) {
        const uint16_t *pal = platform_palette();
        fprintf(stderr, "[pal]     ");
        for (int i = 0; i < 16; i++) fprintf(stderr, " %03x", pal[i]);
        fprintf(stderr, "\n");
        uint32_t pp = ((uint32_t)img[0x1a466] << 24) | ((uint32_t)img[0x1a467] << 16) |
                      ((uint32_t)img[0x1a468] << 8) | (uint32_t)img[0x1a469];
        fprintf(stderr, "[palimg %05x]", pp);
        for (int i = 0; i < 16; i++) {
            unsigned w = ((unsigned)img[pp + 2 * i] << 8) | img[pp + 2 * i + 1];
            fprintf(stderr, " %03x", w);
        }
        fprintf(stderr, "\n");
    }
    platform_log("audio: rms=%u peak=%u samples=%u isr=%u music_isr=%u",
                 platform_audio_rms(), platform_audio_peak(),
                 platform_audio_samples(), platform_mfp_isr_count(),
                 platform_mfp_music_isr_count());
    platform_log_close();
    free(img);
    return rc;
}
