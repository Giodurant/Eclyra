/* menu.h - Menu verticale generico (titolo, opzioni, menu di gioco). */
#ifndef ECLYRA_MENU_H
#define ECLYRA_MENU_H

#include "input.h"
#include <stdbool.h>

#define MENU_MAX_VOCI 8

typedef struct {
    const char *voci[MENU_MAX_VOCI];
    bool abilitata[MENU_MAX_VOCI];
    int numVoci;
    int cursore;
} Menu;

void Menu_Init(Menu *m, const char *const *voci, int numVoci);

/* Ritorna l'indice confermato, -2 se "indietro", -1 se nessuna scelta.
 * Il cursore salta le voci disabilitate. */
int  Menu_Aggiorna(Menu *m, const Input *in);

/* Disegna dentro una finestra; le voci disabilitate sono grigie */
void Menu_Disegna(const Menu *m, int x, int y, int larghezza);

#endif
