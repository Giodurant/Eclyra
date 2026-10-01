/* encounters_data.h - Habitat e tabelle degli incontri selvatici. */
#ifndef ECLYRA_ENCOUNTERS_DATA_H
#define ECLYRA_ENCOUNTERS_DATA_H

#define MAX_SPECIE_HABITAT 8

typedef struct {
    int specie;
    int percentuale;
} VoceIncontro;

typedef struct {
    const char *id;             /* es. "ROUTE1" */
    const char *nome;
    int livelloMin, livelloMax;
    VoceIncontro voci[MAX_SPECIE_HABITAT];
    int numVoci;                /* 0 = nessun incontro (Lunaria) */
} HabitatData;

const HabitatData *GetHabitat(const char *id);
int NumHabitat(void);
const HabitatData *GetHabitatIndex(int i);

#endif
