/* partita.h - Stato persistente della partita (cio' che finira' nel salvataggio).
 * Contiene solo valori, nessun puntatore: si potra' serializzare campo per campo. */
#ifndef ECLYRA_PARTITA_H
#define ECLYRA_PARTITA_H

#include "common.h"
#include "elyon.h"
#include "events.h"
#include "items.h"
#include "map.h"
#include <stdint.h>

typedef struct {
    char nomeGiocatore[16];
    Flags flags;
    Inventario inventario;
    Collezione collezione;          /* squadra (6) + box (5x20) */
    char mappa[MAP_NOME_ID];
    int x, y;
    Direzione dir;
    uint32_t semeRng;
    uint32_t secondiGioco;
} Partita;

void Partita_Nuova(Partita *p, uint32_t seme);

#endif
