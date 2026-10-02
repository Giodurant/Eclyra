/* canvas.h - Canvas logico a pixel interi (MULTIPLATFORM_ARCHITECTURE.md §5).
 * Lato corto 180-240 px, lato lungo 320-400 px, scala intera, nessun filtro. */
#ifndef PROTO_CANVAS_H
#define PROTO_CANVAS_H

#include "raylib.h"
typedef struct { int left, top, right, bottom; } SafeInsets;   /* pixel fisici */

#define CANVAS_W_MIN 320
#define CANVAS_W_MAX 400
#define CANVAS_H_MIN 180
#define CANVAS_H_MAX 240

typedef struct {
    int w, h;            /* dimensione del canvas logico */
    int scale;           /* pixel fisici per pixel logico */
    int offX, offY;      /* bande scure rimaste (pixel fisici) */
    int screenW, screenH;
    /* area sicura in pixel logici, gia' dentro il canvas */
    int safeL, safeT, safeR, safeB;
} Canvas;

Canvas Canvas_Compute(int screenW, int screenH, SafeInsets inset);
/* Da pixel fisici dello schermo a pixel logici del canvas (puo' uscire dai bordi) */
Vector2 Canvas_FromScreen(const Canvas *c, Vector2 p);

#endif
