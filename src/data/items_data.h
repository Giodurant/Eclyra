/* items_data.h - Tabella degli oggetti (ID stabili dal documento di design). */
#ifndef ECLYRA_ITEMS_DATA_H
#define ECLYRA_ITEMS_DATA_H

#include <stdbool.h>

#define ITEM_ID_MAX 36   /* ID validi: 1..35 (alcuni ID sono riservati e non usati) */

typedef enum {
    CAT_NESSUNA = 0, CAT_CURA, CAT_CORE, CAT_EVOLUZIONE, CAT_CHIAVE, CAT_EQUIPAGGIAMENTO
} CategoriaOggetto;

typedef struct {
    int id;
    const char *nome;
    CategoriaOggetto categoria;
    int prezzo;              /* in Lumen; 0 = non vendibile */
    const char *descrizione;
} ItemData;

/* NULL se l'ID non esiste */
const ItemData *GetItem(int id);

/* ID usati nel codice */
enum {
    ITEM_ERBA_VITALE = 1, ITEM_FRUTTO_LUMEN = 2, ITEM_ELISIR = 3, ITEM_ELISIR_MAX = 4,
    ITEM_ANTIDOTO = 5, ITEM_STABILIZZATORE = 6, ITEM_RISVEGLIO = 7, ITEM_FIAMMIFUGA = 8,
    ITEM_GELORISCALDO = 9, ITEM_REVITA = 10, ITEM_REVITA_MAX = 11,
    ITEM_CORE = 12, ITEM_CORE_PLUS = 13, ITEM_CORE_MAX = 14, ITEM_CORE_VOID = 15,
    ITEM_PIETRA_FIAMMA = 20, ITEM_PIETRA_ACQUA = 21, ITEM_PIETRA_NATURA = 22, ITEM_PIETRA_LUMEN = 23,
    ITEM_NUCLEO_CONDIVISO = 29,
};

#endif
