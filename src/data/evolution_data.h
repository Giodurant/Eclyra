/* evolution_data.h - Evoluzioni canoniche per livello. */
#ifndef ECLYRA_EVOLUTION_DATA_H
#define ECLYRA_EVOLUTION_DATA_H

typedef struct {
    int da, a;          /* ID specie */
    int livello;        /* livello minimo */
} EvolutionData;

/* Evoluzione della specie (NULL se non evolve) */
const EvolutionData *GetEvolution(int specie);
/* Da quale specie si evolve (NULL se e' una forma base) */
const EvolutionData *GetPreEvolution(int specie);
/* Livello minimo al quale una specie puo' esistere per evoluzione naturale (1 per le forme base) */
int Evolution_LivelloMinimo(int specie);

int NumEvoluzioni(void);
const EvolutionData *GetEvolutionIndex(int i);

#endif
