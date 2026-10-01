/* elyon.h - Un singolo Elyon (istanza): statistiche, nature, IV, XP, mosse, stato.
 *
 * Formule del documento di design:
 *   HP    = baseHP + 3*L + floor(L/2) + IV
 *   altre = floor((base + 2*L + IV) * natura)        natura = 0.9 / 1.0 / 1.1
 *   XP cumulativa per il livello L = floor(crescita * L^3)
 * Struttura senza puntatori: si salva campo per campo. */
#ifndef ECLYRA_ELYON_H
#define ECLYRA_ELYON_H

#include "data/moves_data.h"
#include "data/species_data.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>

#define LIVELLO_MAX 100
#define IV_MAX 15
#define MAX_MOSSE_ELYON 4
#define AMICIZIA_MAX 255

typedef enum {
    NATURA_NEUTRO = 0, NATURA_CORAGGIOSO, NATURA_MENTALE, NATURA_FERMO, NATURA_RAPIDO,
    NATURA_TENACE, NATURA_IMPULSIVO, NATURA_CALMO, NATURA_ARDENTE, NATURA_SERENO,
    NUM_NATURE
} Natura;

typedef struct {
    int specie;                     /* 0 = slot vuoto */
    int forma;
    char soprannome[16];
    int livello;
    int32_t xp;
    int iv[NUM_STAT];
    Natura natura;
    int hp;
    int stat[NUM_STAT];             /* stat[STAT_HP] = HP massimi */
    int mosse[MAX_MOSSE_ELYON];
    int pp[MAX_MOSSE_ELYON];
    StatusId status;                /* solo stati persistenti (la confusione vive in lotta) */
    int turniSonno;
    int contatoreVelenoGrave;
    int amicizia;
    int oggetto;                    /* oggetto equipaggiato (0 = nessuno) */
    bool varianteLumen;             /* variante cosmetica 1/256 */
} Elyon;

const char *Natura_Nome(Natura n);
/* Moltiplicatore x100 (90, 100, 110) della natura per una statistica (non HP) */
int  Natura_Percento(Natura n, StatId s);

int32_t Elyon_XpPerLivello(const SpeciesData *s, int livello);
int  Elyon_StatFormula(StatId s, int base, int livello, int iv, Natura n);

/* Crea un Elyon con IV e natura casuali e le ultime 4 mosse apprese entro il livello */
void Elyon_Crea(Elyon *e, int specie, int livello, Rng *rng);
/* Ricalcola le statistiche mantenendo la proporzione degli HP */
void Elyon_RicalcolaStat(Elyon *e);

bool Elyon_Vuoto(const Elyon *e);
bool Elyon_Esausto(const Elyon *e);
const char *Elyon_Nome(const Elyon *e);
int  Elyon_NumMosse(const Elyon *e);
void Elyon_Cura(Elyon *e);                  /* HP, PP e stato */

/* Prova a imparare una mossa: true se c'era posto (o la conosce gia') */
bool Elyon_ImparaMossa(Elyon *e, int mossa);
void Elyon_SostituisciMossa(Elyon *e, int slot, int mossa);

/* Esperienza: aggiunge XP senza superare il livello massimo.
 * Ritorna il numero di livelli guadagnati (le mosse NON vengono imparate qui). */
int  Elyon_AggiungiXp(Elyon *e, int32_t xp);
/* Mosse che la specie impara esattamente a un livello (fino a 4) */
int  Elyon_MosseAlLivello(const Elyon *e, int livello, int *mosse, int max);
/* Specie in cui puo' evolvere ora (0 se nessuna). Mai prima del livello definito. */
int  Elyon_PuoEvolvere(const Elyon *e);
void Elyon_Evolvi(Elyon *e, int nuovaSpecie);

/* ------------------------------------------------------------------
 * Squadra (max 6) e box (5 x 20)
 * ------------------------------------------------------------------ */
#define MAX_SQUADRA 6
#define NUM_BOX 5
#define POSTI_BOX 20

typedef struct {
    Elyon squadra[MAX_SQUADRA];
    Elyon box[NUM_BOX][POSTI_BOX];
} Collezione;

void Collezione_Azzera(Collezione *c);
int  Collezione_NumSquadra(const Collezione *c);
int  Collezione_SquadraViva(const Collezione *c);
int  Collezione_NumBox(const Collezione *c);
/* Aggiunge in squadra, altrimenti nel primo posto libero del box.
 * Ritorna 0 = squadra, 1 = box, -1 = nessuno spazio. */
int  Collezione_Aggiungi(Collezione *c, const Elyon *e);
bool Collezione_Deposita(Collezione *c, int indiceSquadra);   /* non lascia la squadra vuota */
bool Collezione_Ritira(Collezione *c, int box, int posto);
void Collezione_Compatta(Collezione *c);                      /* niente buchi in squadra */
void Collezione_CuraSquadra(Collezione *c);

#endif
