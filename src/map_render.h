/* map_render.h - Disegno delle mappe (esterni e interni) in vista dall'alto 3/4. */
#ifndef ECLYRA_MAP_RENDER_H
#define ECLYRA_MAP_RENDER_H

#include "map.h"

/* 1) terreno di tutte le tile visibili */
void MapRender_Terreno(const Map *m, int camX, int camY, int frame);
/* 2) oggetti alti di una riga (alberi, edifici): chi sta piu' in basso copre chi sta dietro */
void MapRender_OggettiRiga(const Map *m, int riga, int camX, int camY);
void MapRender_RigheVisibili(int camY, int *prima, int *ultima);

#endif
