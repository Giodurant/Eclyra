/* game.h - Stato globale, scene e game loop a passo fisso. */
#ifndef ECLYRA_GAME_H
#define ECLYRA_GAME_H

#include "input.h"
#include "menu.h"
#include "utils.h"
#include "dialogue.h"
#include "partita.h"
#include "world.h"
#include <stdbool.h>

#define ECLYRA_VERSIONE "0.3.0-fase3"
#define PASSO_LOGICO (1.0f / 60.0f)

typedef enum { SCENA_TITOLO, SCENA_OPZIONI, SCENA_MONDO } Scena;
typedef enum { TESTO_LENTO, TESTO_MEDIO, TESTO_VELOCE } VelocitaTesto;

typedef struct {
    VelocitaTesto velocitaTesto;
    bool schermoIntero;
} Opzioni;

typedef struct {
    Scena scena;
    Scena scenaPrimaOpzioni;   /* per tornare indietro dalle opzioni */
    Input input;
    Opzioni opzioni;
    Rng rng;
    Partita partita;
    DialogoDb dialoghi;
    World world;
    bool zainoAperto;
    int debugMappa;            /* F7: indice della prossima mappa di teletrasporto */

    Menu menuTitolo;
    Menu menuOpzioni;
    Menu menuGioco;
    bool menuGiocoAperto;
    const char *avviso;        /* finestra "non ancora disponibile" */

    float accumulatore;
    int frameTitolo;
    float dissolvenza;         /* 1 = nero, scende a 0 */
    bool esci;
} Game;

bool Game_Init(Game *g);
void Game_Frame(Game *g);      /* input + logica a passo fisso + disegno */
void Game_Shutdown(Game *g);

/* Per i test automatici: esegue un passo logico e disegna */
void Game_Passo(Game *g);
void Game_Disegna(Game *g);

#endif
