/* player.h - Il protagonista: movimento a griglia, corsa, animazione. */
#ifndef ECLYRA_PLAYER_H
#define ECLYRA_PLAYER_H

#include "input.h"
#include "map.h"
#include <stdbool.h>

typedef enum { DIR_GIU = 0, DIR_SU, DIR_SINISTRA, DIR_DESTRA } Direzione;

typedef struct {
    int tx, ty;          /* tile attuale (o di destinazione durante il passo) */
    int px, py;          /* posizione in pixel */
    Direzione dir;
    bool inMovimento;
    bool corsa;
    int attesaGiro;      /* un tocco breve gira il personaggio senza muoverlo */
    int passi;
} Player;

void Player_Init(Player *p, int tx, int ty, Direzione dir);
/* Ritorna true nel passo in cui arriva su una nuova tile */
bool Player_Aggiorna(Player *p, const Input *in, const Map *m);
void Player_TileDavanti(const Player *p, int *x, int *y);
void Player_Disegna(const Player *p, int camX, int camY);

#endif
