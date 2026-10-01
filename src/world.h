/* world.h - Il mondo esplorabile: mappa corrente, giocatore, camera. */
#ifndef ECLYRA_WORLD_H
#define ECLYRA_WORLD_H

#include "map.h"
#include "player.h"

typedef struct {
    Map mappa;
    Player player;
    int camX, camY;          /* angolo in alto a sinistra della camera, in pixel */
    int frame;
    int timerNomeMappa;      /* banner con il nome della zona all'ingresso */
    const char *messaggio;   /* messaggio semplice (cartelli); NULL = nessuno */
} World;

void World_Init(World *w);
/* Ritorna true se il mondo ha "catturato" l'input (es. messaggio aperto) */
void World_Aggiorna(World *w, const Input *in);
void World_Disegna(const World *w);
void World_AggiornaCamera(World *w);

#endif
