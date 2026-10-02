/* bestiario.c - Il codice dei Custodi (Design System §9, mockup 05_bestiario).
 * Copertina blu notte e due pagine di pergamena; stati IGNOTO, VISTO, CATTURATO con sblocco
 * progressivo. Lista dei 100 numeri, voce a destra, CONFERMA apre la voce a schermo intero. */
#include "slice.h"
#include "gfx.h"
#include "gen/atlante_ui.h"
#include "data/encounters_data.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Campi che il database non ha ancora (Design System §9 "Dati da aggiungere al database").
 * PROPOSTA per i 9 Elyon di riferimento, da approvare: vedi docs/VERTICAL_SLICE.md. */
typedef struct { const char *categoria; int altezzaDm, pesoHg; } DatiVoce;
static const DatiVoce DATI[10] = {
    { NULL, 0, 0 },
    { "Cucciolo di brace", 4, 52 }, { "Corridore di brace", 9, 185 }, { "Fucina vivente", 16, 960 },
    { "Lontra-pinna", 5, 68 }, { "Danzatore d'onde", 10, 224 }, { "Guida stellare", 18, 840 },
    { "Zolla di muschio", 4, 95 }, { "Custode dei germogli", 9, 410 }, { "Collina che cammina", 20, 3100 },
};

typedef enum { ST_IGNOTO = 0, ST_VISTO, ST_CATTURATO } StatoVoce;
static int filtro, scroll;
static bool dettaglio;
static const char *FILTRI[3] = { "TUTTI", "VISTI", "CATTURATI" };

static StatoVoce Stato(const Slice *s, int n) { return s->catturato[n] ? ST_CATTURATO : s->visto[n] ? ST_VISTO : ST_IGNOTO; }

static int Voci(const Slice *s, int *lista) {
    int k = 0;
    for (int n = 1; n <= 100; n++) {
        StatoVoce st = Stato(s, n);
        if (filtro == 1 && st == ST_IGNOTO) continue;
        if (filtro == 2 && st != ST_CATTURATO) continue;
        lista[k++] = n;
    }
    return k;
}

#define RIGHE 9

static Rectangle RigaLista(const Slice *s, int i) {
    int ox = (s->canvas.w - 320) / 2, oy = (s->canvas.h - 180) / 2;
    return (Rectangle){ (float)(ox + 11), (float)(oy + 39 + i * 13), 144, 12 };
}

void Bestiario_Aggiorna(Slice *s) {
    Input *in = &s->in;
    int lista[100];
    int n = Voci(s, lista);
    if (dettaglio) {
        if (in->indietro || in->conferma || s->az.tap) dettaglio = false;
        if (n > 0 && in->suP) s->cursoreBest = (s->cursoreBest + n - 1) % n;
        if (n > 0 && in->giuP) s->cursoreBest = (s->cursoreBest + 1) % n;
        return;
    }
    if (in->sinistraP) { filtro = (filtro + 2) % 3; s->cursoreBest = 0; scroll = 0; }
    if (in->destraP) { filtro = (filtro + 1) % 3; s->cursoreBest = 0; scroll = 0; }
    n = Voci(s, lista);
    if (n > 0) {
        if (in->suP) s->cursoreBest = (s->cursoreBest + n - 1) % n;
        if (in->giuP) s->cursoreBest = (s->cursoreBest + 1) % n;
    }
    if (s->cursoreBest >= n) s->cursoreBest = n > 0 ? n - 1 : 0;
    for (int i = 0; i < RIGHE && scroll + i < n; i++)
        if (Azioni_Tap(&s->az, RigaLista(s, i))) {
            if (s->cursoreBest == scroll + i) in->conferma = true;
            s->cursoreBest = scroll + i;
        }
    int ox = (s->canvas.w - 320) / 2, oy = (s->canvas.h - 180) / 2, fx = ox + 13;
    for (int i = 0; i < 3; i++) {
        int w = T_LarghezzaMicro(FILTRI[i]) + 6;
        if (Azioni_Tap(&s->az, (Rectangle){ (float)fx, (float)(oy + 27), (float)w, 9 })) { filtro = i; s->cursoreBest = 0; scroll = 0; }
        fx += w + 3;
    }
    if (s->cursoreBest < scroll) scroll = s->cursoreBest;
    if (s->cursoreBest >= scroll + RIGHE) scroll = s->cursoreBest - RIGHE + 1;
    if (in->indietro) { s->pannello = PAN_MENU; dettaglio = false; return; }
    if (in->conferma && n > 0 && Stato(s, lista[s->cursoreBest]) != ST_IGNOTO) dettaglio = true;
}

static void Medaglione(int cx, int cy, int rx, int ry) {
    for (int yy = -ry; yy <= ry; yy++)
        for (int xx = -rx; xx <= rx; xx++) {
            float d = (float)(xx * xx) / (float)(rx * rx) + (float)(yy * yy) / (float)(ry * ry);
            if (d <= 1) {
                float t = 1 - d;
                Color a = PAL_PERGAMENA[2], b = PAL_PERGAMENA[4];
                DrawPixel(cx + xx, cy + yy, (Color){ (unsigned char)(a.r + (b.r - a.r) * t), (unsigned char)(a.g + (b.g - a.g) * t), (unsigned char)(a.b + (b.b - a.b) * t), 255 });
            }
        }
    for (int k = 0; k < 360; k += 4) {
        float a = (float)k * 0.0174533f;
        DrawPixel(cx + (int)(cosf(a) * (rx - 1)), cy + (int)(sinf(a) * (ry - 1)), PAL_ORO[2]);
    }
}

static void Sigillo(int sx, int sy) {
    for (int k = 0; k < 360; k += 6) {
        float a = (float)k * 0.0174533f;
        DrawPixel(sx + (int)(cosf(a) * 10), sy + (int)(sinf(a) * 10), PAL_LUMEN[1]);
        DrawPixel(sx + (int)(cosf(a) * 8), sy + (int)(sinf(a) * 8), PAL_LUMEN[1]);
    }
    G_Icona(UI_ICONA_CORE, sx - 5, sy - 5);
}

static void Habitat(const Slice *s, int n, char *out, int dim) {
    if (s->dove[n][0]) { snprintf(out, (size_t)dim, "%s", s->dove[n]); return; }
    for (int h = 0; h < NumHabitat(); h++) {
        const HabitatData *hd = GetHabitatIndex(h);
        for (int v = 0; v < hd->numVoci; v++) if (hd->voci[v].specie == n) { snprintf(out, (size_t)dim, "%s", hd->nome); return; }
    }
    snprintf(out, (size_t)dim, "Non osservato in natura");
}

/* Voce del codice. Pagina normale: 144 x 145 px. Schermo intero (grande): 300 x 145 px. */
static void Voce(const Slice *s, int n, int x0, int y0, bool grande) {
    const SpeciesData *sp = GetSpecies(n);
    StatoVoce st = Stato(s, n);
    const DatiVoce *d = n <= 9 ? &DATI[n] : NULL;
    int larg = grande ? 290 : 140;
    T_Micro(TextFormat("N. %03d", n), x0 + 2, y0, PAL_PERGAMENA[0]);
    T_Grassetto(sp->nome, x0 + 2, y0 + 8, PAL_INCHIOSTRO, BLANK);
    G_PillolaTipo(sp->tipo1, x0 + 2, y0 + 19, 56);
    int yc = y0 + 31;
    if (sp->tipo2 != TYPE_NONE) { G_PillolaTipo(sp->tipo2, x0 + 2, y0 + 30, 56); yc += 11; }
    if (st == ST_CATTURATO && d && d->categoria) T_Testo(d->categoria, x0 + 2, yc, PAL_TERRA[1]);
    /* medaglione con lo sprite, a destra */
    int mx = x0 + (grande ? 220 : 104), my = y0 + (grande ? 46 : 38);
    Medaglione(mx, my, grande ? 46 : 34, grande ? 30 : 22);
    if (G_HaSprite(n)) {
        int f = grande ? (s->frame / 30) % 2 : 0;
        if (st == ST_CATTURATO) G_Elyon(n, VISTA_FRONTE, f, mx - 32, my - 42, WHITE);
        else G_ElyonSagoma(n, VISTA_FRONTE, mx - 32, my - 42, PAL_TIPI[sp->tipo1][0]);   /* VISTO: sagoma colorata */
    }
    if (st == ST_CATTURATO) Sigillo(x0 + (grande ? 140 : 66), y0 + 10);
    char hab[48];
    Habitat(s, n, hab, sizeof(hab));
    char alt[24] = "?", peso[24] = "?";
    if (st == ST_CATTURATO && d && d->categoria) {
        snprintf(alt, sizeof(alt), "%d,%d m", d->altezzaDm / 10, d->altezzaDm % 10);
        snprintf(peso, sizeof(peso), "%d,%d kg", d->pesoHg / 10, d->pesoHg % 10);
    }
    const char *k[3] = { "HABITAT", "ALTEZZA", "PESO" };
    const char *v[3] = { hab, alt, peso };
    int ky = y0 + (grande ? 70 : 62);
    for (int i = 0; i < 3; i++) {
        T_Micro(k[i], x0 + 2, ky + i * 11 + 2, PAL_TERRA[1]);
        T_Testo(v[i], x0 + 36, ky + i * 11, PAL_INCHIOSTRO);
    }
    int ys = ky + 35;
    G_LineaH(x0 + 2, ys, larg, PAL_PERGAMENA[1]);
    if (st == ST_CATTURATO) T_ACapo(sp->descrizione, x0 + 2, ys + 4, larg, 11, grande ? 3 : 3, PAL_INCHIOSTRO, false);
    else T_ACapo("Cattura questo Elyon per completare la voce.", x0 + 2, ys + 4, larg, 11, 2, PAL_PERGAMENA[0], false);
}

void Bestiario_Disegna(const Slice *s) {
    const Canvas *c = &s->canvas;
    int ox = (c->w - 320) / 2, oy = (c->h - 180) / 2;
    G_Gradiente(0, 0, c->w, c->h, PAL_NOTTE[2], PAL_NOTTE[0], true);
    G_Intestazione(ox + 6, oy + 3, "BESTIARIO");
    int visti = 0, catt = 0;
    for (int n = 1; n <= 100; n++) { visti += s->visto[n]; catt += s->catturato[n]; }
    char cont[48];
    snprintf(cont, sizeof(cont), "SCOPERTI %d   CATTURATI %d   DI 100", visti, catt);
    T_Micro(cont, ox + 316 - T_LarghezzaMicro(cont), oy + 7, PAL_ORO[3]);
    /* copertina e pagine */
    Color cop = { 14, 22, 56, 255 };
    G_Rett(ox + 4, oy + 19, 312, 159, cop);
    G_LineaH(ox + 5, oy + 20, 310, PAL_ORO[2]);
    G_LineaH(ox + 5, oy + 176, 310, PAL_ORO[1]);
    if (dettaglio) {
        G_Gradiente(ox + 9, oy + 23, 302, 150, PAL_PERGAMENA[4], PAL_PERGAMENA[3], true);
        int lista[100];
        int n = Voci(s, lista);
        if (n > 0) Voce(s, lista[s->cursoreBest], ox + 14, oy + 28, true);
        return;
    }
    G_Gradiente(ox + 9, oy + 23, 148, 150, PAL_PERGAMENA[4], PAL_PERGAMENA[3], true);
    G_Gradiente(ox + 163, oy + 23, 148, 150, PAL_PERGAMENA[4], PAL_PERGAMENA[3], true);
    for (int j = 23; j < 173; j++) {
        DrawPixel(ox + 158, oy + j, PAL_PERGAMENA[0]); DrawPixel(ox + 159, oy + j, cop);
        DrawPixel(ox + 160, oy + j, cop); DrawPixel(ox + 162, oy + j, PAL_PERGAMENA[0]);
    }
    for (int j = 20; j < 177; j += 6) DrawPixel(ox + 160, oy + j, PAL_ORO[2]);
    /* filtri */
    int fx = ox + 13;
    for (int i = 0; i < 3; i++) {
        int w = T_LarghezzaMicro(FILTRI[i]) + 6;
        G_Rett(fx, oy + 27, w, 9, i == filtro ? PAL_TERRA[2] : PAL_PERGAMENA[2]);
        T_Micro(FILTRI[i], fx + 3, oy + 29, i == filtro ? PAL_PERGAMENA[4] : PAL_PERGAMENA[0]);
        fx += w + 3;
    }
    int lista[100];
    int n = Voci(s, lista);
    for (int i = 0; i < RIGHE && scroll + i < n; i++) {
        int num = lista[scroll + i];
        Rectangle r = RigaLista(s, i);
        int y = (int)r.y + 1;
        StatoVoce st = Stato(s, num);
        if (scroll + i == s->cursoreBest) {
            G_Rett((int)r.x, y - 1, 144, 12, (Color){ PAL_ORO[3].r, PAL_ORO[3].g, PAL_ORO[3].b, 110 });
            G_LineaH((int)r.x, y - 1, 144, PAL_ORO[2]); G_LineaH((int)r.x, y + 10, 144, PAL_ORO[2]);
            G_Cursore(ox + 13, y + 5, s->frame);
        }
        T_Micro(TextFormat("%03d", num), ox + 21, y + 3, PAL_PERGAMENA[0]);
        if (st == ST_CATTURATO) G_Icona(UI_ICONA_CORE, ox + 36, y);
        else if (st == ST_VISTO) G_Icona(UI_OCCHIO, ox + 39, y + 4);
        else T_Micro("?", ox + 40, y + 3, PAL_PERGAMENA[1]);
        T_Testo(st != ST_IGNOTO ? GetSpecies(num)->nome : "- - -", ox + 52, y + 2, st != ST_IGNOTO ? PAL_INCHIOSTRO : PAL_PERGAMENA[1]);
    }
    /* barra di scorrimento */
    if (n > RIGHE) {
        int h = 117 * RIGHE / n, y = oy + 39 + (117 - h) * scroll / (n - RIGHE);
        G_Rett(ox + 154, oy + 39, 2, 117, PAL_PERGAMENA[2]);
        G_Rett(ox + 154, y, 2, h, PAL_TERRA[2]);
    }
    T_Micro("CATTURATO", ox + 18, oy + 162, PAL_TERRA[1]); G_Icona(UI_ICONA_CORE, ox + 54, oy + 159);
    G_Icona(UI_OCCHIO, ox + 70, oy + 163); T_Micro("VISTO", ox + 78, oy + 162, PAL_TERRA[1]);
    T_Micro("?  IGNOTO", ox + 106, oy + 162, PAL_PERGAMENA[1]);
    if (n > 0) {
        int num = lista[s->cursoreBest];
        if (Stato(s, num) == ST_IGNOTO) {
            T_Micro(TextFormat("N. %03d", num), ox + 169, oy + 28, PAL_PERGAMENA[0]);
            T_Testo("- - -", ox + 169, oy + 36, PAL_PERGAMENA[1]);
            T_ACapo("Nessuna notizia di questo Elyon nel codice.", ox + 169, oy + 60, 136, 11, 3, PAL_PERGAMENA[0], false);
        } else {
            Voce(s, num, ox + 167, oy + 28, false);
        }
    } else {
        T_ACapo("Nessuna voce con questo filtro.", ox + 169, oy + 60, 136, 11, 3, PAL_PERGAMENA[0], false);
    }
}
