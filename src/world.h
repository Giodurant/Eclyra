/* world.h - Il mondo esplorabile: mappa corrente, NPC, giocatore, camera,
 * porte e transizioni, interazioni e dialoghi. */
#ifndef ECLYRA_WORLD_H
#define ECLYRA_WORLD_H

#include "dialogue.h"
#include "map.h"
#include "npc.h"
#include "partita.h"
#include "player.h"
#include "utils.h"

typedef enum { TRANS_NESSUNA, TRANS_USCITA, TRANS_ENTRATA } StatoTransizione;

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

#endif
