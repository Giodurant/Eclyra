/* vista_titolo.c - Titolo (mockup 01_titolo) e scelta dello starter della slice.
 * La scelta dello starter usa solo componenti approvati (cornici, pillole, cursore, sprite). */
#include "slice.h"
#include "gfx.h"
#include <math.h>
#include <string.h>

static const char *VOCI[4] = { "NUOVA PARTITA", "CONTINUA", "OPZIONI", "ESCI" };

void VistaTitolo_Disegna(const Slice *s) {
    const Canvas *c = &s->canvas;
    ClearBackground(PAL_NOTTE[0]);
    DrawTexture(Gfx_Tex(TX_TITOLO), (c->w - 400) / 2, (c->h - 240) / 2, WHITE);
    /* scintille di Lumen che salgono */
    for (int i = 0; i < 18; i++) {
        unsigned h = (unsigned)(i * 2654435761u);
        int x = (int)(h % (unsigned)c->w);
        int y = c->h - (int)((s->frame * (1 + i % 3) / 3 + (int)((h >> 8) % 300)) % (c->h + 20));
        DrawPixel(x, y, (i % 2) ? PAL_LUMEN[4] : PAL_LUMEN[3]);
    }
    int cx = c->w / 2 - 20;
    int oy = (c->h - 180) / 2;
    Texture2D logo = Gfx_Tex(TX_LOGO);
    DrawTexture(logo, cx - logo.width / 2, oy + 14, WHITE);
    const char *sott = "LE CRONACHE DELL'EQUILIBRIO";
    T_Testo(sott, cx - T_Larghezza(sott) / 2, oy + 65, PAL_AZZURRO[3]);
    T_Testo(sott, cx - T_Larghezza(sott) / 2, oy + 64, PAL_AZZURRO[3]);
    int mx = cx - 52, my = oy + 82;
    G_Cornice(mx, my, 104, 64, CORNICE_NOTTE, true);
    for (int i = 0; i < 4; i++) {
        int y = my + 8 + i * 13;
        bool sel = s->cursoreTitolo == i, attiva = i == 0 || i == 3;
        if (sel) { G_RigaSelezionata(mx + 3, y - 2, 98, 12); G_Cursore(mx + 6, y + 3, s->frame); }
        T_TestoOmbra(VOCI[i], mx + 16, y, sel ? PAL_ORO[4] : attiva ? PAL_CREMA : PAL_GRIGIO_UI);
    }
    T_Micro("VERTICAL SLICE " SLICE_VERSIONE, 4 + c->safeL, c->h - 8 - c->safeB, PAL_GRIGIO_UI);
    if ((s->frame / 30) % 2 == 0 && s->az.ultimo != DISP_TOUCH) {
        const char *p = TextFormat("PREMI %s", Azioni_Prompt(&s->az, AZ_CONFERMA));
        T_Micro(p, c->w - c->safeR - 6 - T_LarghezzaMicro(p), c->h - 8 - c->safeB, PAL_ORO[3]);
    }
}

void VistaStarter_Disegna(const Slice *s) {
    const Canvas *c = &s->canvas;
    G_Gradiente(0, 0, c->w, c->h, PAL_NOTTE[2], PAL_NOTTE[0], true);
    int oy = (c->h - 180) / 2, ox = (c->w - 320) / 2;
    G_Intestazione(ox + 6, oy + 6, "IL TUO PRIMO ELYON");
    static const int specie[3] = { 1, 4, 7 };
    static const ElementType tipo[3] = { TYPE_FIAMMA, TYPE_ACQUA, TYPE_NATURA };
    for (int i = 0; i < 3; i++) {
        int x = ox + 10 + i * 102, y = oy + 30;
        bool sel = s->cursoreStarter == i;
        G_Cornice(x, y, 96, 118, sel ? CORNICE_NOTTE : CORNICE_NOTTE, sel);
        if (sel) {
            G_LineaH(x + 1, y + 1, 94, PAL_ORO[4]);
            G_Bagliore(x + 48, y + 52, 34, PAL_TIPI[tipo[i]][2], 50);
        }
        /* pedana runica in miniatura */
        DrawEllipse(x + 48, y + 82, 30, 6, (Color){ PAL_TERRA[2].r, PAL_TERRA[2].g, PAL_TERRA[2].b, 255 });
        DrawEllipseLines(x + 48, y + 82, 30, 6, sel ? PAL_ORO[3] : PAL_PIETRA[2]);
        int f = sel ? ((s->frame / 30) % 2) : 0;
        G_Elyon(specie[i], VISTA_FRONTE, f, x + 16, y + 22, WHITE);
        const SpeciesData *sp = GetSpecies(specie[i]);
        T_Grassetto(sp->nome, x + (96 - T_LarghezzaGrassetto(sp->nome)) / 2, y + 92, sel ? PAL_ORO[4] : PAL_CREMA, PAL_OMBRA_TESTO);
        G_PillolaTipo(tipo[i], x + 22, y + 104, 54);
        if (sel) G_Cursore(x + 4, y + 96, s->frame);
    }
    const SpeciesData *sp = GetSpecies(specie[s->cursoreStarter]);
    G_Cornice(ox + 6, oy + 152, 308, 26, CORNICE_PERGAMENA, false);
    T_ACapo(sp->descrizione, ox + 14, oy + 156, 292, 10, 2, PAL_INCHIOSTRO, false);
}
