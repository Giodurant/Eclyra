/* dialogue_ui.c */
#include "dialogue_ui.h"
#include "graphics.h"
#include <string.h>

#define BOX_X 4
#define BOX_W (LOGICAL_W - 8)
#define BOX_H 50
#define BOX_Y (LOGICAL_H - BOX_H - 4)
#define TESTO_W (BOX_W - 22)

/* Sostituisce con '\n' gli spazi dove serve andare a capo (stesso numero di byte) */
static int AvvolgiRighe(char *s) {
    int righe = 1;
    char *inizioRiga = s;
    char *ultimoSpazio = NULL;
    for (char *c = s; ; c++) {
        if (*c == ' ' || *c == '\n' || *c == '\0') {
            char salvato = *c;
            *c = '\0';
            bool troppoLunga = Gfx_LarghezzaTesto(inizioRiga, 10) > TESTO_W;
            *c = salvato;
            if (troppoLunga && ultimoSpazio) {
                *ultimoSpazio = '\n';
                inizioRiga = ultimoSpazio + 1;
                righe++;
                ultimoSpazio = NULL;
                c = inizioRiga - 1;   /* ricontrolla dalla nuova riga */
                continue;
            }
            if (salvato == '\0') break;
            if (salvato == '\n') { inizioRiga = c + 1; righe++; ultimoSpazio = NULL; }
            else ultimoSpazio = c;
        }
    }
    return righe;
}

void DialogoUI_Impagina(Dialogo *d) {
    char nuove[DIALOGO_MAX_PAGINE][DIALOGO_PAGINA];
    int n = 0;
    for (int p = 0; p < d->numPagine && n < DIALOGO_MAX_PAGINE; p++) {
        char buf[DIALOGO_PAGINA];
        memcpy(buf, d->pagine[p], DIALOGO_PAGINA);
        AvvolgiRighe(buf);
        /* spezza ogni DIALOGO_RIGHE_VISIBILI righe */
        char *inizio = buf;
        int righe = 0;
        for (char *c = buf; ; c++) {
            bool fine = (*c == '\0');
            if (*c == '\n' || fine) {
                righe++;
                if (righe == DIALOGO_RIGHE_VISIBILI || fine) {
                    int len = (int)(c - inizio);
                    if (n < DIALOGO_MAX_PAGINE) {
                        memcpy(nuove[n], inizio, (size_t)len);
                        nuove[n][len] = '\0';
                        n++;
                    }
                    inizio = c + 1;
                    righe = 0;
                }
                if (fine) break;
            }
        }
    }
    memcpy(d->pagine, nuove, sizeof(nuove));
    d->numPagine = n > 0 ? n : 1;
    d->pagina = 0;
    d->caratteri = 0;
    d->lunghezzaPagina = (int)strlen(d->pagine[0]);
}

void DialogoUI_Disegna(const Dialogo *d, int frame) {
    if (!d->attivo) return;
    if (d->parlante[0]) {
        int w = Gfx_LarghezzaTesto(d->parlante, 10) + 16;
        Gfx_Finestra(BOX_X + 4, BOX_Y - 15, w, 18);
        Gfx_Testo(d->parlante, BOX_X + 12, BOX_Y - 10, (Color){ 40, 88, 152, 255 });
    }
    Gfx_Finestra(BOX_X, BOX_Y, BOX_W, BOX_H);
    char vis[DIALOGO_PAGINA];
    Dialogo_TestoVisibile(d, vis, sizeof(vis));
    Gfx_Testo(vis, BOX_X + 11, BOX_Y + 8, COL_TESTO);
    if (Dialogo_PaginaCompleta(d) && (frame / 16) % 2 == 0) {
        int x = BOX_X + BOX_W - 14, y = BOX_Y + BOX_H - 11;
        DrawTriangle((Vector2){ (float)x, (float)y }, (Vector2){ (float)x + 3, (float)y + 4 },
                     (Vector2){ (float)x + 6, (float)y }, (Color){ 224, 72, 64, 255 });
    }
}
