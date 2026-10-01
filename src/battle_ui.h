/* battle_ui.h - Schermata di lotta: riproduce gli eventi del motore, menu COMBATTI/ELYON/ZAINO/FUGA,
 * animazioni (entrata, attacco, colpo, KO, cattura, livello), apprendimento mosse ed evoluzioni. */
#ifndef ECLYRA_BATTLE_UI_H
#define ECLYRA_BATTLE_UI_H

#include "battle.h"
#include "input.h"
#include "partita.h"

#define BUI_MAX_CODA 256

typedef enum {
    BUI_TRANSIZIONE = 0, BUI_EVENTI, BUI_MENU, BUI_MOSSE, BUI_SQUADRA, BUI_ZAINO, BUI_ELISIR,
    BUI_AVVISO, BUI_IMPARA_SINO, BUI_IMPARA_SCELTA, BUI_EVOLUZIONE, BUI_FINE
} StatoBattleUI;

typedef enum { SQ_CAMBIO = 0, SQ_FORZATO, SQ_OGGETTO } ModoSquadra;

typedef struct {
    bool visibile;
    int indice, specie, livello;
    float hp;
    int hpTarget, hpMax;
    StatusId status;
    char nome[24];
    int anim, animTimer;
} LatoUI;

typedef struct {
    Lotta lotta;
    Partita *partita;
    StatoBattleUI stato, statoPrimaAvviso;

    EventoLotta coda[BUI_MAX_CODA];
    int numCoda, pos;
    bool eventoIniziato;
    int timer;

    char testo[200];
    float caratteri;
    int attesaTesto;
    float caratteriPerPasso;

    LatoUI lato[2];
    float xpMostrato;
    int32_t xpTarget;
    Meteo meteo;
    int effettoTimer, effettoLato;      /* lampo colorato della mossa */
    ElementType effettoTipo;
    int boostTimer, boostLato, boostDir;
    bool coreVisibile;
    int coreTimer, coreAnim;            /* 1 = lancio, 2 = scossa, 3 = stelle */

    int cursoreMenu, cursoreMossa, cursoreSquadra, cursoreZaino, cursoreElisir, cursoreSiNo, cursoreImpara;
    ModoSquadra modoSquadra;
    int oggettoScelto, bersaglioOggetto;
    char avviso[TESTO_EVENTO];

    int imparaIndice, imparaMossa;
    int evoIndice, evoDa, evoA, evoFase, evoTimer;
    char evoNome[24];

    int frame;
    bool finita;
    EsitoLotta esito;
} BattleUI;

bool BattleUI_AvviaSelvatica(BattleUI *u, Partita *p, Rng *rng, int specie, int livello);
bool BattleUI_AvviaAllenatore(BattleUI *u, Partita *p, Rng *rng, const TrainerData *t);
void BattleUI_Aggiorna(BattleUI *u, const Input *in, float caratteriPerPasso);
void BattleUI_Disegna(const BattleUI *u);
bool BattleUI_Finita(const BattleUI *u);

#endif
