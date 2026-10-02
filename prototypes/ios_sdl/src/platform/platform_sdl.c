/* platform_sdl.c - Platform layer su SDL2: iOS, iPadOS, Linux, Steam Deck, Windows, macOS.
 * raylib usa SDL come backend (PLATFORM_DESKTOP_SDL), quindi SDL e' gia' inizializzata da InitWindow. */
#include "platform.h"

#include "SDL.h"
#include "SDL_syswm.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__APPLE__)
    #include <TargetConditionals.h>
#endif
#if defined(__APPLE__) && TARGET_OS_IPHONE
    #define PROTO_IOS 1
    void PlatformIOS_SafeArea(float *l, float *t, float *r, float *b);   /* platform_ios.m */
    void PlatformIOS_Haptic(void);
#endif

static LifecycleStats stato;
static void (*cbPausa)(void);
static void (*cbRipresa)(void);
static double inizioPausa;
static unsigned fboSchermo;
typedef void (*PfnBindFramebuffer)(unsigned target, unsigned fb);
static PfnBindFramebuffer glBindFramebufferPtr;

/* ------------------------------------------------------------------ tempo */

double Time_Monotonic(void) {
    static Uint64 freq;
    if (!freq) freq = SDL_GetPerformanceFrequency();
    return (double)SDL_GetPerformanceCounter() / (double)freq;
}

long long Time_WallClock(void) { return (long long)time(NULL); }

/* ------------------------------------------------------------------ ciclo di vita */

static void EntraInPausa(void) {
    if (stato.paused) return;
    stato.paused = true;
    stato.backgroundCount++;
    inizioPausa = Time_Monotonic();
    if (cbPausa) cbPausa();     /* salvataggio di sospensione e audio in pausa: subito, iOS puo' chiudere l'app */
}

static void Riprendi(void) {
    if (!stato.paused) return;
    stato.paused = false;
    stato.foregroundCount++;
    stato.pausedSeconds += Time_Monotonic() - inizioPausa;
    if (cbRipresa) cbRipresa();
}

/* Chiamata da SDL nel momento in cui arriva l'evento: su iOS gli eventi APP_* vanno gestiti qui,
 * non nel ciclo, perche' il sistema aspetta che l'app abbia finito prima di sospenderla. */
static int OsservaEventi(void *user, SDL_Event *e) {
    (void)user;
    switch (e->type) {
    case SDL_APP_WILLENTERBACKGROUND: EntraInPausa(); break;
    case SDL_APP_DIDENTERFOREGROUND:  Riprendi(); break;
    case SDL_APP_LOWMEMORY:           stato.lowMemoryCount++; break;
    case SDL_APP_TERMINATING:         if (cbPausa) cbPausa(); break;
    default: break;
    }
    return 1;
}

void Platform_SimulateBackground(void) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_APP_WILLENTERBACKGROUND; SDL_PushEvent(&e);
    e.type = SDL_APP_DIDENTERBACKGROUND;  SDL_PushEvent(&e);
}

void Platform_SimulateForeground(void) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_APP_WILLENTERFOREGROUND; SDL_PushEvent(&e);
    e.type = SDL_APP_DIDENTERFOREGROUND;  SDL_PushEvent(&e);
}

const LifecycleStats *Platform_Lifecycle(void) { return &stato; }

/* ------------------------------------------------------------------ init */

void Platform_PreInit(void) {
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");        /* un dito non deve diventare anche un clic */
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, "2");   /* il primo swipe dal basso non esce dal gioco */
    SDL_SetHint(SDL_HINT_IDLE_TIMER_DISABLED, "1");       /* lo schermo non si spegne mentre si gioca */
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0"); /* l'accelerometro non e' un controller */
}

void Platform_Init(void (*onPause)(void), void (*onResume)(void)) {
    cbPausa = onPause;
    cbRipresa = onResume;
    SDL_AddEventWatch(OsservaEventi, NULL);
    glBindFramebufferPtr = (PfnBindFramebuffer)SDL_GL_GetProcAddress("glBindFramebuffer");
    fboSchermo = 0;
#if defined(PROTO_IOS)
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &info) && info.subsystem == SDL_SYSWM_UIKIT)
        fboSchermo = info.info.uikit.framebuffer;
#endif
}

const char *Platform_Name(void) {
#if defined(PROTO_IOS)
    return SDL_GetPlatform();      /* "iOS" (anche su iPad) */
#else
    return SDL_GetPlatform();
#endif
}

bool Platform_IsMobile(void) {
#if defined(PROTO_IOS) || defined(__ANDROID__) || defined(PROTO_FORCE_MOBILE)   /* l'ultimo solo per provare il ramo mobile su Linux */
    return true;
#else
    return false;
#endif
}

void Platform_DrawableSize(int *w, int *h) {
    SDL_Window *win = SDL_GL_GetCurrentWindow();
    if (win) SDL_GL_GetDrawableSize(win, w, h);
    else { *w = 0; *h = 0; }
}

SafeInsets Platform_SafeArea(void) {
    SafeInsets s = { 0, 0, 0, 0 };
#if defined(PROTO_IOS)
    float l, t, r, b;
    PlatformIOS_SafeArea(&l, &t, &r, &b);
    s.left = (int)(l + 0.5f); s.top = (int)(t + 0.5f); s.right = (int)(r + 0.5f); s.bottom = (int)(b + 0.5f);
#endif
    return s;
}

void Platform_BindScreenFramebuffer(void) {
    if (glBindFramebufferPtr) glBindFramebufferPtr(0x8D40 /* GL_FRAMEBUFFER */, fboSchermo);
}

unsigned Platform_ScreenFramebufferId(void) { return fboSchermo; }

void Platform_Haptic(void) {
#if defined(PROTO_IOS)
    PlatformIOS_Haptic();
#endif
}

/* ------------------------------------------------------------------ storage */

const char *Storage_SaveDir(void) {
    static char dir[1024];
    if (!dir[0]) {
        char *p = SDL_GetPrefPath("Eclyra", "ECLYRA-Proto");   /* iOS: Library/Application Support/ */
        if (p) { SDL_strlcpy(dir, p, sizeof(dir)); SDL_free(p); }
        else SDL_strlcpy(dir, "./", sizeof(dir));
    }
    return dir;
}

static bool Esiste(const char *p) {
    FILE *f = fopen(p, "rb");
    if (f) fclose(f);
    return f != NULL;
}

const char *Storage_AssetPath(const char *nome) {
    static char base[1024], out[1280];
    if (!base[0]) {
        char *b = SDL_GetBasePath();     /* iOS: cartella del bundle; desktop: cartella dell'eseguibile */
        if (b) { SDL_strlcpy(base, b, sizeof(base)); SDL_free(b); }
        else SDL_strlcpy(base, "./", sizeof(base));
    }
    SDL_snprintf(out, sizeof(out), "%sassets/%s", base, nome);
    if (!Esiste(out)) SDL_snprintf(out, sizeof(out), "%s%s", base, nome);   /* bundle iOS "piatto" */
    return out;
}

bool Storage_WriteAtomic(const char *nome, const void *dati, size_t n) {
    char finale[1280], tmp[1300];
    SDL_snprintf(finale, sizeof(finale), "%s%s", Storage_SaveDir(), nome);
    SDL_snprintf(tmp, sizeof(tmp), "%s.tmp", finale);
    FILE *f = fopen(tmp, "wb");
    if (!f) return false;
    bool ok = fwrite(dati, 1, n, f) == n;
    ok = (fflush(f) == 0) && ok;
    ok = (fclose(f) == 0) && ok;
    if (!ok) { remove(tmp); return false; }
#if defined(_WIN32)
    remove(finale);                 /* su Windows rename non sovrascrive */
#endif
    return rename(tmp, finale) == 0;
}

long Storage_Read(const char *nome, void *buf, size_t max) {
    char p[1280];
    SDL_snprintf(p, sizeof(p), "%s%s", Storage_SaveDir(), nome);
    FILE *f = fopen(p, "rb");
    if (!f) return -1;
    size_t n = fread(buf, 1, max, f);
    fclose(f);
    return (long)n;
}
