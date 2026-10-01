/* player.h - Il protagonista: movimento a griglia, corsa, animazione. */
#ifndef ECLYRA_PLAYER_H
#define ECLYRA_PLAYER_H

#include "common.h"
#include "input.h"
#include <stdbool.h>

typedef struct {
    int tx, ty;          /* tile attuale (o di destinazione durante il passo) */
    int px, py;          /* posizione in pixel */
    Direzione dir;
    bool inMovimento;
    bool corsa;
    int attesaGiro;      /* un tocco breve gira il personaggio senza muoverlo */
    int passi;
} Player;

typedef enum {
    PASSO_NIENTE,        /* fermo o a meta' passo */
    PASSO_ARRIVATO,      /* e' appena arrivato su una nuova tile */
    PASSO_URTO           /* ha provato a entrare in una tile bloccata (urtoX, urtoY) */
} EsitoPasso;

typedef bool (*FnTileBloccata)(void *ctx, int x, int y);

void Player_Init(Player *p, int tx, int ty, Direzione dir);
EsitoPasso Player_Aggiorna(Player *p, const Input *in, FnTileBloccata bloccata, void *ctx,
                           int *urtoX, int *urtoY);
void Player_TileDavanti(const Player *p, int *x, int *y);
void Player_Disegna(const Player *p, int camX, int camY);

#endif
