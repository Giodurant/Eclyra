/* map.c - Dati e caricamento delle mappe (nessuna dipendenza da raylib). */
#include "map.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *const MAPPE_GIOCO[] = {
    "lunaria", "route1", "casa_giocatore", "laboratorio", "negozio_lunaria", "casa_vicini",
};
const int NUM_MAPPE_GIOCO = (int)(sizeof(MAPPE_GIOCO) / sizeof(MAPPE_GIOCO[0]));

/* Carattere nel file -> tipo di tile */
typedef struct { char c; TileTipo t; } CharTile;
static const CharTile LEGENDA[] = {
    {'.', T_ERBA}, {'w', T_ERBA_ALTA}, {'=', T_SENTIERO}, {'T', T_ALBERO}, {'~', T_ACQUA},
    {'*', T_FIORI}, {'C', T_CARTELLO}, {'H', T_CASA}, {'K', T_CASA_BLU}, {'N', T_NEGOZIO},
    {'L', T_LAB},
    {'_', T_PAVIMENTO}, {'#', T_MURO}, {'x', T_TAPPETINO}, {'r', T_TAPPETO}, {'h', T_SEDIA},
    {'1', T_LETTO_A}, {'2', T_LETTO_B}, {'3', T_LIBRERIA_A}, {'4', T_LIBRERIA_B},
    {'5', T_MACCHINA_A}, {'6', T_MACCHINA_B}, {'p', T_PIANTA}, {'v', T_TV}, {'c', T_BANCONE},
    {'7', T_SCAFFALE_A}, {'8', T_SCAFFALE_B}, {'o', T_TAVOLO}, {'g', T_PC},
};

static bool CarattereATile(char c, TileTipo *t) {
    for (size_t i = 0; i < sizeof(LEGENDA) / sizeof(LEGENDA[0]); i++)
        if (LEGENDA[i].c == c) { *t = LEGENDA[i].t; return true; }
    return false;
}

void Map_DimensioniEdificio(TileTipo t, int *w, int *h) {
    switch (t) {
        case T_LAB: *w = 7; *h = 5; break;
        case T_CASA: case T_CASA_BLU: case T_NEGOZIO: *w = 5; *h = 4; break;
        default: *w = 1; *h = 1; break;
    }
}

bool Map_IsEdificio(TileTipo t) {
    return t == T_CASA || t == T_CASA_BLU || t == T_NEGOZIO || t == T_LAB;
}

bool Map_TileBloccata(TileTipo t) {
    switch (t) {
        case T_ERBA: case T_ERBA_ALTA: case T_SENTIERO: case T_FIORI:
        case T_PAVIMENTO: case T_TAPPETINO: case T_TAPPETO: case T_SEDIA:
            return false;
        default:
            return true;
    }
}

bool Map_Dentro(const Map *m, int x, int y) {
    return x >= 0 && y >= 0 && x < m->w && y < m->h;
}

TileTipo Map_Tile(const Map *m, int x, int y) {
    if (!Map_Dentro(m, x, y)) return m->tipo == MAPPA_INTERNO ? T_MURO : T_ALBERO;
    return (TileTipo)m->tile[y][x];
}

bool Map_Bloccata(const Map *m, int x, int y) {
    return Map_TileBloccata(Map_Tile(m, x, y));
}

const Warp *Map_WarpIn(const Map *m, int x, int y) {
    for (int i = 0; i < m->numWarp; i++) {
        const Warp *w = &m->warp[i];
        if (x >= w->x && x < w->x + w->w && y >= w->y && y < w->y + w->h) return w;
    }
    return NULL;
}

void Map_DestinazioneWarp(const Warp *w, int x, int y, int *dx, int *dy) {
    *dx = w->dx + (x - w->x);
    *dy = w->dy + (y - w->y);
}

const Cartello *Map_CartelloIn(const Map *m, int x, int y) {
    for (int i = 0; i < m->numCartelli; i++)
        if (m->cartello[i].x == x && m->cartello[i].y == y) return &m->cartello[i];
    return NULL;
}

/* ------------------------------------------------------------------
 * Parser
 * ------------------------------------------------------------------ */
static bool LeggiDir(const char *s, Direzione *d) {
    if (strcmp(s, "GIU") == 0) { *d = DIR_GIU; return true; }
    if (strcmp(s, "SU") == 0) { *d = DIR_SU; return true; }
    if (strcmp(s, "SINISTRA") == 0) { *d = DIR_SINISTRA; return true; }
    if (strcmp(s, "DESTRA") == 0) { *d = DIR_DESTRA; return true; }
    return false;
}

static bool LeggiMov(const char *s, MovimentoNpc *mv) {
    if (strcmp(s, "FERMO") == 0) { *mv = MOV_FERMO; return true; }
    if (strcmp(s, "GIRA") == 0) { *mv = MOV_GIRA; return true; }
    if (strcmp(s, "VAGA") == 0) { *mv = MOV_VAGA; return true; }
    return false;
}

#define ERRORE(...) do { snprintf(errore, (size_t)dimErrore, __VA_ARGS__); return false; } while (0)

/* Legge la riga successiva (senza '\n' e '\r'); ritorna false a fine testo */
static bool RigaSuccessiva(const char **p, char *riga, size_t dim) {
    if (**p == '\0') return false;
    size_t n = 0;
    while (**p != '\0' && **p != '\n') {
        if (**p != '\r' && n + 1 < dim) riga[n++] = **p;
        (*p)++;
    }
    if (**p == '\n') (*p)++;
    riga[n] = '\0';
    return true;
}

bool Map_CaricaDaTesto(Map *m, const char *id, const char *testo, char *errore, int dimErrore) {
    char dummy[8];
    if (!errore || dimErrore <= 0) { errore = dummy; dimErrore = (int)sizeof(dummy); }
    errore[0] = '\0';
    memset(m, 0, sizeof(*m));
    Utils_StrCopy(m->id, sizeof(m->id), id);
    Utils_StrCopy(m->nome, sizeof(m->nome), id);
    m->spawnDir = DIR_GIU;
    if (!testo) ERRORE("%s: testo vuoto", id);

    const char *p = testo;
    char riga[512];
    int numRiga = 0;
    bool haTiles = false;

    while (RigaSuccessiva(&p, riga, sizeof(riga))) {
        numRiga++;
        char cmd[32] = "";
        if (riga[0] == '#' || sscanf(riga, "%31s", cmd) != 1) continue;

        if (strcmp(cmd, "NOME") == 0) {
            const char *v = riga + 4;
            while (*v == ' ') v++;
            Utils_StrCopy(m->nome, sizeof(m->nome), v);
        } else if (strcmp(cmd, "TIPO") == 0) {
            char v[32];
            if (sscanf(riga, "%*s %31s", v) != 1) ERRORE("%s:%d: TIPO mancante", id, numRiga);
            m->tipo = strcmp(v, "INTERNO") == 0 ? MAPPA_INTERNO : MAPPA_ESTERNO;
        } else if (strcmp(cmd, "DIMENSIONI") == 0) {
            if (sscanf(riga, "%*s %d %d", &m->w, &m->h) != 2 || m->w < 1 || m->h < 1 ||
                m->w > MAP_MAX_W || m->h > MAP_MAX_H)
                ERRORE("%s:%d: DIMENSIONI non valide (max %dx%d)", id, numRiga, MAP_MAX_W, MAP_MAX_H);
        } else if (strcmp(cmd, "TILES") == 0) {
            if (m->w == 0) ERRORE("%s:%d: TILES prima di DIMENSIONI", id, numRiga);
            for (int y = 0; y < m->h; y++) {
                if (!RigaSuccessiva(&p, riga, sizeof(riga))) ERRORE("%s: mancano righe di tile (%d/%d)", id, y, m->h);
                numRiga++;
                if ((int)strlen(riga) < m->w) ERRORE("%s:%d: riga di tile corta (%d invece di %d)", id, numRiga, (int)strlen(riga), m->w);
                for (int x = 0; x < m->w; x++) {
                    TileTipo t;
                    if (!CarattereATile(riga[x], &t)) ERRORE("%s:%d: carattere sconosciuto '%c' in colonna %d", id, numRiga, riga[x], x);
                    m->tile[y][x] = (uint8_t)t;
                }
            }
            haTiles = true;
        } else if (strcmp(cmd, "PORTA") == 0 || strcmp(cmd, "USCITA") == 0) {
            if (m->numWarp >= MAP_MAX_WARP) ERRORE("%s:%d: troppi warp", id, numRiga);
            Warp *w = &m->warp[m->numWarp];
            char dir[16];
            int letti;
            if (cmd[0] == 'P') {
                w->w = w->h = 1;
                w->suUrto = true;
                letti = sscanf(riga, "%*s %d %d %23s %d %d %15s", &w->x, &w->y, w->dest, &w->dx, &w->dy, dir);
                if (letti != 6) ERRORE("%s:%d: PORTA x y DEST dx dy DIR", id, numRiga);
            } else {
                w->suUrto = false;
                letti = sscanf(riga, "%*s %d %d %d %d %23s %d %d %15s", &w->x, &w->y, &w->w, &w->h, w->dest, &w->dx, &w->dy, dir);
                if (letti != 8 || w->w < 1 || w->h < 1) ERRORE("%s:%d: USCITA x y w h DEST dx dy DIR", id, numRiga);
            }
            if (!LeggiDir(dir, &w->dir)) ERRORE("%s:%d: direzione '%s' non valida", id, numRiga, dir);
            m->numWarp++;
        } else if (strcmp(cmd, "NPC") == 0) {
            if (m->numNpc >= MAP_MAX_NPC) ERRORE("%s:%d: troppi NPC", id, numRiga);
            NpcDef *n = &m->npc[m->numNpc];
            char dir[16], mov[16];
            int consumati = 0;
            if (sscanf(riga, "%*s %23s %d %d %d %15s %15s %31s %n", n->id, &n->sprite, &n->x, &n->y, dir, mov,
                       n->dialogo, &consumati) < 7 || consumati == 0)
                ERRORE("%s:%d: NPC id sprite x y DIR MOV dialogo Nome", id, numRiga);
            Utils_StrCopy(n->nome, sizeof(n->nome), riga + consumati);
            if (!LeggiDir(dir, &n->dir)) ERRORE("%s:%d: direzione NPC non valida", id, numRiga);
            if (!LeggiMov(mov, &n->mov)) ERRORE("%s:%d: movimento NPC '%s' non valido", id, numRiga, mov);
            m->numNpc++;
        } else if (strcmp(cmd, "CARTELLO") == 0) {
            if (m->numCartelli >= MAP_MAX_CARTELLI) ERRORE("%s:%d: troppi cartelli", id, numRiga);
            Cartello *c = &m->cartello[m->numCartelli];
            if (sscanf(riga, "%*s %d %d %31s", &c->x, &c->y, c->dialogo) != 3) ERRORE("%s:%d: CARTELLO x y dialogo", id, numRiga);
            m->numCartelli++;
        } else if (strcmp(cmd, "OGGETTO") == 0) {
            if (m->numOggetti >= MAP_MAX_OGGETTI) ERRORE("%s:%d: troppi oggetti", id, numRiga);
            OggettoMappa *o = &m->oggetto[m->numOggetti];
            if (sscanf(riga, "%*s %d %d %d %d %31s", &o->x, &o->y, &o->oggetto, &o->quantita, o->flag) != 5)
                ERRORE("%s:%d: OGGETTO x y id quantita flag", id, numRiga);
            m->numOggetti++;
        } else if (strcmp(cmd, "SPAWN") == 0) {
            char dir[16];
            if (sscanf(riga, "%*s %d %d %15s", &m->spawnX, &m->spawnY, dir) != 3 || !LeggiDir(dir, &m->spawnDir))
                ERRORE("%s:%d: SPAWN x y DIR", id, numRiga);
        } else {
            ERRORE("%s:%d: direttiva sconosciuta '%s'", id, numRiga, cmd);
        }
    }
    if (!haTiles) ERRORE("%s: sezione TILES mancante", id);

    /* coordinate dentro la mappa */
    for (int i = 0; i < m->numNpc; i++)
        if (!Map_Dentro(m, m->npc[i].x, m->npc[i].y)) ERRORE("%s: NPC %s fuori mappa", id, m->npc[i].id);
    for (int i = 0; i < m->numWarp; i++)
        if (!Map_Dentro(m, m->warp[i].x, m->warp[i].y) ||
            !Map_Dentro(m, m->warp[i].x + m->warp[i].w - 1, m->warp[i].y + m->warp[i].h - 1))
            ERRORE("%s: warp %d fuori mappa", id, i);
    if (!Map_Dentro(m, m->spawnX, m->spawnY)) ERRORE("%s: SPAWN fuori mappa", id);
    return true;
}

bool Map_CaricaFile(Map *m, const char *cartella, const char *id, char *errore, int dimErrore) {
    char percorso[512];
    snprintf(percorso, sizeof(percorso), "%s/%s.map", cartella, id);
    char *testo = Utils_LeggiFile(percorso);
    if (!testo) {
        if (errore && dimErrore > 0) snprintf(errore, (size_t)dimErrore, "impossibile aprire %s", percorso);
        return false;
    }
    bool ok = Map_CaricaDaTesto(m, id, testo, errore, dimErrore);
    free(testo);
    return ok;
}
