/* utils.c */
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void Rng_Init(Rng *r, uint32_t seme) {
    r->stato = seme ? seme : 0x9E3779B9u;   /* lo stato non puo' essere zero */
}

uint32_t Rng_Next(Rng *r) {
    uint32_t x = r->stato;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->stato = x;
    return x;
}

int Rng_Range(Rng *r, int n) {
    if (n <= 1) return 0;
    return (int)(Rng_Next(r) % (uint32_t)n);
}

float Rng_Float(Rng *r) {
    return (float)(Rng_Next(r) >> 8) / 16777216.0f;   /* 24 bit di mantissa */
}

int Utils_ClampInt(int v, int min, int max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

float Utils_ClampF(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

int Utils_DivSafe(int a, int b) {
    return a / (b == 0 ? 1 : b);
}

void Utils_StrCopy(char *dst, size_t dstSize, const char *src) {
    if (!dst || dstSize == 0) return;
    size_t i = 0;
    if (src) {
        for (; i + 1 < dstSize && src[i] != '\0'; i++) dst[i] = src[i];
    }
    dst[i] = '\0';
}

char *Utils_LeggiFile(const char *percorso) {
    FILE *f = fopen(percorso, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0 || n > 16 * 1024 * 1024) { fclose(f); return NULL; }   /* limite di sicurezza */
    rewind(f);
    char *buf = (char *)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t letti = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[letti] = '\0';
    return buf;
}
