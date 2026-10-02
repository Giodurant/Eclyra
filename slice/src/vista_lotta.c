/* vista_lotta.c - La battaglia di ECLYRA (Design System §11, mockup 04_lotta, 04b_lotta_mosse,
 * 04c_lotta_telefono, 11_cattura).
 *
 * Il flusso (eventi del motore, menu, mosse, squadra, zaino, apprendimento, evoluzione) e' quello di
 * battle_ui.c del gioco, invariato. Qui cambiano solo il disegno e l'input:
 *  - Elyon di profilo uno di fronte all'altro, giocatore a sinistra, avversario a destra;
 *  - rosa dei comandi: basso LOTTA (A), destra FUGA (B), sinistra ZAINO (X), alto ELYON (Y);
 *  - quattro mosse come carte, selezionabili con frecce, controller o tocco diretto. */
#include "slice.h"
#include "gfx.h"
#include "ui_common.h"
#include "vista_dialogo.h"
#include "data/items_data.h"
#include "gen/atlante_lotta.h"
#include "gen/atlante_ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* stessi valori di battle_ui.c (animazioni dei lati) */
enum { A_NESSUNA = 0, A_ENTRATA, A_ATTACCO, A_COLPITO, A_KO, A_RITIRO, A_CATTURATO, A_RIAPPARE };
static const int DURATA[] = { 0, 22, 14, 18, 22, 16, 16, 12 };

/* posizioni del menu nel cursore di battle_ui: 0 LOTTA, 1 ELYON, 2 ZAINO, 3 FUGA */
typedef struct { const char *nome; const char *tasto; Rectangle icona; int dx, dy; } VoceRosa;

static VoceRosa Voce(int i) {
    switch (i) {
    case 0:  return (VoceRosa){ "LOTTA", "A", UI_ICONA_SPADA, 0, 1 };
    case 1:  return (VoceRosa){ "ELYON", "Y", UI_ICONA_ELYON, 0, -1 };
    case 2:  return (VoceRosa){ "ZAINO", "X", UI_ICONA_ZAINO, -1, 0 };
    default: return (VoceRosa){ "FUGA", "B", UI_ICONA_FUGA, 1, 0 };
    }
}

/* ------------------------------------------------------------------ geometria (condivisa da disegno e tocco) */

typedef struct { int o, oy; int cx, cy; } Layout;

static Layout Lay(const Canvas *c) {
    Layout l;
    l.o = (c->w - 320) / 2;
    l.oy = (c->h - 180) / 2;
    l.cx = c->w - c->safeR - 54;          /* centro della rosa */
    l.cy = c->h - c->safeB - 30;
    return l;
}

static Rectangle RettRosa(const Layout *l, int i) {
    VoceRosa v = Voce(i);
    return (Rectangle){ (float)(l->cx - 23 + v.dx * 29), (float)(l->cy - 8 + v.dy * 18), 46, 16 };
}

static Rectangle RettCarta(const Canvas *c, const Layout *l, int i, bool sel) {
    (void)c;
    return (Rectangle){ (float)(l->o + 4 + i * 79), (float)(c->h - c->safeB - 32 - (sel ? 4 : 0)), 75, 30 };
}

static Rectangle RettPannello(const Canvas *c) {
    int w = 200, h = 132;
    return (Rectangle){ (float)((c->w - w) / 2), (float)((c->h - h) / 2 - 10), (float)w, (float)h };
}

static Rectangle RettRiga(const Canvas *c, int i) {
    Rectangle p = RettPannello(c);
    return (Rectangle){ p.x + 4, p.y + 18 + (float)i * 18, p.width - 8, 17 };
}

static int OggettiLotta(const BattleUI *u, int *lista) {
    int n = 0;
    for (int id = 1; id < ITEM_ID_MAX; id++) {
        const ItemData *it = GetItem(id);
        if (!it || Inventario_Quantita(u->lotta.inv, id) <= 0) continue;
        if (it->categoria == CAT_CURA || it->categoria == CAT_CORE) lista[n++] = id;
    }
    return n;
}

/* ------------------------------------------------------------------ input */

void VistaLotta_PreparaInput(Slice *s) {
    BattleUI *u = s->lotta;
    Input *in = &s->in;
    const Canvas *c = &s->canvas;
    Layout l = Lay(c);
    switch (u->stato) {
    case BUI_MENU: {
        if (in->giuP) u->cursoreMenu = 0;
        if (in->suP) u->cursoreMenu = 1;
        if (in->sinistraP) u->cursoreMenu = 2;
        if (in->destraP) u->cursoreMenu = 3;
        if (s->az.premuto[AZ_FACCIA_Y]) { u->cursoreMenu = 1; in->conferma = true; }
        if (s->az.premuto[AZ_FACCIA_X]) { u->cursoreMenu = 2; in->conferma = true; }
        if (in->indietro) u->cursoreMenu = 3;              /* B porta su FUGA: serve A per confermare */
        for (int i = 0; i < 4; i++) if (Azioni_Tap(&s->az, RettRosa(&l, i))) { u->cursoreMenu = i; in->conferma = true; }
        in->suP = in->giuP = in->sinistraP = in->destraP = false;
        break;
    }
    case BUI_MOSSE: {
        const Elyon *e = u->lotta.lato[LATO_GIOCATORE].e;
        int n = Elyon_NumMosse(e);
        if (n > 0) {
            if (in->sinistraP) u->cursoreMossa = (u->cursoreMossa + n - 1) % n;
            if (in->destraP) u->cursoreMossa = (u->cursoreMossa + 1) % n;
        }
        for (int i = 0; i < n; i++)
            if (Azioni_Tap(&s->az, RettCarta(c, &l, i, i == u->cursoreMossa))) {
                if (u->cursoreMossa == i) in->conferma = true;
                u->cursoreMossa = i;
            }
        in->suP = in->giuP = in->sinistraP = in->destraP = false;
        break;
    }
    case BUI_SQUADRA: {
        int n = Collezione_NumSquadra(u->lotta.coll);
        for (int i = 0; i < n; i++)
            if (Azioni_Tap(&s->az, RettRiga(c, i))) { if (u->cursoreSquadra == i) in->conferma = true; u->cursoreSquadra = i; }
        break;
    }
    case BUI_ZAINO: {
        int lista[ITEM_ID_MAX];
        int n = OggettiLotta(u, lista);
        for (int i = 0; i < n && i < 6; i++)
            if (Azioni_Tap(&s->az, RettRiga(c, i))) { if (u->cursoreZaino == i) in->conferma = true; u->cursoreZaino = i; }
        break;
    }
    case BUI_ELISIR: case BUI_IMPARA_SCELTA: {
        int *cur = u->stato == BUI_ELISIR ? &u->cursoreElisir : &u->cursoreImpara;
        int n = u->stato == BUI_ELISIR ? 4 : 5;
        for (int i = 0; i < n; i++)
            if (Azioni_Tap(&s->az, RettRiga(c, i))) { if (*cur == i) in->conferma = true; *cur = i; }
        break;
    }
    case BUI_IMPARA_SINO:
        for (int i = 0; i < 2; i++) {
            Rectangle r = { (float)((c->w - 312) / 2 + 312 - 55), (float)(c->h - c->safeB - 52 - 40 + 5 + i * 13), 48, 12 };
            if (Azioni_Tap(&s->az, r)) { if (u->cursoreSiNo == i) in->conferma = true; u->cursoreSiNo = i; }
        }
        break;
    default:
        if (s->az.tap) in->conferma = true;      /* messaggi: un tocco ovunque avanza */
        break;
    }
}

/* ------------------------------------------------------------------ pezzi grafici */

static void Cerchio(int cx, int cy, bool grande, bool giocatore, int frame) {
    const Rectangle P[8] = { LOTTA_CERCHIO_PICCOLO_LUMEN_0, LOTTA_CERCHIO_PICCOLO_LUMEN_1, LOTTA_CERCHIO_PICCOLO_LUMEN_2,
                                    LOTTA_CERCHIO_PICCOLO_LUMEN_3, LOTTA_CERCHIO_PICCOLO_LUMEN_4, LOTTA_CERCHIO_PICCOLO_LUMEN_5,
                                    LOTTA_CERCHIO_PICCOLO_LUMEN_6, LOTTA_CERCHIO_PICCOLO_LUMEN_7 };
    const Rectangle G[8] = { LOTTA_CERCHIO_GRANDE_ORO_0, LOTTA_CERCHIO_GRANDE_ORO_1, LOTTA_CERCHIO_GRANDE_ORO_2,
                                    LOTTA_CERCHIO_GRANDE_ORO_3, LOTTA_CERCHIO_GRANDE_ORO_4, LOTTA_CERCHIO_GRANDE_ORO_5,
                                    LOTTA_CERCHIO_GRANDE_ORO_6, LOTTA_CERCHIO_GRANDE_ORO_7 };
    (void)giocatore;
    Rectangle r = grande ? G[(frame / 8) % 8] : P[(frame / 8) % 8];
    DrawTextureRec(Gfx_Tex(TX_LOTTA), r, (Vector2){ (float)(cx - (int)r.width / 2), (float)(cy - (int)r.height / 2) }, WHITE);
}

static void Targa(int x, int y, int w, const LatoUI *l, bool giocatore, const BattleUI *u) {
    int h = giocatore ? 30 : 23;
    G_Rett(x, y, w, h, (Color){ PAL_NOTTE[0].r, PAL_NOTTE[0].g, PAL_NOTTE[0].b, 210 });
    Color c1 = PAL_NOTTE[3], c2 = PAL_NOTTE[1];
    c1.a = c2.a = 210;
    G_Gradiente(x + 1, y + 1, w - 2, h - 2, c1, c2, true);
    G_LineaH(x + 1, y + h - 1, w - 2, PAL_ORO[2]);
    G_LineaH(x + 1, y, w - 2, giocatore ? PAL_ORO[3] : PAL_LUMEN[2]);
    const SpeciesData *sp = GetSpecies(l->specie);
    int tx = x + 4;
    if (sp) {
        G_Badge(sp->tipo1, tx, y + 3); tx += 12;
        if (sp->tipo2 != TYPE_NONE) { G_Badge(sp->tipo2, tx, y + 3); tx += 12; }
    }
    T_TestoOmbra(l->nome, tx + 1, y + 4, PAL_CREMA);
    char lv[8];
    snprintf(lv, sizeof(lv), "%d", l->livello);
    T_Micro("LV", x + w - 16 - T_Larghezza(lv), y + 6, PAL_ORO[3]);
    T_TestoOmbra(lv, x + w - 5 - T_Larghezza(lv), y + 4, PAL_ORO[4]);
    float f = l->hpMax > 0 ? l->hp / (float)l->hpMax : 0;
    if (giocatore) {
        char num[16];
        snprintf(num, sizeof(num), "%d/%d", (int)ceilf(l->hp), l->hpMax);
        G_Barra(x + 4, y + 17, w - 26 - T_Larghezza(num), f, G_RampaHp(f), "HP", 4);
        T_TestoOmbra(num, x + w - 4 - T_Larghezza(num), y + 15, PAL_CREMA);
        G_Barra(x + 4, y + 26, w - 10, UI_FrazioneXp(l->specie, l->livello, (int32_t)u->xpMostrato), PAL_XP, NULL, 1);
    } else {
        G_Barra(x + 4, y + 16, w - 24, f, G_RampaHp(f), "HP", 4);
    }
    if (l->status) G_PillolaStato(l->status, tx + T_Larghezza(l->nome) + 5, y + 4);
}

static void Chip(int cx, int y, const char *testo, Color col) {
    int w = T_LarghezzaMicro(testo) + 12, x = cx - w / 2;
    G_Rett(x, y, w, 9, (Color){ PAL_NOTTE[0].r, PAL_NOTTE[0].g, PAL_NOTTE[0].b, 200 });
    G_LineaH(x, y + 8, w, PAL_ORO[1]);
    G_Gemma(x + 4, y + 4);
    T_Micro(testo, x + 8, y + 2, col);
}

static void Nastro(int x, int y, int w, int h, const char *testo, float caratteri, bool cristallo, int frame) {
    G_Rett(x, y, w, h, (Color){ PAL_NOTTE[0].r, PAL_NOTTE[0].g, PAL_NOTTE[0].b, 220 });
    G_LineaH(x, y, w, PAL_ORO[2]);
    G_LineaH(x, y + h - 1, w, PAL_NOTTE[0]);
    char vis[220];
    int n = (int)caratteri;
    int len = (int)strlen(testo);
    if (n > len) n = len;
    if (n < 0) n = 0;
    memcpy(vis, testo, (size_t)n);
    vis[n] = 0;
    /* non spezzare una lettera accentata a meta' */
    while (n > 0 && ((unsigned char)vis[n - 1] & 0xC0) == 0xC0) vis[--n] = 0;
    T_ACapo(vis, x + 6, y + 5, w - 16, 11, 2, PAL_CREMA, true);
    if (cristallo) G_Cristallo(x + w - 7, y + h - 10 + (frame / 16) % 2, 7, true);
}

static void Rosa(const Layout *l, int scelta, bool prompt, bool spenta, int frame) {
    for (int i = 0; i < 4; i++) {
        VoceRosa v = Voce(i);
        Rectangle r = RettRosa(l, i);
        int x = (int)r.x, y = (int)r.y, w = 46, h = 16;
        bool att = scelta == i && !spenta;
        G_Rett(x, y, w, h, PAL_NOTTE[0]);
        if (att) {
            G_Gradiente(x + 1, y + 1, w - 2, h - 2, PAL_AZZURRO[1], PAL_NOTTE[3], true);
            G_LineaH(x + 1, y + 1, w - 2, PAL_ORO[4]); G_LineaH(x + 1, y + h - 2, w - 2, PAL_ORO[2]);
            G_LineaV(x + 1, y + 1, h - 2, PAL_ORO[3]); G_LineaV(x + w - 2, y + 1, h - 2, PAL_ORO[2]);
        } else {
            Color a = PAL_NOTTE[3], b = PAL_NOTTE[2];
            a.a = b.a = 230;
            G_Gradiente(x + 1, y + 1, w - 2, h - 2, a, b, true);
            G_LineaH(x + 1, y + 1, w - 2, PAL_NOTTE[4]);
        }
        G_Icona(v.icona, x + 3, y + 3);
        T_TestoOmbra(v.nome, x + 16, y + 5, att ? PAL_ORO[4] : (spenta ? PAL_GRIGIO_UI : PAL_CREMA));
        if (prompt) {
            int gx = v.dx == 1 ? x + w + 1 : x - 10;
            G_GlifoTasto(gx, y + 4, v.tasto, att);
        }
    }
    G_Bagliore(l->cx, l->cy, 10, PAL_LUMEN[3], 90);
    G_Cristallo(l->cx, l->cy - 6, 13, true);
    (void)frame;
}

static void Carta(int x, int y, int w, const MoveData *m, int pp, bool attiva) {
    ElementType t = m->tipo;
    Color sc = PAL_TIPI[t][0], base = PAL_TIPI[t][1], chiaro = PAL_TIPI[t][2];
    int h = 30;
    G_Rett(x, y, w, h, PAL_NOTTE[0]);
    G_Gradiente(x + 1, y + 1, w - 2, h - 2, attiva ? PAL_NOTTE[3] : PAL_NOTTE[2], PAL_NOTTE[1], true);
    G_Rett(x + 1, y + 1, w - 2, 9, base);
    G_LineaH(x + 1, y + 1, w - 2, chiaro);
    G_LineaH(x + 1, y + 9, w - 2, sc);
    float lum = 0.3f * base.r + 0.59f * base.g + 0.11f * base.b;
    Color ct = lum > 150 ? sc : (Color){ (unsigned char)(chiaro.r + (255 - chiaro.r) * 7 / 10), (unsigned char)(chiaro.g + (255 - chiaro.g) * 7 / 10), (unsigned char)(chiaro.b + (255 - chiaro.b) * 7 / 10), 255 };
    T_Micro(NOMI_TIPI_GEN[t], x + 14, y + 3, ct);
    G_Badge(t, x + 1, y);
    T_TestoOmbra(m->nome, x + 4, y + 12, attiva ? PAL_ORO[4] : PAL_CREMA);
    T_Micro("PP", x + 4, y + 23, PAL_GRIGIO_UI);
    char s[16];
    snprintf(s, sizeof(s), "%d/%d", pp, m->pp);
    T_Micro(s, x + 14, y + 23, pp == 0 ? PAL_TIPI[TYPE_FIAMMA][1] : PAL_CREMA);
    G_Barra(x + 32 + (T_LarghezzaMicro(s) > 17 ? 4 : 0), y + 24, w - 38 - (T_LarghezzaMicro(s) > 17 ? 4 : 0), m->pp ? (float)pp / (float)m->pp : 0, PAL_XP, NULL, 1);
    if (attiva) {
        G_LineaH(x, y, w, PAL_ORO[4]); G_LineaH(x, y + h - 1, w, PAL_ORO[2]);
        G_LineaV(x, y, h, PAL_ORO[3]); G_LineaV(x + w - 1, y, h, PAL_ORO[2]);
    }
}

static void DettaglioMossa(int cx, int y, const BattleUI *u, const MoveData *m) {
    int dw = 104, dx = cx - dw / 2;
    if (dx < 2) dx = 2;
    G_Rett(dx, y, dw, 30, (Color){ PAL_NOTTE[0].r, PAL_NOTTE[0].g, PAL_NOTTE[0].b, 225 });
    G_LineaH(dx, y, dw, PAL_ORO[3]);
    G_LineaH(dx, y + 29, dw, PAL_ORO[1]);
    T_Micro(Categoria_Nome(m->categoria), dx + 5, y + 4, PAL_GRIGIO_UI);
    char s[16];
    T_Micro("POT", dx + 5, y + 13, PAL_ORO[3]);
    snprintf(s, sizeof(s), "%s", m->potenza ? TextFormat("%d", m->potenza) : "-");
    T_Testo(s, dx + 19, y + 11, PAL_CREMA);
    T_Micro("PRE", dx + 45, y + 13, PAL_ORO[3]);
    snprintf(s, sizeof(s), "%s", (m->flag & MOSSA_PRECISIONE_AUTOMATICA) ? "-" : TextFormat("%d", m->precisione));
    T_Testo(s, dx + 59, y + 11, PAL_CREMA);
    const char *eff = "EFFICACIA NORMALE";
    Color ce = PAL_CREMA;
    if (m->categoria != CAT_STATO) {
        float f = Lotta_EfficaciaContro(&u->lotta, LATO_NEMICO, m->tipo);
        if (f > 1.01f) { eff = "SUPER EFFICACE"; ce = PAL_HP_ALTO[2]; }
        else if (f < 0.01f) { eff = "NESSUN EFFETTO"; ce = PAL_GRIGIO_UI; }
        else if (f < 0.99f) { eff = "POCO EFFICACE"; ce = PAL_HP_BASSO[2]; }
    } else eff = "MOSSA DI STATO";
    T_Micro(eff, dx + 5, y + 22, ce);
    for (int k = 0; k < 3; k++) DrawPixel(dx + dw / 2 - 1 + k, y + 30, PAL_ORO[1]);
    DrawPixel(dx + dw / 2, y + 31, PAL_ORO[1]);
}

/* Sprite di un lato con le animazioni di battle_ui */
static void Lato(const BattleUI *u, int lato, int cx, int baseY, int frame) {
    const LatoUI *l = &u->lato[lato];
    if (!l->visibile && !(l->anim == A_KO && l->animTimer < DURATA[A_KO])) return;
    if (!G_HaSprite(l->specie)) return;
    bool mio = lato == LATO_GIOCATORE;
    VistaElyon v = mio ? VISTA_MIO : VISTA_NEMICO;
    Vector2 d = G_DimElyon(v);
    int x = cx - (int)d.x / 2, y = baseY - (mio ? 72 : 64);
    int f = (frame / 30) % 2;
    if ((frame + lato * 97) % 240 < 8) f = 2;   /* battito di ciglia ogni tanto */
    Color tinta = WHITE;
    int t = l->animTimer, dur = DURATA[l->anim < 8 ? l->anim : 0];
    switch (l->anim) {
    case A_ENTRATA: {
        float k = 1.0f - (float)t / (float)dur;
        x += (int)((mio ? -1 : 1) * 120 * k * k);
        tinta.a = (unsigned char)(255 * (1 - k * 0.5f));
        break;
    }
    case A_ATTACCO:
        f = 3;
        x += (mio ? 1 : -1) * (t < dur / 2 ? t : dur - t);
        break;
    case A_COLPITO:
        if ((t / 3) % 2 == 0) f = 4;
        else x += (t % 2) ? 1 : -1;
        break;
    case A_KO:
        y += t * 2;
        tinta.a = (unsigned char)(255 * (1.0f - (float)t / (float)dur));
        break;
    case A_RITIRO: case A_CATTURATO: {
        float k = (float)t / (float)dur;
        G_ElyonSagoma(l->specie, v, x, y, (Color){ 230, 255, 250, (unsigned char)(255 * (1 - k)) });
        G_Bagliore(cx, y + (int)d.y / 2, (int)(30 * (1 - k)) + 4, PAL_LUMEN[3], 140);
        return;
    }
    case A_RIAPPARE: {
        float k = (float)t / (float)dur;
        tinta.a = (unsigned char)(255 * k);
        G_Bagliore(cx, y + (int)d.y / 2, 24, PAL_LUMEN[3], (int)(140 * (1 - k)));
        break;
    }
    default: break;
    }
    G_Elyon(l->specie, v, f, x, y, tinta);
}

static void EffettoColpo(const BattleUI *u, int cxMio, int yMio, int cxSuo, int ySuo) {
    if (u->effettoTimer < 0 || u->effettoTimer >= 14) return;
    bool suNemico = u->effettoLato == LATO_NEMICO;
    int cx = suNemico ? cxSuo : cxMio, cy = suNemico ? ySuo - 30 : yMio - 34;
    ElementType t = u->effettoTipo;
    int k = u->effettoTimer;
    G_Bagliore(cx, cy, 10 + k * 2, PAL_TIPI[t][2], 160 - k * 10);
    for (int i = 0; i < 10; i++) {
        float a = (float)i / 10.0f * 6.2831853f;
        int r = 4 + k * 2;
        DrawPixel(cx + (int)(cosf(a) * r), cy + (int)(sinf(a) * r * 0.8f), PAL_TIPI[t][2]);
        DrawPixel(cx + (int)(cosf(a) * (r - 2)), cy + (int)(sinf(a) * (r - 2) * 0.8f), PAL_TIPI[t][1]);
    }
}

static void Boost(const BattleUI *u, int cxMio, int yMio, int cxSuo, int ySuo, int frame) {
    if (u->boostTimer <= 0) return;
    bool mio = u->boostLato == LATO_GIOCATORE;
    int cx = mio ? cxMio : cxSuo, y = mio ? yMio - 40 : ySuo - 36;
    Color c = u->boostDir < 0 ? PAL_LUMEN[3] : PAL_TIPI[TYPE_FIAMMA][1];
    for (int i = -2; i <= 2; i++) {
        int yy = y + ((frame * 2 + i * 7) % 24) * (u->boostDir < 0 ? -1 : 1);
        int xx = cx + i * 9;
        DrawTriangle((Vector2){ (float)xx, (float)(yy + (u->boostDir < 0 ? -3 : 3)) }, (Vector2){ (float)(xx - 3), (float)yy },
                     (Vector2){ (float)(xx + 3), (float)yy }, c);
        DrawTriangle((Vector2){ (float)xx, (float)(yy + (u->boostDir < 0 ? -3 : 3)) }, (Vector2){ (float)(xx + 3), (float)yy },
                     (Vector2){ (float)(xx - 3), (float)yy }, c);
    }
}

static void Core(const BattleUI *u, int cxMio, int yMio, int cxSuo, int ySuo, int frame) {
    if (!u->coreVisibile) return;
    int cx = cxSuo, cy = ySuo - 12;
    if (u->coreAnim == 1) {            /* lancio ad arco */
        float k = (float)u->coreTimer / 30.0f;
        if (k > 1) k = 1;
        cx = (int)(cxMio + (cxSuo - cxMio) * k);
        cy = (int)(yMio - 40 + (ySuo - 12 - (yMio - 40)) * k - sinf(k * 3.14159f) * 40);
    } else if (u->coreAnim == 2) {     /* scossa */
        int t = u->coreTimer % 34;
        cx += t < 12 ? (int)(sinf(t * 0.8f) * 3) : 0;
    }
    if (u->coreAnim >= 2) {
        for (int r = 14, a = 220; r <= 30; r += 8, a -= 70)
            for (int k = 0; k < 360; k += 4) {
                float ang = (float)k * 0.0174533f;
                Color c = PAL_LUMEN[4];
                c.a = (unsigned char)a;
                DrawPixel(cx + (int)(cosf(ang) * r), cy + 8 + (int)(sinf(ang) * r * 0.3f), c);
            }
    }
    G_Bagliore(cx, cy, 24, PAL_LUMEN[3], 110);
    for (int yy = -7; yy <= 7; yy++)
        for (int xx = -7; xx <= 7; xx++) {
            float d = sqrtf((float)(xx * xx + yy * yy));
            if (d > 7) continue;
            Color c = d > 5 ? (xx + yy < -3 ? PAL_ORO[4] : PAL_ORO[2]) : (xx + yy < -3 ? PAL_LUMEN[4] : xx + yy < 3 ? PAL_LUMEN[3] : PAL_LUMEN[2]);
            DrawPixel(cx + xx, cy + yy, c);
        }
    G_Cristallo(cx, cy - 7, 7, true);
    if (u->coreAnim == 3)               /* cattura riuscita: cristalli che salgono */
        for (int i = 0; i < 4; i++) G_Cristallo(cx - 16 + i * 11, cy - 10 - (u->coreTimer + i * 5) % 20, 7, true);
    (void)frame;
}

static void Lista(const Canvas *c, const char *titolo, int frame) {
    Rectangle p = RettPannello(c);
    G_Oscura(0, 0, c->w, c->h, 110);
    G_Cornice((int)p.x, (int)p.y, (int)p.width, (int)p.height, CORNICE_NOTTE, true);
    G_Intestazione((int)p.x + 8, (int)p.y - 6, titolo);
    (void)frame;
}

static void RigaElyon(const Canvas *c, int i, const Elyon *e, bool sel, int frame) {
    Rectangle r = RettRiga(c, i);
    int x = (int)r.x, y = (int)r.y;
    if (sel) { G_RigaSelezionata(x, y, (int)r.width, 17); G_Cursore(x + 3, y + 8, frame); }
    if (G_HaSprite(e->specie)) {
        BeginScissorMode(x + 8, y, 22, 17);
        G_Elyon(e->specie, VISTA_ICONA, sel ? (frame / 15) % 2 : 0, x + 3, y - 10, WHITE);
        EndScissorMode();
    }
    T_TestoOmbra(Elyon_Nome(e), x + 32, y + 2, sel ? PAL_ORO[4] : (Elyon_Esausto(e) ? PAL_GRIGIO_UI : PAL_CREMA));
    char lv[12];
    snprintf(lv, sizeof(lv), "LV %d", e->livello);
    T_Micro(lv, x + (int)r.width - 6 - T_LarghezzaMicro(lv), y + 3, PAL_ORO[3]);
    float f = e->stat[STAT_HP] ? (float)e->hp / (float)e->stat[STAT_HP] : 0;
    G_Barra(x + 32, y + 12, 80, f, G_RampaHp(f), NULL, 2);
    char hp[16];
    snprintf(hp, sizeof(hp), "%d/%d", e->hp, e->stat[STAT_HP]);
    T_Micro(hp, x + 118, y + 10, PAL_CREMA);
    if (e->status) G_PillolaStato(e->status, x + (int)r.width - 30, y + 8);
}

/* ------------------------------------------------------------------ disegno */

void VistaLotta_Disegna(const Slice *s) {
    const BattleUI *u = s->lotta;
    const Canvas *c = &s->canvas;
    Layout l = Lay(c);
    int frame = s->frame;

    if (u->stato == BUI_EVOLUZIONE) {
        VistaEvoluzione_Disegna(c, u->evoDa, u->evoFase == 3 ? u->evoDa : u->evoA, u->evoTimer, u->evoFase, u->testo, frame);
        return;
    }

    DrawTexture(Gfx_Tex(TX_LOTTA_PRATO), (c->w - 400) / 2, (c->h - 240) / 2, WHITE);
    int cxSuo = 226 + l.o, ySuo = 112 + l.oy, cxMio = 92 + l.o, yMio = 130 + l.oy;
    Cerchio(cxSuo, ySuo, false, false, frame);
    Cerchio(cxMio, yMio, true, true, frame);
    Lato(u, LATO_NEMICO, cxSuo, ySuo, frame);
    Lato(u, LATO_GIOCATORE, cxMio, yMio, frame);
    EffettoColpo(u, cxMio, yMio, cxSuo, ySuo);
    Boost(u, cxMio, yMio, cxSuo, ySuo, frame);
    Core(u, cxMio, yMio, cxSuo, ySuo, frame);

    /* HUD: una targa per lato, ancorata al bordo; chip centrale del turno e del meteo */
    if (u->lato[LATO_GIOCATORE].visibile || u->lato[LATO_GIOCATORE].anim)
        Targa(4 + c->safeL, 4 + c->safeT, 148, &u->lato[LATO_GIOCATORE], true, u);
    if (u->lato[LATO_NEMICO].visibile || u->lato[LATO_NEMICO].anim)
        Targa(c->w - c->safeR - 152, 4 + c->safeT, 148, &u->lato[LATO_NEMICO], false, u);
    if (u->stato != BUI_TRANSIZIONE) {
        char chip[48];
        if (u->meteo) snprintf(chip, sizeof(chip), "TURNO %d  %s", u->lotta.turno + 1, Meteo_Nome(u->meteo));
        else snprintf(chip, sizeof(chip), "TURNO %d", u->lotta.turno + 1);
        Chip(c->w / 2, 38 + c->safeT, chip, PAL_CREMA);
        if (u->lotta.tipo == LOTTA_ALLENATORE && u->lotta.allenatore) {
            char nome[48];
            snprintf(nome, sizeof(nome), "%s %s", u->lotta.allenatore->classe, u->lotta.allenatore->nome);
            for (char *p = nome; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
            int nv = 0;
            for (int i = 0; i < u->lotta.numNemici; i++) nv += !Elyon_Esausto(&u->lotta.nemici[i]);
            Chip(c->w / 2, 49 + c->safeT, nome, PAL_LUMEN[3]);
            for (int i = 0; i < u->lotta.numNemici; i++)
                G_Cristallo(c->w - c->safeR - 10 - i * 8, 29 + c->safeT, 7, i < nv);
        }
    }

    bool prompt = s->az.ultimo != DISP_TOUCH;
    int nx = 4 + c->safeL, ny = c->h - c->safeB - 34;
    switch (u->stato) {
    case BUI_TRANSIZIONE: {
        /* apertura a iride di Lumen */
        float k = (float)u->timer / 48.0f;
        int r = (int)(k * k * 260);
        G_Oscura(0, 0, c->w, c->h, (unsigned char)(255 * (1 - k)));
        G_Bagliore(c->w / 2, c->h / 2, r / 2 + 10, PAL_LUMEN[3], (int)(160 * (1 - k)));
        break;
    }
    case BUI_MENU:
        Nastro(nx, ny, c->w - c->safeL - c->safeR - 114, 30, u->testo, (float)strlen(u->testo), false, frame);
        Rosa(&l, u->cursoreMenu, prompt, false, frame);
        break;
    case BUI_MOSSE: {
        const Elyon *e = u->lotta.lato[LATO_GIOCATORE].e;
        for (int i = 0; i < MAX_MOSSE_ELYON; i++) {
            const MoveData *m = GetMove(e->mosse[i]);
            if (!m) continue;
            Rectangle r = RettCarta(c, &l, i, i == u->cursoreMossa);
            Carta((int)r.x, (int)r.y, 75, m, e->pp[i], i == u->cursoreMossa);
        }
        const MoveData *m = GetMove(e->mosse[u->cursoreMossa]);
        if (m) {
            Rectangle r = RettCarta(c, &l, u->cursoreMossa, true);
            DettaglioMossa((int)r.x + 37, (int)r.y - 38, u, m);
        }
        if (prompt) {
            G_GlifoTasto(c->w - c->safeR - 56, c->h - c->safeB - 47, Azioni_Prompt(&s->az, AZ_ANNULLA), false);
            T_Micro("INDIETRO", c->w - c->safeR - 44, c->h - c->safeB - 45, PAL_ORO[3]);
        }
        break;
    }
    case BUI_SQUADRA: {
        Lista(c, u->modoSquadra == SQ_FORZATO ? "CHI MANDI IN CAMPO?" : "SQUADRA", frame);
        int n = Collezione_NumSquadra(u->lotta.coll);
        for (int i = 0; i < n; i++) RigaElyon(c, i, &u->lotta.coll->squadra[i], i == u->cursoreSquadra, frame);
        break;
    }
    case BUI_ZAINO: {
        Lista(c, "ZAINO", frame);
        int lista[ITEM_ID_MAX];
        int n = OggettiLotta(u, lista);
        Rectangle p = RettPannello(c);
        if (n == 0) T_TestoOmbra("Nessun oggetto utilizzabile.", (int)p.x + 12, (int)p.y + 22, PAL_GRIGIO_UI);
        for (int i = 0; i < n && i < 6; i++) {
            Rectangle r = RettRiga(c, i);
            bool sel = i == u->cursoreZaino;
            if (sel) { G_RigaSelezionata((int)r.x, (int)r.y, (int)r.width, 17); G_Cursore((int)r.x + 3, (int)r.y + 8, frame); }
            G_IconaOggetto(lista[i], (int)r.x + 12, (int)r.y + 3);
            T_TestoOmbra(GetItem(lista[i])->nome, (int)r.x + 28, (int)r.y + 4, sel ? PAL_ORO[4] : PAL_CREMA);
            char q[8];
            snprintf(q, sizeof(q), "x%d", Inventario_Quantita(u->lotta.inv, lista[i]));
            T_TestoOmbra(q, (int)(r.x + r.width) - 8 - T_Larghezza(q), (int)r.y + 4, PAL_CREMA);
        }
        if (n > 0) {
            int cur = u->cursoreZaino < n ? u->cursoreZaino : n - 1;
            G_Cornice((int)p.x, (int)(p.y + p.height) + 2, (int)p.width, 26, CORNICE_PERGAMENA, false);
            T_ACapo(GetItem(lista[cur])->descrizione, (int)p.x + 8, (int)(p.y + p.height) + 6, (int)p.width - 16, 10, 2, PAL_INCHIOSTRO, false);
        }
        break;
    }
    case BUI_ELISIR: case BUI_IMPARA_SCELTA: {
        bool impara = u->stato == BUI_IMPARA_SCELTA;
        const Elyon *e = &u->lotta.coll->squadra[impara ? u->imparaIndice : u->bersaglioOggetto];
        Lista(c, impara ? "QUALE MOSSA DIMENTICA?" : "QUALE MOSSA?", frame);
        int cur = impara ? u->cursoreImpara : u->cursoreElisir;
        for (int i = 0; i < (impara ? 5 : 4); i++) {
            const MoveData *m = GetMove(i < 4 ? e->mosse[i] : u->imparaMossa);
            if (!m) continue;
            Rectangle r = RettRiga(c, i);
            if (cur == i) { G_RigaSelezionata((int)r.x, (int)r.y, (int)r.width, 17); G_Cursore((int)r.x + 3, (int)r.y + 8, frame); }
            G_PillolaTipo(m->tipo, (int)r.x + 12, (int)r.y + 3, 52);
            T_TestoOmbra(m->nome, (int)r.x + 70, (int)r.y + 4, cur == i ? PAL_ORO[4] : (i == 4 ? PAL_LUMEN[3] : PAL_CREMA));
            if (i < 4) {
                char pp[12];
                snprintf(pp, sizeof(pp), "PP %d/%d", e->pp[i], m->pp);
                T_Micro(pp, (int)(r.x + r.width) - 6 - T_LarghezzaMicro(pp), (int)r.y + 6, PAL_CREMA);
            }
        }
        break;
    }
    case BUI_IMPARA_SINO:
        Nastro(nx, ny, c->w - c->safeL - c->safeR - 8, 30, u->testo, (float)strlen(u->testo), false, frame);
        VistaDialogo_SiNo(u->cursoreSiNo, c, frame);
        break;
    default:   /* eventi, avvisi, fine */
        Nastro(nx, ny, c->w - c->safeL - c->safeR - 8, 30, u->testo, u->stato == BUI_AVVISO ? (float)strlen(u->testo) : u->caratteri,
               u->caratteri >= (float)strlen(u->testo), frame);
        if (u->coreAnim == 2 || u->coreAnim == 3)
            for (int i = 0; i < 3; i++) {
                int fatte = u->pos < u->numCoda && u->coda[u->pos].tipo == EV_SCOSSA ? u->coda[u->pos].a : (u->coreAnim == 3 ? 3 : 0);
                int gx = c->w - c->safeR - 60 + i * 14, gy = ny + 15;
                if (i < fatte) G_Gemma(gx, gy); else { DrawPixel(gx, gy, PAL_PIETRA[3]); DrawCircleLines(gx, gy, 2, PAL_PIETRA[2]); }
            }
        break;
    }
}
