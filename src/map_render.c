/* map_render.c */
#include "map_render.h"
#include "graphics.h"

/* Prima riga del tileset esterno (assets/tilesets/tileset.png) */
enum { TS_ERBA_0 = 0, TS_ERBA_ALTA = 3, TS_FIORI_0 = 5, TS_FIORI_1 = 6, TS_CARTELLO = 7, TS_SOTTOBOSCO = 8 };

/* Tileset degli interni (assets/tilesets/interni.png): indice = colonna + 16 * riga */
enum {
    TI_PAVIMENTO_A = 0, TI_PAVIMENTO_B, TI_MURO, TI_MURO_ALTO, TI_TAPPETINO, TI_TAPPETO, TI_SEDIA,
    TI_LETTO_A, TI_LETTO_B, TI_LIBRERIA_A, TI_LIBRERIA_B, TI_MACCHINA_A, TI_MACCHINA_B,
    TI_PIANTA, TI_TV, TI_BANCONE, TI_SCAFFALE_A, TI_SCAFFALE_B, TI_TAVOLO, TI_PC, TI_FINESTRA
};

static void Esterno(int colonna, int riga, int x, int y) {
    Rectangle src = { (float)(colonna * TILE), (float)(riga * TILE), TILE, TILE };
    DrawTextureRec(Gfx_Texture(TEX_TILESET), src, (Vector2){ (float)x, (float)y }, WHITE);
}

static void Interno(int indice, int x, int y) {
    Rectangle src = { (float)((indice % 16) * TILE), (float)((indice / 16) * TILE), TILE, TILE };
    DrawTextureRec(Gfx_Texture(TEX_INTERNI), src, (Vector2){ (float)x, (float)y }, WHITE);
}

static bool Uguale(const Map *m, int x, int y, TileTipo t) {
    if (!Map_Dentro(m, x, y)) return true;
    return m->tile[y][x] == t;
}

static int Maschera(const Map *m, int x, int y, TileTipo t) {
    return (Uguale(m, x, y - 1, t) ? 1 : 0) | (Uguale(m, x + 1, y, t) ? 2 : 0) |
           (Uguale(m, x, y + 1, t) ? 4 : 0) | (Uguale(m, x - 1, y, t) ? 8 : 0);
}

static unsigned Hash(int x, int y) { return (unsigned)(x * 73856093) ^ (unsigned)(y * 19349663); }

static void TileInterno(const Map *m, TileTipo t, int x, int y, int px, int py) {
    switch (t) {
        case T_MURO:
            /* parete vista di fronte se sotto c'e' il pavimento, altrimenti bordo scuro */
            if (Map_Tile(m, x, y + 1) != T_MURO)
                Interno(x % 4 == 2 ? TI_FINESTRA : TI_MURO, px, py);
            else
                Interno(TI_MURO_ALTO, px, py);
            break;
        case T_TAPPETINO:  Interno(TI_TAPPETINO, px, py); break;
        case T_TAPPETO:    Interno(TI_TAPPETO, px, py); break;
        case T_SEDIA:      Interno(TI_SEDIA, px, py); break;
        case T_LETTO_A:    Interno(TI_LETTO_A, px, py); break;
        case T_LETTO_B:    Interno(TI_LETTO_B, px, py); break;
        case T_LIBRERIA_A: Interno(TI_LIBRERIA_A, px, py); break;
        case T_LIBRERIA_B: Interno(TI_LIBRERIA_B, px, py); break;
        case T_MACCHINA_A: Interno(TI_MACCHINA_A, px, py); break;
        case T_MACCHINA_B: Interno(TI_MACCHINA_B, px, py); break;
        case T_PIANTA:     Interno(TI_PIANTA, px, py); break;
        case T_TV:         Interno(TI_TV, px, py); break;
        case T_BANCONE:    Interno(TI_BANCONE, px, py); break;
        case T_SCAFFALE_A: Interno(TI_SCAFFALE_A, px, py); break;
        case T_SCAFFALE_B: Interno(TI_SCAFFALE_B, px, py); break;
        case T_TAVOLO:     Interno(TI_TAVOLO, px, py); break;
        case T_PC:         Interno(TI_PC, px, py); break;
        default:           Interno((x + y) % 2 ? TI_PAVIMENTO_B : TI_PAVIMENTO_A, px, py); break;
    }
}

void MapRender_Terreno(const Map *m, int camX, int camY, int frame) {
    int x0 = camX / TILE - 1, y0 = camY / TILE - 1;
    int x1 = x0 + LOGICAL_W / TILE + 3, y1 = y0 + LOGICAL_H / TILE + 3;
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            int px = x * TILE - camX, py = y * TILE - camY;
            if (m->tipo == MAPPA_INTERNO) {
                if (Map_Dentro(m, x, y)) TileInterno(m, Map_Tile(m, x, y), x, y, px, py);
                continue;   /* fuori dagli interni resta nero */
            }
            switch (Map_Tile(m, x, y)) {
                case T_ERBA_ALTA: Esterno(TS_ERBA_ALTA, 0, px, py); break;
                case T_FIORI:     Esterno((frame / 30) % 2 ? TS_FIORI_1 : TS_FIORI_0, 0, px, py); break;
                case T_CARTELLO:  Esterno(TS_CARTELLO, 0, px, py); break;
                case T_ALBERO:    Esterno(TS_SOTTOBOSCO, 0, px, py); break;
                case T_SENTIERO:  Esterno(Maschera(m, x, y, T_SENTIERO), 1, px, py); break;
                case T_ACQUA:     Esterno(Maschera(m, x, y, T_ACQUA), 2 + (frame / 32) % 2, px, py); break;
                default:          Esterno(TS_ERBA_0 + (int)((Hash(x, y) >> 3) % 3), 0, px, py); break;
            }
        }
    }
}

void MapRender_RigheVisibili(int camY, int *prima, int *ultima) {
    *prima = camY / TILE - 1;
    *ultima = camY / TILE + LOGICAL_H / TILE + 6;   /* il laboratorio sporge di 4 righe verso l'alto */
}

static TexId TextureEdificio(TileTipo t) {
    switch (t) {
        case T_CASA_BLU: return TEX_CASA_BLU;
        case T_NEGOZIO:  return TEX_NEGOZIO;
        case T_LAB:      return TEX_LAB;
        default:         return TEX_CASA;
    }
}

void MapRender_OggettiRiga(const Map *m, int y, int camX, int camY) {
    if (m->tipo == MAPPA_INTERNO) return;
    int x0 = camX / TILE - 8, x1 = camX / TILE + LOGICAL_W / TILE + 2;
    for (int x = x0; x <= x1; x++) {
        TileTipo t = Map_Tile(m, x, y);
        int px = x * TILE - camX, py = y * TILE - camY;
        if (t == T_ALBERO && x >= -1 && y >= -1 && x <= m->w && y <= m->h) {
            DrawTexture(Gfx_Texture(TEX_ALBERO), px, py - 12, WHITE);
        } else if (Map_IsEdificio(t) && Map_Tile(m, x - 1, y) != t && Map_Tile(m, x, y + 1) != t) {
            /* angolo in basso a sinistra dell'edificio */
            int w, h;
            Map_DimensioniEdificio(t, &w, &h);
            DrawTexture(Gfx_Texture(TextureEdificio(t)), px, py - (h - 1) * TILE, WHITE);
        }
    }
}
