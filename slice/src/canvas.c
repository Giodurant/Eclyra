/* canvas.c - Calcolo del canvas logico; identico a quello previsto per il gioco */
#include "canvas.h"

static int MinI(int a, int b) { return a < b ? a : b; }
static int MaxI(int a, int b) { return a > b ? a : b; }
static int Clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

Canvas Canvas_Compute(int screenW, int screenH, SafeInsets inset) {
    Canvas c = { 0 };
    int s = MinI(screenW / CANVAS_W_MIN, screenH / CANVAS_H_MIN);
    if (s < 1) s = 1;
    c.scale = s;
    c.w = Clamp(screenW / s, CANVAS_W_MIN, CANVAS_W_MAX);
    c.h = Clamp(screenH / s, CANVAS_H_MIN, CANVAS_H_MAX);
    c.offX = (screenW - c.w * s) / 2;
    c.offY = (screenH - c.h * s) / 2;
    c.screenW = screenW;
    c.screenH = screenH;
    /* gli inset fisici diventano logici (arrotondati per eccesso) e si tolgono le bande gia' scure */
    c.safeL = MaxI(0, (inset.left - c.offX + s - 1) / s);
    c.safeR = MaxI(0, (inset.right - c.offX + s - 1) / s);
    c.safeT = MaxI(0, (inset.top - c.offY + s - 1) / s);
    c.safeB = MaxI(0, (inset.bottom - c.offY + s - 1) / s);
    return c;
}

Vector2 Canvas_FromScreen(const Canvas *c, Vector2 p) {
    Vector2 r = { (p.x - (float)c->offX) / (float)c->scale, (p.y - (float)c->offY) / (float)c->scale };
    return r;
}
