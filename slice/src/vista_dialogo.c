/* vista_dialogo.c - Vedi vista_dialogo.h (mockup 09_dialogo) */
#include "vista_dialogo.h"
#include "gfx.h"
#include <string.h>

#define BOX_W 312
#define BOX_H 48

static void Riquadro(const char *parlante, const char *testo, const Canvas *c, int frame, bool indicatore) {
    int x = (c->w - BOX_W) / 2;
    int y = c->h - c->safeB - BOX_H - 4;
    if (parlante && parlante[0]) {
        int w = T_LarghezzaGrassetto(parlante) + 22;
        G_Rett(x + 8, y - 11, w, 13, PAL_NOTTE[0]);
        G_Gradiente(x + 9, y - 10, w - 2, 11, PAL_NOTTE[3], PAL_NOTTE[1], true);
        G_LineaH(x + 9, y - 10, w - 2, PAL_ORO[3]);
        G_Cristallo(x + 15, y - 8, 7, true);
        T_TitoloOro(parlante, x + 21, y - 9);
    }
    G_Cornice(x, y, BOX_W, BOX_H, CORNICE_PERGAMENA, true);
    int righe = 0;
    const char *p = testo;
    while (*p && righe < DIALOGO_RIGHE) {
        const char *fine = strchr(p, '\n');
        char riga[160];
        size_t n = fine ? (size_t)(fine - p) : strlen(p);
        if (n > sizeof(riga) - 1) n = sizeof(riga) - 1;
        memcpy(riga, p, n);
        riga[n] = 0;
        T_Testo(riga, x + 14, y + 9 + righe * 11, PAL_INCHIOSTRO);
        righe++;
        if (!fine) break;
        p = fine + 1;
    }
    if (indicatore) {
        int dy = (frame / 16) % 2;
        G_Bagliore(x + BOX_W - 13, y + BOX_H - 10 + dy, 6, PAL_LUMEN[3], 70);
        G_Cristallo(x + BOX_W - 13, y + BOX_H - 15 + dy, 7, true);
    }
}

void VistaDialogo_Disegna(const Dialogo *d, const Canvas *c, int frame) {
    if (!d->attivo) return;
    char vis[DIALOGO_PAGINA];
    Dialogo_TestoVisibile(d, vis, sizeof(vis));
    Riquadro(d->parlante, vis, c, frame, Dialogo_PaginaCompleta(d));
}

void VistaDialogo_Testo(const char *parlante, const char *testo, const Canvas *c, int frame, bool indicatore) {
    Riquadro(parlante, testo, c, frame, indicatore);
}

void VistaDialogo_SiNo(int cursore, const Canvas *c, int frame) {
    int x = (c->w - BOX_W) / 2 + BOX_W - 58, y = c->h - c->safeB - BOX_H - 4 - 40;
    G_Cornice(x, y, 54, 36, CORNICE_NOTTE, true);
    const char *v[2] = { "SÌ", "NO" };
    for (int i = 0; i < 2; i++) {
        int yy = y + 7 + i * 13;
        if (cursore == i) { G_RigaSelezionata(x + 3, yy - 2, 48, 12); G_Cursore(x + 6, yy + 3, frame); }
        T_TestoOmbra(v[i], x + 18, yy, cursore == i ? PAL_ORO[4] : PAL_CREMA);
    }
}
