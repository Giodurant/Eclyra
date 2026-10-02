/* vista_mondo.c - Esplorazione con la grafica del Design System v2 (mockup 02_overworld).
 *
 * Disegna le mappe VERE del gioco (assets/maps/NOME.map) con i tile, gli edifici, gli alberi e i
 * personaggi della slice: terreno con autotile a 4 vicini, oggetti ordinati per profondita',
 * erba alta sopra le gambe, banner del luogo, dissolvenze. La camera usa il canvas variabile. */
#include "slice.h"
#include "gfx.h"
#include "vista_dialogo.h"
#include "gen/atlante_oggetti.h"
#include "gen/tiles.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define T 16

static unsigned Hash(int x, int y) {
    unsigned h = (unsigned)(x * 73856093) ^ (unsigned)(y * 19349663);
    h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;
    return h;
}

/* ------------------------------------------------------------------ alberi (calcolati una volta per mappa) */

#define MAX_ALBERI 2400
typedef struct { short x, y; unsigned char tipo; } Albero;   /* tipo 0-3 grande (2x2), 4 alberello, 5 cespuglio */
static Albero alberi[MAX_ALBERI];
static int numAlberi;
static char mappaAlberi[32];
static int mx0, my0, mx1, my1;   /* area calcolata (anche fuori mappa, dove il bordo e' bosco) */

static bool EAlbero(const Map *m, int x, int y) { return Map_Tile(m, x, y) == T_ALBERO; }

static void CalcolaAlberi(const Map *m) {
    if (strcmp(mappaAlberi, m->id) == 0) return;
    strncpy(mappaAlberi, m->id, sizeof(mappaAlberi) - 1);
    numAlberi = 0;
    mx0 = -14; my0 = -10; mx1 = m->w + 14; my1 = m->h + 10;
    static unsigned char coperto[MAP_MAX_H + 40][MAP_MAX_W + 40];
    memset(coperto, 0, sizeof(coperto));
    for (int y = my0; y < my1; y++)
        for (int x = mx0; x < mx1; x++) {
            int cx = x - mx0, cy = y - my0;
            if (!EAlbero(m, x, y) || coperto[cy][cx] || numAlberi >= MAX_ALBERI) continue;
            bool grande = EAlbero(m, x + 1, y) && EAlbero(m, x, y + 1) && EAlbero(m, x + 1, y + 1) &&
                          !coperto[cy][cx + 1] && !coperto[cy + 1][cx] && !coperto[cy + 1][cx + 1];
            Albero *a = &alberi[numAlberi++];
            a->x = (short)x; a->y = (short)y;
            if (grande) {
                coperto[cy][cx] = coperto[cy][cx + 1] = coperto[cy + 1][cx] = coperto[cy + 1][cx + 1] = 1;
                unsigned h = Hash(x, y);
                a->tipo = (unsigned char)(h % 23 == 0 ? 3 : h % 3);   /* raramente un albero di Lumen */
            } else {
                coperto[cy][cx] = 1;
                a->tipo = (Hash(x, y) % 3 == 0) ? 5 : 4;
            }
        }
}

/* ------------------------------------------------------------------ terreno */

/* Piazza: le zone di sentiero larghe almeno 5x5 (apertura morfologica), calcolate una volta per mappa.
 * Le strade larghe 4 restano di terra battuta, la piazza centrale di Lunaria diventa lastricata. */
static unsigned char piazza[MAP_MAX_H][MAP_MAX_W];
static char mappaPiazza[32];
static int piazzaX0, piazzaY0, piazzaX1, piazzaY1;   /* riquadro della piazza (tile), per la rosa del Lumen */

static void CalcolaPiazza(const Map *m) {
    if (strcmp(mappaPiazza, m->id) == 0) return;
    strncpy(mappaPiazza, m->id, sizeof(mappaPiazza) - 1);
    static unsigned char nucleo[MAP_MAX_H][MAP_MAX_W];
    memset(nucleo, 0, sizeof(nucleo));
    memset(piazza, 0, sizeof(piazza));
    if (strcmp(m->id, "lunaria") != 0) return;
    for (int y = 0; y < m->h; y++)
        for (int x = 0; x < m->w; x++) {
            bool tutto = true;
            for (int dy = -2; dy <= 2 && tutto; dy++)
                for (int dx = -2; dx <= 2 && tutto; dx++)
                    if (!Map_Dentro(m, x + dx, y + dy)) tutto = false;
                    else {
                        TileTipo t = (TileTipo)m->tile[y + dy][x + dx];   /* aiuole e cartelli stanno dentro la piazza */
                        if (t != T_SENTIERO && t != T_FIORI && t != T_CARTELLO) tutto = false;
                    }
            nucleo[y][x] = tutto;
        }
    piazzaX0 = piazzaY0 = 1 << 20; piazzaX1 = piazzaY1 = -1;
    for (int y = 0; y < m->h; y++)
        for (int x = 0; x < m->w; x++) {
            TileTipo tt = (TileTipo)m->tile[y][x];
            if (tt != T_SENTIERO && tt != T_FIORI && tt != T_CARTELLO) continue;
            for (int dy = -2; dy <= 2; dy++)
                for (int dx = -2; dx <= 2; dx++)
                    if (Map_Dentro(m, x + dx, y + dy) && nucleo[y + dy][x + dx]) piazza[y][x] = 1;
            if (piazza[y][x]) {
                if (x < piazzaX0) piazzaX0 = x;
                if (x > piazzaX1) piazzaX1 = x;
                if (y < piazzaY0) piazzaY0 = y;
                if (y > piazzaY1) piazzaY1 = y;
            }
        }
}

static void Tile(int indice, int x, int y) {
    Rectangle src = { (float)((indice % 16) * T), (float)((indice / 16) * T), T, T };
    DrawTextureRec(Gfx_Tex(TX_TILES), src, (Vector2){ (float)x, (float)y }, WHITE);
}

static bool StessoTipo(const Map *m, int x, int y, TileTipo t) {
    if (!Map_Dentro(m, x, y)) return t != T_SENTIERO;   /* fuori mappa il sentiero finisce, l'acqua continua */
    return m->tile[y][x] == t;
}

static int Maschera(const Map *m, int x, int y, TileTipo t) {
    return (StessoTipo(m, x, y - 1, t) ? 1 : 0) | (StessoTipo(m, x + 1, y, t) ? 2 : 0) |
           (StessoTipo(m, x, y + 1, t) ? 4 : 0) | (StessoTipo(m, x - 1, y, t) ? 8 : 0);
}

static void Terreno(const Map *m, int camX, int camY, int cw, int ch, int frame) {
    bool citta = strcmp(m->id, "lunaria") == 0;
    CalcolaPiazza(m);
    int x0 = camX / T - 1, y0 = camY / T - 1, x1 = (camX + cw) / T + 1, y1 = (camY + ch) / T + 1;
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            int px = x * T - camX, py = y * T - camY;
            unsigned h = Hash(x, y);
            switch (Map_Tile(m, x, y)) {
            case T_ERBA_ALTA: Tile(TILE_ERBA_ALTA + ((frame / 40 + (int)(h % 2)) % 2), px, py); break;
            case T_FIORI:     Tile(TILE_FIORI + (int)(h % 2) * 2 + (frame / 45) % 2, px, py); break;
            case T_ALBERO:    Tile(TILE_SOTTOBOSCO, px, py); break;
            case T_SENTIERO: {
                /* a Lunaria l'interno delle zone larghe di sentiero e' la piazza lastricata (mockup 02_overworld) */
                int mk = Maschera(m, x, y, T_SENTIERO);
                if (citta && Map_Dentro(m, x, y) && piazza[y][x]) {
                    /* bordo della piazza: maschera rispetto alle sole tile di piazza */
                    int mp = (y > 0 && piazza[y - 1][x] ? 1 : 0) | (x + 1 < m->w && piazza[y][x + 1] ? 2 : 0) |
                             (y + 1 < m->h && piazza[y + 1][x] ? 4 : 0) | (x > 0 && piazza[y][x - 1] ? 8 : 0);
                    Tile(TILE_LASTRICATO + 15, px, py);
                    if (!(mp & 1)) G_LineaH(px, py, T, PAL_PIETRA[4]);
                    if (!(mp & 4)) G_LineaH(px, py + T - 1, T, PAL_PIETRA[1]);
                    if (!(mp & 8)) G_LineaV(px, py, T, PAL_PIETRA[4]);
                    if (!(mp & 2)) G_LineaV(px + T - 1, py, T, PAL_PIETRA[1]);
                }
                else Tile(TILE_SENTIERO + (int)(h % 2) * 16 + mk, px, py);
                break;
            }
            case T_ACQUA:     Tile(TILE_ACQUA + ((frame / 36) % 2) * 16 + Maschera(m, x, y, T_ACQUA), px, py); break;
            default:
                if (h % 17 == 0) Tile(TILE_DECORO + (int)((h >> 5) % 4), px, py);
                else Tile(TILE_ERBA + (int)((h >> 3) % 4), px, py);
                break;
            }
        }
}

/* ------------------------------------------------------------------ oggetti ordinati per profondita' */

typedef enum { OG_ALBERO, OG_EDIFICIO, OG_CARTELLO, OG_NPC, OG_GIOCATORE, OG_OGGETTO } TipoOg;
typedef struct { TipoOg tipo; int baseY; int x, y; int idx; } Disegnabile;

static int Confronta(const void *a, const void *b) {
    const Disegnabile *p = a, *q = b;
    if (p->baseY != q->baseY) return p->baseY - q->baseY;
    return (int)p->tipo - (int)q->tipo;
}

static void Personaggio(int riga, Direzione dir, bool inMov, int px, int py, int passi, int x, int y) {
    int blocco = dir == DIR_SU ? 1 : (dir == DIR_GIU ? 0 : 2);
    int f = 0;
    if (inMov) {
        int fase = ((dir == DIR_SINISTRA || dir == DIR_DESTRA) ? px : py) & 15;
        f = fase < 8 ? 1 + (passi & 1) : 0;
    }
    Rectangle src = { (float)((blocco * 3 + f) * 16), (float)(riga * 24), dir == DIR_DESTRA ? -16.0f : 16.0f, 24 };
    DrawTextureRec(Gfx_Tex(TX_PERSONAGGI), src, (Vector2){ (float)x, (float)(y - 8) }, WHITE);
}

static void ErbaSopra(const Map *m, int tx, int ty, int camX, int camY, int frame) {
    if (Map_Tile(m, tx, ty) != T_ERBA_ALTA) return;
    unsigned h = Hash(tx, ty);
    int indice = TILE_ERBA_ALTA + ((frame / 40 + (int)(h % 2)) % 2);
    Rectangle src = { (float)((indice % 16) * T), (float)((indice / 16) * T + 9), T, 7 };
    DrawTextureRec(Gfx_Tex(TX_TILES), src, (Vector2){ (float)(tx * T - camX), (float)(ty * T - camY + 9) }, WHITE);
}

static Rectangle SpriteEdificio(TileTipo t) {
    switch (t) {
    case T_CASA_BLU: return OGGETTI_CASA_BLU;
    case T_NEGOZIO:  return OGGETTI_NEGOZIO;
    case T_LAB:      return OGGETTI_LAB;
    default:         return OGGETTI_CASA;
    }
}

void VistaMondo_Disegna(const Slice *s) {
    const World *w = &s->world;
    const Map *m = &w->mappa;
    const Canvas *c = &s->canvas;
    CalcolaAlberi(m);

    /* camera: segue il giocatore, si ferma ai bordi; mappa piu' piccola del canvas = centrata */
    int maxX = m->w * T - c->w, maxY = m->h * T - c->h;
    int camX = w->player.px + T / 2 - c->w / 2, camY = w->player.py + T / 2 - c->h / 2;
    camX = maxX < 0 ? maxX / 2 : (camX < 0 ? 0 : camX > maxX ? maxX : camX);
    camY = maxY < 0 ? maxY / 2 : (camY < 0 ? 0 : camY > maxY ? maxY : camY);

    ClearBackground(PAL_ERBA[1]);
    Terreno(m, camX, camY, c->w, c->h, s->frame);
    if (piazzaX1 >= 0 && strcmp(mappaPiazza, m->id) == 0) {
        /* rosa del Lumen intarsiata al centro della piazza (mockup 02_overworld) */
        int cx = (piazzaX0 + piazzaX1 + 1) * T / 2 - camX, cy = (piazzaY0 + piazzaY1 + 1) * T / 2 - camY;
        DrawTextureRec(Gfx_Tex(TX_OGGETTI), OGGETTI_ROSA_LUMEN, (Vector2){ (float)(cx - 45), (float)(cy - 31) }, WHITE);
        /* aiuole e cartelli dentro la piazza: aiuola rialzata con bordo di pietra, cartello sul lastricato */
        for (int y = piazzaY0; y <= piazzaY1; y++)
            for (int x = piazzaX0; x <= piazzaX1; x++) {
                TileTipo t = Map_Tile(m, x, y);
                int px = x * T - camX, py = y * T - camY;
                if (t == T_CARTELLO) { Tile(TILE_LASTRICATO + 15, px, py); continue; }
                if (t != T_FIORI) continue;
                Tile(TILE_FIORI + (int)(Hash(x, y) % 2) * 2 + (s->frame / 45) % 2, px, py);
                if (Map_Tile(m, x, y - 1) != T_FIORI) G_LineaH(px, py, T, PAL_PIETRA[4]);
                if (Map_Tile(m, x, y + 1) != T_FIORI) { G_LineaH(px, py + T - 2, T, PAL_PIETRA[2]); G_LineaH(px, py + T - 1, T, PAL_PIETRA[1]); }
                if (Map_Tile(m, x - 1, y) != T_FIORI) G_LineaV(px, py, T, PAL_PIETRA[4]);
                if (Map_Tile(m, x + 1, y) != T_FIORI) { G_LineaV(px + T - 1, py, T, PAL_PIETRA[1]); }
            }
    }

    static Disegnabile lista[1200];
    int n = 0;
    int visX0 = camX - 48, visX1 = camX + c->w + 48, visY0 = camY - 16, visY1 = camY + c->h + 96;
    for (int i = 0; i < numAlberi && n < 1100; i++) {
        const Albero *a = &alberi[i];
        int px = a->x * T, py = a->y * T;
        if (px < visX0 || px > visX1 || py < visY0 - 32 || py > visY1) continue;
        bool grande = a->tipo <= 3;
        lista[n++] = (Disegnabile){ OG_ALBERO, py + (grande ? 2 * T : T) - 2, px, py, i };
    }
    for (int y = 0; y < m->h; y++)
        for (int x = 0; x < m->w; x++) {
            TileTipo t = (TileTipo)m->tile[y][x];
            if (Map_IsEdificio(t) && Map_Tile(m, x - 1, y) != t && Map_Tile(m, x, y + 1) != t) {
                if (x * T < visX0 - 112 || x * T > visX1 || y * T < visY0 || y * T > visY1 + 48) continue;
                lista[n++] = (Disegnabile){ OG_EDIFICIO, (y + 1) * T - 1, x * T, (y + 1) * T, (int)t };
            } else if (t == T_CARTELLO) {
                lista[n++] = (Disegnabile){ OG_CARTELLO, (y + 1) * T - 3, x * T, y * T, 0 };
            }
        }
    for (int i = 0; i < m->numOggetti; i++) {
        const OggettoMappa *o = &m->oggetto[i];
        if (!Flags_Attivo(&w->partita->flags, o->flag))
            lista[n++] = (Disegnabile){ OG_OGGETTO, o->y * T + 8, o->x * T, o->y * T, o->oggetto };
    }
    for (int i = 0; i < w->numNpc; i++)
        lista[n++] = (Disegnabile){ OG_NPC, w->npc[i].py + T - 1, w->npc[i].px, w->npc[i].py, i };
    lista[n++] = (Disegnabile){ OG_GIOCATORE, w->player.py + T - 1, w->player.px, w->player.py, 0 };
    qsort(lista, (size_t)n, sizeof(Disegnabile), Confronta);

    for (int i = 0; i < n; i++) {
        const Disegnabile *d = &lista[i];
        int sx = d->x - camX, sy = d->y - camY;
        switch (d->tipo) {
        case OG_ALBERO: {
            const Albero *a = &alberi[d->idx];
            if (a->tipo <= 3) {
                Rectangle r = a->tipo == 0 ? OGGETTI_ALBERO_0 : a->tipo == 1 ? OGGETTI_ALBERO_1 : a->tipo == 2 ? OGGETTI_ALBERO_2 : OGGETTI_ALBERO_LUMEN;
                unsigned h = Hash(a->x, a->y);
                int jx = (int)(h % 5) - 2, jy = (int)((h >> 4) % 3) - 1;   /* piccolo scarto: il bosco non sembra una griglia */
                DrawTextureRec(Gfx_Tex(TX_OGGETTI), r, (Vector2){ (float)(sx - 1 + jx), (float)(sy + 2 * T - 41 + jy) }, WHITE);
            } else if (a->tipo == 4) {
                DrawTextureRec(Gfx_Tex(TX_OGGETTI), OGGETTI_ALBERELLO, (Vector2){ (float)(sx - 1), (float)(sy + T - 25) }, WHITE);
            } else {
                DrawTextureRec(Gfx_Tex(TX_OGGETTI), OGGETTI_CESPUGLIO, (Vector2){ (float)sx, (float)(sy + 1) }, WHITE);
            }
            break;
        }
        case OG_EDIFICIO: {
            Rectangle r = SpriteEdificio((TileTipo)d->idx);
            DrawTextureRec(Gfx_Tex(TX_OGGETTI), r, (Vector2){ (float)sx, (float)(sy - r.height) }, WHITE);
            break;
        }
        case OG_CARTELLO:
            DrawTextureRec(Gfx_Tex(TX_OGGETTI), OGGETTI_CARTELLO, (Vector2){ (float)sx, (float)(sy - 1) }, WHITE);
            break;
        case OG_OGGETTO: {
            int dy = (s->frame / 20) % 2;
            G_Bagliore(sx + 8, sy + 8, 9, PAL_LUMEN[3], 60);
            G_IconaOggetto(d->idx, sx + 3, sy + 3 - dy);
            break;
        }
        case OG_NPC: {
            const Npc *np = &w->npc[d->idx];
            int riga = np->def->sprite >= 0 && np->def->sprite < 8 ? np->def->sprite : 4;
            Personaggio(riga, np->dir, np->inMovimento, np->px, np->py, np->passi, sx, sy);
            ErbaSopra(m, np->tx, np->ty, camX, camY, s->frame);
            break;
        }
        case OG_GIOCATORE: {
            const Player *p = &w->player;
            Personaggio(8, p->dir, p->inMovimento, p->px, p->py, p->passi, sx, sy);
            if (!p->inMovimento) ErbaSopra(m, p->tx, p->ty, camX, camY, s->frame);
            break;
        }
        }
    }
    VistaMondo_Vignetta(c);
    VistaMondo_Banner(s);
    VistaMondo_Suggerimento(s);
    VistaDialogo_Disegna(&w->dialogo, c, s->frame);
    if (w->domandaStarter) VistaDialogo_SiNo(w->cursoreSiNo, c, s->frame);
    if (w->transizione != TRANS_NESSUNA) {
        float q = w->transizione == TRANS_USCITA ? 1.0f - (float)w->timerTransizione / 14.0f : (float)w->timerTransizione / 14.0f;
        G_Oscura(0, 0, c->w, c->h, (unsigned char)(255.0f * (q < 0 ? 0 : q > 1 ? 1 : q)));
    }
}

/* Banner del luogo (mockups.banner_luogo): targa in alto a sinistra con cristallo, nome in oro e sottotitolo */
void VistaMondo_Banner(const Slice *s) {
    const World *w = &s->world;
    if (w->timerNomeMappa <= 0) return;
    const char *sott = strcmp(w->mappa.id, "lunaria") == 0 ? "Villaggio del Lumen" : "Regione di Verdan";
    int larg = T_LarghezzaGrassetto(w->mappa.nome);
    if (T_Larghezza(sott) > larg) larg = T_Larghezza(sott);
    larg += 30;
    int t = w->timerNomeMappa;
    int x = 4 + s->canvas.safeL;
    int y = 4 + s->canvas.safeT;
    if (t > 134) y -= (t - 134) * 2;            /* scende dall'alto */
    else if (t < 16) y -= (16 - t) * 2;          /* e risale */
    G_Cornice(x, y, larg, 28, CORNICE_NOTTE, false);
    G_Bagliore(x + 9, y + 9, 6, PAL_LUMEN[3], 90);
    G_Cristallo(x + 9, y + 5, 11, true);
    T_TitoloOro(w->mappa.nome, x + 17, y + 5);
    T_Testo(sott, x + 17, y + 16, PAL_GRIGIO_UI);
}

/* Suggerimento contestuale (mockup 02_overworld: "Z Parla") */
void VistaMondo_Suggerimento(const Slice *s) {
    const World *w = &s->world;
    if (!World_Libero(w) || s->az.ultimo == DISP_TOUCH || w->timerNomeMappa > 0) return;
    int x, y;
    Player_TileDavanti(&w->player, &x, &y);
    const char *azione = NULL;
    for (int i = 0; i < w->numNpc; i++) if (w->npc[i].tx == x && w->npc[i].ty == y) azione = "Parla";
    if (!azione && Map_CartelloIn(&w->mappa, x, y)) azione = "Leggi";
    if (!azione)
        for (int i = 0; i < w->mappa.numOggetti; i++)
            if (w->mappa.oggetto[i].x == x && w->mappa.oggetto[i].y == y && !Flags_Attivo(&w->partita->flags, w->mappa.oggetto[i].flag)) azione = "Raccogli";
    if (!azione) return;
    const char *tasto = Azioni_Prompt(&s->az, AZ_CONFERMA);
    int tw = T_LarghezzaMicro(tasto) <= 3 ? 9 : T_LarghezzaMicro(tasto) + 6;
    int larg = tw + T_Larghezza(azione) + 14;
    int bx = s->canvas.w - s->canvas.safeR - 4 - larg, by = s->canvas.h - s->canvas.safeB - 19;
    G_Cornice(bx, by, larg, 15, CORNICE_NOTTE, false);
    G_GlifoTasto(bx + 4, by + 3, tasto, false);
    T_TestoOmbra(azione, bx + 8 + tw, by + 4, PAL_CREMA);
}

/* Illuminazione (mockups.grading): vignetta morbida ai bordi, calcolata una volta per misura del canvas */
void VistaMondo_Vignetta(const Canvas *c) {
    static Texture2D t;
    static int tw, th;
    if (tw != c->w || th != c->h) {
        if (tw) UnloadTexture(t);
        Image im = GenImageColor(c->w, c->h, BLANK);
        for (int y = 0; y < c->h; y++)
            for (int x = 0; x < c->w; x++) {
                float dx = (x - c->w / 2.0f) / (c->w / 2.0f), dy = (y - c->h / 2.0f) / (c->h / 2.0f);
                float v = sqrtf(dx * dx * 0.81f + dy * dy) - 0.55f;
                if (v <= 0) continue;
                v *= 0.35f * 1.6f;
                if (v > 0.6f) v = 0.6f;
                ImageDrawPixel(&im, x, y, (Color){ 7, 10, 24, (unsigned char)(v * 255) });
            }
        t = LoadTextureFromImage(im);
        UnloadImage(im);
        tw = c->w; th = c->h;
    }
    DrawTexture(t, 0, 0, WHITE);
    /* luce calda dall'alto a sinistra (grading caldo 0,5 dei mockup), additiva */
    static Texture2D luce;
    static int lw, lh;
    if (lw != c->w || lh != c->h) {
        if (lw) UnloadTexture(luce);
        Image im = GenImageColor(c->w, c->h, BLANK);
        for (int y = 0; y < c->h; y++)
            for (int x = 0; x < c->w; x++) {
                float l = 1.0f - ((float)x / c->w * 0.5f + (float)y / c->h * 0.5f);
                if (l <= 0) continue;
                ImageDrawPixel(&im, x, y, (Color){ 255, 214, 140, (unsigned char)(l * 34) });
            }
        luce = LoadTextureFromImage(im);
        UnloadImage(im);
        lw = c->w; lh = c->h;
    }
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexture(luce, 0, 0, WHITE);
    EndBlendMode();
}
