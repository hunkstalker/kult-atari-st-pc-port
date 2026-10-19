/* platform/include/os.h — contrato de los servicios OS del port PC.
 *
 * Esta cabecera SOMBREA `tools/recreate_kit/include/os.h` en el build del
 * ejecutable (se pasa `-Iplatform/include` antes que el del kit). El core (`src/`)
 * no se toca: sigue escribiendo `#include "os.h"` y llamando a los mismos nombres,
 * pero las respuestas ya no son el modelo "staged-file" del diferencial, sino el
 * sistema de ficheros real de PC y el estado de la HAL.
 *
 * Sólo se reproducen aquí los nombres y constantes que `src/` usa de verdad
 * (medido con `nm -u` y grep sobre `src/`): el resto del os.h del kit es el modelo
 * del diferencial y no se necesita. `machine.h` (be16/be32/wr16/wr32) se sigue
 * tomando del kit, que no conoce ni SO ni hardware.
 *
 * Frontera: las funciones `pc_*` viven en los .c de `platform/`; aquí sólo se
 * declaran y se envuelven en `static inline` con los nombres que el core espera.
 */
#ifndef KULT_PLATFORM_OS_H
#define KULT_PLATFORM_OS_H

#include <stdint.h>
#include <string.h>

#include "machine.h"   /* be16/be32/wr16/wr32 (del kit, portables) */

/* ---- constantes que `src/` usa de os.h ---- */
/* Base de pantalla (Physbase). El kit usa 0x80000, pero en el port esa zona la
 * ocupa el juego: los bancos ECHE se cargan contiguos desde 0x64c8e y llegan
 * hasta ~0x88b84, pisando la pantalla B (0x78000) y su paleta (0x77fe0) -> ruido
 * y colores mal. Se usa 0x98000 como en el predecesor (pantallas 0x90000/0x98000,
 * por encima de ECHE y por debajo de 0xA0000). El diferencial (`make test`) sigue
 * con el valor del kit (no incluye esta cabecera). */
#define OS_SCREEN_BASE   0x98000u
#define OS_KBDVBASE      0x500u     /* resultado de XBIOS Kbdvbase() (struct in-image) */
#define OS_SUPER_ENTER   0u         /* Super(0) */
#define OS_PSG_DATA_READ 0xffu      /* lectura abierta de $ff8802 (no la usa el port) */

/* ---- estado compartido de la HAL ---- */
/* La imagen plana del juego (1 MiB, índices = direcciones). La fija app/main; la
 * usan los servicios que reciben una dirección sin puntero (paleta). */
extern uint8_t *g_kult_img;

/* DTA actual (Fsetdta) — la usa os_fsfirst para volcar tamaño/nombre. */
extern uint32_t g_os_dta;

/* ---- contabilidad del kit (no-op en el port) ---- */
int32_t os_refused(int32_t sentinel);          /* devuelve `sentinel`, sin tally */
void    g_os_event(uint16_t kind, uint32_t value);

/* ---- back-ends reales (implementados en los .c de platform/) ---- */
int32_t pc_os_fopen(uint8_t *mem, uint32_t name_ptr);
int32_t pc_os_fcreate(uint8_t *mem, uint32_t name_ptr);
int32_t pc_os_fread(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf);
int32_t pc_os_fwrite(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf);
int32_t pc_os_fclose(uint8_t *mem, uint16_t handle);
int32_t pc_os_fsfirst(uint8_t *mem, uint32_t name_ptr);

void pc_os_setscreen(uint32_t log_base, uint32_t phys_base, int16_t resolution);
void pc_os_setpalette(uint32_t table);
void pc_os_setcolor(int16_t index, int16_t colour);
uint32_t pc_os_crawio(uint8_t *mem, uint16_t w);
int      pc_os_cnecin(uint8_t *mem, uint32_t *out);

/* ---- ficheros (GEMDOS) ---- */
static inline int32_t os_fopen(uint8_t *mem, uint32_t name_ptr) { return pc_os_fopen(mem, name_ptr); }
static inline int32_t os_fcreate(uint8_t *mem, uint32_t name_ptr) { return pc_os_fcreate(mem, name_ptr); }
static inline int32_t os_fread(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf) {
    return pc_os_fread(mem, handle, count, buf);
}
static inline int32_t os_fwrite(uint8_t *mem, uint16_t handle, uint32_t count, uint32_t buf) {
    return pc_os_fwrite(mem, handle, count, buf);
}
static inline int32_t os_fclose(uint8_t *mem, uint16_t handle) { return pc_os_fclose(mem, handle); }
static inline void os_fsetdta(uint32_t dta) { g_os_dta = dta; }
static inline int32_t os_fsfirst(uint8_t *mem, uint32_t name_ptr) { return pc_os_fsfirst(mem, name_ptr); }

/* ---- sistema ---- */
/* Super(0) devuelve un token; el token se acepta de vuelta. No hay privilegios en PC. */
#define OS_SUPER_TOKEN 0x00535550u
static inline int os_super(uint32_t arg, uint32_t *out) {
    if (arg == OS_SUPER_ENTER) { if (out) *out = OS_SUPER_TOKEN; return 1; }
    if (out) *out = 0;
    return 1;
}

/* ---- vídeo ---- */
static inline void os_setscreen(uint32_t log_base, uint32_t phys_base, int16_t resolution) {
    pc_os_setscreen(log_base, phys_base, resolution);
}
static inline void os_setpalette(uint32_t table) { pc_os_setpalette(table); }
static inline void os_setcolor(int16_t index, int16_t colour) { pc_os_setcolor(index, colour); }

/* ---- entrada ---- */
static inline uint32_t os_crawio(uint8_t *mem, uint16_t w) { return pc_os_crawio(mem, w); }
static inline int os_cnecin(uint8_t *mem, uint32_t *out) { return pc_os_cnecin(mem, out); }

#endif /* KULT_PLATFORM_OS_H */
