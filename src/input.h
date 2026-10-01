/* input.h - Tastiera unificata.
 * Z / Invio = conferma, X / Esc = indietro, frecce/WASD = movimento,
 * Shift = corsa, M = menu di gioco. */
#ifndef ECLYRA_INPUT_H
#define ECLYRA_INPUT_H

#include <stdbool.h>

typedef struct {
    bool su, giu, sinistra, destra;          /* tenuti premuti */
    bool suP, giuP, sinistraP, destraP;      /* premuti in questo frame (menu) */
    bool conferma, indietro, menu;           /* premuti in questo frame */
    bool corsa;                              /* tenuto */
} Input;

/* Legge la tastiera. I tasti "premuti" si accumulano finche' la logica
 * non li consuma con Input_ConsumaEventi (gestisce il passo fisso). */
void Input_Aggiorna(Input *in);
void Input_ConsumaEventi(Input *in);

#endif
