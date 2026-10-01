/* world.c */
#include "world.h"
#include "dialogue_ui.h"
#include "graphics.h"
#include "map_render.h"
#include <stdio.h>
#include <string.h>

#define PASSI_DISSOLVENZA 14

/* ------------------------------------------------------------------
 * Occupazione delle tile
 * ------------------------------------------------------------------ */
static const OggettoMappa *OggettoIn(const World *w, int x, int y) {
    for (int i = 0; i < w->mappa.numOggetti; i++) {
        const OggettoMappa *o = &w->mappa.oggetto[i];
        if (o->x == x && o->y == y && !Flags_Attivo(&w->partita->flags, o->flag)) return o;
    }
    return NULL;
}

static Npc *NpcIn(World *w, int x, int y) {
    for (int i = 0; i < w->numNpc; i++) {
        Npc *n = &w->npc[i];
        /* durante il passo un NPC occupa sia la tile di partenza sia quella di arrivo */
        if (n->tx == x && n->ty == y) return n;
        if (n->inMovimento && (n->px + TILE - 1) / TILE == x && (n->py + TILE - 1) / TILE == y) return n;
        if (n->inMovimento && n->px / TILE == x && n->py / TILE == y) return n;
    }
    return NULL;
}

static bool BloccataPerGiocatore(void *ctx, int x, int y) {
    World *w = (World *)ctx;
    const Warp *wp = Map_WarpIn(&w->mappa, x, y);
    if (wp && wp->suUrto) return true;            /* porta: si "urta" per entrare */
    return Map_Bloccata(&w->mappa, x, y) || NpcIn(w, x, y) != NULL || OggettoIn(w, x, y) != NULL;
}

static bool BloccataPerNpc(void *ctx, int x, int y, const Npc *chi) {
    World *w = (World *)ctx;
    if (Map_Bloccata(&w->mappa, x, y) || OggettoIn(w, x, y) || Map_WarpIn(&w->mappa, x, y)) return true;
    const Player *p = &w->player;
    if ((p->tx == x && p->ty == y) || (p->px / TILE == x && p->py / TILE == y) ||
        ((p->px + TILE - 1) / TILE == x && (p->py + TILE - 1) / TILE == y))
        return true;
    Npc *altro = NpcIn(w, x, y);
    return altro != NULL && altro != chi;
}

/* ------------------------------------------------------------------
 * Caricamento mappe
 * ------------------------------------------------------------------ */
static void AggiornaCamera(World *w) {
    int maxX = w->mappa.w * TILE - LOGICAL_W;
    int maxY = w->mappa.h * TILE - LOGICAL_H;
    int cx = w->player.px + TILE / 2 - LOGICAL_W / 2;
    int cy = w->player.py + TILE / 2 - LOGICAL_H / 2;
    w->camX = maxX < 0 ? maxX / 2 : Utils_ClampInt(cx, 0, maxX);   /* mappa piccola: centrata */
    w->camY = maxY < 0 ? maxY / 2 : Utils_ClampInt(cy, 0, maxY);
}

static void MostraTesto(World *w, const char *testo, const char *parlante) {
    Dialogo_AvviaTesto(&w->dialogo, testo, parlante);
    DialogoUI_Impagina(&w->dialogo);
}

bool World_Teletrasporta(World *w, const char *mappa, int x, int y, Direzione dir) {
    char errore[200];
    static Map nuova;   /* static: struttura grande */
    if (!Map_CaricaFile(&nuova, TextFormat("%s/maps", Gfx_CartellaAsset()), mappa, errore, sizeof(errore))) {
        TraceLog(LOG_ERROR, "ECLYRA: %s", errore);
        MostraTesto(w, TextFormat("Impossibile caricare la mappa.\n%s", errore), "");
        return false;
    }
    bool stessoNome = strcmp(nuova.nome, w->mappa.nome) == 0;
    w->mappa = nuova;
    if (x < 0 || y < 0) { x = w->mappa.spawnX; y = w->mappa.spawnY; dir = w->mappa.spawnDir; }

    w->numNpc = w->mappa.numNpc;
    for (int i = 0; i < w->numNpc; i++) Npc_Init(&w->npc[i], &w->mappa.npc[i], Rng_Next(&w->rng));
    Player_Init(&w->player, x, y, dir);
    w->npcInDialogo = NULL;
    if (w->mappa.tipo == MAPPA_ESTERNO && !stessoNome) w->timerNomeMappa = 150;

    Partita *p = w->partita;
    Utils_StrCopy(p->mappa, sizeof(p->mappa), mappa);
    p->x = x; p->y = y; p->dir = dir;
    AggiornaCamera(w);
    return true;
}

bool World_Init(World *w, Partita *partita, const DialogoDb *db) {
    memset(w, 0, sizeof(*w));
    w->partita = partita;
    w->db = db;
    Rng_Init(&w->rng, partita->semeRng ^ 0xA5A5A5A5u);
    return World_Teletrasporta(w, partita->mappa, partita->x, partita->y, partita->dir);
}

/* ------------------------------------------------------------------
 * Interazioni
 * ------------------------------------------------------------------ */
static void AvviaWarp(World *w, const Warp *wp, int x, int y) {
    int dx, dy;
    Map_DestinazioneWarp(wp, x, y, &dx, &dy);
    Utils_StrCopy(w->destMappa, sizeof(w->destMappa), wp->dest);
    w->destX = dx;
    w->destY = dy;
    w->destDir = wp->dir;
    w->transizione = TRANS_USCITA;
    w->timerTransizione = PASSI_DISSOLVENZA;
}

static void Interagisci(World *w) {
    int x, y;
    Player_TileDavanti(&w->player, &x, &y);

    Npc *n = NpcIn(w, x, y);
    if (n && !n->inMovimento) {
        Npc_GuardaVerso(n, w->player.tx, w->player.ty);
        n->parlando = true;
        w->npcInDialogo = n;
        Dialogo_Avvia(&w->dialogo, w->db, n->def->dialogo, &w->partita->flags,
                      w->partita->nomeGiocatore, n->def->nome);
        DialogoUI_Impagina(&w->dialogo);
        return;
    }
    const OggettoMappa *o = OggettoIn(w, x, y);
    if (o) {
        const ItemData *it = GetItem(o->oggetto);
        Inventario_Aggiungi(&w->partita->inventario, o->oggetto, o->quantita);
        Flags_Imposta(&w->partita->flags, o->flag);
        if (o->quantita > 1) MostraTesto(w, TextFormat("Hai trovato %s x%d!", it ? it->nome : "???", o->quantita), "");
        else MostraTesto(w, TextFormat("Hai trovato %s!", it ? it->nome : "???"), "");
        return;
    }
    const Cartello *c = Map_CartelloIn(&w->mappa, x, y);
    if (c) {
        Dialogo_Avvia(&w->dialogo, w->db, c->dialogo, &w->partita->flags, w->partita->nomeGiocatore, "");
        DialogoUI_Impagina(&w->dialogo);
    }
}

bool World_Libero(const World *w) {
    return !w->dialogo.attivo && w->transizione == TRANS_NESSUNA && !w->player.inMovimento;
}

void World_Aggiorna(World *w, const Input *in, float caratteriPerPasso) {
    w->frame++;
    if (w->timerNomeMappa > 0) w->timerNomeMappa--;

    /* le persone continuano a muoversi anche durante i dialoghi */
    for (int i = 0; i < w->numNpc; i++) Npc_Aggiorna(&w->npc[i], BloccataPerNpc, w, Rng_Next(&w->rng));

    if (w->transizione != TRANS_NESSUNA) {
        if (--w->timerTransizione <= 0) {
            if (w->transizione == TRANS_USCITA) {
                World_Teletrasporta(w, w->destMappa, w->destX, w->destY, w->destDir);
                w->transizione = TRANS_ENTRATA;
                w->timerTransizione = PASSI_DISSOLVENZA;
            } else {
                w->transizione = TRANS_NESSUNA;
            }
        }
        return;
    }

    if (w->dialogo.attivo) {
        if (!Dialogo_Aggiorna(&w->dialogo, in->conferma || in->indietro, caratteriPerPasso)) {
            if (w->npcInDialogo) w->npcInDialogo->parlando = false;
            w->npcInDialogo = NULL;
        }
        return;
    }

    if (in->conferma && !w->player.inMovimento) {
        Interagisci(w);
        if (w->dialogo.attivo) return;
    }

    int ux = 0, uy = 0;
    EsitoPasso esito = Player_Aggiorna(&w->player, in, BloccataPerGiocatore, w, &ux, &uy);
    if (esito == PASSO_URTO) {
        const Warp *wp = Map_WarpIn(&w->mappa, ux, uy);
        if (wp && wp->suUrto) AvviaWarp(w, wp, ux, uy);
    } else if (esito == PASSO_ARRIVATO) {
        Partita *p = w->partita;
        p->x = w->player.tx; p->y = w->player.ty; p->dir = w->player.dir;
        const Warp *wp = Map_WarpIn(&w->mappa, w->player.tx, w->player.ty);
        if (wp && !wp->suUrto) AvviaWarp(w, wp, w->player.tx, w->player.ty);
    }
    AggiornaCamera(w);
}

/* ------------------------------------------------------------------
 * Disegno
 * ------------------------------------------------------------------ */
static void DisegnaOggetto(int x, int y, int frame) {
    /* CORE a terra: cristallo di Lumen che pulsa */
    int cy = y + 8 + ((frame / 20) % 2);
    DrawEllipse(x + 8, y + 13, 5, 2, (Color){ 0, 0, 0, 60 });
    DrawTriangle((Vector2){ (float)x + 8, (float)cy - 6 }, (Vector2){ (float)x + 3, (float)cy },
                 (Vector2){ (float)x + 13, (float)cy }, (Color){ 120, 232, 208, 255 });
    DrawTriangle((Vector2){ (float)x + 3, (float)cy }, (Vector2){ (float)x + 8, (float)cy + 5 },
                 (Vector2){ (float)x + 13, (float)cy }, (Color){ 56, 152, 168, 255 });
    DrawPixel(x + 7, cy - 3, WHITE);
}

void World_Disegna(const World *w) {
    MapRender_Terreno(&w->mappa, w->camX, w->camY, w->frame);

    int prima, ultima;
    MapRender_RigheVisibili(w->camY, &prima, &ultima);
    for (int riga = prima; riga <= ultima; riga++) {
        MapRender_OggettiRiga(&w->mappa, riga, w->camX, w->camY);
        for (int i = 0; i < w->mappa.numOggetti; i++) {
            const OggettoMappa *o = &w->mappa.oggetto[i];
            if (o->y == riga && !Flags_Attivo(&w->partita->flags, o->flag))
                DisegnaOggetto(o->x * TILE - w->camX, o->y * TILE - w->camY, w->frame);
        }
        for (int i = 0; i < w->numNpc; i++)
            if (w->npc[i].ty == riga) Npc_Disegna(&w->npc[i], w->camX, w->camY);
        if (riga == w->player.ty) Player_Disegna(&w->player, w->camX, w->camY);
    }

    if (w->timerNomeMappa > 0) {
        int y = w->timerNomeMappa > 130 ? -(w->timerNomeMappa - 130) * 2 : 4;   /* scende dall'alto */
        int larg = Gfx_LarghezzaTesto(w->mappa.nome, 10) + 20;
        Gfx_Finestra(4, y, larg, 20);
        Gfx_Testo(w->mappa.nome, 14, y + 6, COL_TESTO);
    }
    DialogoUI_Disegna(&w->dialogo, w->frame);

    if (w->transizione == TRANS_USCITA)
        Gfx_Dissolvenza(1.0f - (float)w->timerTransizione / PASSI_DISSOLVENZA);
    else if (w->transizione == TRANS_ENTRATA)
        Gfx_Dissolvenza((float)w->timerTransizione / PASSI_DISSOLVENZA);
}
