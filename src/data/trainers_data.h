/* trainers_data.h - Allenatori, Guardiani, Nexus, Lega e boss con squadre canoniche. */
#ifndef ECLYRA_TRAINERS_DATA_H
#define ECLYRA_TRAINERS_DATA_H

#include <stdbool.h>

#define MAX_SQUADRA_ALLENATORE 6

/* AI_INTRO: lotta introduttiva (prima sfida di Kael), piu' blanda di AI_SIMPLE.
 * L'ordine conta: i livelli piu' alti sono "piu' furbi" (confronti con < e >=). */
typedef enum { AI_INTRO = 0, AI_SIMPLE, AI_STANDARD, AI_SMART, AI_BOSS } LivelloIA;

typedef struct {
    int specie;
    int livello;
    int mosse[4];               /* 0 = mosse dal learnset */
} MembroSquadra;

typedef struct {
    const char *id;             /* es. "GUARDIANA_LIRA" */
    const char *nome;
    const char *classe;
    int premio;                 /* Lumen */
    LivelloIA ia;
    MembroSquadra squadra[MAX_SQUADRA_ALLENATORE];
    int numMembri;
    const char *ricompensa;     /* es. "SIGILLO_VERDE", "" se nessuna */
    bool interruzioneNarrativa; /* la lotta si interrompe a meta' HP dell'ultimo Elyon (Vael intermezzo) */
    bool eccezioneLivello;      /* squadra canonica con un Elyon sotto il livello di evoluzione */
    const char *nota;
} TrainerData;

const TrainerData *GetTrainer(const char *id);
int NumAllenatori(void);
const TrainerData *GetTrainerIndex(int i);

#endif
