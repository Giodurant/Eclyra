/* items.h - Inventario del giocatore (quantita' per ID oggetto) e valuta Lumen. */
#ifndef ECLYRA_ITEMS_H
#define ECLYRA_ITEMS_H

#include "data/items_data.h"
#include <stdbool.h>

#define QUANTITA_MAX 999
#define LUMEN_MAX 9999999

typedef struct {
    int quantita[ITEM_ID_MAX];
    int lumen;               /* valuta */
} Inventario;

void Inventario_Azzera(Inventario *inv);
/* Ritorna la quantita' effettivamente aggiunta (0 se l'ID non esiste) */
int  Inventario_Aggiungi(Inventario *inv, int id, int quantita);
bool Inventario_Rimuovi(Inventario *inv, int id, int quantita);
int  Inventario_Quantita(const Inventario *inv, int id);
void Inventario_AggiungiLumen(Inventario *inv, int lumen);

#endif
