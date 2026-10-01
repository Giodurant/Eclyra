/* input.c */
#include "input.h"
#include "raylib.h"

void Input_Aggiorna(Input *in) {
    in->su       = IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W);
    in->giu      = IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S);
    in->sinistra = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A);
    in->destra   = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
    in->corsa    = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    in->suP       |= IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W);
    in->giuP      |= IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S);
    in->sinistraP |= IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A);
    in->destraP   |= IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D);
    in->conferma  |= IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    in->indietro  |= IsKeyPressed(KEY_X) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE);
    in->menu      |= IsKeyPressed(KEY_M);
}

void Input_ConsumaEventi(Input *in) {
    in->suP = in->giuP = in->sinistraP = in->destraP = false;
    in->conferma = in->indietro = in->menu = false;
}
