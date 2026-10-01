/* battle.h - Motore di lotta a turni (logica pura, senza grafica).
 *
 * Il motore e' a eventi: ogni chiamata (inizio, turno, cambio forzato) applica
 * subito le regole allo stato e accoda una lista di EventoLotta che l'interfaccia
 * riproduce nell'ordine (messaggi, barre HP, animazioni). Gli HP negli eventi sono
 * quelli DOPO l'evento: la UI non deve ricalcolare nulla.
 *
 * Regole: documento di design (formula danni, STAB 1.25, critico 1.5 al 6%,
 * casuale 0.90-1.00, stati, boost -6..+6, meteo 5 turni, cattura, XP). */
#ifndef ECLYRA_BATTLE_H
#define ECLYRA_BATTLE_H

#include "elyon.h"
#include "items.h"
#include "data/trainers_data.h"
#include "data/encounters_data.h"
#include <stdbool.h>

#define LATO_GIOCATORE 0
#define LATO_NEMICO 1
#define MAX_EVENTI_LOTTA 160
#define TESTO_EVENTO 96
#define BOOST_MAX 6
#define MOSSA_DIMENARSI (-1)     /* azione quando non restano PP */
#define ID_DIMENARSI 1           /* usa COLPO senza consumare PP, con contraccolpo */

typedef enum { METEO_NESSUNO = 0, METEO_SOLE, METEO_PIOGGIA, METEO_NEVE, METEO_TEMPESTA, METEO_NEBBIA, NUM_METEO } Meteo;
typedef enum { LOTTA_SELVATICA = 0, LOTTA_ALLENATORE } TipoLotta;
typedef enum {
    ESITO_IN_CORSO = 0, ESITO_VITTORIA, ESITO_SCONFITTA, ESITO_FUGA, ESITO_CATTURA, ESITO_INTERROTTA
} EsitoLotta;

typedef enum { AZ_MOSSA = 0, AZ_CAMBIO, AZ_OGGETTO, AZ_FUGA } TipoAzione;
typedef struct {
    TipoAzione tipo;
    int param;        /* AZ_MOSSA: slot 0..3 o MOSSA_DIMENARSI; AZ_CAMBIO: indice squadra; AZ_OGGETTO: ID */
    int bersaglio;    /* AZ_OGGETTO: indice squadra (ignorato per i Core) */
    int slotMossa;    /* AZ_OGGETTO: mossa per gli elisir */
} Azione;

typedef enum {
    EV_MESSAGGIO = 0,     /* solo testo */
    EV_ENTRATA,           /* lato, a = indice, b = hp, c = hp max, d = livello */
    EV_RITIRO,            /* lato */
    EV_ATTACCO,           /* lato = attaccante, a = mossa */
    EV_DANNO,             /* lato = chi subisce, a = hp dopo, b = efficacia x100, c = critico */
    EV_CURA,              /* lato, a = hp dopo */
    EV_STATUS,            /* lato, a = StatusId (STATUS_NESSUNO = guarito) */
    EV_BOOST,             /* lato, a = BoostId, b = stadi applicati */
    EV_KO,                /* lato */
    EV_LANCIO_CORE,       /* a = oggetto */
    EV_SCOSSA,            /* a = numero scossa (1..3) */
    EV_CATTURA_OK,        /* a = 0 squadra / 1 box */
    EV_CATTURA_FALLITA,
    EV_XP,                /* a = indice squadra, b = xp totale dopo */
    EV_LIVELLO,           /* a = indice squadra, b = nuovo livello, c = hp, d = hp max */
    EV_NUOVA_MOSSA,       /* a = indice squadra, b = mossa imparata */
    EV_RICHIESTA_MOSSA,   /* a = indice squadra, b = mossa da imparare: la UI chiede cosa dimenticare */
    EV_METEO,             /* a = Meteo */
    EV_RICHIESTA_CAMBIO,  /* il giocatore deve scegliere un nuovo Elyon */
    EV_FINE               /* a = EsitoLotta */
} TipoEvento;

typedef struct {
    TipoEvento tipo;
    int lato;
    int a, b, c, d;
    char testo[TESTO_EVENTO];
} EventoLotta;

/* Stato temporaneo di chi e' in campo (si azzera al cambio) */
typedef struct {
    Elyon *e;
    int indice;              /* indice nella squadra del suo lato */
    int boost[NUM_BOOST];
    int confusione;          /* turni rimanenti, 0 = no */
    bool stordito;           /* salta l'azione di questo turno */
    bool storditoProssimo;
    bool haAgito;
    bool colpitoNelTurno;
    int radicamento;         /* cura 1/n a fine turno (0 = no) */
    bool dimezzaProssimo;
    int riflessioneTurni, riflessionePerc;
    int furia;               /* bonus % alla prossima mossa fisica */
    int bloccoCambio;        /* turni in cui non puo' essere cambiato */
    bool koAnnunciato;
} Combattente;

typedef struct {
    TipoLotta tipo;
    const TrainerData *allenatore;     /* NULL per i selvatici */
    Collezione *coll;                  /* squadra del giocatore (modificata direttamente) */
    Inventario *inv;
    Rng *rng;

    Elyon nemici[MAX_SQUADRA];
    int numNemici;
    Combattente lato[2];
    bool scudoUsato[2][MAX_SQUADRA];   /* SCUDO_LUMEN: una volta per lotta per Elyon */

    Meteo meteo;
    int turniMeteo;                    /* 0 = permanente (es. meteo della zona) */
    ElementType campoTipo;             /* CAMPO ELETTRICO e simili */
    int campoBonus, campoTurni;

    int turno;
    int tentativiFuga;
    int cureBoss, cureBossMax;
    int ultimoCambioIA;
    bool partecipanti[MAX_SQUADRA];    /* hanno affrontato il nemico in campo */
    bool livelloSalito[MAX_SQUADRA];   /* per controllare le evoluzioni a fine lotta */
    bool catturaEvento;                /* permette di catturare un primordiale */

    EsitoLotta esito;
    bool attesaCambio;                 /* il giocatore deve scegliere (KO) */

    EventoLotta eventi[MAX_EVENTI_LOTTA];
    int numEventi;
} Lotta;

/* ---- inizio ---- */
/* Ritornano false se la squadra del giocatore non ha Elyon in grado di lottare */
bool Lotta_InitSelvatica(Lotta *L, Collezione *c, Inventario *inv, Rng *rng, int specie, int livello);
bool Lotta_InitAllenatore(Lotta *L, Collezione *c, Inventario *inv, Rng *rng, const TrainerData *t);
/* Accoda gli eventi di entrata (da chiamare una volta, dopo Init) */
void Lotta_Inizia(Lotta *L);

/* ---- turni ---- */
/* Svuota la coda e risolve un turno completo. Ritorna false se l'azione non e' valida
 * (in quel caso la coda contiene solo il messaggio di spiegazione e il turno non passa). */
bool Lotta_Turno(Lotta *L, Azione az);
/* Dopo EV_RICHIESTA_CAMBIO: manda in campo l'Elyon scelto */
bool Lotta_CambioForzato(Lotta *L, int indice);

bool Lotta_AzioneValida(const Lotta *L, Azione az, char *motivo, int dim);
bool Lotta_Finita(const Lotta *L);
/* true se tutte le mosse dell'Elyon in campo sono senza PP */
bool Lotta_DeveDimenarsi(const Lotta *L);

/* ---- calcoli esposti per IA, UI e test ---- */
float Lotta_MoltBoost(int stadi);                               /* +n=(2+n)/2, -n=2/(2+n) */
float Lotta_MoltMeteo(Meteo m, ElementType tipo);
int   Lotta_VelocitaEffettiva(const Lotta *L, int lato);
/* Danno senza critico e con casuale fisso (1.0 = massimo, 0.9 = minimo) */
int   Lotta_StimaDanno(const Lotta *L, int latoAtt, int mossa, float casuale);
float Lotta_EfficaciaContro(const Lotta *L, int latoDif, ElementType tipoMossa);
float Lotta_ProbCattura(const Elyon *e, float fattoreCore, bool evento);
float Lotta_ProbFuga(int speGiocatore, int speNemico, int tentativo);
int32_t Lotta_XpGuadagnata(int livelloNemico, int baseYield, bool allenatore, int partecipanti);
const char *Meteo_Nome(Meteo m);

/* Sceglie l'azione del lato nemico secondo il livello di IA (esposta per i test) */
Azione Lotta_SceltaIA(Lotta *L);

/* Genera specie e livello di un incontro selvatico dall'habitat. false se non ci sono incontri. */
bool Lotta_GeneraIncontro(const HabitatData *h, Rng *rng, int *specie, int *livello);

#endif
