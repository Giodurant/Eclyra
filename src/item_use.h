/* item_use.h - Effetto degli oggetti curativi su un Elyon (zaino e lotta).
 * Logica pura: non tocca l'inventario, il chiamante rimuove l'oggetto se l'uso riesce. */
#ifndef ECLYRA_ITEM_USE_H
#define ECLYRA_ITEM_USE_H

#include "elyon.h"
#include <stdbool.h>
#include <stddef.h>

/* true se l'oggetto si usa su un Elyon (cure, PP, rianimazione) */
bool Oggetto_SuElyon(int item);
/* true se l'oggetto richiede di scegliere una mossa (ELISIR, ELISIR MAX) */
bool Oggetto_RichiedeMossa(int item);
/* Fattore di cattura del Core (0 se non e' un Core) */
float Oggetto_FattoreCore(int item);

/* Applica l'oggetto. slotMossa serve solo per gli elisir.
 * Ritorna true se ha avuto effetto (e scrive il messaggio), false se non serve a nulla. */
bool Oggetto_Usa(Elyon *e, int item, int slotMossa, char *msg, size_t dim);

#endif
