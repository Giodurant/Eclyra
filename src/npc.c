/* npc.c */
#include "npc.h"
#include "graphics.h"
#include <stdlib.h>

#define RAGGIO_VAGA 3   /* tile massime di distanza dal punto di partenza */

void Npc_Init(Npc *n, const NpcDef *def, unsigned seme) {
    *n = (Npc){ 0 };
    n->def = def;
    n->tx = def->x;
    n->ty = def->y;
    n->px = def->x * TILE;
    n->py = def->y * TILE;
    n->dir = def->dir;
    n->timer = 60 + (int)(seme % 120);
}

void Npc_GuardaVerso(Npc *n, int x, int y) {
    int dx = x - n->tx, dy = y - n->ty;
    if (abs(dx) > abs(dy)) n->dir = dx > 0 ? DIR_DESTRA : DIR_SINISTRA;
    else if (dy != 0) n->dir = dy > 0 ? DIR_GIU : DIR_SU;
}

void Npc_Aggiorna(Npc *n, FnBloccato bloccato, void *ctx, unsigned casuale) {
    if (n->inMovimento) {
        int gx = n->tx * TILE, gy = n->ty * TILE;
        if (n->px < gx) n->px++; else if (n->px > gx) n->px--;
        if (n->py < gy) n->py++; else if (n->py > gy) n->py--;
        if (n->px == gx && n->py == gy) { n->inMovimento = false; n->passi++; }
        return;
    }
    if (n->parlando || n->def->mov == MOV_FERMO) return;
    if (--n->timer > 0) return;
    n->timer = 90 + (int)(casuale % 150);

    Direzione d = (Direzione)((casuale >> 8) % 4);
    n->dir = d;
    if (n->def->mov != MOV_VAGA) return;

    int nx = n->tx + Dir_DX(d), ny = n->ty + Dir_DY(d);
    if (abs(nx - n->def->x) > RAGGIO_VAGA || abs(ny - n->def->y) > RAGGIO_VAGA) return;
    if (bloccato(ctx, nx, ny, n)) return;
    n->tx = nx;
    n->ty = ny;
    n->inMovimento = true;
}

void Npc_Disegna(const Npc *n, int camX, int camY) {
    int x = n->px - camX, y = n->py - camY;
    int progresso = (n->dir == DIR_SU || n->dir == DIR_GIU) ? (n->py % TILE) : (n->px % TILE);
    bool meta = n->inMovimento && progresso >= 4 && progresso < 12;
    int fotogramma = meta ? 1 + (n->passi % 2) : 0;
    int riga = n->dir == DIR_SU ? 1 : (n->dir == DIR_GIU ? 0 : 2);
    float larghezza = n->dir == DIR_DESTRA ? -16.0f : 16.0f;
    int sprite = n->def->sprite % NPC_SPRITE_MAX;

    DrawEllipse(x + 8, y + 15, 6, 2, (Color){ 0, 0, 0, 60 });
    /* foglio: ogni personaggio occupa un blocco 48x72 (3 fotogrammi x 3 direzioni), affiancati */
    Rectangle src = { (float)(sprite * 48 + fotogramma * 16), (float)(riga * 24), larghezza, 24 };
    DrawTextureRec(Gfx_Texture(TEX_NPC), src, (Vector2){ (float)x, (float)(y - 8) }, WHITE);
}
