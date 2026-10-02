/* mappa.c - Mappa di Asterra (Design System §10, mockup 06_mappa).
 * Illustrazione 320x180 con le sette regioni; nebbia sulle regioni non esplorate (calcolata dalla
 * maschera delle regioni), luoghi con rombo d'oro, posizione del giocatore con anelli di Lumen,
 * pannello della regione e barra dei Sigilli. */
#include "slice.h"
#include "gfx.h"
#include "gen/atlante_ui.h"
#include "gen/mappa.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct { const char *nome; const char *regione; int x, y; bool citta; const char *mappa; } Luogo;

/* Luoghi della mappa: posizioni dell'illustrazione approvata (rotte tra le citta') */
static const Luogo LUOGHI[] = {
    { "Lunaria", "VERDAN", 112, 140, true, "lunaria" },
    { "Route 1", "VERDAN", 131, 131, false, "route1" },
    { "Bosco Verdalia", "VERDAN", 141, 126, false, NULL },
    { "Verdan", "VERDAN", 150, 122, true, NULL },
    { "Arkas", "ARKAS", 76, 74, true, NULL },
    { "Nival", "NIVAL", 176, 40, true, NULL },
    { "Talassa", "TALASSA", 250, 132, true, NULL },
    { "Vulkar", "VULKAR", 262, 76, true, NULL },
    { "Elys", "ELYS", 176, 84, true, NULL },
};
#define NUM_LUOGHI ((int)(sizeof(LUOGHI) / sizeof(LUOGHI[0])))

static Texture2D conNebbia;
static bool pronta;

static bool RegioneVisitata(const char *nome) { return strcmp(nome, "LUNARIA") == 0 || strcmp(nome, "VERDAN") == 0; }

static void Prepara(void) {
    if (pronta) return;
    char p1[700], p2[700];
    snprintf(p1, sizeof(p1), "%smappa_asterra.png", Gfx_CartellaSlice());
    snprintf(p2, sizeof(p2), "%smappa_regioni.png", Gfx_CartellaSlice());
    Image base = LoadImage(p1), reg = LoadImage(p2);
    if (!base.data || !reg.data) return;
    ImageFormat(&base, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&reg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color *b = (Color *)base.data, *r = (Color *)reg.data;
    Color notte = PAL_NOTTE[2];
    for (int y = 0; y < base.height; y++)
        for (int x = 0; x < base.width; x++) {
            Color m = r[y * reg.width + x];
            if (m.a == 0 || m.r == 0 || m.r > NUM_REGIONI_GEN) continue;
            if (RegioneVisitata(REGIONI_GEN[m.r - 1].nome)) continue;
            /* nebbia: colore desaturato verso la notte, trama a 4 px */
            float t = (x + y) % 4 ? 0.6f : 0.75f;
            Color *c = &b[y * base.width + x];
            c->r = (unsigned char)(c->r + (notte.r - c->r) * t);
            c->g = (unsigned char)(c->g + (notte.g - c->g) * t);
            c->b = (unsigned char)(c->b + (notte.b - c->b) * t);
        }
    conNebbia = LoadTextureFromImage(base);
    SetTextureFilter(conNebbia, TEXTURE_FILTER_POINT);
    UnloadImage(base);
    UnloadImage(reg);
    pronta = true;
}

void Mappa_Libera(void) { if (pronta) UnloadTexture(conNebbia); pronta = false; }

static int LuogoGiocatore(const Slice *s) {
    for (int i = 0; i < NUM_LUOGHI; i++) if (LUOGHI[i].mappa && strcmp(LUOGHI[i].mappa, s->world.mappa.id) == 0) return i;
    return 0;
}

void Mappa_Aggiorna(Slice *s) {
    Input *in = &s->in;
    int ox = (s->canvas.w - 320) / 2, oy = (s->canvas.h - 180) / 2;
    int dx = in->destraP - in->sinistraP, dy = in->giuP - in->suP;
    if (dx || dy) {   /* il cursore salta al luogo piu' vicino nella direzione premuta */
        const Luogo *a = &LUOGHI[s->cursoreMappa];
        int migliore = -1;
        float dmin = 1e9f;
        for (int i = 0; i < NUM_LUOGHI; i++) {
            if (i == s->cursoreMappa) continue;
            float vx = (float)(LUOGHI[i].x - a->x), vy = (float)(LUOGHI[i].y - a->y);
            if (vx * dx + vy * dy <= 0) continue;
            float d = sqrtf(vx * vx + vy * vy) * (1.0f + 0.8f * fabsf(dx ? vy : vx) / (sqrtf(vx * vx + vy * vy) + 0.01f));
            if (d < dmin) { dmin = d; migliore = i; }
        }
        if (migliore >= 0) s->cursoreMappa = migliore;
    }
    for (int i = 0; i < NUM_LUOGHI; i++)
        if (Azioni_Tap(&s->az, (Rectangle){ (float)(ox + LUOGHI[i].x - 6), (float)(oy + LUOGHI[i].y - 6), 12, 12 })) s->cursoreMappa = i;
    if (in->indietro) s->pannello = PAN_MENU;
}

void Mappa_Disegna(const Slice *s) {
    Prepara();
    const Canvas *c = &s->canvas;
    int ox = (c->w - 320) / 2, oy = (c->h - 180) / 2;
    ClearBackground(PAL_ACQUA[1]);
    DrawTexture(pronta ? conNebbia : Gfx_Tex(TX_MAPPA), ox, oy, WHITE);
    /* targhe delle regioni (con nebbia: nome senza sottotitolo) */
    for (int i = 0; i < NUM_REGIONI_GEN; i++) {
        const RegioneGen *r = &REGIONI_GEN[i];
        bool vis = RegioneVisitata(r->nome);
        char sott[24];
        snprintf(sott, sizeof(sott), "%s", vis ? r->sottotitolo : "???");
        for (char *p = sott; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
        int w = (T_Larghezza(r->nome) > T_LarghezzaMicro(sott) ? T_Larghezza(r->nome) : T_LarghezzaMicro(sott)) + 8;
        int lx = ox + r->cx - w / 2, ly = oy + r->cy + 5;
        G_Rett(lx, ly, w, 17, PAL_NOTTE[0]);
        G_Rett(lx + 1, ly + 1, w - 2, 15, PAL_NOTTE[2]);
        G_LineaH(lx + 1, ly + 1, w - 2, PAL_ORO[3]);
        T_Testo(r->nome, lx + 4, ly + 2, vis ? PAL_CREMA : PAL_GRIGIO_UI);
        T_Micro(sott, lx + 4, ly + 10, vis ? (Color){ r->chiaro[0], r->chiaro[1], r->chiaro[2], 255 } : PAL_GRIGIO_UI);
    }
    /* luoghi */
    for (int i = 0; i < NUM_LUOGHI; i++) {
        const Luogo *l = &LUOGHI[i];
        int x = ox + l->x, y = oy + l->y;
        if (!l->citta) {
            DrawPixel(x, y, PAL_ORO[4]); DrawPixel(x - 1, y, PAL_ORO[2]); DrawPixel(x + 1, y, PAL_ORO[2]);
            DrawPixel(x, y - 1, PAL_ORO[2]); DrawPixel(x, y + 1, PAL_ORO[2]);
        }
        if (i == s->cursoreMappa) {
            int r = 6 + (s->frame / 10) % 2;
            DrawRectangleLines(x - r, y - r, 2 * r + 1, 2 * r + 1, PAL_LUMEN[4]);
        }
    }
    /* giocatore: Esploratore con due anelli di Lumen che pulsano */
    const Luogo *g = &LUOGHI[LuogoGiocatore(s)];
    int gx = ox + g->x, gy = oy + g->y;
    for (int k = 0; k < 2; k++) {
        float r = (k ? 6.0f : 9.0f) + (float)((s->frame / 8) % 3) * 0.5f;
        for (int a = 0; a < 360; a += 12) {
            float an = (float)a * 0.0174533f;
            Color col = PAL_LUMEN[4];
            col.a = k ? 220 : 120;
            DrawPixel(gx + (int)(cosf(an) * r), gy + (int)(sinf(an) * r * 0.6f), col);
        }
    }
    DrawTextureRec(Gfx_Tex(TX_PERSONAGGI), (Rectangle){ 0, 8 * 24, 16, 24 }, (Vector2){ (float)(gx - 8), (float)(gy - 22) }, WHITE);
    G_Intestazione(ox + 6, oy + 4, "ASTERRA");
    /* rosa dei venti */
    int cx = ox + 292, cy = oy + 30;
    for (int k = 0; k < 8; k++) {
        float a = (float)k / 8.0f * 6.2831853f - 1.5708f;
        int L = k % 2 == 0 ? 10 : 5;
        for (int d = 0; d < L; d++) DrawPixel(cx + (int)(cosf(a) * d), cy + (int)(sinf(a) * d), k == 0 ? PAL_ORO[4] : PAL_ORO[2]);
    }
    T_Micro("N", cx - 1, cy - 17, PAL_ORO[4]);
    /* pannello della regione del luogo selezionato */
    const Luogo *sel = &LUOGHI[s->cursoreMappa];
    int px = ox + 214, py = oy + 44, pw = 102;
    int nLuoghi = 0;
    for (int i = 0; i < NUM_LUOGHI; i++) nLuoghi += strcmp(LUOGHI[i].regione, sel->regione) == 0;
    G_Cornice(px, py, pw, 30 + nLuoghi * 10, CORNICE_NOTTE, true);
    bool vis = RegioneVisitata(sel->regione);
    T_TestoOmbra(sel->regione, px + 7, py + 6, PAL_ORO[4]);
    const char *tipo = "???";
    for (int i = 0; i < NUM_REGIONI_GEN; i++) if (strcmp(REGIONI_GEN[i].nome, sel->regione) == 0 && vis) tipo = REGIONI_GEN[i].sottotitolo;
    char t2[40];
    snprintf(t2, sizeof(t2), "REGIONE %s", tipo);
    for (char *p = t2; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
    T_Micro(t2, px + 7, py + 16, PAL_TIPI[TYPE_NATURA][2]);
    int k = 0;
    for (int i = 0; i < NUM_LUOGHI; i++) {
        if (strcmp(LUOGHI[i].regione, sel->regione) != 0) continue;
        int yy = py + 26 + k * 10;
        bool visitato = LUOGHI[i].mappa != NULL;
        if (visitato) G_Gemma(px + 10, yy + 3); else DrawPixel(px + 10, yy + 3, PAL_GRIGIO_UI);
        T_Testo(LUOGHI[i].nome, px + 16, yy, i == s->cursoreMappa ? PAL_ORO[4] : visitato ? PAL_CREMA : PAL_GRIGIO_UI);
        k++;
    }
    /* barra dei Sigilli */
    int by = oy + 164;
    G_Rett(0, by, c->w, c->h - by, PAL_NOTTE[0]);
    G_LineaH(0, by, c->w, PAL_ORO[2]);
    T_Micro("SIGILLI", ox + 6, by + 6, PAL_ORO[3]);
    static const char *SIG[7] = { "VERDE", "TUONO", "MAREE", "FUOCO", "GHIACCIO", "PIETRA", "STELLE" };
    for (int i = 0; i < 7; i++) {
        int x = ox + 46 + i * 39;
        G_Cristallo(x, by + 2, 11, false);
        T_Micro(SIG[i], x + 6, by + 6, PAL_GRIGIO_UI);
    }
    if (s->az.ultimo != DISP_TOUCH) {
        G_GlifoTasto(ox + 6, by - 12, Azioni_Prompt(&s->az, AZ_ANNULLA), false);
        T_MicroOmbra("INDIETRO", ox + 18, by - 10, PAL_ORO[3], PAL_NOTTE[0]);
    }
}
