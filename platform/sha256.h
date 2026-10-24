#ifndef KULT_SHA256_H
#define KULT_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t h[8];
    uint64_t len;
    uint8_t buf[64];
    size_t n;
} kult_sha256;

void kult_sha256_init(kult_sha256 *s);
void kult_sha256_update(kult_sha256 *s, const void *data, size_t len);
void kult_sha256_final(kult_sha256 *s, uint8_t out[32]);

#endif
