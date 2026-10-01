/* world.c */
#include "world.h"
#include "graphics.h"
#include "utils.h"
#include <stddef.h>

void World_Init(World *w) {
    *w = (World){ 0 };
    Map_GeneraLunariaProvvisoria(&w->mappa);
    /* davanti alla porta di casa (casa in (8,18), porta in (10,21)) */
    Player_Init(&w->player, 10, 22, DIR_GIU);
    w->timerNomeMappa = 150;
    World_AggiornaCamera(w);
}

void World_AggiornaCamera(World *w) {
    int maxX = w->mappa.w * TILE - LOGICAL_W;
    int maxY = w->mappa.h * TILE - LOGICAL_H;
    int cx = w->player.px + TILE / 2 - LOGICAL_W / 2;
    int cy = w->player.py + TILE / 2 - LOGICAL_H / 2;
    /* se la mappa e' piu' piccola dello schermo, la centro */
    w->camX = maxX < 0 ? maxX / 2 : Utils_ClampInt(cx, 0, maxX);
    w->camY = maxY < 0 ? maxY / 2 : Utils_ClampInt(cy, 0, maxY);
}

void World_Aggiorna(World *w, const Input *in) {
    w->frame++;
    if (w->timerNomeMappa > 0) w->timerNomeMappa--;

    if (w->messaggio) {
        if (in->conferma || in->indietro) w->messaggio = NULL;
        return;
    }
    if (in->conferma && !w->player.inMovimento) {
        int x, y;
        Player_TileDavanti(&w->player, &x, &y);
        if (Map_Tile(&w->mappa, x, y) == T_CARTELLO)
            w->messaggio = "LUNARIA\nIl villaggio dove ogni viaggio comincia.";
    }
    Player_Aggiorna(&w->player, in, &w->mappa);
    World_AggiornaCamera(w);
}

void World_Disegna(const World *w) {
    Map_DisegnaTerreno(&w->mappa, w->camX, w->camY, w->frame);

    int prima, ultima;
    Map_RigheVisibili(w->camY, &prima, &ultima);
    for (int riga = prima; riga <= ultima; riga++) {
        Map_DisegnaOggettiRiga(&w->mappa, riga, w->camX, w->camY);
        if (riga == w->player.ty) Player_Disegna(&w->player, w->camX, w->camY);
    }

    if (w->timerNomeMappa > 0) {
        int y = w->timerNomeMappa > 130 ? -(w->timerNomeMappa - 130) * 2 : 4;   /* scende dall'alto */
        Gfx_Finestra(4, y, 84, 20);
        Gfx_Testo(w->mappa.nome, 12, y + 6, COL_TESTO);
    }
    if (w->messaggio) {
        Gfx_Finestra(4, LOGICAL_H - 48, LOGICAL_W - 8, 44);
        Gfx_Testo(w->messaggio, 14, LOGICAL_H - 38, COL_TESTO);
    }
}
