/* input.c - Tastiera, controller e touch -> InputAction */
#include "input.h"
#include "../platform/platform.h"
#include <math.h>
#include <string.h>

static const char *NOMI[INPUT_ACTION_COUNT] = { "SU", "GIU", "SINISTRA", "DESTRA", "CONFERMA", "ANNULLA", "MENU", "CORSA" };

const char *Input_ActionName(InputAction a) { return (a >= 0 && a < INPUT_ACTION_COUNT) ? NOMI[a] : "?"; }

void Input_Init(InputState *s) {
    memset(s, 0, sizeof(*s));
    s->layout.scale = 1.0f;
    s->layout.opacity = 0.45f;
    s->layout.visible = true;
    s->lastDevice = Platform_IsMobile() ? DEVICE_TOUCH : DEVICE_KEYBOARD;
}

void Input_Inject(InputState *s, InputAction a) { s->injected[a] = true; }

/* Croce a sinistra, A e B a destra, MENU in alto a destra: tutto dentro l'area sicura */
TouchGeometry Input_TouchGeometry(const InputState *s, const Canvas *c) {
    TouchGeometry g;
    float k = s->layout.scale;
    float margine = 6.0f;
    g.dpadR = 22.0f * k;
    g.aR = 12.0f * k;
    g.bR = 10.0f * k;
    g.dpad = (Vector2){ (float)c->safeL + margine + g.dpadR, (float)(c->h - c->safeB) - margine - g.dpadR };
    float dx = (float)(c->w - c->safeR) - margine;
    float basso = (float)(c->h - c->safeB) - margine;
    g.a = (Vector2){ dx - g.aR, basso - g.aR - 8.0f * k };
    g.b = (Vector2){ dx - 2.0f * g.aR - 6.0f * k - g.bR, basso - g.bR };
    float mw = 30.0f * k, mh = 11.0f * k;
    g.menu = (Rectangle){ dx - mw, (float)c->safeT + margine, mw, mh };
    return g;
}

static float Dist(Vector2 a, Vector2 b) { return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)); }

static void LeggiTouch(InputState *s, const Canvas *c, bool down[INPUT_ACTION_COUNT]) {
    s->touchCount = 0;
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    if (sw <= 0 || sh <= 0) return;
    if (Platform_IsMobile()) {
        int n = GetTouchPointCount();
        for (int i = 0; i < n && s->touchCount < MAX_DITA; i++) {
            Vector2 p = GetTouchPosition(i);      /* punti dello schermo: si riportano ai pixel fisici */
            p.x = p.x / (float)sw * (float)c->screenW;
            p.y = p.y / (float)sh * (float)c->screenH;
            s->touch[s->touchCount++] = Canvas_FromScreen(c, p);
        }
    } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        /* sul desktop il mouse fa da dito, per provare i controlli touch senza telefono */
        Vector2 p = GetMousePosition();
        p.x = p.x / (float)sw * (float)c->screenW;
        p.y = p.y / (float)sh * (float)c->screenH;
        s->touch[s->touchCount++] = Canvas_FromScreen(c, p);
    }
    if (s->touchCount > s->maxTouchSeen) s->maxTouchSeen = s->touchCount;
    if (!s->layout.visible) return;

    TouchGeometry g = Input_TouchGeometry(s, c);
    for (int i = 0; i < s->touchCount; i++) {
        Vector2 p = s->touch[i];
        /* la croce si aggancia in un'area piu' grande del disegno (raggio x 1,4) */
        if (Dist(p, g.dpad) <= g.dpadR * 1.4f) {
            float dx = p.x - g.dpad.x, dy = p.y - g.dpad.y;
            float zonaMorta = g.dpadR * 0.25f;
            if (fabsf(dx) > zonaMorta || fabsf(dy) > zonaMorta) {
                float ang = atan2f(dy, dx) * 57.2958f;   /* 8 settori da 45 gradi: diagonali = due direzioni */
                if (ang > -67.5f && ang < 67.5f) down[INPUT_RIGHT] = true;
                if (ang > 112.5f || ang < -112.5f) down[INPUT_LEFT] = true;
                if (ang > 22.5f && ang < 157.5f) down[INPUT_DOWN] = true;
                if (ang < -22.5f && ang > -157.5f) down[INPUT_UP] = true;
            }
        } else if (Dist(p, g.a) <= g.aR * 1.3f) down[INPUT_CONFIRM] = true;
        else if (Dist(p, g.b) <= g.bR * 1.3f) down[INPUT_CANCEL] = true;
        else if (CheckCollisionPointRec(p, g.menu)) down[INPUT_MENU] = true;
    }
}

void Input_Update(InputState *s, const Canvas *c) {
    bool down[INPUT_ACTION_COUNT] = { false };
    bool tastiera = false, pad = false;

    /* tastiera (Steam / desktop, tastiere Bluetooth su iPad) */
    static const int K[INPUT_ACTION_COUNT][3] = {
        { KEY_W, KEY_UP, 0 }, { KEY_S, KEY_DOWN, 0 }, { KEY_A, KEY_LEFT, 0 }, { KEY_D, KEY_RIGHT, 0 },
        { KEY_Z, KEY_ENTER, KEY_SPACE }, { KEY_X, KEY_ESCAPE, KEY_BACKSPACE }, { KEY_TAB, KEY_M, 0 },
        { KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT, 0 } };
    for (int a = 0; a < INPUT_ACTION_COUNT; a++)
        for (int k = 0; k < 3; k++)
            if (K[a][k] && IsKeyDown(K[a][k])) { down[a] = true; tastiera = true; }

    /* controller: tasti per posizione, quindi Xbox, PlayStation, MFi e generici sono uguali */
    s->gamepadName[0] = 0;
    if (IsGamepadAvailable(0)) {
        const char *n = GetGamepadName(0);
        if (n) { strncpy(s->gamepadName, n, sizeof(s->gamepadName) - 1); s->gamepadName[sizeof(s->gamepadName) - 1] = 0; }
        static const int B[INPUT_ACTION_COUNT] = {
            GAMEPAD_BUTTON_LEFT_FACE_UP, GAMEPAD_BUTTON_LEFT_FACE_DOWN, GAMEPAD_BUTTON_LEFT_FACE_LEFT,
            GAMEPAD_BUTTON_LEFT_FACE_RIGHT, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
            GAMEPAD_BUTTON_MIDDLE_RIGHT, GAMEPAD_BUTTON_RIGHT_FACE_LEFT };
        for (int a = 0; a < INPUT_ACTION_COUNT; a++) if (IsGamepadButtonDown(0, B[a])) { down[a] = true; pad = true; }
        float ax = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X), ay = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        if (ax < -0.25f) { down[INPUT_LEFT] = true; pad = true; }
        if (ax > 0.25f) { down[INPUT_RIGHT] = true; pad = true; }
        if (ay < -0.25f) { down[INPUT_UP] = true; pad = true; }
        if (ay > 0.25f) { down[INPUT_DOWN] = true; pad = true; }
    }

    bool primaTouch[INPUT_ACTION_COUNT];
    memcpy(primaTouch, down, sizeof(down));
    LeggiTouch(s, c, down);
    bool touch = memcmp(primaTouch, down, sizeof(down)) != 0 || s->touchCount > 0;

    for (int a = 0; a < INPUT_ACTION_COUNT; a++) {
        if (s->injected[a]) { down[a] = true; s->injected[a] = false; }
        s->pressed[a] = down[a] && !s->down[a];
        s->released[a] = !down[a] && s->down[a];
        if (s->pressed[a] && touch && !tastiera && !pad) Platform_Haptic();
        s->down[a] = down[a];
    }
    if (tastiera) s->lastDevice = DEVICE_KEYBOARD;
    else if (pad) s->lastDevice = DEVICE_GAMEPAD;
    else if (touch) s->lastDevice = DEVICE_TOUCH;
    /* i controlli touch si nascondono quando si usa un controller o una tastiera */
    if (tastiera || pad) s->layout.visible = false;
    else if (s->touchCount > 0) s->layout.visible = true;   /* un tocco li fa ricomparire */
}
