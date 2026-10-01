/* npc.h - Personaggi non giocanti: stato in gioco, movimento, disegno. */
#ifndef ECLYRA_NPC_H
#define ECLYRA_NPC_H

#include "map.h"
#include <stdbool.h>

#define NPC_SPRITE_MAX 8   /* personaggi nel foglio assets/sprites/npc.png */

typedef struct {
    const NpcDef *def;
    int tx, ty;            /* tile (destinazione durante il passo) */
    int px, py;
    Direzione dir;
    bool inMovimento;
    int timer;             /* passi logici alla prossima azione */
    int passi;
    bool parlando;         /* fermo mentre e' in dialogo */
} Npc;

/* Funzione che dice se una tile e' occupata (mappa, giocatore, altri NPC) */
typedef bool (*FnBloccato)(void *ctx, int x, int y, const Npc *chiChiede);

void Npc_Init(Npc *n, const NpcDef *def, unsigned seme);
void Npc_Aggiorna(Npc *n, FnBloccato bloccato, void *ctx, unsigned casuale);
void Npc_GuardaVerso(Npc *n, int x, int y);
void Npc_Disegna(const Npc *n, int camX, int camY);

#endif
