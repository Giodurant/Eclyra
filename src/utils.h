/* utils.h - Funzioni di supporto senza dipendenze grafiche (testabili). */
#ifndef ECLYRA_UTILS_H
#define ECLYRA_UTILS_H

#include <stdint.h>
#include <stddef.h>

/* Generatore pseudo-casuale deterministico (xorshift32).
 * Il seme va salvato nel file di gioco: stessa partita, stessi risultati. */
typedef struct {
    uint32_t stato;
} Rng;

void     Rng_Init(Rng *r, uint32_t seme);
uint32_t Rng_Next(Rng *r);
/* Intero uniforme in [0, n). Ritorna 0 se n <= 1. */
int      Rng_Range(Rng *r, int n);
/* Reale uniforme in [0, 1). */
float    Rng_Float(Rng *r);

int   Utils_ClampInt(int v, int min, int max);
float Utils_ClampF(float v, float min, float max);

/* Divisione intera protetta: se il divisore e' 0 usa 1. */
int   Utils_DivSafe(int a, int b);

/* Copia sicura di stringhe (termina sempre con '\0'). */
void  Utils_StrCopy(char *dst, size_t dstSize, const char *src);

#endif
