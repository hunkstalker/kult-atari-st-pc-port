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

#include "machine.h"
#include "addrs.h"
#include "os.h"
#include "sched.h"
#include "platform.h"
#include "platform_internal.h"
#include "boot.h"

/* Stub del core para poder probar el tick de VBL aislado (boot/vbl.c). */
uint32_t pal_next_byte(uint8_t *img) { (void)img; return 0; }

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

/* --- T-013: vídeo planar -> RGB --- */
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

int main(void) {
    make_fixture();
    setenv("KULT_DATA_DIR", DIR, 1);
    setenv("KULT_KEY", "E", 1);

    platform_set_image(g_img);
    platform_init();

    test_fs();
    test_video();
    test_input_tick();
    test_vbl();
    test_prg_loader();

    if (g_fail) { fprintf(stderr, "RESULTADO: FALLO\n"); return 1; }
    printf("RESULTADO: OK (fs, video, entrada, reloj, tick VBL, loader PRG)\n");
    return 0;
}
