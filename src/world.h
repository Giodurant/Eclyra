/* world.h - Il mondo esplorabile: mappa corrente, NPC, giocatore, camera,
 * porte e transizioni, interazioni e dialoghi. */
#ifndef ECLYRA_WORLD_H
#define ECLYRA_WORLD_H

#include "battle.h"
#include "dialogue.h"
#include "map.h"
#include "npc.h"
#include "partita.h"
#include "player.h"
#include "utils.h"

typedef enum { TRANS_NESSUNA, TRANS_USCITA, TRANS_ENTRATA } StatoTransizione;

/* Richieste che il mondo passa al gioco (lotte, PC) */
typedef enum { RICH_NESSUNA = 0, RICH_SELVATICO, RICH_ALLENATORE, RICH_PC } TipoRichiesta;

#define FLAG_HA_ELYON "HA_ELYON"
#define PROBABILITA_INCONTRO 12     /* % per passo nell'erba alta */

typedef struct {
    Partita *partita;
    const DialogoDb *db;

    Map mappa;
    Npc npc[MAP_MAX_NPC];
    int numNpc;
    Player player;
    int camX, camY;
    int frame;
    int timerNomeMappa;
    Rng rng;

    Dialogo dialogo;
    Npc *npcInDialogo;

    /* richiesta per il gioco: letta e azzerata da Game */
    TipoRichiesta richiesta;
    int richSpecie, richLivello;
    const TrainerData *richAllenatore;
    char richFlag[32];

    /* azioni rimandate alla fine del dialogo */
    const TrainerData *lottaDopoDialogo;
    char flagDopoDialogo[32];
    bool curaDopoDialogo;
    int starterInAttesa;          /* specie proposta dalla capsula */
    bool domandaStarter;
    int cursoreSiNo;

    StatoTransizione transizione;
    int timerTransizione;
    char destMappa[MAP_NOME_ID];
    int destX, destY;
    Direzione destDir;
} World;

/* Carica la mappa salvata in partita->mappa (posizione -1 = SPAWN della mappa) */
bool World_Init(World *w, Partita *partita, const DialogoDb *db);
/* Cambio mappa immediato, senza dissolvenza (nuova partita, debug) */
bool World_Teletrasporta(World *w, const char *mappa, int x, int y, Direzione dir);

void World_Aggiorna(World *w, const Input *in, float caratteriPerPasso);
/* true se il giocatore e' libero (nessun dialogo, transizione o passo in corso) */
bool World_Libero(const World *w);
void World_Disegna(const World *w);

/* Mostra un testo semplice nel riquadro dei dialoghi */
void World_Messaggio(World *w, const char *testo);
/* Esito della lotta richiesta: flag dell'allenatore, ritorno a casa dopo una sconfitta */
void World_DopoLotta(World *w, EsitoLotta esito);
/* Nome dello starter scelto ("PYRIN", "AQUIL", "MOSSEN") o NULL */
const char *World_Starter(const World *w);

#endif
