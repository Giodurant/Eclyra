/* schermate.h - Schermate del menu di gioco: squadra (con riepilogo e riordino), zaino (con uso
 * degli oggetti), scheda allenatore e PC del box. */
#ifndef ECLYRA_SCHERMATE_H
#define ECLYRA_SCHERMATE_H

#include "input.h"
#include "partita.h"
#include <stdbool.h>

typedef enum { SCH_NESSUNA = 0, SCH_SQUADRA, SCH_ZAINO, SCH_SCHEDA, SCH_PC } TipoSchermata;

typedef enum {
    FASE_LISTA = 0, FASE_AZIONI, FASE_SPOSTA, FASE_RIEPILOGO, FASE_BERSAGLIO, FASE_MOSSA, FASE_MESSAGGIO
} FaseSchermata;

typedef struct {
    TipoSchermata tipo;
    FaseSchermata fase, fasePrimaMessaggio;
    Partita *partita;
    int cursore, cursoreAzioni, cursoreBersaglio, cursoreMossa, spostaDa;
    int oggetto;
    /* PC: 0 = squadra, 1 = box */
    int pcPannello, pcBox, pcPosto;
    char messaggio[128];
    int frame;
} Schermate;

void Schermate_Apri(Schermate *s, TipoSchermata tipo, Partita *p);
/* Ritorna false quando la schermata si chiude */
bool Schermate_Aggiorna(Schermate *s, const Input *in);
void Schermate_Disegna(const Schermate *s);

#endif
