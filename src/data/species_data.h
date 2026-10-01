/* species_data.h - Bestiario: 100 specie di Elyon con ID stabili 1..100.
 * Le forme alternative (es. AURION VOID / ORIGINALE) non occupano un ID specie. */
#ifndef ECLYRA_SPECIES_DATA_H
#define ECLYRA_SPECIES_DATA_H

#include "types.h"

#define NUM_SPECIE 100
#define MAX_LEARNSET 16

typedef enum { STAT_HP = 0, STAT_ATK, STAT_DEF, STAT_SPA, STAT_SPD, STAT_SPE, NUM_STAT } StatId;
typedef enum { RARITA_COMUNE = 0, RARITA_RARO, RARITA_LEGGENDARIO, RARITA_PRIMORDIALE } Rarita;

typedef struct {
    int livello;
    int mossa;          /* ID mossa (moves_data.h) */
} LearnEntry;

typedef struct {
    int id;
    const char *nome;
    ElementType tipo1, tipo2;     /* tipo2 = TYPE_NONE se monotipo */
    int base[NUM_STAT];
    int abilita;                  /* ID abilita' (abilities_data.h) */
    Rarita rarita;
    float catchRate;              /* probabilita' base di cattura (0..1) */
    float crescita;               /* XP cumulativa = floor(crescita * L^3): 0.8 / 1.0 / 1.2 */
    int baseYield;                /* resa di XP quando viene sconfitto */
    const char *descrizione;
    LearnEntry learnset[MAX_LEARNSET];
    int numLearnset;
} SpeciesData;

typedef struct {
    int specie;                   /* ID specie a cui appartiene la forma */
    int forma;                    /* 0 = base */
    const char *nome;
    int abilita;
    const char *nota;
} FormData;

/* NULL se l'ID non e' valido (1..100) */
const SpeciesData *GetSpecies(int id);
/* Cerca per nome (maiuscolo, es. "PYRIN"); NULL se non esiste */
const SpeciesData *GetSpeciesByName(const char *nome);

const FormData *GetForm(int specie, int forma);
int NumForme(void);

const char *Rarita_Nome(Rarita r);

#endif
