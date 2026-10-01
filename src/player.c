/* player.c */
#include "player.h"
#include "graphics.h"

#define VEL_CAMMINO 1   /* pixel per passo logico: 16 passi per tile */
#define VEL_CORSA   2
#define PASSI_GIRO  6

void Player_Init(Player *p, int tx, int ty, Direzione dir) {
    *p = (Player){ 0 };
    p->tx = tx;
    p->ty = ty;
    p->px = tx * TILE;
    p->py = ty * TILE;
    p->dir = dir;
}

void Player_TileDavanti(const Player *p, int *x, int *y) {
    *x = p->tx;
    *y = p->ty;
    switch (p->dir) {
        case DIR_SU:       (*y)--; break;
        case DIR_GIU:      (*y)++; break;
        case DIR_SINISTRA: (*x)--; break;
        case DIR_DESTRA:   (*x)++; break;
    }
}

static bool DirezionePremuta(const Input *in, Direzione *d) {
    if (in->su)       { *d = DIR_SU; return true; }
    if (in->giu)      { *d = DIR_GIU; return true; }
    if (in->sinistra) { *d = DIR_SINISTRA; return true; }
    if (in->destra)   { *d = DIR_DESTRA; return true; }
    return false;
}

bool Player_Aggiorna(Player *p, const Input *in, const Map *m) {
    if (p->inMovimento) {
        int v = p->corsa ? VEL_CORSA : VEL_CAMMINO;
        int gx = p->tx * TILE, gy = p->ty * TILE;
        if (p->px < gx) p->px += v; else if (p->px > gx) p->px -= v;
        if (p->py < gy) p->py += v; else if (p->py > gy) p->py -= v;
        if (p->px == gx && p->py == gy) {
            p->inMovimento = false;
            p->passi++;
            return true;
        }
        return false;
    }

    Direzione d;
    if (!DirezionePremuta(in, &d)) {
        p->attesaGiro = 0;
        return false;
    }
    if (d != p->dir) {
        p->dir = d;
        p->attesaGiro = PASSI_GIRO;
        return false;
    }
    if (p->attesaGiro > 0) {
        p->attesaGiro--;
        return false;
    }
    int nx, ny;
    Player_TileDavanti(p, &nx, &ny);
    if (!Map_Bloccata(m, nx, ny)) {
        p->tx = nx;
        p->ty = ny;
        p->corsa = in->corsa;
        p->inMovimento = true;
    }
    return false;
}

void Player_Disegna(const Player *p, int camX, int camY) {
    int x = p->px - camX, y = p->py - camY;
    int progresso = (p->dir == DIR_SU || p->dir == DIR_GIU) ? (p->py % TILE) : (p->px % TILE);
    bool meta = p->inMovimento && progresso >= 4 && progresso < 12;
    int fotogramma = meta ? 1 + (p->passi % 2) : 0;

    int riga = 0;
    float larghezza = 16;
    switch (p->dir) {
        case DIR_GIU:      riga = 0; break;
        case DIR_SU:       riga = 1; break;
        case DIR_SINISTRA: riga = 2; break;
        case DIR_DESTRA:   riga = 2; larghezza = -16; break;   /* specchiato */
    }
    DrawEllipse(x + 8, y + 15, 6, 2, (Color){ 0, 0, 0, 60 });
    Rectangle src = { (float)(fotogramma * 16), (float)(riga * 24), larghezza, 24 };
    DrawTextureRec(Gfx_Texture(TEX_GIOCATORE), src, (Vector2){ (float)x, (float)(y - 8) }, WHITE);
}
