/* main.c - ECLYRA: Le Cronache dell'Equilibrio
 *
 * Uso:  eclyra                    avvia il gioco
 *       eclyra --screenshot-test  (solo build di debug) percorre title e mondo
 *                                 con input simulato e salva screenshot PNG
 */
#include "game.h"
#include "graphics.h"
#include "items.h"
#include <string.h>

#ifdef ECLYRA_DEBUG
static void Scatta(const char *nome) {
    Gfx_EsportaFrame(nome);   /* la render texture logica 320x180 */
}

static void Attendi(Game *g, int passi) {
    for (int i = 0; i < passi; i++) Game_Passo(g);
}

static void PremiConferma(Game *g) { g->input.conferma = true; Game_Passo(g); }

/* Tiene premuta una direzione per un certo numero di passi logici */
static void Tieni(Game *g, Direzione d, int passi) {
    for (int i = 0; i < passi; i++) {
        g->input.su = d == DIR_SU; g->input.giu = d == DIR_GIU;
        g->input.sinistra = d == DIR_SINISTRA; g->input.destra = d == DIR_DESTRA;
        Game_Passo(g);
    }
    g->input.su = g->input.giu = g->input.sinistra = g->input.destra = false;
}

static void Foto(Game *g, const char *nome) {
    Game_Disegna(g);
    Scatta(nome);
}

static int TestScreenshot(Game *g) {
    SetTargetFPS(0);
    g->opzioni.velocitaTesto = TESTO_VELOCE;
    /* 1) titolo */
    Attendi(g, 90);
    Foto(g, "test_01_titolo.png");
    /* 2) nuova partita: si comincia in casa */
    PremiConferma(g);
    Attendi(g, 40);
    Foto(g, "test_02_casa.png");
    /* 3) parlo con la mamma */
    World_Teletrasporta(&g->world, "casa_giocatore", 7, 5, DIR_DESTRA);
    PremiConferma(g);
    Attendi(g, 60);
    Foto(g, "test_03_dialogo_mamma.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    /* 4) esco dalla porta: il tappetino porta a Lunaria */
    World_Teletrasporta(&g->world, "casa_giocatore", 5, 6, DIR_GIU);
    Tieni(g, DIR_GIU, 20);
    Attendi(g, 60);
    Foto(g, "test_04_lunaria.png");
    TraceLog(LOG_INFO, "ECLYRA TEST: dopo l'uscita sono in %s (%d,%d)", g->world.mappa.id, g->world.player.tx, g->world.player.ty);
    /* 5) entro nel laboratorio urtando la porta e parlo con Elian */
    World_Teletrasporta(&g->world, "lunaria", 31, 13, DIR_SU);
    Tieni(g, DIR_SU, 4);
    Attendi(g, 60);
    TraceLog(LOG_INFO, "ECLYRA TEST: dopo la porta sono in %s (%d,%d)", g->world.mappa.id, g->world.player.tx, g->world.player.ty);
    World_Teletrasporta(&g->world, "laboratorio", 7, 4, DIR_SINISTRA);
    PremiConferma(g);
    Attendi(g, 80);
    Foto(g, "test_05_elian.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    /* 6) Route 1: raccolgo il CORE vicino al lago */
    World_Teletrasporta(&g->world, "route1", 44, 14, DIR_DESTRA);
    Attendi(g, 10);
    PremiConferma(g);
    Attendi(g, 40);
    Foto(g, "test_06_core.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    TraceLog(LOG_INFO, "ECLYRA TEST: CORE nello zaino = %d", Inventario_Quantita(&g->partita.inventario, ITEM_CORE));
    /* 7) Kael davanti al laboratorio e menu ZAINO */
    World_Teletrasporta(&g->world, "lunaria", 30, 15, DIR_SU);
    PremiConferma(g);
    Attendi(g, 80);
    Foto(g, "test_07_kael.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    g->input.menu = true; Game_Passo(g);
    g->input.giuP = true; Game_Passo(g);
    PremiConferma(g);
    Foto(g, "test_08_zaino.png");
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
