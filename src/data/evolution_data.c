/* FILE GENERATO da tools/genera_database.py - modifica lo script, non questo file. */
#include "evolution_data.h"
#include <stddef.h>

static const EvolutionData EVOLUZIONI[] = {
    { 1, 2, 16 },
    { 2, 3, 34 },
    { 4, 5, 16 },
    { 5, 6, 34 },
    { 7, 8, 16 },
    { 8, 9, 34 },
    { 10, 11, 18 },
    { 11, 12, 34 },
    { 13, 14, 18 },
    { 14, 15, 35 },
    { 16, 17, 20 },
    { 18, 19, 20 },
    { 20, 21, 20 },
    { 22, 23, 22 },
    { 24, 25, 22 },
    { 26, 27, 24 },
    { 28, 29, 24 },
    { 30, 31, 23 },
    { 32, 33, 25 },
    { 34, 35, 25 },
    { 37, 93, 27 },
    { 38, 60, 18 },
    { 39, 52, 32 },
    { 40, 95, 26 },
    { 41, 61, 30 },
    { 42, 96, 30 },
    { 43, 97, 28 },
    { 44, 98, 30 },
    { 45, 56, 29 },
    { 46, 89, 30 },
    { 47, 90, 30 },
    { 48, 49, 28 },
    { 50, 91, 34 },
};

#define NUM_EVO ((int)(sizeof(EVOLUZIONI) / sizeof(EVOLUZIONI[0])))

const EvolutionData *GetEvolution(int specie) {
    for (int i = 0; i < NUM_EVO; i++) if (EVOLUZIONI[i].da == specie) return &EVOLUZIONI[i];
    return NULL;
}

const EvolutionData *GetPreEvolution(int specie) {
    for (int i = 0; i < NUM_EVO; i++) if (EVOLUZIONI[i].a == specie) return &EVOLUZIONI[i];
    return NULL;
}

int Evolution_LivelloMinimo(int specie) {
    const EvolutionData *e = GetPreEvolution(specie);
    return e ? e->livello : 1;
}

int NumEvoluzioni(void) { return NUM_EVO; }

const EvolutionData *GetEvolutionIndex(int i) {
    return (i >= 0 && i < NUM_EVO) ? &EVOLUZIONI[i] : NULL;
}
