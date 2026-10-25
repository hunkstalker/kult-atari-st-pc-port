/* platform/diskimage.h — lectura de imágenes de disco ST (Pasti/STX y FAT12).
 *
 * Extraído de ingest.c para poder probar la extracción con imágenes sintéticas
 * (T-095). Solo lo usa la ingesta y sus pruebas; no forma parte del contrato del
 * core.
 */
#ifndef KULT_DISKIMAGE_H
#define KULT_DISKIMAGE_H

#include <stddef.h>
#include <stdint.h>

#define KULT_MAX_FILES 64

typedef struct {
    char name[64];
    uint8_t *data;
    uint32_t size;
} kult_memfile;

typedef struct {
    kult_memfile v[KULT_MAX_FILES];
    int n;
} kult_filelist;

/* 1 si el nombre termina en .stx (sin distinguir mayúsculas). */
int kult_stx_name(const char *name);

/* Convierte un .STX Pasti a imagen de sectores raw (80 pistas, 9 sectores).
 * Devuelve 0 o -1 (fichero ilegible o cabecera Pasti inválida). */
int kult_stx_to_raw(const char *path, uint8_t *raw, size_t raw_size);

/* Extrae los ficheros de una imagen FAT12 a `out` (sin duplicados, sin
 * subdirectorios). Devuelve 0 o -1. */
int kult_fat12_extract(const uint8_t *raw, size_t raw_len, kult_filelist *out);

/* Busca un nombre (sin distinguir mayúsculas) en la lista. */
const kult_memfile *kult_filelist_find(const kult_filelist *fl, const char *name);

/* Libera los blobs de la lista y la deja vacía. */
void kult_filelist_free(kult_filelist *fl);

/* Comprueba los ficheros obligatorios y el SHA-256 de KULT.PRG. */
int kult_ingest_validate(const kult_filelist *fl);

/* 1 si el SHA-256 es un disco conocido; *role = 0 (A) o 1 (B). */
int kult_ingest_known_disk(const uint8_t h[32], int *role);

#endif /* KULT_DISKIMAGE_H */
