/* vista_evoluzione.c - Sequenza di evoluzione (mockup 10_evoluzione, Design System §13 e §15).
 * Raggi di Lumen e oro, tre anelli runici, sagoma luminosa che alterna le due forme sempre piu'
 * in fretta, lampo finale e rivelazione della nuova forma. Usata in lotta e fuori dalla lotta. */
#include "slice.h"
#include "gfx.h"
#include "vista_dialogo.h"
#include <math.h>

static void Raggi(int cx, int cy, int frame, int forza) {
    for (int k = 0; k < 32; k++) {
        float a = (float)k / 32.0f * 6.2831853f + frame * 0.003f;
        Color c = (k % 2) ? PAL_LUMEN[3] : PAL_ORO[3];
        for (int d = 20; d < 220; d += 2) {
            int al = forza - d / 3;
            if (al <= 0) break;
            c.a = (unsigned char)al;
            DrawPixel(cx + (int)(cosf(a) * d), cy + (int)(sinf(a) * d * 0.8f), c);
        }
    }
}

static void Anelli(int cx, int cy, int frame) {
    static const int R[3] = { 24, 38, 54 };
    for (int i = 0; i < 3; i++)
        for (int k = 0; k < 360; k += 2) {
            float a = (float)k * 0.0174533f + (i % 2 ? -1 : 1) * frame * 0.01f;
            Color c = PAL_LUMEN[4];
            c.a = (unsigned char)((k / 2) % 6 ? 140 : 255);
            DrawPixel(cx + (int)(cosf(a) * R[i]), cy + 18 + (int)(sinf(a) * R[i] * 0.5f), c);
        }
}

static void Scintille(int cx, int cy, int frame, int n) {
    for (int i = 0; i < n; i++) {
        unsigned h = (unsigned)(i * 2654435761u);
        int x = cx - 70 + (int)(h % 140), y = cy + 50 - (int)(((unsigned)frame * (1 + i % 3) + (h >> 8)) % 110);
        DrawPixel(x, y, (i % 3) ? PAL_LUMEN[4] : PAL_ORO[4]);
        if (i % 4 == 0) { DrawPixel(x + 1, y, PAL_LUMEN[3]); DrawPixel(x, y + 1, PAL_LUMEN[3]); }
    }
}

void VistaEvoluzione_Disegna(const Canvas *c, int da, int a, int timer, int fase, const char *testo, int frame) {
    G_Gradiente(0, 0, c->w, c->h, PAL_NOTTE[0], PAL_NOTTE[2], true);
    int cx = c->w / 2, cy = (c->h - 180) / 2 + 70;
    Vector2 dim = G_DimElyon(VISTA_NEMICO);
    int sx = cx - (int)dim.x / 2, sy = cy - 40;
    Color sagoma = { 240, 255, 252, 255 };
    if (fase == 0 || fase == 3) {
        Raggi(cx, cy, frame, 40);
        G_Bagliore(cx, cy, 50, PAL_LUMEN[3], 60);
        G_Elyon(da, VISTA_NEMICO, (frame / 30) % 2, sx, sy, WHITE);
    } else if (fase == 1) {
        float t = (float)timer / 200.0f;
        Raggi(cx, cy, frame, 50 + (int)(40 * t));
        Anelli(cx, cy, frame);
        G_Bagliore(cx, cy, 60, PAL_LUMEN[3], 100 + (int)(80 * t));
        /* alternanza: periodo da 32 a 4 frame */
        int periodo = 32 - (int)(28 * t);
        if (periodo < 4) periodo = 4;
        bool nuova = (timer / periodo) % 2 == 1;
        G_ElyonSagoma(nuova ? a : da, VISTA_NEMICO, sx, sy, sagoma);
        Scintille(cx, cy, frame, 40);
        if (timer > 186) DrawRectangle(0, 0, c->w, c->h, (Color){ 255, 255, 255, (unsigned char)((timer - 186) * 18) });
        T_MicroOmbra("B INTERROMPI", c->w - c->safeR - 6 - T_LarghezzaMicro("B INTERROMPI"), 6 + c->safeT, PAL_GRIGIO_UI, PAL_NOTTE[0]);
    } else {
        Raggi(cx, cy, frame, 70);
        Anelli(cx, cy, frame);
        G_Bagliore(cx, cy, 64, PAL_LUMEN[3], 110);
        G_Elyon(a, VISTA_NEMICO, (frame / 30) % 2, sx, sy, WHITE);
        if (timer < 16) DrawRectangle(0, 0, c->w, c->h, (Color){ 255, 255, 255, (unsigned char)(255 - timer * 16) });
        Scintille(cx, cy, frame, 24);
    }
    if (testo && testo[0]) VistaDialogo_Testo("", testo, c, frame, fase != 1);
}
