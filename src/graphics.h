/* graphics.h - Finestra, risoluzione logica 320x180 con scala intera, texture e testo. */
#ifndef ECLYRA_GRAPHICS_H
#define ECLYRA_GRAPHICS_H

#include "raylib.h"
#include <stdbool.h>

/* Cartella degli asset (accanto all'eseguibile, oppure ./assets) */
const char *Gfx_CartellaAsset(void);

#define LOGICAL_W 320
#define LOGICAL_H 180
#define TILE      16

typedef enum {
    TEX_TILESET = 0,
    TEX_ALBERO,
    TEX_CASA,
    TEX_GIOCATORE,
    TEX_CASA_BLU,
    TEX_NEGOZIO,
    TEX_LAB,
    TEX_INTERNI,
    TEX_NPC,
    TEX_ELYON_FRONTE,   /* 640x640: griglia 10x10 di sprite 64x64, specie 1..100 */
    TEX_ELYON_RETRO,
    TEX_COUNT
} TexId;

/* Colori dell'interfaccia */
#define COL_TESTO       (Color){ 72, 72, 80, 255 }
#define COL_OMBRA_TESTO (Color){ 208, 208, 200, 255 }
#define COL_BIANCO_UI   (Color){ 248, 248, 248, 255 }
#define COL_CORNICE     (Color){ 56, 64, 96, 255 }
#define COL_BORDO_UI    (Color){ 104, 168, 208, 255 }

bool Gfx_Init(const char *titolo);
void Gfx_Shutdown(void);

void Gfx_InizioFrame(void);   /* da qui si disegna in coordinate logiche 320x180 */
void Gfx_FineFrame(void);     /* scala intera, centrata, filtro nearest */

int  Gfx_Scala(void);
void Gfx_SchermoIntero(bool attivo);
bool Gfx_IsSchermoIntero(void);

Texture2D Gfx_Texture(TexId id);

/* Testo con ombra in stile GBA (font di default, 10 px) */
void Gfx_Testo(const char *testo, int x, int y, Color colore);
void Gfx_TestoCentrato(const char *testo, int cx, int y, int dimensione, Color colore);
int  Gfx_LarghezzaTesto(const char *testo, int dimensione);

/* Riquadro dell'interfaccia: cornice scura, bordo azzurro, interno bianco */
void Gfx_Finestra(int x, int y, int w, int h);

/* Cursore a triangolo per i menu */
void Gfx_Cursore(int x, int y);

/* Salva l'ultimo frame logico 320x180 in PNG (test e screenshot) */
void Gfx_EsportaFrame(const char *file);

/* Oscura lo schermo (0 = nulla, 1 = nero) per le transizioni */
void Gfx_Dissolvenza(float quantita);

#endif
