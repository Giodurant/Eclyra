/* test_utils.c - RNG deterministico, limiti, divisione sicura, stringhe */
#include "test_framework.h"
#include "utils.h"
#include <string.h>

int main(void) {
    /* Stesso seme -> stessa sequenza (necessario per i salvataggi) */
    Rng a, b;
    Rng_Init(&a, 12345);
    Rng_Init(&b, 12345);
    int uguali = 1;
    for (int i = 0; i < 1000; i++) if (Rng_Next(&a) != Rng_Next(&b)) uguali = 0;
    VERIFICA(uguali);

    /* Seme diverso -> sequenza diversa */
    Rng_Init(&b, 54321);
    VERIFICA(Rng_Next(&a) != Rng_Next(&b));

    /* Seme 0 non blocca il generatore */
    Rng z;
    Rng_Init(&z, 0);
    VERIFICA(Rng_Next(&z) != 0);

    /* Valori noti per il seme 1 (regressione: non cambiare l'algoritmo senza aggiornare i salvataggi) */
    Rng r1;
    Rng_Init(&r1, 1);
    VERIFICA(Rng_Next(&r1) == 270369u);

    /* Range sempre nei limiti, e distribuzione ragionevole */
    int conteggi[6] = { 0 }, fuori = 0;
    for (int i = 0; i < 60000; i++) {
        int v = Rng_Range(&a, 6);
        if (v < 0 || v >= 6) fuori++; else conteggi[v]++;
    }
    VERIFICA(fuori == 0);
    for (int i = 0; i < 6; i++) VERIFICA(conteggi[i] > 9000 && conteggi[i] < 11000);
    VERIFICA(Rng_Range(&a, 0) == 0);
    VERIFICA(Rng_Range(&a, 1) == 0);
    VERIFICA(Rng_Range(&a, -5) == 0);

    float f;
    int floatOk = 1;
    for (int i = 0; i < 10000; i++) { f = Rng_Float(&a); if (f < 0.0f || f >= 1.0f) floatOk = 0; }
    VERIFICA(floatOk);

    /* Limiti e divisione per zero */
    VERIFICA(Utils_ClampInt(-3, 0, 10) == 0);
    VERIFICA(Utils_ClampInt(13, 0, 10) == 10);
    VERIFICA(Utils_ClampInt(5, 0, 10) == 5);
    VERIFICA(Utils_DivSafe(10, 0) == 10);
    VERIFICA(Utils_DivSafe(10, 3) == 3);

    /* Copia di stringhe con troncamento */
    char buf[5];
    Utils_StrCopy(buf, sizeof(buf), "LUNARIA");
    VERIFICA(strcmp(buf, "LUNA") == 0);
    Utils_StrCopy(buf, sizeof(buf), NULL);
    VERIFICA(buf[0] == '\0');

    return FINE_TEST();
}
