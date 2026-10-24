#ifndef KULT_CONTAINER_H
#define KULT_CONTAINER_H

#include <stddef.h>
#include <stdint.h>

#define KULT_CONTAINER_GAME 1u
#define KULT_CONTAINER_LANG 2u

typedef struct {
    char name[64];
    uint32_t offset;
    uint32_t size;
} kult_container_entry;

typedef struct {
    uint8_t *data;
    size_t len;
    uint32_t type;
    uint32_t count;
    kult_container_entry *entries;
} kult_container;

int  kult_container_load(const char *path, uint32_t expected_type, kult_container *out);
void kult_container_free(kult_container *c);
const uint8_t *kult_container_get(const kult_container *c, const char *name, uint32_t *size);
int  kult_container_write(const char *path, uint32_t type,
                          const char *const *names, const uint8_t *const *blobs,
                          const uint32_t *sizes, uint32_t count);

#endif
