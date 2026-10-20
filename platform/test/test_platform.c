/* platform/test/test_platform.c — pruebas funcionales de la HAL PC (M1).
 *
 * Sin oráculo: verifican el contrato del backend (docs/VERIFICACION.md):
 * ficheros (A: -> data/), vídeo (planar->RGB + paleta), entrada (IKBD/Cnecin) y
 * reloj (sched_poll8 avanza el contador de VBL). Se compilan y ejecutan con
 * `make test-port`.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "machine.h"
#include "addrs.h"
#include "os.h"
#include "sched.h"
#include "psg.h"
#include "hw.h"
#include "kult_api.h"
#include "platform.h"
#include "platform_internal.h"
#include "boot.h"

/* Stub del core para poder probar el tick de VBL aislado (boot/vbl.c). */
uint32_t pal_next_byte(uint8_t *img) { (void)img; return 0; }

/* Core: wait_frame de src/input.c (para la prueba de guardia del input). */
void wait_frame(uint8_t *img);

/* Las ISR PCM (src/sound.c) las cubre el diferencial del core; aquí no se enlazan
 * (arrastrarían el resto del core). Se sustituyen por stubs para el enlazado. */
void sound_isr_fwd(uint8_t *img)  { (void)img; }
void sound_isr_back(uint8_t *img) { (void)img; }

#define IMG_SIZE 0x100000u
#define DIR      "/tmp/kult_hal_test"

static uint8_t g_img[IMG_SIZE];
static int g_fail;

#define CHECK(cond, msg) do { \
    if (!(cond)) { fprintf(stderr, "FALLO: %s\n", msg); g_fail = 1; } \
} while (0)

/* --- utilidades --- */
static void put_name(uint32_t addr, const char *s) {
    strcpy((char *)(g_img + addr), s);
}

static void make_fixture(void) {
    char path[256];
    /* nos aseguramos de que existe el directorio y creamos un fichero conocido */
    snprintf(path, sizeof path, "%s", DIR);
    system("mkdir -p " DIR);
    snprintf(path, sizeof path, "%s/HELLO.BIN", DIR);
    FILE *f = fopen(path, "wb");
    const uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    if (f) { fwrite(data, 1, sizeof data, f); fclose(f); }
}

/* --- T-011: ficheros --- */
static void test_fs(void) {
    os_fsetdta(0x1000);
    put_name(0x2000, "A:HELLO.BIN");
    int32_t r = os_fsfirst(g_img, 0x2000);
    CHECK(r == 0, "Fsfirst debe encontrar A:HELLO.BIN");
    CHECK(be32(g_img + 0x1000 + 0x1a) == 8, "Fsfirst debe volcar el tamano (8)");
    CHECK(strncmp((char *)(g_img + 0x1000 + 0x1e), "HELLO.BIN", 9) == 0,
          "Fsfirst debe volcar el nombre sin unidad");

    int32_t h = os_fopen(g_img, 0x2000);
    CHECK(h >= 6, "Fopen debe dar un handle >= 6");
    uint8_t buf[8] = {0};
    int32_t n = os_fread(g_img, (uint16_t)h, 8, 0x3000);
    CHECK(n == 8, "Fread debe leer 8 bytes");
    for (int i = 0; i < 8; i++) buf[i] = g_img[0x3000 + i];
    CHECK(memcmp(buf, "\x01\x02\x03\x04\x05\x06\x07\x08", 8) == 0,
          "Fread debe leer el contenido correcto");
    CHECK(os_fclose(g_img, (uint16_t)h) == 0, "Fclose debe cerrar");

    put_name(0x2000, "A:NOEXISTE.BIN");
    CHECK(os_fopen(g_img, 0x2000) < 0, "Fopen de un fichero ausente debe fallar");
}

/* Log de diagnóstico (M7): KULT_LOG fija la ruta y el mensaje llega al fichero. */
static void test_log(void) {
    const char *path = DIR "/kult.log";
    setenv("KULT_LOG", path, 1);
    platform_log_init();
    platform_log("prueba de log %d", 42);
    platform_log_close();

    FILE *f = fopen(path, "rb");
    CHECK(f != NULL, "el log debe crearse en la ruta de KULT_LOG");
    if (f) {
        char buf[1024] = {0};
        size_t n = fread(buf, 1, sizeof buf - 1, f);
        buf[n] = '\0';
        fclose(f);
        CHECK(strstr(buf, "prueba de log 42") != NULL,
              "el log debe contener el mensaje");
        CHECK(strstr(buf, "Kult ") != NULL, "el log debe incluir la cabecera");
    }
    remove(path);
    unsetenv("KULT_LOG");
}

/* Presentación por VBL: se presenta tanto en el arranque como dentro del bucle
 * (las animaciones de los comandos corren en `wait_vbl` y no pasan por el hook). */
static int g_present_n;
static void present_counter(uint8_t *img) { (void)img; g_present_n++; }
static void test_present_hook(void) {
    platform_set_present(present_counter);
    platform_set_loop(0);
    g_present_n = 0;
    platform_tick(g_img);
    CHECK(g_present_n == 1, "en arranque se presenta en cada tick");

    platform_set_loop(1);
    g_present_n = 0;
    platform_tick(g_img);
    CHECK(g_present_n == 1, "en el bucle también se presenta en cada VBL");

    platform_set_loop(0);
    platform_set_present(NULL);
}

/* Retardo de CPU (cpu_delay de los bucles dbf): no-op sin tiempo real; con
 * tiempo real duerme el equivalente (~10 ciclos/unidad a 8 MHz) y cede VBLs. */
static void test_cpu_delay(void) {
    platform_set_realtime(0);
    uint32_t f0 = platform_frame_count();
    platform_cpu_delay(40000);
    CHECK(platform_frame_count() == f0,
          "sin tiempo real cpu_delay no avanza frames");

    platform_set_realtime(1);
    f0 = platform_frame_count();
    platform_cpu_delay(36000);                  /* ~45 ms a 8 MHz */
    CHECK(platform_frame_count() - f0 >= 2,
          "cpu_delay en tiempo real cede >= 2 VBLs en ~45 ms");
    platform_set_realtime(0);
}

/* Espera de lectura de disquete del ST: no-op sin tiempo real; con tiempo real
 * 800 B a 62,5 us/B = 50 ms -> >= 2 VBLs. */
static void test_disk_delay(void) {
    platform_set_realtime(0);
    uint32_t f0 = platform_frame_count();
    platform_disk_delay(800);
    CHECK(platform_frame_count() == f0,
          "sin tiempo real disk_delay no avanza frames");

    platform_set_realtime(1);
    f0 = platform_frame_count();
    platform_disk_delay(800);                   /* ~50 ms */
    CHECK(platform_frame_count() - f0 >= 2,
          "disk_delay en tiempo real cede >= 2 VBLs en ~50 ms");
    platform_set_realtime(0);
}

/* Coste de CPU en ciclos exactos (bucle subq/bne de 0x16dec): mismo contrato
 * que cpu_delay; 737280 ciclos ~92 ms a 8 MHz -> >= 4 VBLs. */
static void test_cpu_charge_cycles(void) {
    platform_set_realtime(0);
    uint32_t f0 = platform_frame_count();
    platform_cpu_charge_cycles(737280);
    CHECK(platform_frame_count() == f0,
          "sin tiempo real charge_cycles no avanza frames");

    platform_set_realtime(1);
    f0 = platform_frame_count();
    platform_cpu_charge_cycles(737280);
    CHECK(platform_frame_count() - f0 >= 4,
          "charge_cycles en tiempo real cede >= 4 VBLs en ~92 ms");
    platform_set_realtime(0);
}

static void test_video(void) {
    const uint32_t base = 0x80000;
    os_setscreen(base, base, -1);

    /* pixel (0,0) = color 1 (plano 0, bit 15); pixel (1,0) = color 2 (plano 1, bit 14) */
    memset(g_img + base, 0, 32000);
    wr16(g_img + base + 0, 0x8000);
    wr16(g_img + base + 2, 0x4000);
    os_setcolor(1, 0x007);   /* azul   */
    os_setcolor(2, 0x070);   /* verde  */

    CHECK(platform_visible_base() == base, "la base visible debe ser la de Setscreen");
    CHECK(platform_fb_nonzero() == 2, "debe haber 2 pixeles no negros");
    CHECK(platform_palette()[1] == 0x007, "paleta[1] = azul");
    CHECK(platform_palette()[2] == 0x070, "paleta[2] = verde");

    const char *ppm = "/tmp/kult_hal_test.ppm";
    CHECK(platform_dump_ppm(ppm) == 0, "volcado PPM");
    FILE *f = fopen(ppm, "rb");
    CHECK(f != NULL, "abrir PPM");
    if (f) {
        char line[64];
        for (int i = 0; i < 3; i++) fgets(line, sizeof line, f);  /* P6 / dims / 255 */
        uint8_t px[3], py[3];
        fread(px, 1, 3, f);                       /* pixel (0,0) */
        fread(py, 1, 3, f);                       /* pixel (1,0) */
        fclose(f);
        CHECK(px[0] == 0 && px[2] > 200 && px[1] == 0, "pixel(0,0) debe ser azul");
        CHECK(py[1] > 200 && py[0] == 0, "pixel(1,0) debe ser verde");
    }

    /* La misma conversión usada por el renderizador SDL (RGBA8888). */
    static uint8_t rgba[320 * 200 * 4];
    platform_fb_to_rgba(rgba);
    CHECK(rgba[2] > 200 && rgba[0] == 0 && rgba[3] == 0xff, "RGBA(0,0) azul opaco");
    CHECK(rgba[4 + 1] > 200 && rgba[4] == 0, "RGBA(1,0) verde");
}

/* --- T-014/T-015: entrada y reloj --- */
static void test_input_tick(void) {
    /* VBL: cada sondeo avanza el contador */
    g_img[A_DAT_1ACD4] = 10;
    sched_poll8(g_img, A_DAT_1ACD4, 0x1359a);
    CHECK(g_img[A_DAT_1ACD4] == 11, "el sondeo de VBL avanza el contador");

    /* IKBD: el paquete se entrega y fija el cursor */
    g_img[A_IKBD_PACKET] = 0xff;
    uint8_t v = sched_poll8(g_img, A_IKBD_PACKET, 0x136b0);
    CHECK(v != 0xff, "el sondeo de IKBD entrega el paquete");
    CHECK(be16(g_img + A_CURSOR_X) == 160 && be16(g_img + A_CURSOR_Y) == 100,
          "el paquete IKBD fija el cursor (160,100)");

    /* Cnecin devuelve la auto-tecla sin colgar */
    uint32_t key = 0;
    CHECK(os_cnecin(g_img, &key) == 1, "Cnecin debe devolver una tecla");
    CHECK((key & 0xff) == 'E', "Cnecin devuelve la auto-tecla 'E'");
    CHECK(os_crawio(g_img, 0x00ff) == 0, "Crawio sin teclas encoladas devuelve 0");
}

/* --- T-021: tick de VBL 0x160c2 --- */
static void test_vbl(void) {
    g_img[A_DAT_1AD0D] = 0;
    g_img[A_DAT_1ACD4] = 0;
    wr16(g_img + A_DAT_1AB60, 0);
    kult_vbl_tick(g_img);
    CHECK(g_img[A_DAT_1ACD4] == 1, "el tick incrementa el contador de VBL");

    /* 64 VBLs: el contador pasa a 0x40 (&0x3f == 0) -> avanza A_DAT_1AB60 */
    wr16(g_img + A_DAT_1AB60, 5);
    g_img[A_DAT_1ACD4] = 0x3f;
    kult_vbl_tick(g_img);
    CHECK(be16(g_img + A_DAT_1AB60) == 6, "el 64.o VBL avanza A_DAT_1AB60");

    /* A_DAT_1AD0D == 5: rotacion de paleta en los VBLs multiplo de 4 */
    g_img[A_DAT_1AD0D] = 5;
    os_setcolor(12, 0x001); os_setcolor(13, 0x002);
    os_setcolor(14, 0x003); os_setcolor(15, 0x004);
    for (int i = 0; i < 4; i++) kult_vbl_tick(g_img);
    CHECK(platform_palette()[12] == 0x002 && platform_palette()[15] == 0x001,
          "rotacion de paleta 12..15 en el VBL multiplo de 4");
}

/* --- T-031: entrada real (ratón -> IKBD, teclado -> Cnecin) --- */
static void test_real_input(void) {
    platform_input_set_auto(0);
    platform_mouse_set(200, 150, 1);        /* botón izquierdo pulsado */
    g_img[A_IKBD_PACKET] = 0xff;
    sched_poll8(g_img, A_IKBD_PACKET, 0x136b0);
    CHECK(be16(g_img + A_CURSOR_X) == 200 && be16(g_img + A_CURSOR_Y) == 150,
          "el ratón real fija el cursor");
    CHECK(g_img[A_IKBD_PACKET] == 1, "el botón izquierdo viaja en el IKBD");

    platform_key_push('D');
    uint32_t key = 0;
    CHECK(os_cnecin(g_img, &key) == 1 && (key & 0xff) == 'D',
          "Cnecin consume la tecla real encolada");
    platform_input_set_auto(1);             /* restaurar headless */
}

/* --- T-073c: el estado del clic sigue al botón izquierdo real (sin quedarse pegado) --- */
static void test_mouse_click_state(void) {
    platform_input_set_auto(0);

    platform_mouse_set(160, 100, 1);
    g_img[A_INPUT_BUTTONS] = 0;
    platform_ikbd_deliver(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) != 0,
          "el botón izquierdo marca el clic mientras está pulsado");

    platform_mouse_set(160, 100, 0);
    platform_ikbd_deliver(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) == 0,
          "al soltar el botón izquierdo el clic se limpia");

    platform_input_set_auto(1);
}

/* --- Guardia del modelo de botones original:
 * izquierdo = acción sin skip; derecho = acción + skip; sin botón = nada.
 * `platform_input_apply_skip` es el override que la HAL aplica al salir de
 * `wait_frame` (en el port lo llama el hook de instrumentación). --- */
static void test_click_skip(void) {
    platform_input_set_auto(0);

    platform_mouse_set(160, 100, 1);       /* izquierdo: acción, no skip */
    g_img[A_IKBD_PACKET] = 0xff;
    g_img[A_SKIP_TRANSITION] = 0xff;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) != 0,
          "el izquierdo debe marcar acción");
    CHECK(g_img[A_SKIP_TRANSITION] == 0,
          "el izquierdo NO debe saltar la transición");

    platform_mouse_set(160, 100, 2);       /* derecho: acción + skip */
    g_img[A_IKBD_PACKET] = 0xff;
    g_img[A_SKIP_TRANSITION] = 0;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) != 0,
          "el derecho también debe marcar acción");
    CHECK(g_img[A_SKIP_TRANSITION] == 0xff,
          "el derecho SÍ debe saltar la transición");

    platform_mouse_set(160, 100, 0);       /* sin botón: ni acción ni skip */
    g_img[A_IKBD_PACKET] = 0xff;
    g_img[A_SKIP_TRANSITION] = 0xff;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) == 0,
          "sin botón no debe marcar acción");
    CHECK(g_img[A_SKIP_TRANSITION] == 0,
          "sin botón no debe saltar la transición");

    platform_input_set_auto(1);
}

/* --- IKBD absoluto: el juego fija el máximo del ratón a 304x184 para que el
 * cursor de 16x16 no se salga por el borde. La HAL debe respetarlo. --- */
static void test_ikbd_mouse_max(void) {
    const uint8_t cmd[5] = {0x09, 0x01, 0x30, 0x00, 0xb8};  /* 304x184 */
    const uint8_t def[5] = {0x09, 0x01, 0x3f, 0x00, 0xc7};  /* 319x199 */

    platform_input_set_auto(0);
    platform_ikbdws(cmd, 5);

    platform_mouse_set(999, 999, 0);
    g_img[A_IKBD_PACKET] = 0xff;
    platform_ikbd_deliver(g_img);
    CHECK(be16(g_img + A_CURSOR_X) == 304,
          "el IKBD limita la X del cursor a 304");
    CHECK(be16(g_img + A_CURSOR_Y) == 184,
          "el IKBD limita la Y del cursor a 184");

    platform_mouse_set(-10, -10, 0);
    g_img[A_IKBD_PACKET] = 0xff;
    platform_ikbd_deliver(g_img);
    CHECK(be16(g_img + A_CURSOR_X) == 0 && be16(g_img + A_CURSOR_Y) == 0,
          "el cursor no se sale por arriba/izquierda");

    platform_ikbdws(def, 5);                 /* restaurar 319x199 */
    platform_input_set_auto(1);
}

/* --- loader del PRG: relocación DRI, en particular el SALTO (byte 1) --- */
static void put32be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);  p[3] = (uint8_t)v;
}

static void test_prg_loader(void) {
    /* PRG sintético: text=512 B. Fixups en text+0x10 y text+0x11E, con un SALTO
     * de 254 SIN fixup entre medias; text+0x10E (la posición saltada) lleva un
     * valor centinela que NO debe tocarse. */
    enum { TLEN = 512 };
    const uint32_t o1 = 0x10, o2 = 0x11E, osent = o1 + 254;
    uint8_t prg[28 + TLEN + 4 + 3];
    memset(prg, 0, sizeof prg);
    prg[0] = 0x60; prg[1] = 0x1A;                 /* 0x601A */
    put32be(prg + 2, TLEN);                       /* tlen */
    put32be(prg + 26, 0);                         /* absf = 0 (hay tabla) */
    put32be(prg + 28 + o1, o1);                   /* -> 0x00010010 relocalizado */
    put32be(prg + 28 + osent, 0xDEADBEEFu);       /* centinela (no se relocaliza) */
    put32be(prg + 28 + o2, 0x20);                 /* -> 0x00010020 relocalizado */
    uint8_t *rt = prg + 28 + TLEN;                /* sin symbols */
    put32be(rt, o1); rt[4] = 1; rt[5] = (uint8_t)(o2 - osent); rt[6] = 0;

    const char *path = DIR "/RELOC_TEST.PRG";
    FILE *f = fopen(path, "wb");
    CHECK(f != NULL, "crear PRG de prueba");
    if (!f) return;
    fwrite(prg, 1, sizeof prg, f);
    fclose(f);

    uint8_t *im = calloc(1, IMG_SIZE);
    CHECK(kult_load_prg(im, path) == 0, "cargar PRG de prueba");
    CHECK(be32(im + 0x10000 + o1) == 0x00010010u, "fixup 1 (+LOAD_BASE)");
    CHECK(be32(im + 0x10000 + o2) == 0x00010020u, "fixup 2 (+LOAD_BASE)");
    CHECK(be32(im + 0x10000 + osent) == 0xDEADBEEFu,
          "el SALTO (byte 1) NO relocaliza la posicion saltada");
    free(im);
}

/* --- T-020: menú de idioma fiel (_start, boot/language.c) --- */

/* Inyecta una cadena de teclas la primera vez que lo llama Cnecin bloqueante. */
static const char *g_inject;
static void inject_idle(uint8_t *img) {
    (void)img;
    if (g_inject) { platform_key_push_str(g_inject); g_inject = NULL; }
}

static void test_boot_language(void) {
    /* Modo auto (headless): las teclas pendientes se drenan y gana la auto-tecla
     * ('E' por KULT_KEY). */
    platform_input_set_auto(1);
    platform_key_push('z');
    platform_key_push('q');
    CHECK(kult_boot_language(g_img) == 'E',
          "las teclas pendientes se drenan; gana la auto-tecla");

    /* Entrada real (Cnecin bloqueante con idle): minúscula -> mayúscula, y se
     * ignoran las teclas que no son D/E/F. */
    platform_input_set_auto(0);
    platform_set_idle(inject_idle);
    g_inject = "f";
    CHECK(kult_boot_language(g_img) == 'F', "acepta 'f' -> 'F'");
    g_inject = "xd";
    CHECK(kult_boot_language(g_img) == 'D', "ignora invalidas y acepta 'd' -> 'D'");
    platform_set_idle(NULL);
    platform_input_set_auto(1);

    /* La letra va en las 4 entradas de la lista A (0x1a3aa). */
    memset(g_img, 0, IMG_SIZE);
    kult_boot_apply_language(g_img, 'F');
    CHECK(g_img[0x1a3aa + 4] == 'F' && g_img[0x1a3aa + 0x12] == 'F' &&
          g_img[0x1a3aa + 0x20] == 'F' && g_img[0x1a3aa + 0x2e] == 'F',
          "la letra de idioma va en las 4 entradas de la lista A");
}

/* --- M4/T-040..T-042: música (reproductor 0x3a872), Timer A del MFP y YM --- */
/* Escena de música controlada: los tres canales "mudos" (bit 0x40 de [0x27]), de
 * modo que `music_tick` retorna sin tocar los punteros y el único trabajo del
 * reproductor son las tablas sombra + la envolvente. Deja el estado (armado), las
 * tablas de periodo/volumen y la estructura de envolvente con valores conocidos. */
static void music_scenario(void) {
    memset(g_img, 0, IMG_SIZE);
    g_img[0x3ae8c + 0x1a] = 0xff;             /* armado */
    g_img[0x3ae8c + 0x1b] = 0x10;             /* contador */
    g_img[0x3ae8c + 0x1c] = 0x08;             /* recarga */
    wr16(g_img + 0x3ae7c, 0x1234);            /* sombra de periodos */
    wr16(g_img + 0x3ae7e, 0x5678);
    wr16(g_img + 0x3ae80, 0x9abc);
    g_img[0x3ae84] = 0x0a;                    /* sombra de volúmenes */
    g_img[0x3ae85] = 0x0b;
    g_img[0x3ae86] = 0x0c;
    g_img[0x3b19c + 0] = 0x00;                /* envolvente [0..6] */
    g_img[0x3b19c + 1] = 0x02;
    g_img[0x3b19c + 2] = 0x02;
    g_img[0x3b19c + 3] = 0x10;
    g_img[0x3b19c + 4] = 0x20;
    g_img[0x3b19c + 5] = 0x02;
    g_img[0x3b19c + 6] = 0x3f;
    g_img[0x3b1a2] = 0x3f;                    /* máscara de mixer */
    for (int i = 0; i < 3; i++)
        g_img[0x3afbc + i * 0x50 + 0x27] = 0x40;
}

/* Golden del oráculo (Musashi) para `music_scenario()` + un tick del reproductor
 * 0x3a872, capturado con el harness del kit (psg_seed={7:0x3f}). Cada fila es
 * (kind, reg, val): kind 0 = WRITE, 1 = READ. Comparar el ledger del port con
 * esta tabla es la comparación de stream PSG con el oráculo (docs/VERIFICACION.md). */
static const uint8_t GOLD_KIND[23] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1};
static const uint8_t GOLD_REG[23]  = {0,0,1,1,2,2,3,3,4,4,5,5,8,8,9,9,10,10,6,6,7,7,7};
static const uint8_t GOLD_VAL[23]  = {
    0x34,0x34,0x12,0x12,0x78,0x78,0x56,0x56,0xbc,0xbc,0x9a,0x9a,
    0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x00,0x00,0x3f,0x3f,0x3f
};

static void test_music_player_golden(void) {
    music_scenario();
    psg_reset(NULL, 0);
    psg_seed_reg(7, 0x3f);                    /* mixer conocido (RMW) */
    psg_log_reset();

    kult_music_player(g_img);

    CHECK(psg_log_count() == 23, "el reproductor debe emitir 23 accesos PSG");
    uint32_t n = psg_log_count() < 23 ? psg_log_count() : 23;
    for (uint32_t i = 0; i < n; i++) {
        if (psg_log_kinds()[i] != GOLD_KIND[i] ||
            psg_log_regs()[i]  != GOLD_REG[i]  ||
            psg_log_vals()[i]  != GOLD_VAL[i]) {
            fprintf(stderr, "FALLO: ledger PSG[%u] = (%u,%u,%u), oraculo (%u,%u,%u)\n",
                    i, psg_log_kinds()[i], psg_log_regs()[i], psg_log_vals()[i],
                    GOLD_KIND[i], GOLD_REG[i], GOLD_VAL[i]);
            g_fail = 1;
            break;
        }
    }
}

static void test_mfp_timer_a(void) {
    music_scenario();
    wr32(g_img + 0x134, 0x3a872);             /* vector Timer A -> reproductor */
    hw_write8(0xfffa13, 0x20);                /* IMRA: Timer A */
    hw_write8(0xfffa07, 0x20);                /* IERA: Timer A */
    hw_write8(0xfffa19, 4);                   /* TACR: /50 */
    hw_write8(0xfffa1f, 0xaa);                /* TADR = 0xaa */

    CHECK(platform_mfp_timer_a_enabled(), "Timer A debe estar habilitado");
    /* 2.4576 MHz / (50 * 0xaa) = 289 Hz -> ~3458 us */
    CHECK(platform_mfp_timer_a_period_us() == 3458, "periodo del Timer A = 3458 us");

    platform_audio_init();
    platform_mfp_audio_reset();
    psg_reset(NULL, 0);
    psg_seed_reg(7, 0x3f);

    platform_audio_tick(g_img, 20000);        /* un VBL de tiempo emulado */
    CHECK(platform_mfp_isr_count() == 5, "20 ms a 3458 us -> 5 ISR del Timer A");
    CHECK(platform_audio_samples() > 0, "el Timer A debe renderizar audio");
    CHECK(platform_audio_rms() > 0, "la musica del reproductor debe tener energia");

    hw_write8(0xfffa19, 0);                   /* TACR = 0: parado */
    CHECK(!platform_mfp_timer_a_enabled(), "con TACR=0 el Timer A está parado");
    CHECK(platform_mfp_timer_a_period_us() == 0, "periodo 0 si está parado");
}

/* En modo tiempo real el audio se avanza con el reloj de pared: es lo que hace
 * sonar la música de intro mientras `Cnecin` espera una tecla en el menú. */
static void test_audio_pump(void) {
    music_scenario();
    wr32(g_img + 0x134, 0x3a872);
    hw_write8(0xfffa13, 0x20);
    hw_write8(0xfffa07, 0x20);
    hw_write8(0xfffa19, 4);
    hw_write8(0xfffa1f, 0xaa);

    platform_audio_init();
    platform_mfp_audio_reset();
    psg_reset(NULL, 0);
    psg_seed_reg(7, 0x3f);

    platform_set_realtime(1);
    platform_audio_pump(g_img);               /* fija la base del reloj */
    struct timespec ts = { 0, 30000000L };    /* 30 ms */
    nanosleep(&ts, NULL);
    platform_audio_pump(g_img);
    platform_set_realtime(0);

    CHECK(platform_mfp_isr_count() >= 3, "30 ms a 3458 us -> ~8 ISR (>=3)");
    CHECK(platform_audio_samples() > 0, "el bombeo en espera debe renderizar audio");
}

static void test_audio_rms(void) {
    platform_audio_init();
    psg_reset(NULL, 0);
    psg_seed_reg(0, 0x80);                    /* periodo del canal A */
    psg_seed_reg(8, 0x0f);                    /* volumen A máximo */
    psg_seed_reg(7, 0x3e);                    /* mixer: solo tono A activo */

    platform_audio_rms_reset();
    platform_audio_advance(psg_regs(), 100000, 100);   /* 100 ms */
    uint32_t tone = platform_audio_rms();
    CHECK(platform_audio_samples() >= 4000, "100 ms a 48 kHz -> ~4800 muestras");
    CHECK(tone > 100, "un tono con volumen debe producir energía");
    CHECK(platform_audio_peak() > 200, "el pico del tono debe ser apreciable");

    psg_seed_reg(7, 0x3f);                    /* mixer: todo desactivado */
    platform_audio_rms_reset();
    platform_audio_advance(psg_regs(), 100000, 100);
    CHECK(platform_audio_rms() < tone / 4 + 1,
          "sin tono ni ruido el RMS debe caer (queda el transitorio del filtro)");
}

/* Integración real (si el PRG del oráculo está disponible): `init_hardware` arma
 * el Timer A con el vector $134 = 0x3a872 y el bombeo de espera (menús/prompts)
 * hace sonar la música de intro. Sin la imagen del juego, se omite. */
static void test_integration_music(void) {
    const char *prg = getenv("KULT_PRG");
    if (!prg || !*prg) prg = "work/disk_a/KULT.PRG";
    FILE *f = fopen(prg, "rb");
    if (!f) {
        fprintf(stderr, "nota: sin %s; se omite la integracion de audio\n", prg);
        return;
    }
    fclose(f);

    uint8_t *im = calloc(1, IMG_SIZE);
    if (!im || kult_load_prg(im, prg) != 0) { free(im); return; }

    (void)init_hardware(im, 0);
    CHECK(be32(im + 0x134) == 0x3a872, "init_hardware instala el reproductor en $134");
    CHECK(platform_mfp_timer_a_enabled(), "init_hardware arma el Timer A");

    platform_audio_init();
    platform_mfp_audio_reset();
    psg_reset(NULL, 0);
    psg_seed_reg(7, 0x3f);
    platform_set_realtime(1);
    platform_audio_pump(im);
    struct timespec ts = { 0, 30000000L };
    nanosleep(&ts, NULL);
    platform_audio_pump(im);
    platform_set_realtime(0);

    CHECK(platform_mfp_music_isr_count() >= 3, "la musica de intro debe sonar en la espera");
    CHECK(platform_audio_samples() > 0, "la musica de intro debe renderizar audio");
    free(im);
}

int main(void) {
    make_fixture();
    setenv("KULT_DATA_DIR", DIR, 1);
    setenv("KULT_KEY", "E", 1);

    platform_set_image(g_img);
    platform_init();

    test_fs();
    test_log();
    test_present_hook();
    test_cpu_delay();
    test_cpu_charge_cycles();
    test_disk_delay();
    test_video();
    test_input_tick();
    test_real_input();
    test_mouse_click_state();
    test_click_skip();
    test_ikbd_mouse_max();
    test_vbl();
    test_prg_loader();
    test_boot_language();
    test_music_player_golden();
    test_mfp_timer_a();
    test_audio_pump();
    test_audio_rms();
    test_integration_music();

    if (g_fail) { fprintf(stderr, "RESULTADO: FALLO\n"); return 1; }
    printf("RESULTADO: OK (fs, log, present por VBL, cpu_delay, cpu_cycles, disk_delay, "
           "video, entrada/botones, reloj, tick VBL, loader PRG, idioma, "
           "musica PSG, Timer A, RMS)\n");
    return 0;
}
