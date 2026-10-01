/* map.h - Mappe a tile 16x16 con collisioni e disegno in vista dall'alto 3/4. */
#ifndef ECLYRA_MAP_H
#define ECLYRA_MAP_H

#include <stdbool.h>
#include <stdint.h>

#define MAP_MAX_W 80
#define MAP_MAX_H 60
#define EDIFICIO_W 5   /* casa standard: 5x4 tile, porta al centro della riga in basso */
#define EDIFICIO_H 4

typedef enum {
    T_ERBA = 0,
    T_ERBA_ALTA,
    T_SENTIERO,
    T_ALBERO,
    T_ACQUA,
    T_FIORI,
    T_CARTELLO,
    T_EDIFICIO,
    T_COUNT
} TileTipo;

typedef struct {
    char nome[32];
    int w, h;
    uint8_t tile[MAP_MAX_H][MAP_MAX_W];
} Map;

void     Map_Pulisci(Map *m, const char *nome, int w, int h, TileTipo riempimento);
TileTipo Map_Tile(const Map *m, int x, int y);      /* fuori mappa = albero */
void     Map_Imposta(Map *m, int x, int y, TileTipo t);
void     Map_Rettangolo(Map *m, int x, int y, int w, int h, TileTipo t);
bool     Map_Bloccata(const Map *m, int x, int y);
void     Map_PiazzaEdificio(Map *m, int x, int y);   /* (x, y) = angolo in alto a sinistra */

/* Fase 1: Lunaria provvisoria 40x30 con gli edifici alle coordinate del design */
void Map_GeneraLunariaProvvisoria(Map *m);

/* Disegno in due passate: terreno, poi oggetti alti riga per riga */
void Map_DisegnaTerreno(const Map *m, int camX, int camY, int frame);
void Map_DisegnaOggettiRiga(const Map *m, int riga, int camX, int camY);
void Map_RigheVisibili(int camY, int *prima, int *ultima);

#endif
