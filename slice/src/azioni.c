/* azioni.c - Vedi azioni.h. Geometria e regole dei controlli touch da Design System §18. */
#include "azioni.h"
#include "gfx.h"
#include <math.h>
#include <string.h>

static bool mobile;

void Azioni_Init(Azioni *a, bool m) {
    memset(a, 0, sizeof(*a));
    mobile = m;
    a->touchScala = 1.0f;
    a->touchVisibile = m;
    a->ultimo = m ? DISP_TOUCH : DISP_TASTIERA;
}

void Azioni_Inietta(Azioni *a, AzioneIn az) { a->iniettato[az] = true; }

typedef struct { Vector2 croce, a, b; float rCroce, rA, rB; Rectangle menu; } Geometria;

static Geometria Geo(const Azioni *a, const Canvas *c) {
    Geometria g;
    float k = a->touchScala, m = 6.0f;
    g.rCroce = 22.0f * k; g.rA = 12.0f * k; g.rB = 10.0f * k;
    g.croce = (Vector2){ (float)c->safeL + m + g.rCroce, (float)(c->h - c->safeB) - m - g.rCroce };
    float dx = (float)(c->w - c->safeR) - m, basso = (float)(c->h - c->safeB) - m;
    g.a = (Vector2){ dx - g.rA, basso - g.rA - 8.0f * k };
    g.b = (Vector2){ dx - 2.0f * g.rA - 6.0f * k - g.rB, basso - g.rB };
    if (a->contesto == TOUCH_MENU) g.b = (Vector2){ dx - g.rB, basso - g.rB - 30.0f };
    /* in lotta la rosa occupa l'angolo: B sta sopra la rosa, al bordo destro */
    if (a->contesto == TOUCH_LOTTA) g.b = (Vector2){ dx - g.rB, basso - 60.0f - g.rB };
    g.menu = (Rectangle){ dx - 30.0f * k, (float)c->safeT + m, 30.0f * k, 11.0f * k };
    return g;
}

static float Dist(Vector2 p, Vector2 q) { return sqrtf((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y)); }

static Vector2 ACanvas(const Canvas *c, Vector2 p) {
    /* le coordinate di raylib sono in "punti" dello schermo: si riportano ai pixel fisici e poi al canvas */
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    if (sw > 0 && sh > 0) { p.x = p.x / (float)sw * (float)c->screenW; p.y = p.y / (float)sh * (float)c->screenH; }
    return Canvas_FromScreen(c, p);
}

void Azioni_Aggiorna(Azioni *a, const Canvas *c) {
    bool giu[NUM_AZIONI] = { false };
    bool tastiera = false, pad = false, touch = false;

    static const int K[NUM_AZIONI][3] = {
        { KEY_W, KEY_UP, 0 }, { KEY_S, KEY_DOWN, 0 }, { KEY_A, KEY_LEFT, 0 }, { KEY_D, KEY_RIGHT, 0 },
        { KEY_Z, KEY_ENTER, KEY_SPACE }, { KEY_X, KEY_ESCAPE, KEY_BACKSPACE }, { KEY_TAB, KEY_M, 0 },
        { KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT, 0 }, { KEY_C, 0, 0 }, { KEY_V, 0, 0 } };
    for (int i = 0; i < NUM_AZIONI; i++)
        for (int k = 0; k < 3; k++)
            if (K[i][k] && IsKeyDown(K[i][k])) { giu[i] = true; tastiera = true; }

    a->nomeController[0] = 0;
    if (IsGamepadAvailable(0)) {
        const char *n = GetGamepadName(0);
        if (n) { strncpy(a->nomeController, n, sizeof(a->nomeController) - 1); a->nomeController[sizeof(a->nomeController) - 1] = 0; }
        /* per posizione: Xbox, PlayStation, Steam Deck e generici sono uguali */
        static const int B[NUM_AZIONI] = {
            GAMEPAD_BUTTON_LEFT_FACE_UP, GAMEPAD_BUTTON_LEFT_FACE_DOWN, GAMEPAD_BUTTON_LEFT_FACE_LEFT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
            GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_BUTTON_MIDDLE_RIGHT,
            GAMEPAD_BUTTON_RIGHT_TRIGGER_1, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, GAMEPAD_BUTTON_RIGHT_FACE_UP };
        for (int i = 0; i < NUM_AZIONI; i++) if (IsGamepadButtonDown(0, B[i])) { giu[i] = true; pad = true; }
        float ax = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X), ay = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        if (ax < -0.25f) { giu[AZ_SINISTRA] = true; pad = true; }
        if (ax > 0.25f) { giu[AZ_DESTRA] = true; pad = true; }
        if (ay < -0.25f) { giu[AZ_SU] = true; pad = true; }
        if (ay > 0.25f) { giu[AZ_GIU] = true; pad = true; }
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_LEFT)) { giu[AZ_MENU] = true; pad = true; }
        /* corsa: tasto a sinistra (X / quadrato) tenuto, come da architettura; anche RB */
        if (giu[AZ_FACCIA_X]) giu[AZ_CORSA] = true;
    }

    /* puntatore: dito (mobile) o mouse (desktop) */
    bool attivo = false;
    Vector2 pos = a->punt;
    Vector2 dita[4];
    int nDita = 0;
    if (mobile) {
        int n = GetTouchPointCount();
        for (int i = 0; i < n && nDita < 4; i++) dita[nDita++] = ACanvas(c, GetTouchPosition(i));
        if (nDita) { attivo = true; pos = dita[0]; }
    } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        dita[nDita++] = ACanvas(c, GetMousePosition());
        attivo = true;
        pos = dita[0];
    }
    if (!mobile && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) giu[AZ_ANNULLA] = true;

    a->puntInizio = attivo && !a->puntAttivo;
    a->puntFine = !attivo && a->puntAttivo;
    if (a->puntInizio) { a->puntPartenza = pos; a->puntDurata = 0; }
    if (attivo) a->puntDurata++;
    a->tap = false;

    /* controlli touch */
    bool suiControlli = false;
    bool mostra = a->touchVisibile || a->forzaTouch;
    if (mostra && a->contesto != TOUCH_NESSUNO && a->contesto != TOUCH_DIALOGO) {
        Geometria g = Geo(a, c);
        for (int i = 0; i < nDita; i++) {
            Vector2 p = dita[i];
            bool croce = a->contesto == TOUCH_ESPLORA;
            if (croce && Dist(p, g.croce) <= g.rCroce * 1.4f) {
                float dx = p.x - g.croce.x, dy = p.y - g.croce.y, zm = g.rCroce * 0.25f;
                if (fabsf(dx) > zm || fabsf(dy) > zm) {
                    float ang = atan2f(dy, dx) * 57.2958f;
                    if (ang > -67.5f && ang < 67.5f) giu[AZ_DESTRA] = true;
                    if (ang > 112.5f || ang < -112.5f) giu[AZ_SINISTRA] = true;
                    if (ang > 22.5f && ang < 157.5f) giu[AZ_GIU] = true;
                    if (ang < -22.5f && ang > -157.5f) giu[AZ_SU] = true;
                }
                if (i == 0) suiControlli = true;
                touch = true;
            } else if (croce && Dist(p, g.a) <= g.rA * 1.3f) { giu[AZ_CONFERMA] = true; touch = true; if (i == 0) suiControlli = true; }
            else if (Dist(p, g.b) <= g.rB * 1.3f) { giu[AZ_ANNULLA] = true; touch = true; if (i == 0) suiControlli = true; }
            else if (croce && CheckCollisionPointRec(p, g.menu)) { giu[AZ_MENU] = true; touch = true; if (i == 0) suiControlli = true; }
        }
    }
    /* tocco breve fuori dai controlli = tap sull'interfaccia (dialoghi: avanza) */
    if (a->puntFine && !a->puntSuiControlli_ && a->puntDurata < 30 && Dist(a->punt, a->puntPartenza) < 8.0f) {
        a->tap = true;
        a->tapPos = a->punt;
        touch = mobile || touch;
    }
    a->puntSuiControlli_ = attivo ? (a->puntInizio ? suiControlli : a->puntSuiControlli_) : false;
    if (a->contesto == TOUCH_DIALOGO && a->tap) giu[AZ_CONFERMA] = true;
    a->puntAttivo = attivo;
    if (attivo) a->punt = pos;

    for (int i = 0; i < NUM_AZIONI; i++) {
        if (a->iniettato[i]) { giu[i] = true; a->iniettato[i] = false; }
        bool prima = a->giu[i];
        a->premuto[i] = giu[i] && !prima;
        a->rilasciato[i] = !giu[i] && prima;
        a->tenuto[i] = giu[i] ? a->tenuto[i] + 1 : 0;
        /* ripetizione per i menu: dopo 15 frame, ogni 5 */
        if (i <= AZ_DESTRA && a->tenuto[i] > 15 && (a->tenuto[i] - 15) % 5 == 0) a->premuto[i] = true;
        a->giu[i] = giu[i];
    }
    if (tastiera) a->ultimo = DISP_TASTIERA;
    else if (pad) a->ultimo = DISP_CONTROLLER;
    else if (touch || (mobile && attivo)) a->ultimo = DISP_TOUCH;
    if ((tastiera || pad) && !a->forzaTouch) a->touchVisibile = false;
    else if (attivo && (mobile || a->forzaTouch)) a->touchVisibile = true;
}

void Azioni_VersoInput(const Azioni *a, Input *in) {
    in->su = a->giu[AZ_SU]; in->giu = a->giu[AZ_GIU]; in->sinistra = a->giu[AZ_SINISTRA]; in->destra = a->giu[AZ_DESTRA];
    in->suP |= a->premuto[AZ_SU]; in->giuP |= a->premuto[AZ_GIU];
    in->sinistraP |= a->premuto[AZ_SINISTRA]; in->destraP |= a->premuto[AZ_DESTRA];
    in->conferma |= a->premuto[AZ_CONFERMA];
    in->indietro |= a->premuto[AZ_ANNULLA];
    in->menu |= a->premuto[AZ_MENU];
    in->corsa = a->giu[AZ_CORSA];
}

bool Azioni_Tap(const Azioni *a, Rectangle r) { return a->tap && CheckCollisionPointRec(a->tapPos, r); }

const char *Azioni_Prompt(const Azioni *a, AzioneIn az) {
    if (a->ultimo == DISP_CONTROLLER) {
        static const char *P[NUM_AZIONI] = { "^", "V", "<", ">", "A", "B", "=", "RB", "X", "Y" };
        return P[az];
    }
    static const char *T[NUM_AZIONI] = { "W", "S", "A", "D", "Z", "X", "TAB", "SH", "C", "V" };
    return T[az];
}

static void Croce(Vector2 c, float r, const Azioni *a, Color fondo, Color bordo, Color attivo) {
    DrawCircleV(c, r, fondo);
    DrawCircleLinesV(c, r, bordo);
    struct { AzioneIn az; float dx, dy; } f[4] = { { AZ_SU, 0, -1 }, { AZ_GIU, 0, 1 }, { AZ_SINISTRA, -1, 0 }, { AZ_DESTRA, 1, 0 } };
    for (int i = 0; i < 4; i++) {
        Vector2 p = { c.x + f[i].dx * r * 0.58f, c.y + f[i].dy * r * 0.58f };
        Color col = a->giu[f[i].az] ? attivo : bordo;
        /* freccia a triangolo verso l'esterno */
        Vector2 punta = { p.x + f[i].dx * 4, p.y + f[i].dy * 4 };
        Vector2 b1 = { p.x - f[i].dy * 4 - f[i].dx * 2, p.y + f[i].dx * 4 - f[i].dy * 2 };
        Vector2 b2 = { p.x + f[i].dy * 4 - f[i].dx * 2, p.y - f[i].dx * 4 - f[i].dy * 2 };
        DrawTriangle(punta, b1, b2, col);
        DrawTriangle(punta, b2, b1, col);
    }
    G_Cristallo((int)c.x, (int)c.y - 3, 7, true);
}

void Azioni_DisegnaTouch(const Azioni *a, const Canvas *c, int frame) {
    (void)frame;
    if (!(a->touchVisibile || a->forzaTouch) || a->contesto == TOUCH_NESSUNO || a->contesto == TOUCH_DIALOGO) return;
    Geometria g = Geo(a, c);
    unsigned char al = 120;
    Color fondo = { 13, 19, 48, al }, bordo = PAL_ORO[3], attivo = PAL_LUMEN[3];
    bordo.a = 200;
    if (a->contesto == TOUCH_ESPLORA) {
        Croce(g.croce, g.rCroce, a, fondo, bordo, attivo);
        DrawCircleV(g.a, g.rA, a->giu[AZ_CONFERMA] ? (Color){ 43, 196, 192, 170 } : fondo);
        DrawCircleLinesV(g.a, g.rA, bordo);
        T_Testo("A", (int)g.a.x - 2, (int)g.a.y - 3, PAL_CREMA);
        G_Rett((int)g.menu.x, (int)g.menu.y, (int)g.menu.width, (int)g.menu.height, a->giu[AZ_MENU] ? (Color){ 43, 196, 192, 170 } : fondo);
        DrawRectangleLines((int)g.menu.x, (int)g.menu.y, (int)g.menu.width, (int)g.menu.height, bordo);
        T_Micro("MENU", (int)g.menu.x + ((int)g.menu.width - T_LarghezzaMicro("MENU")) / 2, (int)g.menu.y + 3, PAL_CREMA);
    }
    DrawCircleV(g.b, g.rB, a->giu[AZ_ANNULLA] ? (Color){ 43, 196, 192, 170 } : fondo);
    DrawCircleLinesV(g.b, g.rB, bordo);
    T_Testo("B", (int)g.b.x - 2, (int)g.b.y - 3, PAL_CREMA);
}
