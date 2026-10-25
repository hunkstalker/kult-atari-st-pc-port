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
#include "container.h"
#include "diskimage.h"
#include "sha256.h"
#include "display.h"
#include "osd.h"

/* Stub del core para poder probar el tick de VBL aislado (boot/vbl.c). */
uint32_t pal_next_byte(uint8_t *img) { (void)img; return 0; }

/* Core: wait_frame de src/input.c (para la prueba de guardia del input). */
void wait_frame(uint8_t *img);

/* Las ISR PCM (src/sound.c) las cubre el diferencial del core; aquí no se enlazan
 * (arrastrarían el resto del core). Se sustituyen por stubs para el enlazado. */
void sound_isr_fwd(uint8_t *img)  { (void)img; }
void sound_isr_back(uint8_t *img) { (void)img; }
/* Stubs de las rutinas de cursor/sonido que usa platform_slots_apply (el test
 * del savestate no enlaza el core completo). */
void cursor_draw(uint8_t *img)    { (void)img; }
void cursor_restore(uint8_t *img) { (void)img; }
void sound_stop(uint8_t *img)     { (void)img; }

#define IMG_SIZE 0x100000u
#define DIR      "/tmp/kult_hal_test"

static uint8_t g_img[IMG_SIZE];
static int g_fail;

#define CHECK(cond, msg) do { \
    if (!(cond)) { fprintf(stderr, "FALLO: %s\n", msg); g_fail = 1; } \
} while (0)

/* Inyecta una cadena de teclas la primera vez que lo llama Cnecin bloqueante. */
static const char *g_inject;
static void inject_idle(uint8_t *img) {
    (void)img;
    if (g_inject) { platform_key_push_str(g_inject); g_inject = NULL; }
}

/* --- utilidades --- */
static void put_name(uint32_t addr, const char *s) {
    strcpy((char *)(g_img + addr), s);
}

static void make_fixture(void) {
    char path[256];
    /* nos aseguramos de que existe el directorio y creamos un fichero conocido */
    snprintf(path, sizeof path, "%s", DIR);
    system("mkdir -p " DIR "/data " DIR "/saves");
    snprintf(path, sizeof path, "%s/data/HELLO.BIN", DIR);
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

    /* Las escrituras del juego van a saves/, no a data/. */
    put_name(0x2000, "A:OUT.BIN");
    int32_t hw = os_fcreate(g_img, 0x2000);
    CHECK(hw >= 6, "Fcreate debe crear el fichero en saves/");
    if (hw >= 6) {
        const uint8_t out[3] = {9, 8, 7};
        memcpy(g_img + 0x3000, out, sizeof out);
        CHECK(os_fwrite(g_img, (uint16_t)hw, sizeof out, 0x3000) == 3,
              "Fwrite debe escribir 3 bytes");
        CHECK(os_fclose(g_img, (uint16_t)hw) == 0, "Fclose del fichero de salida");
    }
    FILE *sf = fopen(DIR "/saves/OUT.BIN", "rb");
    CHECK(sf != NULL, "la escritura debe quedar en saves/, no en data/");
    if (sf) fclose(sf);
    remove(DIR "/saves/OUT.BIN");
}

/* Contenedores: formato, lectura y prioridad language.data > kult.data. */
static void test_containers(void) {
    char game[256], lang[256];
    snprintf(game, sizeof game, "%s/data/kult.data", DIR);
    snprintf(lang, sizeof lang, "%s/data/language.data", DIR);
    const char *names[] = {"OVER.BIN"};
    const uint8_t gdata[] = {1, 2, 3};
    const uint8_t ldata[] = {9, 8, 7, 6};
    const uint8_t *gblob[] = {gdata}, *lblob[] = {ldata};
    uint32_t gsize[] = {sizeof gdata}, lsize[] = {sizeof ldata};

    CHECK(kult_container_write(game, KULT_CONTAINER_GAME, names, gblob, gsize, 1) == 0,
          "crear kult.data de prueba");
    CHECK(kult_container_write(lang, KULT_CONTAINER_LANG, names, lblob, lsize, 1) == 0,
          "crear language.data de prueba");
    platform_fs_reload_containers();

    uint8_t *out = NULL; size_t n = 0;
    CHECK(platform_file_read("OVER.BIN", &out, &n) == 0, "leer OVER.BIN");
    CHECK(n == sizeof ldata && memcmp(out, ldata, sizeof ldata) == 0,
          "language.data debe tener prioridad sobre kult.data");
    free(out);

    remove(lang);
    platform_fs_reload_containers();
    out = NULL; n = 0;
    CHECK(platform_file_read("OVER.BIN", &out, &n) == 0, "leer OVER.BIN sin language");
    CHECK(n == sizeof gdata && memcmp(out, gdata, sizeof gdata) == 0,
          "sin language.data debe usarse kult.data");
    free(out);
    remove(game);
    platform_fs_reload_containers();
}

/* --- T-095: imágenes de disco (STX/Pasti y FAT12) --- */
static void wr16le(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void wr32le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

static uint8_t *read_whole(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    uint8_t *b = malloc((size_t)n ? (size_t)n : 1);
    if (!b) { fclose(f); return NULL; }
    if (n && fread(b, 1, (size_t)n, f) != (size_t)n) { free(b); fclose(f); return NULL; }
    fclose(f);
    *len = (size_t)n;
    return b;
}

static int write_whole(const char *path, const uint8_t *b, size_t n) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    int ok = fwrite(b, 1, n, f) == n;
    if (fclose(f) != 0) ok = 0;
    return ok ? 0 : -1;
}

#define TEST_RAW_SIZE (2u * 80u * 9u * 512u)

static void test_diskimage_stx(void) {
    const char *p = DIR "/TESTU.STX";
    /* Pista 0 sin comprimir: 9 sectores de 512 B. */
    size_t n = 16 + 16 + 9 * 512;
    uint8_t *stx = calloc(1, n);
    CHECK(stx != NULL, "crear STX sintetico");
    if (stx) {
        memcpy(stx, "RSY", 3);
        wr32le(stx + 16, (uint32_t)(16 + 9 * 512));
        wr16le(stx + 16 + 8, 9);
        wr16le(stx + 16 + 10, 0);
        stx[16 + 14] = 0;
        for (int i = 0; i < 9; i++)
            memset(stx + 16 + 16 + i * 512, 0x10 + i, 512);

        CHECK(write_whole(p, stx, n) == 0, "escribir STX sin comprimir");
        uint8_t *raw = calloc(1, TEST_RAW_SIZE);
        CHECK(raw != NULL, "reservar imagen raw");
        if (raw) {
            CHECK(kult_stx_to_raw(p, raw, TEST_RAW_SIZE) == 0,
                  "extraer STX sin comprimir");
            for (int i = 0; i < 9; i++)
                CHECK(raw[i * 512] == (uint8_t)(0x10 + i) &&
                      raw[i * 512 + 511] == (uint8_t)(0x10 + i),
                      "los 9 sectores de la pista 0 caen en su LBA");
            CHECK(raw[9 * 512] == 0, "la pista 1 no se toca");
            free(raw);
        }
        free(stx);
    }

    /* Pista 1 comprimida (tabla de cabeceras + datos). */
    p = DIR "/TESTC.STX";
    n = 16 + 16 + 9 * 16 + 9 * 512;
    stx = calloc(1, n);
    CHECK(stx != NULL, "crear STX comprimido");
    if (stx) {
        memcpy(stx, "RSY", 3);
        wr32le(stx + 16, (uint32_t)(16 + 9 * 16 + 9 * 512));
        wr16le(stx + 16 + 8, 9);
        wr16le(stx + 16 + 10, 1);
        stx[16 + 14] = 1;                       /* pista 1, cara 0 */
        for (int i = 0; i < 9; i++) {
            uint8_t *h = stx + 16 + 16 + i * 16;
            wr32le(h, (uint32_t)(i * 512));
            h[8] = 1;                           /* stn */
            h[9] = 0;                           /* sid */
            h[10] = (uint8_t)(i + 1);           /* snum (1-based) */
            h[11] = 2;                          /* 128 << 2 = 512 B */
            memset(stx + 16 + 16 + 9 * 16 + i * 512, 0x40 + i, 512);
        }
        CHECK(write_whole(p, stx, n) == 0, "escribir STX comprimido");
        uint8_t *raw = calloc(1, TEST_RAW_SIZE);
        if (raw) {
            CHECK(kult_stx_to_raw(p, raw, TEST_RAW_SIZE) == 0,
                  "extraer STX comprimido");
            for (int i = 0; i < 9; i++)
                CHECK(raw[(9 + i) * 512] == (uint8_t)(0x40 + i),
                      "la tabla de cabeceras coloca cada sector en su LBA");
            free(raw);
        }
        free(stx);
    }

    /* Cabecera Pasti inválida. */
    p = DIR "/BAD.STX";
    const uint8_t bad[16] = {'N', 'O', 'P', 'E'};
    CHECK(write_whole(p, bad, sizeof bad) == 0, "escribir STX invalido");
    uint8_t raw[512] = {0};
    CHECK(kult_stx_to_raw(p, raw, sizeof raw) != 0,
          "un STX con cabecera no Pasti debe rechazarse");

    CHECK(kult_stx_name("Kult.STX") && kult_stx_name("kult.stx") &&
          !kult_stx_name("Kult.ST") && !kult_stx_name("Kult.STX.bak") &&
          !kult_stx_name(".stx"),
          "deteccion de extension .stx");

    remove(DIR "/TESTU.STX");
    remove(DIR "/TESTC.STX");
    remove(DIR "/BAD.STX");
}

static void fat12_set(uint8_t *img, uint32_t fat_start, uint16_t n, uint16_t v) {
    size_t off = fat_start + (size_t)(n * 3u) / 2u;
    uint16_t cur = (uint16_t)(img[off] | (img[off + 1] << 8));
    if (n & 1u) cur = (uint16_t)((cur & 0x000fu) | ((v & 0x0fffu) << 4));
    else        cur = (uint16_t)((cur & 0xf000u) | (v & 0x0fffu));
    img[off] = (uint8_t)cur; img[off + 1] = (uint8_t)(cur >> 8);
}

static void fat12_root(uint8_t *img, size_t off, const char name[8],
                       const char ext[3], uint8_t attr, uint16_t cluster,
                       uint32_t size) {
    memcpy(img + off, name, 8);
    memcpy(img + off + 8, ext, 3);
    img[off + 11] = attr;
    wr16le(img + off + 26, cluster);
    wr32le(img + off + 28, size);
}

static void test_diskimage_fat12(void) {
    enum { IMG = 32768, FAT_START = 512, ROOT_START = 1024, DATA_START = 1536 };
    uint8_t *img = calloc(1, IMG);
    CHECK(img != NULL, "crear imagen FAT12");
    if (!img) return;

    wr16le(img + 0x0b, 512);                    /* bytes por sector */
    img[0x0d] = 1;                              /* sectores por cluster */
    wr16le(img + 0x0e, 1);                      /* reservados */
    img[0x10] = 1;                              /* FATs */
    wr16le(img + 0x11, 16);                     /* entradas de root */
    wr16le(img + 0x16, 1);                      /* sectores por FAT */

    fat12_set(img, FAT_START, 1, 0xfff);
    fat12_set(img, FAT_START, 2, 3);            /* FOO: 2 -> 3 -> fin */
    fat12_set(img, FAT_START, 3, 0xfff);
    fat12_set(img, FAT_START, 4, 0xfff);        /* BAR: 4 -> fin */

    /* FOO.BIN: 600 B (cluster 2 completo + 88 B del 3). */
    fat12_root(img, ROOT_START + 0 * 32, "FOO     ", "BIN", 0x20, 2, 600);
    memset(img + DATA_START, 0x11, 512);
    memset(img + DATA_START + 512, 0x22, 88);
    /* Entrada borrada (0xE5), LFN (0x0F) y directorio (0x10): se ignoran. */
    fat12_root(img, ROOT_START + 1 * 32, "DEL     ", "BIN", 0x20, 2, 100);
    img[ROOT_START + 1 * 32] = 0xe5;
    fat12_root(img, ROOT_START + 2 * 32, "LFN     ", "   ", 0x0f, 0, 0);
    fat12_root(img, ROOT_START + 3 * 32, "DIR     ", "   ", 0x10, 2, 0);
    /* Duplicado (mismo nombre sin distinguir mayúsculas): gana el primero. */
    fat12_root(img, ROOT_START + 4 * 32, "foo     ", "bin", 0x20, 4, 512);
    /* BAR.DAT desde el cluster 4. */
    fat12_root(img, ROOT_START + 5 * 32, "BAR     ", "DAT", 0x20, 4, 512);
    memset(img + DATA_START + 2 * 512, 0x33, 512);

    kult_filelist fl = {0};
    CHECK(kult_fat12_extract(img, IMG, &fl) == 0, "extraer FAT12 sintetica");
    CHECK(fl.n == 2, "solo se extraen FOO.BIN y BAR.DAT");
    const kult_memfile *foo = kult_filelist_find(&fl, "foo.bin");
    CHECK(foo != NULL && foo->size == 600, "FOO.BIN debe tener 600 B");
    if (foo && foo->size == 600) {
        CHECK(foo->data[0] == 0x11 && foo->data[511] == 0x11 &&
              foo->data[512] == 0x22 && foo->data[599] == 0x22,
              "FOO.BIN sigue la cadena de clusters 2 -> 3");
    }
    const kult_memfile *bar = kult_filelist_find(&fl, "BAR.DAT");
    CHECK(bar != NULL && bar->size == 512 && bar->data[0] == 0x33,
          "BAR.DAT se extrae del cluster 4");
    kult_filelist_free(&fl);
    CHECK(fl.n == 0 && fl.v[0].data == NULL, "la lista queda vacia al liberar");

    uint8_t zero[512] = {0};
    CHECK(kult_fat12_extract(zero, sizeof zero, &fl) != 0,
          "un BPB invalido debe rechazarse");
    free(img);
}

static const char *const k_required[] = {
    "KULT.PRG","PRES.PI1","DRAP.PI1","FOND.PI1","BRUIT.BIN","PUZZL.BIN","CLEAR.BIN",
    "ECHE0.BIN","ECHE1.BIN","ECHE2.BIN","ECHE3.BIN","ECHE4.BIN","ECHE5.BIN","ECHE6.BIN","ECHE7.BIN",
    "DESCD.BIN","DESCE.BIN","DESCF.BIN","DIALD.BIN","DIALE.BIN","DIALF.BIN",
    "MOTSD.BIN","MOTSE.BIN","MOTSF.BIN","VERBD.BIN","VERBE.BIN","VERBF.BIN"
};

static void add_list_file(kult_filelist *fl, const char *name,
                          const uint8_t *data, uint32_t size) {
    kult_memfile *m = &fl->v[fl->n++];
    snprintf(m->name, sizeof m->name, "%s", name);
    m->data = malloc(size ? size : 1);
    if (m->data && size) memcpy(m->data, data, size);
    m->size = size;
}

static void test_ingest_validate(void) {
    const uint8_t dummy[4] = {1, 2, 3, 4};
    kult_filelist fl = {0};
    for (size_t i = 0; i < sizeof k_required / sizeof k_required[0]; i++)
        add_list_file(&fl, k_required[i], dummy, sizeof dummy);
    CHECK(kult_ingest_validate(&fl) != 0,
          "un KULT.PRG con hash desconocido debe rechazarse");
    kult_filelist_free(&fl);

    /* Falta un fichero obligatorio. */
    for (size_t i = 0; i < sizeof k_required / sizeof k_required[0] - 1; i++)
        add_list_file(&fl, k_required[i], dummy, sizeof dummy);
    CHECK(kult_ingest_validate(&fl) != 0,
          "la falta de un fichero obligatorio debe rechazarse");
    kult_filelist_free(&fl);

    /* Camino positivo condicional con el PRG real del oráculo. */
    static const char *const prg_paths[] = {
        "work/disk_a/KULT.PRG", "build/data/KULT.PRG", "build-win/data/KULT.PRG"
    };
    uint8_t *prg = NULL; size_t prg_len = 0;
    for (size_t i = 0; i < sizeof prg_paths / sizeof prg_paths[0] && !prg; i++)
        prg = read_whole(prg_paths[i], &prg_len);
    if (!prg) {
        fprintf(stderr, "nota: sin KULT.PRG; se omite la validacion positiva\n");
        return;
    }
    for (size_t i = 0; i < sizeof k_required / sizeof k_required[0]; i++)
        add_list_file(&fl, k_required[i], dummy, sizeof dummy);
    kult_memfile *kp = (kult_memfile *)kult_filelist_find(&fl, "KULT.PRG");
    if (kp) {
        free(kp->data);
        kp->data = prg;
        kp->size = (uint32_t)prg_len;
        prg = NULL;
    }
    CHECK(kult_ingest_validate(&fl) == 0,
          "el KULT.PRG real debe validar por SHA-256");
    kult_filelist_free(&fl);
    free(prg);
}

/* Discos conocidos y pipeline completo (detección + Pasti + FAT12 + escritura
 * atómica de kult.data). Condicional: usa los STX reales si están presentes. */
static void test_ingest_pipeline(void) {
    static const char *const dirs[] = { "build/data", "build-win/data", "data" };
    char a[256] = {0}, b[256] = {0};
    for (size_t i = 0; i < sizeof dirs / sizeof dirs[0] && !a[0]; i++) {
        char pa[256], pb[256];
        snprintf(pa, sizeof pa, "%s/Kult.STX", dirs[i]);
        snprintf(pb, sizeof pb, "%s/Kult (B).STX", dirs[i]);
        FILE *fa = fopen(pa, "rb"), *fb = fopen(pb, "rb");
        if (fa && fb) { snprintf(a, sizeof a, "%s", pa); snprintf(b, sizeof b, "%s", pb); }
        if (fa) fclose(fa);
        if (fb) fclose(fb);
    }

    if (!a[0]) {
        fprintf(stderr, "nota: sin STX reales; se omite el pipeline de ingesta\n");
    } else {
        size_t la = 0, lb = 0;
        uint8_t *ba = read_whole(a, &la), *bb = read_whole(b, &lb);
        CHECK(ba && bb, "leer los STX reales");
        if (ba && bb) {
            kult_sha256 s; uint8_t h[32]; int role = -1;
            kult_sha256_init(&s); kult_sha256_update(&s, ba, la); kult_sha256_final(&s, h);
            CHECK(kult_ingest_known_disk(h, &role) == 1 && role == 0,
                  "el STX A se detecta por SHA-256");
            kult_sha256_init(&s); kult_sha256_update(&s, bb, lb); kult_sha256_final(&s, h);
            role = -1;
            CHECK(kult_ingest_known_disk(h, &role) == 1 && role == 1,
                  "el STX B se detecta por SHA-256");
            h[0] ^= 0xff;
            CHECK(kult_ingest_known_disk(h, &role) == 0,
                  "un SHA-256 alterado no es un disco conocido");

            CHECK(write_whole(DIR "/data/A.STX", ba, la) == 0 &&
                  write_whole(DIR "/data/B.STX", bb, lb) == 0,
                  "copiar los STX a la carpeta de prueba");
            remove(DIR "/data/kult.data");
            CHECK(platform_ingest_ensure() == 0,
                  "la ingesta completa debe generar kult.data");
            CHECK(fopen(DIR "/data/kult.data.tmp", "rb") == NULL,
                  "no debe quedar el temporal tras el rename atomico");

            kult_container c;
            if (kult_container_load(DIR "/data/kult.data", KULT_CONTAINER_GAME,
                                    &c) == 0) {
                CHECK(c.count >= sizeof k_required / sizeof k_required[0],
                      "kult.data contiene todos los ficheros");
                uint32_t sz = 0;
                CHECK(kult_container_get(&c, "KULT.PRG", &sz) != NULL && sz > 0,
                      "kult.data contiene KULT.PRG");
                CHECK(kult_container_get(&c, "PRES.PI1", &sz) != NULL,
                      "kult.data contiene PRES.PI1");
                kult_container_free(&c);
            } else {
                CHECK(0, "kult.data generado debe cargar como tipo GAME");
            }

            platform_fs_reload_containers();
            uint8_t *rd = NULL; size_t rn = 0;
            CHECK(platform_file_read("KULT.PRG", &rd, &rn) == 0 && rn > 0,
                  "kult.data generado se sirve por la capa de ficheros");
            free(rd);
        }
        free(ba); free(bb);
    }

    /* Sin STX ni kult.data, la ingesta debe fallar de forma limpia. */
    remove(DIR "/data/A.STX");
    remove(DIR "/data/B.STX");
    remove(DIR "/data/kult.data");
    platform_fs_reload_containers();
    CHECK(platform_ingest_ensure() != 0,
          "sin discos conocidos la ingesta debe fallar");
}

/* Escritura atómica y validación del contenedor. */
static void test_container_atomic(void) {
    const char *path = DIR "/data/atomic.data";
    const char *names[] = {"A.BIN", "B.BIN"};
    const uint8_t da[] = {1, 2, 3}, db[] = {4, 5};
    const uint8_t *blobs[] = {da, db};
    const uint32_t sizes[] = {sizeof da, sizeof db};

    CHECK(kult_container_write(path, KULT_CONTAINER_GAME, names, blobs, sizes, 2) == 0,
          "escribir contenedor atomico");
    size_t before = 0;
    uint8_t *orig = read_whole(path, &before);
    CHECK(orig != NULL, "leer contenedor escrito");
    CHECK(fopen(DIR "/data/atomic.data.tmp", "rb") == NULL,
          "el temporal debe desaparecer tras el rename");

    CHECK(kult_container_write(path, KULT_CONTAINER_GAME, NULL, NULL, NULL, 4097) != 0,
          "un contenedor con demasiadas entradas debe rechazarse");
    size_t after = 0;
    uint8_t *now = read_whole(path, &after);
    CHECK(now && before == after && memcmp(orig, now, before) == 0,
          "un fallo de escritura no debe tocar el contenedor anterior");
    free(now);

    kult_container c;
    CHECK(kult_container_load(path, KULT_CONTAINER_LANG, &c) != 0,
          "cargar un tipo distinto debe fallar");
    CHECK(kult_container_load(path, KULT_CONTAINER_GAME, &c) == 0,
          "el contenedor propio debe cargar");
    if (c.data) {
        uint32_t sz = 0;
        const uint8_t *p = kult_container_get(&c, "b.bin", &sz);
        CHECK(p && sz == 2 && p[0] == 4 && p[1] == 5,
              "lectura sin distinguir mayusculas");
        kult_container_free(&c);
    }

    uint8_t bad[32] = {'X'};
    CHECK(write_whole(path, bad, sizeof bad) == 0, "escribir contenedor invalido");
    CHECK(kult_container_load(path, KULT_CONTAINER_GAME, &c) != 0,
          "un magic invalido debe rechazarse");

    free(orig);
    remove(path);
}

/* --- T-095: selección y carga de los packs S/C --- */
static void test_language_packs(void) {
    const char *lang = DIR "/data/language.data";
    const char *names[] = {"VERBS.BIN", "VERBC.BIN"};
    const uint8_t es[] = {0x53, 0x01, 0x02};
    const uint8_t ca[] = {0x43, 0x03, 0x04};
    const uint8_t *blobs[] = {es, ca};
    const uint32_t sizes[] = {sizeof es, sizeof ca};

    CHECK(kult_container_write(lang, KULT_CONTAINER_LANG, names, blobs, sizes, 2) == 0,
          "crear language.data sintetico con S y C");
    platform_fs_reload_containers();

    uint8_t *out = NULL; size_t n = 0;
    CHECK(platform_file_read("VERBS.BIN", &out, &n) == 0 &&
          n == sizeof es && memcmp(out, es, sizeof es) == 0,
          "carga del pack S");
    free(out);
    out = NULL; n = 0;
    CHECK(platform_file_read("VERBC.BIN", &out, &n) == 0 &&
          n == sizeof ca && memcmp(out, ca, sizeof ca) == 0,
          "carga del pack C");
    free(out);

    platform_input_set_auto(0);
    platform_set_idle(inject_idle);
    g_inject = "s";
    CHECK(kult_boot_language(g_img) == 'S', "la tecla 's' elige el pack S instalado");
    g_inject = "c";
    CHECK(kult_boot_language(g_img) == 'C', "la tecla 'c' elige el pack C instalado");
    platform_set_idle(NULL);

    remove(lang);
    platform_fs_reload_containers();
    platform_set_idle(inject_idle);
    g_inject = "s";
    CHECK(kult_boot_language(g_img) == 'E',
          "sin pack S instalado se cae a E");
    platform_set_idle(NULL);
    platform_input_set_auto(1);
}

/* El pack real versionado (assets/language.data) debe incluir DRAP y los dos
 * idiomas completos. Condicional: se omite si no está en el árbol de trabajo. */
static void test_language_assets(void) {
    static const char *const paths[] = {
        "assets/language.data", "../assets/language.data"
    };
    const char *path = NULL;
    for (size_t i = 0; i < sizeof paths / sizeof paths[0] && !path; i++) {
        FILE *f = fopen(paths[i], "rb");
        if (f) { fclose(f); path = paths[i]; }
    }
    if (!path) {
        fprintf(stderr, "nota: sin assets/language.data; se omite el pack real\n");
        return;
    }
    kult_container c;
    CHECK(kult_container_load(path, KULT_CONTAINER_LANG, &c) == 0,
          "assets/language.data debe ser un contenedor LANG");
    if (!c.data) return;

    static const char *const req[] = {
        "DRAP.PI1", "CARACS.BIN",
        "VERBS.BIN", "VERBC.BIN", "MOTSS.BIN", "MOTSC.BIN",
        "DESCS.BIN", "DESCC.BIN", "DIALS.BIN", "DIALC.BIN"
    };
    for (size_t i = 0; i < sizeof req / sizeof req[0]; i++) {
        uint32_t sz = 0;
        CHECK(kult_container_get(&c, req[i], &sz) != NULL && sz > 0,
              "el pack real debe incluir un fichero no vacio");
    }
    static const char *const pairs[][2] = {
        {"VERBS.BIN", "VERBC.BIN"}, {"MOTSS.BIN", "MOTSC.BIN"},
        {"DESCS.BIN", "DESCC.BIN"}, {"DIALS.BIN", "DIALC.BIN"}
    };
    for (size_t i = 0; i < sizeof pairs / sizeof pairs[0]; i++) {
        uint32_t se = 0, sc = 0;
        const uint8_t *pe = kult_container_get(&c, pairs[i][0], &se);
        const uint8_t *pc = kult_container_get(&c, pairs[i][1], &sc);
        CHECK(pe && pc && !(se == sc && memcmp(pe, pc, se) == 0),
              "los packs S y C no deben ser identicos");
    }
    kult_container_free(&c);
}

/* Log de diagnóstico: siempre en <base>/kult.log. */
static void test_log(void) {
    const char *path = DIR "/kult.log";
    platform_log_init();
    platform_log("prueba de log %d", 42);
    platform_log_close();

    FILE *f = fopen(path, "rb");
    CHECK(f != NULL, "el log debe crearse en <base>/kult.log");
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
}

/* Presentación por VBL: se presenta tanto en el arranque como dentro del bucle
 * (las animaciones de los comandos corren en `wait_vbl` y no pasan por el hook). */
static int g_present_n;
static void present_counter(uint8_t *img) { (void)img; g_present_n++; }
static void test_present_hook(void) {
    platform_set_present(present_counter);
    platform_set_loop(0);
    CHECK(!platform_in_loop(), "platform_in_loop debe ser 0 en el arranque");
    g_present_n = 0;
    platform_tick(g_img);
    CHECK(g_present_n == 1, "en arranque se presenta en cada tick");

    platform_set_loop(1);
    CHECK(platform_in_loop(), "platform_in_loop debe ser 1 dentro del bucle");
    g_present_n = 0;
    platform_tick(g_img);
    CHECK(g_present_n == 1, "en el bucle también se presenta en cada VBL");

    platform_set_loop(0);
    platform_set_present(NULL);
}

/* T-102: viewport entero Auto/1x..6x y OSD breve. */
static void test_display_viewport(void) {
    platform_viewport vp;

    CHECK(platform_display_scale(PLATFORM_SCALE_AUTO, 1920, 1080) == 5,
          "Auto en 1080p debe elegir x5");
    platform_display_viewport(PLATFORM_SCALE_AUTO, 1920, 1080, &vp);
    CHECK(vp.x == 160 && vp.y == 40 && vp.w == 1600 && vp.h == 1000 &&
          vp.scale == 5, "viewport Auto centrado en 1080p");

    CHECK(platform_display_scale(PLATFORM_SCALE_AUTO, 1280, 800) == 4,
          "Auto en 1280x800 debe encajar exacto a x4");
    platform_display_viewport(PLATFORM_SCALE_AUTO, 1280, 800, &vp);
    CHECK(vp.x == 0 && vp.y == 0 && vp.w == 1280 && vp.h == 800,
          "viewport Auto exacto sin bandas");

    CHECK(platform_display_scale(PLATFORM_SCALE_AUTO, 640, 400) == 2,
          "Auto en 640x400 debe elegir x2");
    CHECK(platform_display_scale(PLATFORM_SCALE_AUTO, 100, 100) == 1,
          "una salida menor que 320x200 no baja de x1");
    CHECK(platform_display_scale(PLATFORM_SCALE_AUTO, 1366, 768) == 3,
          "1366x768 solo admite x3 (x4 no cabe de alto)");

    CHECK(platform_display_scale(6, 1920, 1080) == 5,
          "una escala fija mayor que la salida se recorta al mayor entero");
    CHECK(platform_display_scale(2, 1920, 1080) == 2,
          "una escala fija que cabe se respeta");

    CHECK(platform_display_next(PLATFORM_SCALE_AUTO) == 1,
          "F2: Auto -> x1");
    CHECK(platform_display_next(3) == 4, "F2: x3 -> x4");
    CHECK(platform_display_next(PLATFORM_SCALE_MAX) == PLATFORM_SCALE_FILL,
          "F2: x6 -> Rellenar");
    CHECK(platform_display_next(PLATFORM_SCALE_FILL) == PLATFORM_SCALE_AUTO,
          "F2: Rellenar -> Auto");
    CHECK(platform_display_next(99) == 1, "un modo invalido se normaliza a Auto");

    CHECK(platform_display_scale(PLATFORM_SCALE_FILL, 1920, 1080) == 0,
          "Rellenar no usa escala entera");
    platform_display_viewport(PLATFORM_SCALE_FILL, 1920, 1080, &vp);
    CHECK(vp.fill && vp.x == 0 && vp.y == 0 && vp.w == 1920 && vp.h == 1080,
          "el viewport Rellenar ocupa toda la salida");

    int px = -1, py = -1;
    platform_display_viewport(PLATFORM_SCALE_AUTO, 1920, 1080, &vp);
    platform_display_point(&vp, 960, 540, &px, &py);
    CHECK(px == 160 && py == 100, "raton Auto centrado en 1080p");
    platform_display_point(&vp, 160, 40, &px, &py);
    CHECK(px == 0 && py == 0, "raton Auto en la esquina del viewport");
    platform_display_point(&vp, 0, 0, &px, &py);
    CHECK(px == 0 && py == 0, "raton Auto en banda se recorta al borde");

    platform_display_viewport(PLATFORM_SCALE_FILL, 1920, 1080, &vp);
    platform_display_point(&vp, 960, 540, &px, &py);
    CHECK(px == 160 && py == 100, "raton Rellenar centrado");
    platform_display_point(&vp, 0, 0, &px, &py);
    CHECK(px == 0 && py == 0, "raton Rellenar en esquina superior");
    platform_display_point(&vp, 1919, 1079, &px, &py);
    CHECK(px == 319 && py == 199, "raton Rellenar en esquina inferior");
}

static void test_osd(void) {
    platform_osd_clear();
    CHECK(!platform_osd_active(), "OSD vacio no debe estar activo");

    platform_osd_show_frames("RANURA 3", 1000);
    CHECK(platform_osd_active(), "OSD con margen debe estar activo");
    CHECK(strcmp(platform_osd_text(), "RANURA 3") == 0,
          "OSD debe conservar el texto");

    platform_osd_show_frames("X", 0);
    CHECK(!platform_osd_active(), "OSD con 0 frames debe caducar");
    platform_osd_clear();
    CHECK(!platform_osd_active(), "clear debe apagar el OSD");
}

/* T-103: savestate completo, validación de fichero y persistencia de ranura. */
static void test_osd_font(void) {
    uint8_t *rgba = malloc(320u * 200u * 4u);
    uint8_t *img = calloc(1, PLATFORM_IMAGE_SIZE);
    CHECK(rgba && img, "reservar buffers del OSD");
    if (!rgba || !img) { free(rgba); free(img); return; }

    const uint8_t glyph_a[6] = { 0x10, 0x08, 0x04, 0x02, 0x10, 0x00 };
    const uint8_t glyph_1[6] = { 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(img + A_FONT_DATA + 0x21 * 6, glyph_a, 6);
    memcpy(img + A_FONT_DATA + 0x11 * 6, glyph_1, 6);

    memset(rgba, 0x11, 320u * 200u * 4u);
    platform_osd_draw_rgba(rgba, img, "A1");
    const uint8_t *bg = rgba + ((189u * 320u + 3u) * 4u);
    const uint8_t *pa = rgba + ((190u * 320u + 4u) * 4u);
    const uint8_t *pa2 = rgba + ((191u * 320u + 5u) * 4u);
    const uint8_t *p1 = rgba + ((190u * 320u + 9u) * 4u);
    CHECK(bg[0] == 0 && bg[1] == 0 && bg[2] == 0 && bg[3] == 255,
          "el OSD debe pintar fondo negro");
    CHECK(pa[0] == 255 && pa[1] == 255 && pa[2] == 255 && pa[3] == 255,
          "el glifo A debe salir blanco en su bit");
    CHECK(pa2[0] == 255, "el segundo bit del glifo A debe salir blanco");
    CHECK(p1[0] == 255, "el digito 1 debe usar el glifo del juego");
    CHECK(rgba[0] == 0x11, "el OSD no debe tocar fuera de su caja");

    memset(rgba, 0x11, 320u * 200u * 4u);
    platform_osd_draw_rgba(rgba, img, "a");
    pa = rgba + ((190u * 320u + 4u) * 4u);
    CHECK(pa[0] == 255, "las minusculas deben mapearse a mayusculas");

    memset(rgba, 0x11, 320u * 200u * 4u);
    platform_osd_draw_rgba(rgba, img, "?");
    pa = rgba + ((190u * 320u + 4u) * 4u);
    CHECK(pa[0] == 0, "un caracter no soportado debe caer a espacio");

    memset(img + A_FONT_DATA + 0x21 * 6, 0, 6);
    memset(rgba, 0x11, 320u * 200u * 4u);
    platform_osd_draw_rgba(rgba, img, "A");
    CHECK(rgba[0] == 0x11, "sin fuente cargada no debe pintarse el OSD");

    free(rgba);
    free(img);
}

static void test_slots(void) {
    const int slot = 4;
    const char *path4 = DIR "/saves/slot-4.save";
    uint8_t *a = malloc(PLATFORM_IMAGE_SIZE);
    uint8_t *b = malloc(PLATFORM_IMAGE_SIZE);
    uint16_t oldpal[16];
    CHECK(a && b, "reservar buffers de ranura");
    if (!a || !b) { free(a); free(b); return; }

    for (size_t i = 0; i < PLATFORM_IMAGE_SIZE; i++)
        a[i] = (uint8_t)(i * 37u + 11u);
    memcpy(oldpal, platform_palette(), sizeof oldpal);
    uint32_t oldvis = platform_visible_base();
    pc_os_setscreen(0x98000, 0x98000, 0);
    for (int i = 0; i < 16; i++)
        pc_os_setcolor((int16_t)i, (int16_t)(0x100 + i * 0x11));

    remove(DIR "/kult.ini");
    platform_slots_init();
    CHECK(platform_slots_current() == 0, "sin kult.ini la ranura inicial es 0");
    for (int i = 0; i < 3; i++)
        platform_slots_select_next();
    CHECK(platform_slots_current() == 3, "F5 debe ciclar las ranuras");
    FILE *ini = fopen(DIR "/kult.ini", "rb");
    CHECK(ini != NULL, "la ranura debe persistirse en kult.ini");
    if (ini) {
        char txt[256] = {0};
        fread(txt, 1, sizeof txt - 1, ini);
        fclose(ini);
        CHECK(strstr(txt, "save_slot = 3") != NULL,
              "kult.ini debe contener la ranura activa");
    }
    platform_slots_init();
    CHECK(platform_slots_current() == 3, "kult.ini debe recordar la ranura");

    CHECK(platform_slots_save(a, slot) == 0, "guardar una ranura");
    FILE *sf = fopen(path4, "rb");
    CHECK(sf != NULL, "debe existir saves/slot-4.save");
    if (sf) {
        fseek(sf, 0, SEEK_END);
        long sz = ftell(sf);
        fclose(sf);
        CHECK(sz == (long)(64u + PLATFORM_IMAGE_SIZE),
              "la ranura debe tener cabecera + 1 MiB");
    }

    memset(a, 0, PLATFORM_IMAGE_SIZE);
    pc_os_setscreen(0x90000, 0x90000, 0);
    for (int i = 0; i < 16; i++)
        pc_os_setcolor((int16_t)i, 0);
    CHECK(platform_slots_load(a, slot) == 0, "cargar una ranura");
    int ok = 1;
    for (size_t i = 0; i < PLATFORM_IMAGE_SIZE; i++)
        if (a[i] != (uint8_t)(i * 37u + 11u)) { ok = 0; break; }
    CHECK(ok, "la carga debe restaurar la imagen completa");
    CHECK(platform_visible_base() == 0x98000,
          "la carga debe restaurar la base visible");
    ok = 1;
    for (int i = 0; i < 16; i++)
        if (platform_palette()[i] != (uint16_t)(0x100 + i * 0x11)) ok = 0;
    CHECK(ok, "la carga debe restaurar la paleta");

    memset(b, 0x5a, PLATFORM_IMAGE_SIZE);
    remove(DIR "/saves/slot-9.save");
    CHECK(platform_slots_load(b, 9) != 0, "una ranura ausente debe fallar");
    CHECK(b[0] == 0x5a, "una ranura ausente no debe tocar la imagen");

    const char *bad = DIR "/saves/slot-8.save";
    FILE *bf = fopen(bad, "wb");
    if (bf) {
        fwrite("KSLOTP01", 1, 8, bf);
        fclose(bf);
    }
    CHECK(platform_slots_load(b, 8) != 0, "una ranura truncada debe fallar");
    CHECK(b[0] == 0x5a, "una ranura truncada no debe tocar la imagen");

    CHECK(platform_slots_save(a, -1) != 0, "slot negativo invalido");
    CHECK(platform_slots_load(a, 10) != 0, "slot fuera de rango invalido");

    const char *path3 = DIR "/saves/slot-3.save";
    wr32(a + A_CURSOR_SAVE_DEST, 0x50000);
    wr32(a + A_SCREEN_PTR_A, 0x80000);
    wr32(a + A_SCREEN_PTR_B, 0x90000);
    wr32(a + A_CURSOR_SPRITE_PTR, 0x50000);
    wr16(a + A_CURSOR_X, 100);
    wr16(a + A_CURSOR_Y, 100);
    platform_set_loop(0);
    remove(path3);
    platform_slots_request_save();
    platform_slots_apply(a);
    CHECK(fopen(path3, "rb") == NULL,
          "F6 fuera del bucle no debe guardar");
    CHECK(strcmp(platform_osd_text(), "SOLO EN PARTIDA") == 0,
          "F6 fuera del bucle debe avisar");

    platform_set_loop(1);
    platform_slots_request_save();
    platform_slots_apply(a);
    CHECK(fopen(path3, "rb") != NULL, "F6 en el bucle debe guardar");
    CHECK(strstr(platform_osd_text(), "GUARDADA") != NULL,
          "el aviso de guardado debe ser visible");

    a[0] = 0xa5;
    platform_slots_request_load();
    platform_slots_apply(a);
    CHECK(a[0] == 11, "F7 debe restaurar la imagen guardada");
    CHECK(strstr(platform_osd_text(), "CARGADA") != NULL,
          "el aviso de carga debe ser visible");
    platform_set_loop(0);

    remove(path3);
    remove(path4);
    remove(bad);
    remove(DIR "/kult.ini");
    platform_video_restore(oldvis, oldpal);
    free(a);
    free(b);
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

    /* Ciclo `dbra d0` con d0=0xffff del retrato/intro (0x16a20), del anillo
     * (0x1566a) y de palette_cycle (0x16310, SOLAR EYES/T-090): 65536
     * iteraciones ~82 ms -> >= 4 VBLs. */
    f0 = platform_frame_count();
    platform_cpu_delay(0x10000u);
    CHECK(platform_frame_count() - f0 >= 4,
          "cpu_delay de 0x10000 unidades cede >= 4 VBLs (~82 ms)");
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
    platform_disk_timing(0);
    f0 = platform_frame_count();
    platform_disk_delay(800);
    CHECK(platform_frame_count() == f0,
          "disk_timing off no avanza frames");

    platform_disk_timing(1);
    f0 = platform_frame_count();
    platform_disk_delay(800);                   /* ~50 ms */
    CHECK(platform_frame_count() - f0 >= 2,
          "disk_delay en tiempo real cede >= 2 VBLs en ~50 ms");
    platform_disk_timing(1);
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

/* --- T-084: clic mantenido = UN clic; soltar y volver a pulsar = otro clic.
 * El IKBD original responde a 0x0D con eventos "desde la interrogarción
 * anterior", no con el nivel del botón. --- */
static void test_click_held(void) {
    platform_input_set_auto(0);

    platform_mouse_set(160, 100, 1);       /* bajada izquierda */
    g_img[A_IKBD_PACKET] = 0xff;
    g_img[A_SKIP_TRANSITION] = 0xff;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) != 0,
          "la bajada del izquierdo marca acción");
    CHECK(g_img[A_SKIP_TRANSITION] == 0,
          "el izquierdo NO debe saltar la transición");

    for (int i = 0; i < 32; i++) {         /* sigue pulsado: sin evento nuevo */
        g_img[A_IKBD_PACKET] = 0xff;
        g_img[A_SKIP_TRANSITION] = 0xff;
        wait_frame(g_img);
        platform_input_apply_skip(g_img);
        CHECK((g_img[A_INPUT_BUTTONS] & 0x04) == 0,
              "mantener el izquierdo NO repite el clic");
        CHECK(g_img[A_SKIP_TRANSITION] == 0,
              "mantener el izquierdo NO activa el skip");
    }

    platform_mouse_set(160, 100, 0);       /* soltar: tampoco es clic */
    g_img[A_IKBD_PACKET] = 0xff;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) == 0,
          "soltar el izquierdo no marca acción");

    platform_mouse_set(160, 100, 1);       /* nueva pulsación: nuevo clic */
    g_img[A_IKBD_PACKET] = 0xff;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) != 0,
          "una nueva pulsación tras soltar vuelve a marcar acción");

    platform_mouse_set(160, 100, 0);
    g_img[A_IKBD_PACKET] = 0xff;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    platform_input_set_auto(1);
}

/* --- T-084: derecho = acción + skip, también sólo en la bajada.
 * `platform_input_apply_skip` es el override que la HAL aplica al salir de
 * `wait_frame` (en el port lo llama el hook de instrumentación). --- */
static void test_click_skip(void) {
    platform_input_set_auto(0);

    platform_mouse_set(160, 100, 2);       /* bajada derecha: acción + skip */
    g_img[A_IKBD_PACKET] = 0xff;
    g_img[A_SKIP_TRANSITION] = 0;
    wait_frame(g_img);
    platform_input_apply_skip(g_img);
    CHECK((g_img[A_INPUT_BUTTONS] & 0x04) != 0,
          "el derecho debe marcar acción");
    CHECK(g_img[A_SKIP_TRANSITION] == 0xff,
          "el derecho SÍ debe saltar la transición");

    for (int i = 0; i < 32; i++) {         /* mantenido: ni acción ni skip */
        g_img[A_IKBD_PACKET] = 0xff;
        g_img[A_SKIP_TRANSITION] = 0xff;
        wait_frame(g_img);
        platform_input_apply_skip(g_img);
        CHECK((g_img[A_INPUT_BUTTONS] & 0x04) == 0,
              "mantener el derecho NO repite la acción");
        CHECK(g_img[A_SKIP_TRANSITION] == 0,
              "mantener el derecho NO repite el skip");
    }

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

/* --- T-084: dos teclas encoladas se leen como dos Crawio (no se pierde una). --- */
static void test_crawio_queue(void) {
    uint32_t k;
    while (os_crawio(g_img, 0x00ff) != 0) {}
    platform_key_push('A');
    platform_key_push('B');
    k = os_crawio(g_img, 0x00ff);
    CHECK((k & 0xff) == 'A', "la primera tecla encolada se lee primero");
    k = os_crawio(g_img, 0x00ff);
    CHECK((k & 0xff) == 'B', "la segunda tecla encolada no se pierde");
    CHECK(os_crawio(g_img, 0x00ff) == 0, "la cola queda vacía tras leer ambas");
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

static void test_replay_input(void) {
    char path[256];
    snprintf(path, sizeof path, "%s/replay.bin", DIR);
    unsetenv("KULT_REPLAY_PLAY");
    setenv("KULT_REPLAY_REC", path, 1);
    platform_replay_init();
    CHECK(platform_replay_recording(), "KULT_REPLAY_REC debe activar la grabacion");

    platform_input_set_auto(0);
    platform_mouse_set(12, 34, 1);
    platform_ikbd_deliver(g_img);
    platform_key_push('A');
    CHECK(pc_os_crawio(g_img, 0x00ff) == 'A', "Crawio grabado debe devolver A");
    platform_key_push('B');
    uint32_t out = 0;
    CHECK(pc_os_cnecin(g_img, &out) == 1 && out == 'B',
          "Cnecin grabado debe devolver B");
    platform_replay_shutdown();

    unsetenv("KULT_REPLAY_REC");
    setenv("KULT_REPLAY_PLAY", path, 1);
    platform_replay_init();
    CHECK(platform_replay_playing(), "KULT_REPLAY_PLAY debe activar el replay");

    memset(g_img + A_IKBD_PACKET, 0xff, 5);
    platform_ikbd_deliver(g_img);
    CHECK(g_img[A_IKBD_PACKET] == 1, "replay: boton izquierdo");
    CHECK(be16(g_img + A_CURSOR_X) == 12 && be16(g_img + A_CURSOR_Y) == 34,
          "replay: posicion del cursor");
    CHECK(pc_os_crawio(g_img, 0x00ff) == 'A', "replay: Crawio A");
    out = 0;
    CHECK(pc_os_cnecin(g_img, &out) == 1 && out == 'B', "replay: Cnecin B");
    platform_replay_shutdown();
    unsetenv("KULT_REPLAY_PLAY");
    remove(path);
}

int main(void) {
    make_fixture();
    platform_set_base_dir_for_tests(DIR);
    setenv("KULT_KEY", "E", 1);

    platform_set_image(g_img);
    platform_init();
    CHECK(platform_storage_ready(), "data/ y saves/ deben ser escribibles");

    test_fs();
    test_containers();
    test_diskimage_stx();
    test_diskimage_fat12();
    test_ingest_validate();
    test_ingest_pipeline();
    test_container_atomic();
    test_language_packs();
    test_language_assets();
    test_log();
    test_present_hook();
    test_display_viewport();
    test_osd();
    test_osd_font();
    test_slots();
    test_cpu_delay();
    test_cpu_charge_cycles();
    test_disk_delay();
    test_video();
    test_input_tick();
    test_real_input();
    test_click_held();
    test_click_skip();
    test_crawio_queue();
    test_replay_input();
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
    printf("RESULTADO: OK (fs/containers, ingesta STX/FAT12 y validacion PRG, "
           "contenedor atomico, packs S/C, log, present por VBL, display/OSD (fuente del juego), ranuras, "
           "cpu_delay, cpu_cycles, disk_delay, video, entrada/botones (clic mantenido), Crawio, replay, reloj, "
           "tick VBL, loader PRG, idioma, musica PSG, Timer A, RMS)\n");
    return 0;
}
