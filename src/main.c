/* main.c - ECLYRA: Le Cronache dell'Equilibrio
 *
 * Uso:  eclyra                    avvia il gioco
 *       eclyra --screenshot-test  (solo build di debug) percorre title e mondo
 *                                 con input simulato e salva screenshot PNG
 */
#include "game.h"
#include "graphics.h"
#include <string.h>

#ifdef ECLYRA_DEBUG
static void Scatta(const char *nome) {
    Gfx_EsportaFrame(nome);   /* la render texture logica 320x180 */
}

static int TestScreenshot(Game *g) {
    SetTargetFPS(0);
    /* 1) titolo */
    for (int i = 0; i < 90; i++) { Game_Passo(g); }
    Game_Disegna(g);
    Scatta("test_titolo.png");
    /* 2) nuova partita */
    g->input.conferma = true;
    Game_Passo(g);
    for (int i = 0; i < 40; i++) Game_Passo(g);
    Game_Disegna(g);
    Scatta("test_mondo.png");
    /* 3) cammino verso nord per 6 tile e verso destra per 6 tile */
    for (int i = 0; i < 6 * 16 + 8; i++) { g->input.su = true; Game_Passo(g); }
    g->input.su = false;
    for (int i = 0; i < 8 * 16 + 8; i++) { g->input.destra = true; Game_Passo(g); }
    g->input.destra = false;
    for (int i = 0; i < 160; i++) Game_Passo(g);
    Game_Disegna(g);
    Scatta("test_camminata.png");
    /* 4) menu di gioco */
    g->input.menu = true;
    Game_Passo(g);
    Game_Disegna(g);
    Scatta("test_menu.png");
    TraceLog(LOG_INFO, "ECLYRA: giocatore in tile (%d,%d)", g->world.player.tx, g->world.player.ty);
    return 0;
}
#endif

int main(int argc, char **argv) {
    static Game game;   /* static: la mappa e' grande, meglio non sullo stack */
    if (!Game_Init(&game)) return 1;

#ifdef ECLYRA_DEBUG
    if (argc > 1 && strcmp(argv[1], "--screenshot-test") == 0) {
        int r = TestScreenshot(&game);
        Game_Shutdown(&game);
        return r;
    }
#else
    (void)argc; (void)argv;
#endif

    SetTargetFPS(60);
    while (!game.esci) Game_Frame(&game);
    Game_Shutdown(&game);
    return 0;
}
