/* legacy_shim.c - Il "livello di rendering" del gioco sostituito per la vertical slice.
 *
 * La slice compila SENZA modifiche la logica del gioco (world.c, player.c, npc.c, battle_ui.c,
 * ui_common.c, input.c): qui si forniscono le funzioni grafiche che quei file si aspettano
 * (graphics.h, dialogue_ui.h, map_render.h). Le funzioni di disegno sono vuote, perche' la slice
 * disegna tutto con le proprie viste; restano vere solo le misure del testo e l'impaginazione dei
 * dialoghi, che ora usano il font e il riquadro del Design System v2.
 * E' la prova pratica che logica e rendering sono separabili (MULTIPLATFORM_ARCHITECTURE §1). */
#include "graphics.h"
#include "dialogue_ui.h"
#include "map_render.h"
#include "gfx.h"
#include "vista_dialogo.h"
#include <string.h>

static char cartellaGioco[512] = "assets";

void Shim_ImpostaCartellaGioco(const char *c) { strncpy(cartellaGioco, c, sizeof(cartellaGioco) - 1); }

const char *Gfx_CartellaAsset(void) { return cartellaGioco; }
bool Gfx_Init(const char *t) { (void)t; return true; }
void Gfx_Shutdown(void) {}
void Gfx_InizioFrame(void) {}
void Gfx_FineFrame(void) {}
int  Gfx_Scala(void) { return 1; }
void Gfx_SchermoIntero(bool a) { (void)a; }
bool Gfx_IsSchermoIntero(void) { return false; }
Texture2D Gfx_Texture(TexId id) { (void)id; return (Texture2D){ 0 }; }
void Gfx_Testo(const char *t, int x, int y, Color c) { (void)t; (void)x; (void)y; (void)c; }
void Gfx_TestoCentrato(const char *t, int cx, int y, int d, Color c) { (void)t; (void)cx; (void)y; (void)d; (void)c; }
int  Gfx_LarghezzaTesto(const char *t, int d) { (void)d; return T_Larghezza(t); }
void Gfx_Finestra(int x, int y, int w, int h) { (void)x; (void)y; (void)w; (void)h; }
void Gfx_Cursore(int x, int y) { (void)x; (void)y; }
void Gfx_EsportaFrame(const char *f) { (void)f; }
void Gfx_Dissolvenza(float q) { (void)q; }

void MapRender_Terreno(const Map *m, int camX, int camY, int frame) { (void)m; (void)camX; (void)camY; (void)frame; }
void MapRender_RigheVisibili(int camY, int *prima, int *ultima) { (void)camY; *prima = 0; *ultima = -1; }
void MapRender_OggettiRiga(const Map *m, int y, int camX, int camY) { (void)m; (void)y; (void)camX; (void)camY; }

void DialogoUI_Disegna(const Dialogo *d, int frame) { (void)d; (void)frame; }

/* Sostituisce con '\n' gli spazi dove serve andare a capo, con il font Eclyra Corpo */
static void AvvolgiRighe(char *s, int larghezza) {
    char *inizioRiga = s, *ultimoSpazio = NULL;
    for (char *c = s;; c++) {
        if (*c == ' ' || *c == '\n' || *c == '\0') {
            char salvato = *c;
            *c = '\0';
            bool troppo = T_Larghezza(inizioRiga) > larghezza;
            *c = salvato;
            if (troppo && ultimoSpazio) {
                *ultimoSpazio = '\n';
                inizioRiga = ultimoSpazio + 1;
                ultimoSpazio = NULL;
                c = inizioRiga - 1;
                continue;
            }
            if (salvato == '\0') break;
            if (salvato == '\n') { inizioRiga = c + 1; ultimoSpazio = NULL; }
            else ultimoSpazio = c;
        }
    }
}

void DialogoUI_Impagina(Dialogo *d) {
    char nuove[DIALOGO_MAX_PAGINE][DIALOGO_PAGINA];
    int n = 0;
    for (int p = 0; p < d->numPagine && n < DIALOGO_MAX_PAGINE; p++) {
        char buf[DIALOGO_PAGINA];
        memcpy(buf, d->pagine[p], DIALOGO_PAGINA);
        AvvolgiRighe(buf, DIALOGO_LARGHEZZA_TESTO);
        char *inizio = buf;
        int righe = 0;
        for (char *c = buf;; c++) {
            bool fine = *c == '\0';
            if (*c == '\n' || fine) {
                righe++;
                if (righe == DIALOGO_RIGHE || fine) {
                    int len = (int)(c - inizio);
                    if (n < DIALOGO_MAX_PAGINE) { memcpy(nuove[n], inizio, (size_t)len); nuove[n][len] = '\0'; n++; }
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
