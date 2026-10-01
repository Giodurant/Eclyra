/* dialogue_ui.h - Disegno del riquadro di dialogo (a capo automatico, nome di chi parla). */
#ifndef ECLYRA_DIALOGUE_UI_H
#define ECLYRA_DIALOGUE_UI_H

#include "dialogue.h"

#define DIALOGO_RIGHE_VISIBILI 3

/* Va a capo in base alla larghezza reale del font e divide in piu' pagine
 * le pagine che superano le righe visibili. Da chiamare dopo Dialogo_Avvia*. */
void DialogoUI_Impagina(Dialogo *d);
void DialogoUI_Disegna(const Dialogo *d, int frame);

#endif
