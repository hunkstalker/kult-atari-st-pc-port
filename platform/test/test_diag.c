/* platform/test/test_diag.c — humo del diagnóstico opcional (T-100).
 *
 * Se ejecuta como dos procesos (`off` y `on`) porque los ficheros de log se
 * abren una sola vez por proceso:
 *   off: sin variables no se crea ningún fichero;
 *   on:  KULT_MARKS/RINGLOG/RANDLOG/INPUTLOG producen sus líneas y F12 vuelca
 *        el historial y la última línea de texto.
 * Solo enlaza platform/diag.c: el reloj y la imagen son stubs locales.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "machine.h"
#include "addrs.h"
#include "platform.h"
#include "kult_port.h"
#include "diag.h"

#define IMG_SIZE 0x100000u
#define DIR "/tmp/kult_diag_test"

static uint8_t g_img[IMG_SIZE];
uint8_t *g_kult_img = g_img;
static uint32_t g_vbl;
static int g_fail;

uint32_t platform_frame_count(void) { return g_vbl; }

#define CHECK(cond, msg) do { \
    if (!(cond)) { fprintf(stderr, "FALLO: %s\n", msg); g_fail = 1; } \
} while (0)

static int read_text(const char *path, char *buf, size_t cap) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    size_t n = fread(buf, 1, cap - 1, f);
    buf[n] = '\0';
    fclose(f);
    return (int)n;
}

static int contains(const char *path, const char *needle) {
    static char buf[65536];
    int n = read_text(path, buf, sizeof buf);
    return n >= 0 && strstr(buf, needle) != NULL;
}

static int file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void remove_logs(void) {
    remove(DIR "/marks.log");
    remove(DIR "/ring.log");
    remove(DIR "/rand.log");
    remove(DIR "/input.log");
}

static void call_all_without_logs(void) {
    g_vbl = 10;
    platform_diag_tick();
    platform_diag_func_begin(0x15da6, 10, 1);
    platform_diag_func_end(0x15da6, 10);
    platform_diag_func_end(0x11f2e, 10);
    platform_diag_func_end(0x17ca4, 10);
    platform_diag_bank(0x15a90, 0x15da6);
    platform_diag_ev("ring_open", g_img, 1, 2, 3);
    platform_text_ev("line", g_img, 0x21000, 0x21004, 12, 0x34);
    platform_diag_rnd("rnd_b", g_img, 1, 2, 3);
    platform_diag_input("click", g_img, 10, 20, 1);
    platform_marks_line("D prueba");
    platform_marks_byte(10, 0x3d, 0x12345);
    platform_marks_state(10, 0x3d, 0x12345, 1, 2, 3, 4, 5, 6, 7, 8,
                         9, 10, 11, 12, 13);
    platform_marks_event("f12", 0, 0);
}

static void test_off(void) {
    unsetenv("KULT_MARKS");
    unsetenv("KULT_RINGLOG");
    unsetenv("KULT_RANDLOG");
    unsetenv("KULT_INPUTLOG");
    call_all_without_logs();
    CHECK(!file_exists(DIR "/marks.log"), "sin KULT_MARKS no se crea el log");
    CHECK(!file_exists(DIR "/ring.log"), "sin KULT_RINGLOG no se crea el log");
    CHECK(!file_exists(DIR "/rand.log"), "sin KULT_RANDLOG no se crea el log");
    CHECK(!file_exists(DIR "/input.log"), "sin KULT_INPUTLOG no se crea el log");
}

static void test_on(void) {
    setenv("KULT_MARKS", DIR "/marks.log", 1);
    setenv("KULT_RINGLOG", DIR "/ring.log", 1);
    setenv("KULT_RANDLOG", DIR "/rand.log", 1);
    setenv("KULT_INPUTLOG", DIR "/input.log", 1);

    memcpy(g_img + 0x21000, "TEST", 4);
    g_img[A_SKIP_TRANSITION] = 0xff;
    g_vbl = 10;
    platform_diag_tick();
    platform_diag_func_begin(0x15da6, 10, 1);
    platform_diag_func_end(0x15da6, 10);
    platform_diag_func_begin(0x12345, 10, 1);   /* fuera de la lista curada */
    platform_diag_func_end(0x11f2e, 10);
    platform_diag_func_end(0x17ca4, 10);
    platform_diag_bank(0x15a90, 0x15da6);
    platform_diag_ev("ring_open", g_img, 1, 2, 3);
    platform_text_ev("line", g_img, 0x21000, 0x21004, 12, 0x34);
    platform_diag_rnd("rnd_b", g_img, 1, 2, 3);
    platform_diag_input("click", g_img, 10, 20, 1);
    platform_marks_line("D %u prueba", 10);
    platform_marks_byte(10, 0x3d, 0x12345);
    platform_marks_state(10, 0x3d, 0x12345, 1, 2, 3, 4, 5, 6, 7, 8,
                         9, 10, 11, 12, 13);
    platform_marks_event("f12", 0, 0);
    platform_diag_func_begin(0x12345, 11, 1);   /* ventana F12: ahora sí */
    platform_diag_func_begin(0x12345, 200, 1);  /* fuera de la ventana */

    CHECK(contains(DIR "/marks.log", "# RUN"), "cabecera del log de marcas");
    CHECK(contains(DIR "/marks.log", "K 10 255"), "cambio de skip (K)");
    CHECK(contains(DIR "/marks.log", "F 15da6 10"), "marca curada (F)");
    CHECK(contains(DIR "/marks.log", "S 10 0 0 0 0"), "marca de animación (S)");
    CHECK(contains(DIR "/marks.log", "B 10 fn=11f2e"), "traza de blit (B)");
    CHECK(contains(DIR "/marks.log", "U 10 sel="), "traza de objeto (U)");
    CHECK(contains(DIR "/marks.log", "B 10 15a90 15da6"), "traza de banco (B)");
    CHECK(contains(DIR "/marks.log", "D 10 prueba"), "línea libre (D)");
    CHECK(contains(DIR "/marks.log", "Y 10 3d 12345"), "byte de VM (Y)");
    CHECK(contains(DIR "/marks.log", "V 10 3d 12345"), "estado de VM (V)");
    CHECK(contains(DIR "/marks.log", "X line 10"), "instantánea de texto (X)");
    CHECK(contains(DIR "/marks.log", "raw=54455354"), "bytes de la línea de texto");
    CHECK(contains(DIR "/marks.log", "M f12 10"), "marca manual F12 (M)");
    CHECK(contains(DIR "/marks.log", "F 12345 11"), "ventana posterior a F12");
    CHECK(!contains(DIR "/marks.log", "F 12345 200"),
          "fuera de la ventana F12 no hay traza completa");

    CHECK(contains(DIR "/ring.log", "R 10 ring_open"), "evento del anillo (R)");
    CHECK(contains(DIR "/ring.log", "T 10 line"), "evento de texto (T)");
    CHECK(contains(DIR "/rand.log", "D 10 rnd_b"), "lectura ALEAT (D)");
    CHECK(contains(DIR "/input.log", "I 10 click"), "evento de entrada (I)");
}

int main(int argc, char **argv) {
    system("mkdir -p " DIR);
    remove_logs();
    if (argc == 2 && strcmp(argv[1], "off") == 0) {
        test_off();
    } else if (argc == 2 && strcmp(argv[1], "on") == 0) {
        test_on();
    } else {
        fprintf(stderr, "uso: %s off|on\n", argv[0]);
        return 2;
    }
    if (g_fail) { fprintf(stderr, "RESULTADO: FALLO\n"); return 1; }
    printf("RESULTADO: OK (diag %s)\n", argv[1]);
    return 0;
}
