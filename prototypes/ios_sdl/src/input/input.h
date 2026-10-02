/* input.h - Azioni astratte (MULTIPLATFORM_ARCHITECTURE.md §3): tastiera, controller e touch
 * producono le stesse azioni. Il gioco non sa da dove arrivano. */
#ifndef PROTO_INPUT_H
#define PROTO_INPUT_H

#include <stdbool.h>
#include "raylib.h"
#include "../render/canvas.h"

typedef enum {
    INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT,
    INPUT_CONFIRM, INPUT_CANCEL, INPUT_MENU, INPUT_RUN,
    INPUT_ACTION_COUNT
} InputAction;

typedef enum { DEVICE_KEYBOARD, DEVICE_GAMEPAD, DEVICE_TOUCH } InputDevice;

#define MAX_DITA 4

typedef struct {
    float scale;          /* 0,75 - 1,5 */
    float opacity;        /* 0,2 - 0,8 */
    bool visible;
} TouchLayout;

typedef struct {
    bool down[INPUT_ACTION_COUNT];
    bool pressed[INPUT_ACTION_COUNT];
    bool released[INPUT_ACTION_COUNT];
    InputDevice lastDevice;
    int touchCount;                 /* dita appoggiate in questo frame */
    Vector2 touch[MAX_DITA];        /* in pixel logici del canvas */
    int maxTouchSeen;               /* massimo di dita contemporanee visto (test multitouch) */
    char gamepadName[64];
    TouchLayout layout;
    /* azioni iniettate dai test automatici, valgono un frame */
    bool injected[INPUT_ACTION_COUNT];
} InputState;

void Input_Init(InputState *s);
void Input_Update(InputState *s, const Canvas *c);
void Input_Inject(InputState *s, InputAction a);
const char *Input_ActionName(InputAction a);

/* Geometria dei controlli touch nel canvas logico (usata anche per disegnarli) */
typedef struct { Vector2 dpad; float dpadR; Vector2 a, b; float aR, bR; Rectangle menu; } TouchGeometry;
TouchGeometry Input_TouchGeometry(const InputState *s, const Canvas *c);

#endif
