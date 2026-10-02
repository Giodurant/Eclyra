/* gfx.h - Primitive grafiche della vertical slice, fedeli al Design System v2.
 *
 * Tutto si disegna nel canvas logico (320-400 x 180-240) a pixel interi. Le forme che nei mockup
 * sono generate in Python (cornici, barre, pillole, targhe) sono riprodotte qui pixel per pixel;
 * la pixel art (Elyon, tile, edifici, personaggi, icone, fondali) arriva dagli atlanti esportati
 * da tools/design/slice_export.py. Nessuno stile nuovo. */
#ifndef SLICE_GFX_H
#define SLICE_GFX_H

#include "raylib.h"
#include "data/types.h"
#include "data/moves_data.h"
#include "gen/palette.h"
#include <stdbool.h>

typedef enum {
    TX_UI = 0, TX_ELYON, TX_TILES, TX_OGGETTI, TX_PERSONAGGI, TX_LOTTA, TX_LOTTA_PRATO,
    TX_TITOLO, TX_LOGO, TX_MAPPA, TX_FONT, TX_FONT_MICRO, TX_BAGLIORE, TX_ELYON_SAGOMA, TX_COUNT
} TexSlice;

typedef enum { CORNICE_NOTTE = 0, CORNICE_PERGAMENA } StileCornice;
typedef enum { VISTA_FRONTE = 0, VISTA_NEMICO, VISTA_MIO, VISTA_ICONA } VistaElyon;

bool Gfx_Carica(const char *cartellaAsset);
void Gfx_Libera(void);
Texture2D Gfx_Tex(TexSlice t);
/* Cartella degli asset della slice (con '/' finale) */
const char *Gfx_CartellaSlice(void);

/* ---- testo (font originali "Eclyra Corpo" e "Eclyra Micro", UTF-8) ---- */
int  T_Larghezza(const char *s);
int  T_LarghezzaGrassetto(const char *s);
int  T_LarghezzaMicro(const char *s);
int  T_Testo(const char *s, int x, int y, Color c);
int  T_TestoOmbra(const char *s, int x, int y, Color c);            /* ombra 1 px sotto (OMBRA_TESTO) */
int  T_Grassetto(const char *s, int x, int y, Color c, Color ombra);
int  T_TitoloOro(const char *s, int x, int y);                       /* grassetto con oro verticale */
int  T_Micro(const char *s, int x, int y, Color c);
int  T_MicroOmbra(const char *s, int x, int y, Color c, Color ombra);
/* A capo automatico entro w; ritorna le righe disegnate (al massimo maxRighe, 0 = illimitate) */
int  T_ACapo(const char *s, int x, int y, int w, int interlinea, int maxRighe, Color c, bool ombra);
/* Conta le righe senza disegnare */
int  T_ContaRighe(const char *s, int w);

/* ---- componenti ---- */
void G_Rett(int x, int y, int w, int h, Color c);
void G_LineaH(int x, int y, int w, Color c);
void G_LineaV(int x, int y, int h, Color c);
void G_Gradiente(int x, int y, int w, int h, Color c1, Color c2, bool dither);
void G_Cornice(int x, int y, int w, int h, StileCornice stile, bool gemme);
int  G_Intestazione(int x, int y, const char *titolo);
void G_RigaSelezionata(int x, int y, int w, int h);
void G_Cursore(int x, int y, int frame);
void G_Separatore(int x, int y, int w);
void G_Cristallo(int x, int y, int h, bool acceso);    /* x, y = punta in alto; h = 7, 11, 13, 17 */
void G_Gemma(int cx, int cy);
void G_Bagliore(int cx, int cy, int r, Color c, int forza);
/* Barra HP o XP; rampa = 3 colori (scuro, base, chiaro). Ritorna la x dopo la barra. */
int  G_Barra(int x, int y, int w, float frazione, const Color *rampa, const char *etichetta, int h);
const Color *G_RampaHp(float frazione);
void G_Badge(ElementType t, int x, int y);
void G_PillolaTipo(ElementType t, int x, int y, int w);
int  G_PillolaStato(StatusId s, int x, int y);
void G_Icona(Rectangle sorgente, int x, int y);
void G_IconaOggetto(int idOggetto, int x, int y);
/* Prompt di un tasto (9x9) */
void G_GlifoTasto(int x, int y, const char *lettera, bool attivo);
void G_Oscura(int x, int y, int w, int h, unsigned char alfa);

/* ---- Elyon (solo le 9 specie di riferimento: 1..9) ---- */
bool G_HaSprite(int specie);
/* fotogramma: 0 riposo, 1 respiro, 2 battito, 3 affondo, 4 colpito (icona: 0-1) */
void G_Elyon(int specie, VistaElyon vista, int fotogramma, int x, int y, Color tinta);
Vector2 G_DimElyon(VistaElyon vista);
/* Silhouette (Bestiario: VISTO non catturato, IGNOTO) */
void G_ElyonSagoma(int specie, VistaElyon vista, int x, int y, Color colore);

#endif
