/* test_mappe.c - Caricamento, raggiungibilita', porte e coordinate delle mappe.
 * Uso: test_mappe <cartella assets> */
#include "map.h"
#include "data/encounters_data.h"
#include "data/trainers_data.h"
#include "test_framework.h"
#include <stdlib.h>
#include <string.h>

static Map mappe[16];
static char cartellaMappe[512];

static int IndiceMappa(const char *id) {
    for (int i = 0; i < NUM_MAPPE_GIOCO; i++)
        if (strcmp(MAPPE_GIOCO[i], id) == 0) return i;
    return -1;
}

/* Raggiungibilita' a piedi dallo SPAWN (le porte "PORTA" non si attraversano) */
static void Raggiungibili(const Map *m, unsigned char vis[MAP_MAX_H][MAP_MAX_W]) {
    memset(vis, 0, MAP_MAX_H * MAP_MAX_W);
    static int coda[MAP_MAX_W * MAP_MAX_H][2];
    int testa = 0, fine = 0;
    coda[fine][0] = m->spawnX; coda[fine][1] = m->spawnY; fine++;
    vis[m->spawnY][m->spawnX] = 1;
    while (testa < fine) {
        int x = coda[testa][0], y = coda[testa][1];
        testa++;
        static const int d[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
        for (int k = 0; k < 4; k++) {
            int nx = x + d[k][0], ny = y + d[k][1];
            if (!Map_Dentro(m, nx, ny) || vis[ny][nx] || Map_Bloccata(m, nx, ny)) continue;
            vis[ny][nx] = 1;
            coda[fine][0] = nx; coda[fine][1] = ny; fine++;
        }
    }
}

static int VicinoRaggiungibile(const Map *m, unsigned char vis[MAP_MAX_H][MAP_MAX_W], int x, int y) {
    static const int d[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
    for (int k = 0; k < 4; k++) {
        int nx = x + d[k][0], ny = y + d[k][1];
        if (Map_Dentro(m, nx, ny) && vis[ny][nx]) return 1;
    }
    return 0;
}

static void TestParserRobusto(void) {
    Map m;
    char err[200];
    VERIFICA(!Map_CaricaDaTesto(&m, "x", NULL, err, sizeof(err)));
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "", err, sizeof(err)));
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI 3 2\nTILES\n...\n", err, sizeof(err)));          /* riga mancante */
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI 3 1\nTILES\n.Z.\n", err, sizeof(err)));          /* carattere ignoto */
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI 3 1\nTILES\n..\n", err, sizeof(err)));           /* riga corta */
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI 999 999\nTILES\n", err, sizeof(err)));           /* troppo grande */
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI -1 3\n", err, sizeof(err)));
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI 3 1\nTILES\n...\nPORTA 9 9 y 0 0 SU\n", err, sizeof(err)));
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI 3 1\nTILES\n...\nNPC a 0 1 0 GIU BALLA D Nome\n", err, sizeof(err)));
    VERIFICA(!Map_CaricaDaTesto(&m, "x", "DIMENSIONI 3 1\nTILES\n...\nFOO 1\n", err, sizeof(err)));
    VERIFICA(Map_CaricaDaTesto(&m, "x", "# ok\nNOME Prova\nDIMENSIONI 3 1\nTILES\n.=.\r\nSPAWN 1 0 SU\n", err, sizeof(err)));
    VERIFICA(strcmp(m.nome, "Prova") == 0 && m.w == 3 && Map_Tile(&m, 1, 0) == T_SENTIERO);
    VERIFICA(Map_Tile(&m, -1, 0) == T_ALBERO && Map_Tile(&m, 99, 99) == T_ALBERO);   /* fuori mappa */
    VERIFICA(!Map_CaricaFile(&m, "/percorso/inesistente", "nulla", err, sizeof(err)));
    /* NPC con nome composto da piu' parole */
    VERIFICA(Map_CaricaDaTesto(&m, "x", "DIMENSIONI 3 1\nTILES\n...\nNPC a 0 1 0 GIU FERMO DLG Prof. Elian\n", err, sizeof(err)));
    VERIFICA(m.numNpc == 1 && strcmp(m.npc[0].nome, "Prof. Elian") == 0);
}

int main(int argc, char **argv) {
    const char *assets = argc > 1 ? argv[1] : "assets";
    snprintf(cartellaMappe, sizeof(cartellaMappe), "%s/maps", assets);
    char err[200];

    TestParserRobusto();

    /* 1) tutte le mappe si caricano */
    for (int i = 0; i < NUM_MAPPE_GIOCO; i++) {
        int ok = Map_CaricaFile(&mappe[i], cartellaMappe, MAPPE_GIOCO[i], err, sizeof(err));
        if (!ok) printf("  errore: %s\n", err);
        VERIFICA(ok);
    }

    /* 2) per ogni mappa: spawn, warp, NPC, cartelli, oggetti raggiungibili e destinazioni valide */
    static unsigned char vis[MAP_MAX_H][MAP_MAX_W];
    for (int i = 0; i < NUM_MAPPE_GIOCO; i++) {
        const Map *m = &mappe[i];
        VERIFICA(!Map_Bloccata(m, m->spawnX, m->spawnY));
        Raggiungibili(m, vis);
        for (int k = 0; k < m->numWarp; k++) {
            const Warp *w = &m->warp[k];
            int dest = IndiceMappa(w->dest);
            if (dest < 0) printf("  %s: destinazione sconosciuta %s\n", m->id, w->dest);
            VERIFICA(dest >= 0);
            int raggiunto = 0;
            for (int y = w->y; y < w->y + w->h; y++)
                for (int x = w->x; x < w->x + w->w; x++) {
                    if (w->suUrto) raggiunto |= VicinoRaggiungibile(m, vis, x, y) && Map_Bloccata(m, x, y);
                    else raggiunto |= vis[y][x];
                    if (dest >= 0) {
                        int dx, dy;
                        Map_DestinazioneWarp(w, x, y, &dx, &dy);
                        const Map *d = &mappe[dest];
                        int valida = Map_Dentro(d, dx, dy) && !Map_Bloccata(d, dx, dy);
                        const Warp *ritorno = Map_WarpIn(d, dx, dy);
                        if (!valida || ritorno) printf("  %s -> %s (%d,%d) non valida o sopra un'uscita\n", m->id, d->id, dx, dy);
                        VERIFICA(valida && ritorno == NULL);   /* niente rimbalzo immediato */
                    }
                }
            if (!raggiunto) printf("  %s: warp %d non raggiungibile\n", m->id, k);
            VERIFICA(raggiunto);
        }
        for (int k = 0; k < m->numNpc; k++) {
            const NpcDef *n = &m->npc[k];
            int ok = !Map_Bloccata(m, n->x, n->y) && VicinoRaggiungibile(m, vis, n->x, n->y);
            if (!ok) printf("  %s: NPC %s non raggiungibile\n", m->id, n->id);
            VERIFICA(ok);
        }
        for (int k = 0; k < m->numCartelli; k++) {
            int ok = VicinoRaggiungibile(m, vis, m->cartello[k].x, m->cartello[k].y);
            if (!ok) printf("  %s: cartello %d non raggiungibile\n", m->id, k);
            VERIFICA(ok);
        }
        for (int k = 0; k < m->numOggetti; k++) {
            const OggettoMappa *o = &m->oggetto[k];
            VERIFICA(!Map_Bloccata(m, o->x, o->y) && VicinoRaggiungibile(m, vis, o->x, o->y));
        }
    }

    /* 3) da casa del giocatore si raggiungono tutte le mappe attraverso i warp */
    int visitata[16] = { 0 }, coda[16], t = 0, f = 0;
    int partenza = IndiceMappa("casa_giocatore");
    VERIFICA(partenza >= 0);
    if (partenza >= 0) { coda[f++] = partenza; visitata[partenza] = 1; }
    while (t < f) {
        const Map *m = &mappe[coda[t++]];
        for (int k = 0; k < m->numWarp; k++) {
            int d = IndiceMappa(m->warp[k].dest);
            if (d >= 0 && !visitata[d]) { visitata[d] = 1; coda[f++] = d; }
        }
    }
    for (int i = 0; i < NUM_MAPPE_GIOCO; i++) {
        if (!visitata[i]) printf("  mappa %s non collegata\n", MAPPE_GIOCO[i]);
        VERIFICA(visitata[i]);
    }

    /* 4) dimensioni e coordinate del documento di design */
    const Map *lun = &mappe[IndiceMappa("lunaria")];
    const Map *r1 = &mappe[IndiceMappa("route1")];
    VERIFICA(lun->w == 40 && lun->h == 30);
    VERIFICA(r1->w == 60 && r1->h == 25);
    VERIFICA(Map_Tile(lun, 8, 18) == T_CASA);        /* casa del giocatore */
    VERIFICA(Map_Tile(lun, 28, 8) == T_LAB);         /* laboratorio */
    VERIFICA(Map_Tile(lun, 6, 7) == T_NEGOZIO);      /* negozio */
    VERIFICA(!Map_Bloccata(lun, 19, 15));            /* piazza */
    VERIFICA(Map_WarpIn(lun, 19, 0) != NULL);        /* uscita verso la Route 1 */
    VERIFICA(Map_WarpIn(r1, 30, 24) != NULL);        /* entrata da Lunaria */
    VERIFICA(Map_Tile(r1, 30, 0) == T_SENTIERO);     /* varco del Bosco */
    int haCore = 0, erba = 0;
    for (int k = 0; k < r1->numOggetti; k++)
        if (r1->oggetto[k].x == 45 && r1->oggetto[k].y == 14 && r1->oggetto[k].oggetto == 12) haCore = 1;
    VERIFICA(haCore);
    for (int y = 0; y < r1->h; y++) for (int x = 0; x < r1->w; x++) erba += Map_Tile(r1, x, y) == T_ERBA_ALTA;
    VERIFICA(erba > 100);
    int haElian = 0, haKael = 0;
    const Map *lab = &mappe[IndiceMappa("laboratorio")];
    for (int k = 0; k < lab->numNpc; k++) haElian |= strcmp(lab->npc[k].id, "elian") == 0;
    for (int k = 0; k < lun->numNpc; k++) haKael |= strcmp(lun->npc[k].id, "kael") == 0;
    VERIFICA(haElian && haKael);

    /* Fase 4: incontri, allenatori, curatore, PC e capsule degli starter */
    VERIFICA(strcmp(r1->habitat, "ROUTE1") == 0 && GetHabitat(r1->habitat) != NULL);
    VERIFICA(r1->numAllenatori == 2 && lun->numAllenatori == 1);
    for (int i = 0; i < NUM_MAPPE_GIOCO; i++) {
        const Map *m = &mappe[i];
        if (m->habitat[0]) VERIFICA(GetHabitat(m->habitat) != NULL);
        for (int k = 0; k < m->numAllenatori; k++) {
            const char *id = m->allenatore[k].allenatore;
            const char *seg = strstr(id, "{STARTER}");
            if (!seg) { VERIFICA(GetTrainer(id) != NULL); continue; }
            static const char *st[3] = { "PYRIN", "AQUIL", "MOSSEN" };
            for (int j = 0; j < 3; j++) {
                char vero[64];
                snprintf(vero, sizeof(vero), "%.*s%s%s", (int)(seg - id), id, st[j], seg + 9);
                VERIFICA(GetTrainer(vero) != NULL);
            }
        }
        /* gli NPC allenatori devono essere raggiungibili dal giocatore (tile adiacente libera) */
        for (int k = 0; k < m->numAllenatori; k++) VERIFICA(Map_AllenatoreDi(m, m->allenatore[k].npc) != NULL);
    }
    VERIFICA(lab->numStarter == 3 && lab->numPc == 2);
    VERIFICA(lab->starter[0].specie == 1 && lab->starter[1].specie == 4 && lab->starter[2].specie == 7);
    VERIFICA(Map_StarterIn(lab, 6, 5) != NULL && Map_StarterIn(lab, 6, 6) == NULL);
    VERIFICA(Map_PcIn(lab, 1, 7) && !Map_PcIn(lab, 2, 7));
    VERIFICA(!Map_Bloccata(lab, 6, 6));                 /* si sta davanti alle capsule */
    VERIFICA(!Map_Bloccata(lab, 2, 7));                 /* e accanto al PC */
    VERIFICA(strcmp(mappe[IndiceMappa("casa_giocatore")].curatore, "mamma") == 0);
    /* direttive non valide */
    {
        static Map tm;
        char e2[200];
        VERIFICA(!Map_CaricaDaTesto(&tm, "x", "DIMENSIONI 2 2\nTILES\n..\n..\nALLENATORE fantasma X F\n", e2, sizeof(e2)));
        VERIFICA(!Map_CaricaDaTesto(&tm, "x", "DIMENSIONI 2 2\nTILES\n..\n..\nSTARTER 0 0 NONESISTE\n", e2, sizeof(e2)));
        VERIFICA(!Map_CaricaDaTesto(&tm, "x", "DIMENSIONI 2 2\nTILES\n..\n..\nPC 5 5\n", e2, sizeof(e2)));
        VERIFICA(Map_CaricaDaTesto(&tm, "x", "DIMENSIONI 2 2\nTILES\n..\n..\nHABITAT ROUTE1\nPC 1 1\n", e2, sizeof(e2)));
    }

    return FINE_TEST();
}
