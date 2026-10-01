/* map.c */
#include "map.h"
#include "graphics.h"
#include "utils.h"

/* Indici della prima riga del tileset (assets/tilesets/tileset.png) */
enum { TS_ERBA_0 = 0, TS_ERBA_1, TS_ERBA_2, TS_ERBA_ALTA, TS_ERBA_ALTA_SOPRA,
       TS_FIORI_0, TS_FIORI_1, TS_CARTELLO, TS_SOTTOBOSCO };

void Map_Pulisci(Map *m, const char *nome, int w, int h, TileTipo riempimento) {
    Utils_StrCopy(m->nome, sizeof(m->nome), nome);
    m->w = Utils_ClampInt(w, 1, MAP_MAX_W);
    m->h = Utils_ClampInt(h, 1, MAP_MAX_H);
    for (int y = 0; y < MAP_MAX_H; y++)
        for (int x = 0; x < MAP_MAX_W; x++) m->tile[y][x] = (uint8_t)riempimento;
}

TileTipo Map_Tile(const Map *m, int x, int y) {
    if (x < 0 || y < 0 || x >= m->w || y >= m->h) return T_ALBERO;
    return (TileTipo)m->tile[y][x];
}

void Map_Imposta(Map *m, int x, int y, TileTipo t) {
    if (x < 0 || y < 0 || x >= m->w || y >= m->h) return;
    m->tile[y][x] = (uint8_t)t;
}

void Map_Rettangolo(Map *m, int x, int y, int w, int h, TileTipo t) {
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++) Map_Imposta(m, i, j, t);
}

bool Map_Bloccata(const Map *m, int x, int y) {
    TileTipo t = Map_Tile(m, x, y);
    return t == T_ALBERO || t == T_ACQUA || t == T_CARTELLO || t == T_EDIFICIO;
}

void Map_PiazzaEdificio(Map *m, int x, int y) {
    Map_Rettangolo(m, x, y, EDIFICIO_W, EDIFICIO_H, T_EDIFICIO);
}

/* ------------------------------------------------------------------
 * Lunaria provvisoria (Fase 1). Coordinate dal documento di design:
 * casa giocatore (8,18), piazza (19,15), laboratorio (28,8),
 * negozio (6,7), uscita verso la Route 1 (19,0).
 * Nella Fase 2 viene sostituita da una mappa con porte e interni.
 * ------------------------------------------------------------------ */
void Map_GeneraLunariaProvvisoria(Map *m) {
    Map_Pulisci(m, "LUNARIA", 40, 30, T_ERBA);

    /* bosco di confine (2 tile), con il varco a nord verso la Route 1 */
    Map_Rettangolo(m, 0, 0, 40, 2, T_ALBERO);
    Map_Rettangolo(m, 0, 28, 40, 2, T_ALBERO);
    Map_Rettangolo(m, 0, 0, 2, 30, T_ALBERO);
    Map_Rettangolo(m, 38, 0, 2, 30, T_ALBERO);
    Map_Rettangolo(m, 18, 0, 4, 2, T_SENTIERO);

    /* strada principale nord-sud e piazza */
    Map_Rettangolo(m, 18, 2, 4, 26, T_SENTIERO);
    Map_Rettangolo(m, 15, 13, 10, 6, T_SENTIERO);
    Map_Imposta(m, 19, 15, T_FIORI);   /* aiuola al centro della piazza */
    Map_Imposta(m, 20, 15, T_FIORI);
    Map_Imposta(m, 19, 16, T_FIORI);
    Map_Imposta(m, 20, 16, T_FIORI);

    /* edifici con vialetto davanti alla porta */
    Map_PiazzaEdificio(m, 8, 18);      /* casa del giocatore */
    Map_Rettangolo(m, 10, 22, 1, 2, T_SENTIERO);
    Map_Rettangolo(m, 10, 23, 8, 1, T_SENTIERO);

    Map_PiazzaEdificio(m, 28, 8);      /* laboratorio del professor Elian */
    Map_Rettangolo(m, 30, 12, 1, 2, T_SENTIERO);
    Map_Rettangolo(m, 22, 13, 9, 1, T_SENTIERO);

    Map_PiazzaEdificio(m, 6, 7);       /* negozio */
    Map_Rettangolo(m, 8, 11, 1, 1, T_SENTIERO);
    Map_Rettangolo(m, 8, 12, 10, 1, T_SENTIERO);

    Map_Imposta(m, 17, 4, T_CARTELLO); /* cartello all'uscita nord */

    /* laghetto e alberi decorativi */
    Map_Rettangolo(m, 28, 20, 6, 4, T_ACQUA);
    Map_Rettangolo(m, 3, 3, 2, 2, T_ALBERO);
    Map_Rettangolo(m, 34, 3, 2, 2, T_ALBERO);
    Map_Rettangolo(m, 3, 25, 3, 2, T_FIORI);
    Map_Rettangolo(m, 25, 4, 2, 1, T_FIORI);
}

/* ------------------------------------------------------------------ */

static bool Uguale(const Map *m, int x, int y, TileTipo t) {
    if (x < 0 || y < 0 || x >= m->w || y >= m->h) return true;
    return m->tile[y][x] == t;
}

/* bit0 sopra, bit1 destra, bit2 sotto, bit3 sinistra: per i bordi automatici */
static int Maschera(const Map *m, int x, int y, TileTipo t) {
    return (Uguale(m, x, y - 1, t) ? 1 : 0) | (Uguale(m, x + 1, y, t) ? 2 : 0) |
           (Uguale(m, x, y + 1, t) ? 4 : 0) | (Uguale(m, x - 1, y, t) ? 8 : 0);
}

static void Casella(int colonna, int riga, int x, int y) {
    Rectangle src = { (float)(colonna * TILE), (float)(riga * TILE), TILE, TILE };
    DrawTextureRec(Gfx_Texture(TEX_TILESET), src, (Vector2){ (float)x, (float)y }, WHITE);
}

static int VarianteErba(int x, int y) {
    unsigned h = (unsigned)(x * 73856093) ^ (unsigned)(y * 19349663);
    return TS_ERBA_0 + (int)((h >> 3) % 3);
}

void Map_DisegnaTerreno(const Map *m, int camX, int camY, int frame) {
    int x0 = camX / TILE - 1, y0 = camY / TILE - 1;
    int x1 = x0 + LOGICAL_W / TILE + 3, y1 = y0 + LOGICAL_H / TILE + 3;
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            int px = x * TILE - camX, py = y * TILE - camY;
            switch (Map_Tile(m, x, y)) {
                case T_ERBA_ALTA: Casella(TS_ERBA_ALTA, 0, px, py); break;
                case T_FIORI:     Casella((frame / 30) % 2 ? TS_FIORI_1 : TS_FIORI_0, 0, px, py); break;
                case T_CARTELLO:  Casella(TS_CARTELLO, 0, px, py); break;
                case T_ALBERO:    Casella(TS_SOTTOBOSCO, 0, px, py); break;
                case T_SENTIERO:  Casella(Maschera(m, x, y, T_SENTIERO), 1, px, py); break;
                case T_ACQUA:     Casella(Maschera(m, x, y, T_ACQUA), 2 + (frame / 32) % 2, px, py); break;
                default:          Casella(VarianteErba(x, y), 0, px, py); break;
            }
        }
    }
}

void Map_RigheVisibili(int camY, int *prima, int *ultima) {
    *prima = camY / TILE - 1;
    *ultima = camY / TILE + LOGICAL_H / TILE + EDIFICIO_H + 1;
}

void Map_DisegnaOggettiRiga(const Map *m, int y, int camX, int camY) {
    int x0 = camX / TILE - EDIFICIO_W, x1 = camX / TILE + LOGICAL_W / TILE + 2;
    for (int x = x0; x <= x1; x++) {
        TileTipo t = Map_Tile(m, x, y);
        int px = x * TILE - camX, py = y * TILE - camY;
        bool dentro = x >= 0 && y >= 0 && x < m->w && y < m->h;
        if (t == T_ALBERO && dentro) {
            /* l'albero e' alto 2 tile: la chioma sporge sulla riga sopra */
            DrawTexture(Gfx_Texture(TEX_ALBERO), px, py - 12, WHITE);
        }
        /* edificio: disegnato una volta, dal suo angolo in basso a sinistra */
        if (t == T_EDIFICIO && Map_Tile(m, x - 1, y) != T_EDIFICIO && Map_Tile(m, x, y + 1) != T_EDIFICIO) {
            DrawTexture(Gfx_Texture(TEX_CASA), px, py - (EDIFICIO_H - 1) * TILE, WHITE);
        }
    }
}
