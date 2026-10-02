/* gfx.c - Vedi gfx.h. Le funzioni riproducono quelle di tools/design/ui.py e font.py. */
#include "gfx.h"
#include "gen/atlante_elyon.h"
#include "gen/atlante_ui.h"
#include "gen/font.h"
#include "data/items_data.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static Texture2D tex[TX_COUNT];
static char cartella[512];

/* ------------------------------------------------------------------ font */

typedef struct { uint32_t cp; Rectangle r; int w; } GlifoTex;
static GlifoTex corpo[FONT_CORPO_N], micro[FONT_MICRO_N];
static int corpoAscii[256], microAscii[256];   /* indice+1, 0 = assente */

static Texture2D CostruisciFont(const GlifoGen *g, int n, int h, GlifoTex *out, int *ascii) {
    int larg = 1;
    for (int i = 0; i < n; i++) larg += g[i].w + 1;
    Image im = GenImageColor(larg, h, BLANK);
    int x = 1;
    for (int i = 0; i < n; i++) {
        for (int y = 0; y < h; y++)
            for (int k = 0; k < g[i].w; k++)
                if (g[i].righe[y] & (1u << k)) ImageDrawPixel(&im, x + k, y, WHITE);
        out[i].cp = g[i].cp;
        out[i].w = g[i].w;
        out[i].r = (Rectangle){ (float)x, 0, (float)g[i].w, (float)h };
        if (g[i].cp < 256) ascii[g[i].cp] = i + 1;
        x += g[i].w + 1;
    }
    Texture2D t = LoadTextureFromImage(im);
    UnloadImage(im);
    SetTextureFilter(t, TEXTURE_FILTER_POINT);
    return t;
}

static uint32_t ProssimoCp(const char **s) {
    const unsigned char *p = (const unsigned char *)*s;
    uint32_t cp;
    if (p[0] < 0x80) { cp = p[0]; *s += 1; }
    else if ((p[0] & 0xE0) == 0xC0 && p[1]) { cp = ((p[0] & 0x1Fu) << 6) | (p[1] & 0x3Fu); *s += 2; }
    else if ((p[0] & 0xF0) == 0xE0 && p[1] && p[2]) { cp = ((p[0] & 0x0Fu) << 12) | ((p[1] & 0x3Fu) << 6) | (p[2] & 0x3Fu); *s += 3; }
    else { cp = '?'; *s += 1; }
    return cp;
}

static const GlifoTex *Glifo(uint32_t cp) {
    if (cp < 256 && corpoAscii[cp]) return &corpo[corpoAscii[cp] - 1];
    for (int i = 0; i < FONT_CORPO_N; i++) if (corpo[i].cp == cp) return &corpo[i];
    return &corpo[corpoAscii['?'] - 1];
}

static uint32_t Maiuscola(uint32_t cp) {
    if (cp >= 'a' && cp <= 'z') return cp - 32;
    switch (cp) {
    case 0xE0: return 0xC0; case 0xE8: return 0xC8; case 0xE9: return 0xC9;
    case 0xEC: return 0xCC; case 0xF2: return 0xD2; case 0xF9: return 0xD9;
    default: return cp;
    }
}

static const GlifoTex *GlifoMicro(uint32_t cp) {
    cp = Maiuscola(cp);
    if (cp == 0xC0) cp = 'A';
    if (cp == 0xC8 || cp == 0xC9) cp = 'E';
    if (cp == 0xCC) cp = 'I';
    if (cp == 0xD2) cp = 'O';
    if (cp == 0xD9) cp = 'U';
    if (cp < 256 && microAscii[cp]) return &micro[microAscii[cp] - 1];
    return &micro[microAscii[' '] - 1];
}

int T_Larghezza(const char *s) {
    int w = 0, n = 0;
    while (*s && *s != '\n') { w += Glifo(ProssimoCp(&s))->w; n++; }
    return n ? w + (n - 1) * FONT_CORPO_SPAZIO : 0;
}

int T_LarghezzaGrassetto(const char *s) {
    int n = 0;
    for (const char *p = s; *p && *p != '\n';) { ProssimoCp(&p); n++; }
    return T_Larghezza(s) + n;
}

int T_LarghezzaMicro(const char *s) {
    int w = 0, n = 0;
    while (*s) { w += GlifoMicro(ProssimoCp(&s))->w + 1; n++; }
    return n ? w - 1 : 0;
}

static int TestoBase(const char *s, int x, int y, Color c, bool grassetto, const Color *righe) {
    int cx = x;
    while (*s) {
        uint32_t cp = ProssimoCp(&s);
        if (cp == '\n') break;
        const GlifoTex *g = Glifo(cp);
        for (int b = 0; b <= (grassetto ? 1 : 0); b++) {
            if (righe) {
                for (int r = 0; r < FONT_CORPO_ALTEZZA; r++) {
                    Rectangle src = { g->r.x, (float)r, g->r.width, 1 };
                    DrawTextureRec(tex[TX_FONT], src, (Vector2){ (float)(cx + b), (float)(y + r) }, righe[r]);
                }
            } else {
                DrawTextureRec(tex[TX_FONT], g->r, (Vector2){ (float)(cx + b), (float)y }, c);
            }
        }
        cx += g->w + FONT_CORPO_SPAZIO + (grassetto ? 1 : 0);
    }
    return cx - x;
}

int T_Testo(const char *s, int x, int y, Color c) { return TestoBase(s, x, y, c, false, NULL); }

int T_TestoOmbra(const char *s, int x, int y, Color c) {
    TestoBase(s, x, y + 1, PAL_OMBRA_TESTO, false, NULL);
    return TestoBase(s, x, y, c, false, NULL);
}

int T_Grassetto(const char *s, int x, int y, Color c, Color ombra) {
    if (ombra.a) { TestoBase(s, x, y + 1, ombra, true, NULL); }
    return TestoBase(s, x, y, c, true, NULL);
}

int T_TitoloOro(const char *s, int x, int y) {
    /* oro verticale di ui.oro_verticale: dalla riga 0 alla 8 */
    Color righe[FONT_CORPO_ALTEZZA] = { PAL_ORO[5], PAL_ORO[4], PAL_ORO[4], PAL_ORO[3], PAL_ORO[3],
                                        PAL_ORO[2], PAL_ORO[2], PAL_ORO[2], PAL_ORO[1] };
    TestoBase(s, x, y + 1, PAL_ORO[0], true, NULL);
    return TestoBase(s, x, y, WHITE, true, righe);
}

int T_Micro(const char *s, int x, int y, Color c) {
    int cx = x;
    while (*s) {
        const GlifoTex *g = GlifoMicro(ProssimoCp(&s));
        DrawTextureRec(tex[TX_FONT_MICRO], g->r, (Vector2){ (float)cx, (float)y }, c);
        cx += g->w + 1;
    }
    return cx - x;
}

int T_MicroOmbra(const char *s, int x, int y, Color c, Color ombra) {
    T_Micro(s, x, y + 1, ombra);
    return T_Micro(s, x, y, c);
}

/* Divide s in righe che stanno in w (spezzando sugli spazi); chiama fn per ogni riga */
typedef void (*FnRiga)(const char *riga, int indice, void *ctx);

static int Spezza(const char *s, int w, FnRiga fn, void *ctx) {
    char riga[400], prova[400];
    int n = 0;
    riga[0] = 0;
    const char *p = s;
    while (1) {
        const char *ini = p;
        while (*p && *p != ' ' && *p != '\n') p++;
        char parola[128];
        size_t lp = (size_t)(p - ini) < sizeof(parola) - 1 ? (size_t)(p - ini) : sizeof(parola) - 1;
        memcpy(parola, ini, lp);
        parola[lp] = 0;
        /* riga + spazio + parola, troncando in modo esplicito se un testo fosse lunghissimo */
        size_t lr = strlen(riga);
        if (lr > sizeof(prova) - sizeof(parola) - 2) lr = sizeof(prova) - sizeof(parola) - 2;
        memcpy(prova, riga, lr);
        if (lr) prova[lr++] = ' ';
        memcpy(prova + lr, parola, lp + 1);
        if (riga[0] && T_Larghezza(prova) > w) {
            if (fn) fn(riga, n, ctx);
            n++;
            snprintf(riga, sizeof(riga), "%s", parola);
        } else {
            snprintf(riga, sizeof(riga), "%s", prova);
        }
        if (*p == '\n' || *p == 0) {
            if (fn) fn(riga, n, ctx);
            n++;
            riga[0] = 0;
            if (*p == 0) break;
        }
        p++;
    }
    return n;
}

typedef struct { int x, y, interlinea, max; Color c; bool ombra; } CtxACapo;

static void DisegnaRiga(const char *riga, int i, void *ctx) {
    CtxACapo *k = (CtxACapo *)ctx;
    if (k->max && i >= k->max) return;
    if (k->ombra) T_TestoOmbra(riga, k->x, k->y + i * k->interlinea, k->c);
    else T_Testo(riga, k->x, k->y + i * k->interlinea, k->c);
}

int T_ACapo(const char *s, int x, int y, int w, int interlinea, int maxRighe, Color c, bool ombra) {
    CtxACapo k = { x, y, interlinea, maxRighe, c, ombra };
    int n = Spezza(s, w, DisegnaRiga, &k);
    return maxRighe && n > maxRighe ? maxRighe : n;
}

int T_ContaRighe(const char *s, int w) { return Spezza(s, w, NULL, NULL); }

/* ------------------------------------------------------------------ caricamento */

static Texture2D Carica(const char *nome) {
    char p[700];
    snprintf(p, sizeof(p), "%s%s", cartella, nome);
    Texture2D t = LoadTexture(p);
    if (t.id == 0) TraceLog(LOG_ERROR, "SLICE: asset mancante %s", p);
    SetTextureFilter(t, TEXTURE_FILTER_POINT);
    return t;
}

bool Gfx_Carica(const char *cartellaAsset) {
    snprintf(cartella, sizeof(cartella), "%s", cartellaAsset);
    static const char *file[] = { "ui.png", "elyon.png", "tiles.png", "oggetti.png", "personaggi.png", "lotta.png",
                                  "lotta_prato.png", "titolo.png", "logo.png", "mappa_asterra.png" };
    for (int i = 0; i <= TX_MAPPA; i++) tex[i] = Carica(file[i]);
    tex[TX_FONT] = CostruisciFont(FONT_CORPO, FONT_CORPO_N, FONT_CORPO_ALTEZZA, corpo, corpoAscii);
    tex[TX_FONT_MICRO] = CostruisciFont(FONT_MICRO, FONT_MICRO_N, FONT_MICRO_ALTEZZA, micro, microAscii);
    /* alone radiale per i bagliori additivi (come ui.bagliore: (1-d)^2) */
    Image b = GenImageColor(64, 64, BLANK);
    for (int y = 0; y < 64; y++)
        for (int x = 0; x < 64; x++) {
            float d = sqrtf((x - 31.5f) * (x - 31.5f) + (y - 31.5f) * (y - 31.5f)) / 32.0f;
            if (d < 1) ImageDrawPixel(&b, x, y, (Color){ 255, 255, 255, (unsigned char)(255 * (1 - d) * (1 - d)) });
        }
    tex[TX_BAGLIORE] = LoadTextureFromImage(b);
    UnloadImage(b);
    /* sagome degli Elyon: stesso atlante con i pixel opachi bianchi */
    char p[700];
    snprintf(p, sizeof(p), "%selyon.png", cartella);
    Image s = LoadImage(p);
    if (s.data) {
        ImageFormat(&s, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        Color *c = (Color *)s.data;
        for (int i = 0; i < s.width * s.height; i++) c[i] = c[i].a > 200 ? WHITE : BLANK;
        tex[TX_ELYON_SAGOMA] = LoadTextureFromImage(s);
        SetTextureFilter(tex[TX_ELYON_SAGOMA], TEXTURE_FILTER_POINT);
        UnloadImage(s);
    }
    for (int i = 0; i < TX_COUNT; i++) if (tex[i].id == 0) return false;
    return true;
}

void Gfx_Libera(void) {
    for (int i = 0; i < TX_COUNT; i++) if (tex[i].id) UnloadTexture(tex[i]);
}

Texture2D Gfx_Tex(TexSlice t) { return tex[t]; }
const char *Gfx_CartellaSlice(void) { return cartella; }

/* ------------------------------------------------------------------ primitive */

void G_Rett(int x, int y, int w, int h, Color c) { if (w > 0 && h > 0) DrawRectangle(x, y, w, h, c); }
void G_LineaH(int x, int y, int w, Color c) { G_Rett(x, y, w, 1, c); }
void G_LineaV(int x, int y, int h, Color c) { G_Rett(x, y, 1, h, c); }

static Color Mix(Color a, Color b, float t) {
    return (Color){ (unsigned char)(a.r + (b.r - a.r) * t), (unsigned char)(a.g + (b.g - a.g) * t),
                    (unsigned char)(a.b + (b.b - a.b) * t), (unsigned char)(a.a + (b.a - a.a) * t) };
}

void G_Gradiente(int x, int y, int w, int h, Color c1, Color c2, bool dither) {
    (void)dither;   /* righe piene: la differenza con il dithering 2x2 dei mockup e' sotto il passo di colore */
    for (int j = 0; j < h; j++) G_LineaH(x, y + j, w, Mix(c1, c2, h > 1 ? (float)j / (float)(h - 1) : 0));
}

void G_Oscura(int x, int y, int w, int h, unsigned char alfa) {
    Color c = PAL_NOTTE[0];
    c.a = alfa;
    G_Rett(x, y, w, h, c);
}

static Color Alfa(Color c, unsigned char a) { c.a = a; return c; }

static void BordoOro(int x, int y, int w, int h) {
    G_LineaH(x, y, w, PAL_NOTTE[0]); G_LineaH(x, y + h - 1, w, PAL_NOTTE[0]);
    G_LineaV(x, y, h, PAL_NOTTE[0]); G_LineaV(x + w - 1, y, h, PAL_NOTTE[0]);
    G_LineaH(x + 1, y + 1, w - 2, PAL_ORO[3]); G_LineaV(x + 1, y + 1, h - 2, PAL_ORO[3]);
    G_LineaH(x + 1, y + h - 2, w - 2, PAL_ORO[2]); G_LineaV(x + w - 2, y + 1, h - 2, PAL_ORO[2]);
}

static void Angolo(int cx, int cy) {
    for (int dy = -2; dy <= 2; dy++)
        for (int dx = -2; dx <= 2; dx++) {
            int d = abs(dx) + abs(dy);
            Color c;
            if (d == 2) c = (dx + dy < 0) ? PAL_ORO[4] : PAL_ORO[1];
            else if (d < 2) c = (dx + dy < 0) ? PAL_LUMEN[4] : (d == 0 ? PAL_LUMEN[3] : PAL_LUMEN[2]);
            else continue;
            DrawPixel(cx + dx, cy + dy, c);
        }
}

void G_Cornice(int x, int y, int w, int h, StileCornice stile, bool gemme) {
    if (stile == CORNICE_PERGAMENA) {
        G_Gradiente(x + 2, y + 2, w - 4, h - 4, PAL_PERGAMENA[4], PAL_PERGAMENA[3], true);
        unsigned seme = (unsigned)(w * 31 + h);
        for (int i = 0; i < w * h / 40; i++) {
            seme = seme * 1103515245u + 12345u;
            int px = (int)((seme >> 8) % (unsigned)(w - 4 > 0 ? w - 4 : 1));
            seme = seme * 1103515245u + 12345u;
            int py = (int)((seme >> 8) % (unsigned)(h - 4 > 0 ? h - 4 : 1));
            DrawPixel(x + 2 + px, y + 2 + py, Alfa(PAL_PERGAMENA[1], 50));
        }
    } else {
        G_Gradiente(x + 2, y + 2, w - 4, h - 4, Alfa(PAL_NOTTE[2], 240), Alfa(PAL_NOTTE[1], 240), true);
        G_LineaH(x + 2, y + 2, w - 4, PAL_NOTTE[3]);
    }
    BordoOro(x, y, w, h);
    if (gemme && w >= 24 && h >= 18) {
        Angolo(x + 1, y + 1); Angolo(x + w - 2, y + 1); Angolo(x + 1, y + h - 2); Angolo(x + w - 2, y + h - 2);
    } else if (gemme && w >= 24) {
        Angolo(x + w / 2, y + 1);
    }
}

int G_Intestazione(int x, int y, const char *titolo) {
    int larg = T_LarghezzaGrassetto(titolo) + 20;
    G_Rett(x, y, larg, 13, PAL_NOTTE[0]);
    G_Gradiente(x + 1, y + 1, larg - 2, 11, PAL_NOTTE[3], PAL_NOTTE[1], true);
    G_LineaH(x + 1, y + 12, larg - 2, PAL_ORO[2]);
    G_Cristallo(x + 6, y + 3, 7, true);
    T_TitoloOro(titolo, x + 12, y + 2);
    return larg;
}

void G_RigaSelezionata(int x, int y, int w, int h) {
    for (int i = 0; i < w; i++) {
        int a = (int)(150.0f * (1.0f - (float)i / (float)w) + 30.0f);
        G_LineaV(x + i, y, h, Alfa(PAL_AZZURRO[1], (unsigned char)a));
    }
    G_LineaH(x, y, w, Alfa(PAL_ORO[3], 110));
    G_LineaH(x, y + h - 1, w, Alfa(PAL_ORO[2], 80));
}

void G_Icona(Rectangle s, int x, int y) {
    DrawTextureRec(tex[TX_UI], s, (Vector2){ (float)x, (float)y }, WHITE);
}

void G_Cursore(int x, int y, int frame) {
    G_Icona((frame / 20) % 2 ? UI_CURSORE_1 : UI_CURSORE_0, x, y - 3);
}

void G_Cristallo(int x, int y, int h, bool acceso) {
    Rectangle r;
    switch (h) {
    case 7:  r = acceso ? UI_CRISTALLO_7 : UI_CRISTALLO_SPENTO_7; break;
    case 11: r = acceso ? UI_CRISTALLO_11 : UI_CRISTALLO_SPENTO_11; break;
    case 13: r = acceso ? UI_CRISTALLO_13 : UI_CRISTALLO_SPENTO_13; break;
    default: r = acceso ? UI_CRISTALLO_17 : UI_CRISTALLO_SPENTO_17; break;
    }
    G_Icona(r, x - (int)r.width / 2, y);
}

void G_Gemma(int cx, int cy) { G_Icona(UI_GEMMA, cx - 2, cy - 2); }

void G_Separatore(int x, int y, int w) {
    G_LineaH(x, y, w, PAL_ORO[1]);
    G_LineaH(x, y + 1, w, PAL_NOTTE[0]);
    G_Cristallo(x + w / 2, y - 3, 7, true);
}

void G_Bagliore(int cx, int cy, int r, Color c, int forza) {
    if (r <= 0) return;
    c.a = (unsigned char)(forza > 255 ? 255 : forza);
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(tex[TX_BAGLIORE], (Rectangle){ 0, 0, 64, 64 },
                   (Rectangle){ (float)(cx - r), (float)(cy - r), (float)(2 * r), (float)(2 * r) }, (Vector2){ 0, 0 }, 0, c);
    EndBlendMode();
}

const Color *G_RampaHp(float f) { return f > 0.5f ? PAL_HP_ALTO : f > 0.2f ? PAL_HP_MEDIO : PAL_HP_BASSO; }

int G_Barra(int x, int y, int w, float frazione, const Color *rampa, const char *etichetta, int h) {
    int lx = x;
    if (etichetta) {
        int ew = T_LarghezzaMicro(etichetta) + 4;
        G_Rett(x, y - 1, ew, h + 2, PAL_NOTTE[0]);
        T_Micro(etichetta, x + 2, y - 1 + (h + 2 - 5) / 2, PAL_ORO[3]);
        lx = x + ew;
    }
    G_Rett(lx, y - 1, w + 2, h + 2, PAL_NOTTE[0]);
    G_Rett(lx + 1, y, w, h, PAL_NOTTE[1]);
    G_LineaH(lx + 1, y + h - 1, w, PAL_NOTTE[2]);
    if (frazione < 0) frazione = 0;
    if (frazione > 1) frazione = 1;
    int pieno = (int)lroundf((float)w * frazione);
    if (frazione > 0 && pieno == 0) pieno = 1;
    if (pieno) {
        G_Rett(lx + 1, y, pieno, h, rampa[1]);
        G_LineaH(lx + 1, y, pieno, rampa[2]);
        G_LineaH(lx + 1, y + h - 1, pieno, rampa[0]);
        DrawPixel(lx + pieno, y, (Color){ 255, 255, 255, 200 });
    }
    for (int t = 1; t < 10; t++) {
        int tx = lx + 1 + w * t / 10;
        if (tx < lx + 1 + pieno) DrawPixel(tx, y + h - 1, rampa[0]);
    }
    return lx + w + 2;
}

static const Rectangle *BadgeDi(ElementType t) {
    static Rectangle r[13];
    static bool pronto;
    if (!pronto) {
        Rectangle v[13] = { UI_BADGE_NEUTRO, UI_BADGE_FIAMMA, UI_BADGE_ACQUA, UI_BADGE_NATURA, UI_BADGE_IMPULSO,
                            UI_BADGE_VENTO, UI_BADGE_ROCCIA, UI_BADGE_GELO, UI_BADGE_OMBRA, UI_BADGE_ASTRALE,
                            UI_BADGE_TOSSICO, UI_BADGE_PSICHE, UI_BADGE_METALLO };
        memcpy(r, v, sizeof(r));
        pronto = true;
    }
    return &r[(t >= 0 && t < 13) ? t : 0];
}

void G_Badge(ElementType t, int x, int y) { G_Icona(*BadgeDi(t), x, y); }

void G_PillolaTipo(ElementType t, int x, int y, int larg) {
    if (t < 0 || t >= 13) t = TYPE_NONE;
    Color sc = PAL_TIPI[t][0], base = PAL_TIPI[t][1], chiaro = PAL_TIPI[t][2];
    G_Rett(x + 5, y + 1, larg - 5, 9, PAL_NOTTE[0]);
    G_Rett(x + 6, y + 2, larg - 7, 7, base);
    G_LineaH(x + 6, y + 2, larg - 7, Mix(base, chiaro, 0.5f));
    G_LineaH(x + 6, y + 8, larg - 7, sc);
    float lum = 0.3f * base.r + 0.59f * base.g + 0.11f * base.b;
    if (lum > 150) T_Micro(NOMI_TIPI_GEN[t], x + 13, y + 3, sc);
    else T_MicroOmbra(NOMI_TIPI_GEN[t], x + 13, y + 3, Mix(chiaro, WHITE, 0.7f), sc);
    G_Badge(t, x, y);
}

int G_PillolaStato(StatusId s, int x, int y) {
    const char *sigla;
    Color c;
    switch (s) {
    case STATUS_VELENO:       sigla = "VEL";  c = PAL_TIPI[TYPE_TOSSICO][1]; break;
    case STATUS_VELENO_GRAVE: sigla = "VEL+"; c = PAL_TIPI[TYPE_TOSSICO][0]; break;
    case STATUS_BRUCIATURA:   sigla = "BRU";  c = PAL_TIPI[TYPE_FIAMMA][1]; break;
    case STATUS_PARALISI:     sigla = "PAR";  c = PAL_TIPI[TYPE_IMPULSO][1]; break;
    case STATUS_SONNO:        sigla = "SON";  c = (Color){ 124, 132, 168, 255 }; break;
    case STATUS_GELO:         sigla = "GEL";  c = PAL_TIPI[TYPE_GELO][1]; break;
    case STATUS_CONFUSIONE:   sigla = "CNF";  c = PAL_TIPI[TYPE_PSICHE][1]; break;
    default: return 0;
    }
    int w = T_LarghezzaMicro(sigla) + 6;
    G_Rett(x, y, w, 9, PAL_NOTTE[0]);
    G_Rett(x + 1, y + 1, w - 2, 7, c);
    G_LineaH(x + 1, y + 1, w - 2, Mix(c, WHITE, 0.35f));
    T_Micro(sigla, x + 3, y + 2, WHITE);
    return w;
}

void G_IconaOggetto(int id, int x, int y) {
    Rectangle r;
    switch (id) {
    case ITEM_ERBA_VITALE: r = UI_OGG_ERBA_VITALE; break;
    case ITEM_FRUTTO_LUMEN: r = UI_OGG_FRUTTO_LUMEN; break;
    case ITEM_ELISIR: r = UI_OGG_ELISIR; break;
    case ITEM_ELISIR_MAX: r = UI_OGG_ELISIR_MAX; break;
    case ITEM_ANTIDOTO: r = UI_OGG_ANTIDOTO; break;
    case ITEM_STABILIZZATORE: r = UI_OGG_STABILIZZATORE; break;
    case ITEM_RISVEGLIO: r = UI_OGG_RISVEGLIO; break;
    case ITEM_FIAMMIFUGA: r = UI_OGG_FIAMMIFUGA; break;
    case ITEM_GELORISCALDO: r = UI_OGG_GELORISCALDO; break;
    case ITEM_REVITA: r = UI_OGG_REVITA; break;
    case ITEM_REVITA_MAX: r = UI_OGG_REVITA_MAX; break;
    case ITEM_CORE: r = UI_OGG_CORE; break;
    case ITEM_CORE_PLUS: r = UI_OGG_CORE_PLUS; break;
    case ITEM_CORE_MAX: r = UI_OGG_CORE_MAX; break;
    case ITEM_CORE_VOID: r = UI_OGG_CORE_VOID; break;
    case ITEM_PIETRA_FIAMMA: r = UI_OGG_PIETRA_FIAMMA; break;
    case ITEM_PIETRA_ACQUA: r = UI_OGG_PIETRA_ACQUA; break;
    case ITEM_PIETRA_NATURA: r = UI_OGG_PIETRA_NATURA; break;
    default: r = UI_OGG_PIETRA_LUMEN; break;
    }
    G_Icona(r, x, y);
}

void G_GlifoTasto(int x, int y, const char *lettera, bool attivo) {
    /* mockups_v2.glifo_tasto: cerchio scuro con anello d'oro; per le sigle lunghe una pillola */
    int lw = T_LarghezzaMicro(lettera);
    if (lw <= 3) {
        for (int dy = -4; dy <= 4; dy++)
            for (int dx = -4; dx <= 4; dx++) {
                float d = sqrtf((float)(dx * dx + dy * dy));
                if (d <= 4.5f) DrawPixel(x + 4 + dx, y + 4 + dy, d > 3.5f ? PAL_ORO[3] : (attivo ? PAL_NOTTE[3] : PAL_NOTTE[1]));
            }
        T_Micro(lettera, x + 3, y + 2, PAL_ORO[5]);
        return;
    }
    int w = lw + 6;
    G_Rett(x + 1, y, w - 2, 9, PAL_ORO[3]);
    G_Rett(x, y + 1, w, 7, PAL_ORO[3]);
    G_Rett(x + 1, y + 1, w - 2, 7, attivo ? PAL_NOTTE[3] : PAL_NOTTE[1]);
    T_Micro(lettera, x + 3, y + 2, PAL_ORO[5]);
}

/* ------------------------------------------------------------------ Elyon */

static bool RettElyon(int specie, VistaElyon v, Rectangle *r, int *frames) {
    const Rectangle R[9][4] = {
        { ELYON_PYRIN_FRONTE, ELYON_PYRIN_NEMICO, ELYON_PYRIN_MIO, ELYON_PYRIN_ICONA },
        { ELYON_PYRION_FRONTE, ELYON_PYRION_NEMICO, ELYON_PYRION_MIO, ELYON_PYRION_ICONA },
        { ELYON_PYROVAR_FRONTE, ELYON_PYROVAR_NEMICO, ELYON_PYROVAR_MIO, ELYON_PYROVAR_ICONA },
        { ELYON_AQUIL_FRONTE, ELYON_AQUIL_NEMICO, ELYON_AQUIL_MIO, ELYON_AQUIL_ICONA },
        { ELYON_AQUILON_FRONTE, ELYON_AQUILON_NEMICO, ELYON_AQUILON_MIO, ELYON_AQUILON_ICONA },
        { ELYON_AQUARYS_FRONTE, ELYON_AQUARYS_NEMICO, ELYON_AQUARYS_MIO, ELYON_AQUARYS_ICONA },
        { ELYON_MOSSEN_FRONTE, ELYON_MOSSEN_NEMICO, ELYON_MOSSEN_MIO, ELYON_MOSSEN_ICONA },
        { ELYON_MOSSKA_FRONTE, ELYON_MOSSKA_NEMICO, ELYON_MOSSKA_MIO, ELYON_MOSSKA_ICONA },
        { ELYON_VERDANTOR_FRONTE, ELYON_VERDANTOR_NEMICO, ELYON_VERDANTOR_MIO, ELYON_VERDANTOR_ICONA },
    };
    if (specie < 1 || specie > 9) return false;
    *r = R[specie - 1][v];
    *frames = v == VISTA_ICONA ? 2 : 5;
    return true;
}

bool G_HaSprite(int specie) { return specie >= 1 && specie <= 9; }

Vector2 G_DimElyon(VistaElyon v) {
    switch (v) {
    case VISTA_NEMICO: return (Vector2){ 70, 70 };
    case VISTA_MIO:    return (Vector2){ 80, 80 };
    case VISTA_ICONA:  return (Vector2){ 32, 32 };
    default:           return (Vector2){ 64, 64 };
    }
}

static void ElyonDa(TexSlice t, int specie, VistaElyon v, int f, int x, int y, Color tinta) {
    Rectangle r;
    int n;
    if (!RettElyon(specie, v, &r, &n)) return;
    float fw = r.width / (float)n;
    if (f < 0 || f >= n) f = 0;
    Rectangle src = { r.x + fw * (float)f, r.y, fw, r.height };
    DrawTextureRec(tex[t], src, (Vector2){ (float)x, (float)y }, tinta);
}

void G_Elyon(int specie, VistaElyon v, int f, int x, int y, Color tinta) { ElyonDa(TX_ELYON, specie, v, f, x, y, tinta); }
void G_ElyonSagoma(int specie, VistaElyon v, int x, int y, Color c) { ElyonDa(TX_ELYON_SAGOMA, specie, v, 0, x, y, c); }
