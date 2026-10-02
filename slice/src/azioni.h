/* azioni.h - Input astratto della slice (MULTIPLATFORM_ARCHITECTURE.md §3).
 * Tastiera, controller, mouse e touch producono le stesse azioni; il resto del codice
 * non sa da dove arrivano. Contiene anche l'adattatore verso l'Input del gioco (world.c, battle_ui.c). */
#ifndef SLICE_AZIONI_H
#define SLICE_AZIONI_H

#include "canvas.h"
#include "input.h"
#include <stdbool.h>

typedef enum {
    AZ_SU, AZ_GIU, AZ_SINISTRA, AZ_DESTRA, AZ_CONFERMA, AZ_ANNULLA, AZ_MENU, AZ_CORSA,
    AZ_FACCIA_X, AZ_FACCIA_Y,          /* tasti frontali sinistro e alto (lotta: ZAINO, ELYON) */
    NUM_AZIONI
} AzioneIn;

typedef enum { DISP_TASTIERA = 0, DISP_CONTROLLER, DISP_TOUCH } Dispositivo;
typedef enum { TOUCH_ESPLORA = 0, TOUCH_DIALOGO, TOUCH_MENU, TOUCH_LOTTA, TOUCH_NESSUNO } ContestoTouch;

typedef struct {
    bool giu[NUM_AZIONI];
    bool premuto[NUM_AZIONI];      /* premuto in questo frame (o ripetuto, per le direzioni nei menu) */
    bool rilasciato[NUM_AZIONI];
    int tenuto[NUM_AZIONI];        /* frame da cui e' tenuto */
    Dispositivo ultimo;
    char nomeController[64];

    /* puntatore (mouse o dito) nel canvas logico */
    bool puntAttivo, puntInizio, puntFine;
    Vector2 punt, puntPartenza;
    int puntDurata;
    /* tocco breve concluso in questo frame (per toccare voci, carte e pulsanti) */
    bool tap;
    Vector2 tapPos;

    ContestoTouch contesto;
    bool touchVisibile;            /* controlli touch disegnati */
    float touchScala;
    bool forzaTouch;               /* --touch: controlli sempre visibili (prove su desktop e foto) */
    bool iniettato[NUM_AZIONI];    /* test automatici */
    bool puntSuiControlli_;        /* il dito e' partito su un controllo touch: non e' un tap */
} Azioni;

void Azioni_Init(Azioni *a, bool mobile);
void Azioni_Aggiorna(Azioni *a, const Canvas *c);
void Azioni_Inietta(Azioni *a, AzioneIn az);
/* Accumula le pressioni nell'Input del gioco; Input_ConsumaEventi le azzera dopo il passo logico */
void Azioni_VersoInput(const Azioni *a, Input *in);
/* true se il tap e' caduto nel rettangolo (coordinate canvas) */
bool Azioni_Tap(const Azioni *a, Rectangle r);
void Azioni_DisegnaTouch(const Azioni *a, const Canvas *c, int frame);
/* Etichetta del tasto per l'azione sul dispositivo usato per ultimo ("Z", "A", "TAB"...) */
const char *Azioni_Prompt(const Azioni *a, AzioneIn az);

#endif
