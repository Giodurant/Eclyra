/* platform.h - Platform layer minimo del prototipo (anteprima di src/platform/ del gioco).
 *
 * Il resto del codice non sa su quale sistema gira: chiede tempo, cartelle, area sicura,
 * stato di pausa e il framebuffer dello schermo solo tramite queste funzioni.
 * Implementazione: platform_sdl.c (tutte le piattaforme SDL) + platform_ios.m (solo iOS). */
#ifndef PROTO_PLATFORM_H
#define PROTO_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>

typedef struct { int left, top, right, bottom; } SafeInsets;   /* pixel fisici */

typedef struct {
    int backgroundCount;      /* volte in cui l'app e' andata in background */
    int foregroundCount;      /* volte in cui e' tornata in primo piano */
    int lowMemoryCount;
    double pausedSeconds;     /* tempo totale passato in pausa */
    bool paused;              /* in background: niente logica, niente GL */
    bool autosaveOk;          /* l'ultimo salvataggio di sospensione e' riuscito */
} LifecycleStats;

/* Da chiamare PRIMA di InitWindow: suggerimenti per SDL (orientamento, touch, indicatore home) */
void Platform_PreInit(void);
/* Da chiamare DOPO InitWindow: ascolto degli eventi di ciclo di vita, framebuffer dello schermo */
void Platform_Init(void (*onPause)(void), void (*onResume)(void));

const char *Platform_Name(void);
bool Platform_IsMobile(void);

/* Dimensione reale in pixel della superficie di disegno (non i "punti" di iOS) */
void Platform_DrawableSize(int *w, int *h);
SafeInsets Platform_SafeArea(void);
/* Su iOS lo schermo non e' il framebuffer 0: va ribindato dopo ogni EndTextureMode() */
void Platform_BindScreenFramebuffer(void);
unsigned Platform_ScreenFramebufferId(void);

const LifecycleStats *Platform_Lifecycle(void);
/* Solo per i test automatici: simula l'andata in background e il ritorno */
void Platform_SimulateBackground(void);
void Platform_SimulateForeground(void);

/* Piccola vibrazione per i tasti touch (no-op dove non c'e') */
void Platform_Haptic(void);

/* ---- platform_time ---- */
double Time_Monotonic(void);       /* secondi, mai all'indietro */
long long Time_WallClock(void);    /* secondi Unix, per la data dei salvataggi */

/* ---- platform_storage ---- */
const char *Storage_SaveDir(void);                     /* termina con il separatore */
const char *Storage_AssetPath(const char *nome);       /* percorso assoluto nel bundle / cartella dell'eseguibile */
bool Storage_WriteAtomic(const char *nome, const void *dati, size_t n);
long Storage_Read(const char *nome, void *buf, size_t max);   /* -1 se manca */

#endif
