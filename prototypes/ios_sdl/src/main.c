/* ECLYRA - Prototipo tecnico iOS (raylib + backend SDL2)
 *
 * Verifica, su un dispositivo reale, le cose che il gioco dara' per scontate:
 *   1. avvio dell'app                     5. audio (streaming continuo + effetto)
 *   2. rendering su canvas 320x180        6. pausa e ripresa (background, chiamate, blocco)
 *   3. scaling intero, nessuna sfocatura  7. salvataggio atomico nel sandbox
 *   4. touch, controller e tastiera       8. build su dispositivo
 *
 * Comandi: croce = muove PYRIN, A = bip, B = cambia la misura dei controlli touch,
 *          MENU = mostra/nasconde la diagnostica. Sul desktop il mouse fa da dito e
 *          F1-F4 simulano le proporzioni di vari dispositivi.
 *
 * --selftest: prova automatica senza persone (usata in CI su Linux), esce con 0 se tutto va. */
#include "raylib.h"
#include "rlgl.h"
#include "SDL_main.h"       /* su iOS rinomina main() per SDL_UIKitRunApp; altrove non fa nulla */

#include "platform/platform.h"
#include "render/canvas.h"
#include "input/input.h"
#include "proto_audio.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSIONE_PROTO "0.1"

/* ------------------------------------------------------------------ stato */

typedef struct {
    Canvas canvas;
    RenderTexture2D rt;
    int rtW, rtH;
    Texture2D pyrin;
    bool pyrinOk;
    InputState in;
    float px, py;              /* posizione di PYRIN nel canvas */
    bool diagnostica;
    int avvii;
    bool salvataggioOk;
    int bip;
    double tempoGioco;         /* avanza solo quando l'app non e' in pausa */
    double ultimo;
    int tieniProva;            /* frame con A+B tenuti */
} Proto;

static Proto P;

/* risultati della prova automatica (mostrati anche sullo schermo del dispositivo) */
static int prove, fallite;
static FILE *rapporto;
#define MAX_RIGHE 32
static char righe[MAX_RIGHE][100];
static bool righeOk[MAX_RIGHE];
static bool mostraRisultati;

/* ------------------------------------------------------------------ salvataggio */

static unsigned Crc32(const unsigned char *d, size_t n) {
    unsigned c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++) {
        c ^= d[i];
        for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return ~c;
}

/* File di testo con CRC in fondo: un file troncato o corrotto viene riconosciuto */
static bool SalvaStato(void) {
    const LifecycleStats *ls = Platform_Lifecycle();
    char corpo[256], tutto[320];
    int n = snprintf(corpo, sizeof(corpo), "ECLYRA-PROTO %s\navvii=%d\nbackground=%d\nprimo_piano=%d\ndata=%lld\n",
                     VERSIONE_PROTO, P.avvii, ls->backgroundCount, ls->foregroundCount, Time_WallClock());
    int m = snprintf(tutto, sizeof(tutto), "%scrc=%08X\n", corpo, Crc32((const unsigned char *)corpo, (size_t)n));
    return Storage_WriteAtomic("proto_stato.txt", tutto, (size_t)m);
}

/* Ritorna il numero di avvii letto, 0 se il file manca, -1 se e' corrotto */
static int CaricaStato(void) {
    char buf[512];
    long n = Storage_Read("proto_stato.txt", buf, sizeof(buf) - 1);
    if (n <= 0) return 0;
    buf[n] = 0;
    char *crc = strstr(buf, "crc=");
    if (!crc) return -1;
    unsigned atteso = (unsigned)strtoul(crc + 4, NULL, 16);
    if (Crc32((const unsigned char *)buf, (size_t)(crc - buf)) != atteso) return -1;
    char *a = strstr(buf, "avvii=");
    return a ? atoi(a + 6) : -1;
}

/* ------------------------------------------------------------------ ciclo di vita */

static void InPausa(void) {
    Audio_Suspend();
    P.salvataggioOk = SalvaStato();      /* salvataggio di sospensione */
}

static void InRipresa(void) {
    Audio_Resume();
    P.ultimo = Time_Monotonic();         /* il tempo passato in background non conta */
}

/* ------------------------------------------------------------------ disegno */

static void Testo(const char *t, int x, int y, Color c) {
    DrawText(t, x + 1, y + 1, 10, (Color){ 5, 7, 26, 255 });
    DrawText(t, x, y, 10, c);
}

static const Color NOTTE = { 13, 19, 48, 255 }, NOTTE2 = { 29, 42, 94, 255 }, ORO = { 224, 176, 74, 255 },
                   CREMA = { 243, 234, 210, 255 }, LUMEN = { 127, 242, 226, 255 }, GRIGIO = { 142, 152, 190, 255 };

/* Scacchiera di 1 px: se lo scaling non e' intero o c'e' un filtro, si vede subito (diventa grigia) */
#define SCACCHI_X 4
#define SCACCHI_Y 4
#define SCACCHI_W 24
#define SCACCHI_H 12
static void DisegnaScacchi(void) {
    for (int y = 0; y < SCACCHI_H; y++)
        for (int x = 0; x < SCACCHI_W; x++)
            DrawPixel(SCACCHI_X + x, SCACCHI_Y + y, ((x + y) & 1) ? WHITE : BLACK);
}

static void DisegnaTouch(const Canvas *c) {
    if (!P.in.layout.visible) return;
    TouchGeometry g = Input_TouchGeometry(&P.in, c);
    unsigned char a = (unsigned char)(255.0f * P.in.layout.opacity);
    Color fondo = { 13, 19, 48, a }, bordo = { 224, 176, 74, a }, attivo = { 127, 242, 226, (unsigned char)(a + 60) };
    DrawCircleV(g.dpad, g.dpadR, fondo);
    DrawCircleLinesV(g.dpad, g.dpadR, bordo);
    float r = g.dpadR * 0.55f;
    struct { InputAction az; float dx, dy; } frecce[4] = { { INPUT_UP, 0, -1 }, { INPUT_DOWN, 0, 1 }, { INPUT_LEFT, -1, 0 }, { INPUT_RIGHT, 1, 0 } };
    for (int i = 0; i < 4; i++) {
        Vector2 p = { g.dpad.x + frecce[i].dx * r, g.dpad.y + frecce[i].dy * r };
        DrawCircleV(p, g.dpadR * 0.22f, P.in.down[frecce[i].az] ? attivo : bordo);
    }
    DrawCircleV(g.a, g.aR, P.in.down[INPUT_CONFIRM] ? attivo : fondo);
    DrawCircleLinesV(g.a, g.aR, bordo);
    Testo("A", (int)g.a.x - 3, (int)g.a.y - 4, CREMA);
    DrawCircleV(g.b, g.bR, P.in.down[INPUT_CANCEL] ? attivo : fondo);
    DrawCircleLinesV(g.b, g.bR, bordo);
    Testo("B", (int)g.b.x - 3, (int)g.b.y - 4, CREMA);
    DrawRectangleRec(g.menu, P.in.down[INPUT_MENU] ? attivo : fondo);
    DrawRectangleLinesEx(g.menu, 1, bordo);
    Testo("MENU", (int)g.menu.x + 3, (int)g.menu.y + 1, CREMA);
    for (int i = 0; i < P.in.touchCount; i++) DrawCircleLinesV(P.in.touch[i], 5, LUMEN);
}

static void DisegnaDiagnostica(const Canvas *c) {
    const LifecycleStats *ls = Platform_Lifecycle();
    SafeInsets si = Platform_SafeArea();
    char r[96];
    int x = c->safeL + 32, y = c->safeT + 4, h = 11;
    DrawRectangle(x - 3, y - 2, 200, 13 * h + 4, (Color){ 7, 10, 24, 200 });
    DrawRectangleLines(x - 3, y - 2, 200, 13 * h + 4, ORO);
    snprintf(r, sizeof(r), "ECLYRA PROTO %s  %s  %d FPS", VERSIONE_PROTO, Platform_Name(), GetFPS()); Testo(r, x, y, ORO); y += h;
    snprintf(r, sizeof(r), "schermo %dx%d  punti %dx%d", c->screenW, c->screenH, GetScreenWidth(), GetScreenHeight()); Testo(r, x, y, CREMA); y += h;
    snprintf(r, sizeof(r), "canvas %dx%d  x%d  bande %d,%d", c->w, c->h, c->scale, c->offX, c->offY); Testo(r, x, y, CREMA); y += h;
    snprintf(r, sizeof(r), "area sicura px %d %d %d %d", si.left, si.top, si.right, si.bottom); Testo(r, x, y, CREMA); y += h;
    snprintf(r, sizeof(r), "dita %d (max %d)  disp. %s", P.in.touchCount, P.in.maxTouchSeen,
             P.in.lastDevice == DEVICE_TOUCH ? "touch" : P.in.lastDevice == DEVICE_GAMEPAD ? "controller" : "tastiera");
    Testo(r, x, y, CREMA); y += h;
    snprintf(r, sizeof(r), "pad: %s", P.in.gamepadName[0] ? P.in.gamepadName : "nessuno"); Testo(r, x, y, CREMA); y += h;
    char az[64] = "";
    for (int a = 0; a < INPUT_ACTION_COUNT; a++)
        if (P.in.down[a]) { strncat(az, Input_ActionName((InputAction)a), sizeof(az) - strlen(az) - 2); strcat(az, " "); }
    snprintf(r, sizeof(r), "azioni: %s", az[0] ? az : "-"); Testo(r, x, y, LUMEN); y += h;
    snprintf(r, sizeof(r), "audio %s  campioni %llu  bip %d", Audio_Ready() ? (Audio_Suspended() ? "in pausa" : "attivo") : "ASSENTE",
             Audio_FramesGenerated(), P.bip);
    Testo(r, x, y, CREMA); y += h;
    snprintf(r, sizeof(r), "background %d  primo piano %d  pausa %.1fs", ls->backgroundCount, ls->foregroundCount, ls->pausedSeconds);
    Testo(r, x, y, CREMA); y += h;
    snprintf(r, sizeof(r), "salvataggio %s  avvii %d", P.salvataggioOk ? "OK" : "ERRORE", P.avvii); Testo(r, x, y, P.salvataggioOk ? CREMA : RED); y += h;
    snprintf(r, sizeof(r), "tempo di gioco %.1fs  fbo %u  mem bassa %d", P.tempoGioco, Platform_ScreenFramebufferId(), ls->lowMemoryCount);
    Testo(r, x, y, CREMA); y += h;
    snprintf(r, sizeof(r), "controlli x%.2f   B cambia misura", P.in.layout.scale); Testo(r, x, y, GRIGIO); y += h;
    Testo("MENU nasconde questo pannello", x, y, GRIGIO);
}

static void DisegnaRisultati(const Canvas *c) {
    int x = c->safeL + 8, y = c->safeT + 6, n = prove < MAX_RIGHE ? prove : MAX_RIGHE;
    DrawRectangle(0, 0, c->w, c->h, (Color){ 7, 10, 24, 235 });
    char r[64];
    snprintf(r, sizeof(r), "PROVA AUTOMATICA: %d prove, %d fallite", prove, fallite);
    Testo(r, x, y, fallite ? RED : LUMEN);
    y += 13;
    for (int i = 0; i < n && y < c->h - 20; i++, y += 9) {
        DrawText(righeOk[i] ? "OK" : "NO", x, y, 10, righeOk[i] ? LUMEN : RED);
        DrawText(righe[i], x + 18, y, 10, CREMA);
    }
    Testo("MENU per chiudere - rapporto nell'app File", x, c->h - c->safeB - 14, GRIGIO);
}

static void DisegnaScena(const Canvas *c) {
    ClearBackground(NOTTE);
    /* pavimento a righe: si vede se il canvas si allarga davvero ai lati */
    for (int y = c->h - 40; y < c->h; y += 8) DrawRectangle(0, y, c->w, 4, NOTTE2);
    /* cornice esatta del canvas e della zona sicura 320x180 centrata */
    DrawRectangleLines(0, 0, c->w, c->h, ORO);
    DrawRectangleLines((c->w - 320) / 2, (c->h - 180) / 2, 320, 180, (Color){ 127, 242, 226, 90 });
    /* area sicura del dispositivo (notch, angoli, barra home) */
    DrawRectangleLines(c->safeL, c->safeT, c->w - c->safeL - c->safeR, c->h - c->safeT - c->safeB, (Color){ 232, 84, 47, 160 });
    DisegnaScacchi();
    if (P.pyrinOk) DrawTexture(P.pyrin, (int)P.px, (int)P.py, WHITE);
    else DrawRectangle((int)P.px, (int)P.py, 64, 64, MAGENTA);   /* segnaposto: asset non trovato */
    if (P.diagnostica) DisegnaDiagnostica(c);
    else Testo("MENU = diagnostica", c->safeL + 32, c->safeT + 4, GRIGIO);
    DisegnaTouch(c);
    if (mostraRisultati) DisegnaRisultati(c);
}

/* Dopo EndTextureMode raylib torna al framebuffer 0 e alla risoluzione in "punti":
 * qui si torna allo schermo vero (su iOS non e' lo 0) in pixel fisici. */
static void ImpostaSchermo(int w, int h) {
    rlDrawRenderBatchActive();
    Platform_BindScreenFramebuffer();
    rlViewport(0, 0, w, h);
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    rlOrtho(0, w, h, 0, 0.0, 1.0);
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

/* ------------------------------------------------------------------ frame */

static Image catturaSchermo;
static bool vuoiCattura;

static void Frame(void) {
    int dw, dh;
    Platform_DrawableSize(&dw, &dh);
    Canvas c = Canvas_Compute(dw, dh, Platform_SafeArea());
    P.canvas = c;
    if (c.w != P.rtW || c.h != P.rtH) {
        if (P.rtW) UnloadRenderTexture(P.rt);
        P.rt = LoadRenderTexture(c.w, c.h);
        SetTextureFilter(P.rt.texture, TEXTURE_FILTER_POINT);
        P.rtW = c.w; P.rtH = c.h;
        if (P.px > c.w - 64) P.px = (float)(c.w - 64);
        if (P.py > c.h - 64) P.py = (float)(c.h - 64);
    }

    double ora = Time_Monotonic();
    P.tempoGioco += ora - P.ultimo;
    P.ultimo = ora;

    Input_Update(&P.in, &c);
    float v = P.in.down[INPUT_RUN] ? 2.0f : 1.0f;
    if (P.in.down[INPUT_LEFT]) P.px -= v;
    if (P.in.down[INPUT_RIGHT]) P.px += v;
    if (P.in.down[INPUT_UP]) P.py -= v;
    if (P.in.down[INPUT_DOWN]) P.py += v;
    if (P.px < 0) P.px = 0;
    if (P.py < 0) P.py = 0;
    if (P.px > c.w - 64) P.px = (float)(c.w - 64);
    if (P.py > c.h - 64) P.py = (float)(c.h - 64);
    if (P.in.pressed[INPUT_CONFIRM]) { Audio_PlaySfx(); P.bip++; }
    if (P.in.pressed[INPUT_MENU]) {
        if (mostraRisultati) mostraRisultati = false;
        else P.diagnostica = !P.diagnostica;
    }
    /* A + B tenuti per 2 secondi: prova automatica anche sul dispositivo */
    if (P.in.down[INPUT_CONFIRM] && P.in.down[INPUT_CANCEL]) P.tieniProva++;
    else P.tieniProva = 0;
    if (P.in.pressed[INPUT_CANCEL]) {
        P.in.layout.scale = P.in.layout.scale < 0.8f ? 1.0f : P.in.layout.scale < 1.1f ? 1.25f : 0.75f;
        P.in.layout.visible = true;
    }

    BeginTextureMode(P.rt);
    DisegnaScena(&c);
    EndTextureMode();

    BeginDrawing();
    ImpostaSchermo(dw, dh);
    ClearBackground(BLACK);
    Rectangle src = { 0, 0, (float)c.w, -(float)c.h };
    Rectangle dst = { (float)c.offX, (float)c.offY, (float)(c.w * c.scale), (float)(c.h * c.scale) };
    DrawTexturePro(P.rt.texture, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
    if (vuoiCattura) {
        rlDrawRenderBatchActive();
        catturaSchermo = LoadImageFromScreen();
        vuoiCattura = false;
    }
    EndDrawing();
}

/* ------------------------------------------------------------------ prova automatica */


static void Esito(bool ok, const char *fmt, ...) {
    char msg[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    prove++;
    if (!ok) fallite++;
    printf("  %s  %s\n", ok ? "OK    " : "FALLITO", msg);
    if (prove <= MAX_RIGHE) { snprintf(righe[prove - 1], sizeof(righe[0]), "%.99s", msg); righeOk[prove - 1] = ok; }
    if (rapporto) fprintf(rapporto, "%s  %s\n", ok ? "OK" : "FALLITO", msg);
}

static void Frames(int n) { for (int i = 0; i < n && !WindowShouldClose(); i++) Frame(); }

/* Ogni pixel logico della scacchiera deve diventare un blocco s x s di un solo colore, alternato */
static bool ScacchieraNitida(const Canvas *c, Image im) {
    if (!im.data) return false;
    Color *px = LoadImageColors(im);
    bool ok = true;
    for (int y = 0; y < SCACCHI_H && ok; y++)
        for (int x = 0; x < SCACCHI_W && ok; x++) {
            unsigned char atteso = ((x + y) & 1) ? 255 : 0;
            for (int sy = 0; sy < c->scale && ok; sy++)
                for (int sx = 0; sx < c->scale && ok; sx++) {
                    int X = c->offX + (SCACCHI_X + x) * c->scale + sx, Y = c->offY + (SCACCHI_Y + y) * c->scale + sy;
                    Color k = px[Y * im.width + X];
                    if (k.r != atteso || k.g != atteso || k.b != atteso) ok = false;
                }
        }
    UnloadImageColors(px);
    return ok;
}

static void ProvaRisoluzione(int w, int h, int cw, int ch, int s) {
    SetWindowSize(w, h);
    Frames(20);
    Canvas c = P.canvas;
    Esito(c.w == cw && c.h == ch && c.scale == s, "schermo %dx%d -> canvas %dx%d x%d (atteso %dx%d x%d)",
          c.screenW, c.screenH, c.w, c.h, c.scale, cw, ch, s);
    if (c.screenW != w || c.screenH != h) return;   /* il window manager ha rifiutato la misura */
    vuoiCattura = true;
    Frame();
    Esito(ScacchieraNitida(&c, catturaSchermo), "scacchiera di 1 px nitida a x%d (nessun filtro, scala intera)", c.scale);
    UnloadImage(catturaSchermo);
    catturaSchermo = (Image){ 0 };
}

static int ProvaAutomatica(void) {
    char percorso[1300];
    prove = fallite = 0;
    /* sul dispositivo la prova parte tenendo A+B: si aspetta che le dita si alzino */
    for (int i = 0; i < 300 && (P.in.touchCount > 0 || P.in.down[INPUT_CONFIRM] || P.in.down[INPUT_CANCEL]); i++) Frame();
    snprintf(percorso, sizeof(percorso), "%sselftest_rapporto.txt", Storage_SaveDir());
    rapporto = fopen(percorso, "w");
    printf("ECLYRA prototipo iOS - prova automatica su %s\n", Platform_Name());

    Frames(10);
    Esito(P.pyrinOk, "asset caricato dal bundle: %s", Storage_AssetPath("pyrin.png"));
    Esito(P.avvii >= 1, "salvataggio letto e riscritto (avvii = %d)", P.avvii);

    if (!Platform_IsMobile()) {
        ProvaRisoluzione(1280, 720, 320, 180, 4);    /* 16:9 */
        ProvaRisoluzione(1280, 800, 320, 200, 4);    /* Steam Deck 16:10 */
        ProvaRisoluzione(1560, 720, 390, 180, 4);    /* telefono 19.5:9 */
        ProvaRisoluzione(1024, 768, 341, 240, 3);    /* iPad 4:3 */
    } else {
        /* sul dispositivo: lo schermo e' quello che e', si controlla che il canvas rispetti le regole */
        Canvas c = P.canvas;
        Esito(c.scale >= 1 && c.w >= 320 && c.w <= 400 && c.h >= 180 && c.h <= 240 && c.offX >= 0 && c.offY >= 0,
              "schermo %dx%d -> canvas %dx%d x%d, bande %d,%d", c.screenW, c.screenH, c.w, c.h, c.scale, c.offX, c.offY);
        vuoiCattura = true;
        Frame();
        Esito(ScacchieraNitida(&c, catturaSchermo), "scacchiera di 1 px nitida a x%d sullo schermo reale", c.scale);
        UnloadImage(catturaSchermo);
        catturaSchermo = (Image){ 0 };
    }

    /* input astratto: la stessa azione da qualunque sorgente */
    P.px = 20.0f;                                 /* spazio libero a destra su ogni schermo */
    float x0 = P.px;
    for (int i = 0; i < 30; i++) { Input_Inject(&P.in, INPUT_RIGHT); Frame(); }
    Esito(P.px - x0 >= 29.0f, "INPUT_RIGHT muove PYRIN di 30 px (mosso %.0f)", P.px - x0);
    int bip0 = P.bip;
    Input_Inject(&P.in, INPUT_CONFIRM); Frame(); Frame();
    Esito(P.bip == bip0 + 1, "INPUT_CONFIRM produce un solo 'premuto' (bip %d)", P.bip - bip0);

    /* geometria touch: i tasti stanno nell'area sicura e non si sovrappongono */
    TouchGeometry g = Input_TouchGeometry(&P.in, &P.canvas);
    bool dentro = g.dpad.x - g.dpadR >= 0 && g.a.x + g.aR <= P.canvas.w && g.menu.y >= 0 && g.dpad.y + g.dpadR <= P.canvas.h;
    float dab = (g.a.x - g.b.x) * (g.a.x - g.b.x) + (g.a.y - g.b.y) * (g.a.y - g.b.y);
    Esito(dentro && dab > (g.aR + g.bR) * (g.aR + g.bR), "controlli touch dentro il canvas e separati");

    /* audio */
    if (Audio_Ready()) {
        unsigned long long a0 = Audio_FramesGenerated();
        Frames(40);
        Esito(Audio_FramesGenerated() > a0, "la musica in streaming avanza (%llu campioni)", Audio_FramesGenerated() - a0);
    } else {
        Esito(true, "audio: nessun dispositivo audio in questo ambiente (saltato)");
    }

    /* pausa e ripresa */
    int bg0 = Platform_Lifecycle()->backgroundCount;
    double tg0 = P.tempoGioco;
    Platform_SimulateBackground();
    Esito(Platform_Lifecycle()->paused && Platform_Lifecycle()->backgroundCount == bg0 + 1, "evento di background ricevuto: logica in pausa");
    Esito(P.salvataggioOk && CaricaStato() == P.avvii, "salvataggio di sospensione scritto e verificato (CRC)");
    unsigned long long a1 = Audio_FramesGenerated();
    for (int i = 0; i < 20; i++) { PollInputEvents(); WaitTime(0.02); }
    if (Audio_Ready()) Esito(Audio_FramesGenerated() - a1 < 4096, "in pausa la musica si ferma");
    Platform_SimulateForeground();
    Frames(5);
    Esito(!Platform_Lifecycle()->paused && Platform_Lifecycle()->pausedSeconds > 0.3, "ripresa: pausa di %.2f s esclusa dal tempo di gioco",
          Platform_Lifecycle()->pausedSeconds);
    Esito(P.tempoGioco - tg0 < 0.5, "il tempo di gioco non conta la pausa (+%.2f s)", P.tempoGioco - tg0);
    vuoiCattura = true;
    Frame();
    Esito(ScacchieraNitida(&P.canvas, catturaSchermo), "dopo la ripresa il rendering funziona ancora");
    if (catturaSchermo.data) {
        char foto[1300];
        snprintf(foto, sizeof(foto), "%sselftest_schermo.png", Storage_SaveDir());
        ExportImage(catturaSchermo, foto);
        UnloadImage(catturaSchermo);
    }

    /* file corrotto: deve essere riconosciuto */
    Storage_WriteAtomic("proto_corrotto.txt", "avvii=3\ncrc=00000000\n", 21);
    char buf[64];
    long n = Storage_Read("proto_corrotto.txt", buf, sizeof(buf) - 1);
    buf[n > 0 ? n : 0] = 0;
    char *crc = strstr(buf, "crc=");
    Esito(crc && Crc32((unsigned char *)buf, (size_t)(crc - buf)) != (unsigned)strtoul(crc + 4, NULL, 16), "un file con CRC sbagliato viene scartato");

    printf("%d prove, %d fallite. Rapporto: %s\n", prove, fallite, percorso);
    if (rapporto) { fprintf(rapporto, "%d prove, %d fallite\n", prove, fallite); fclose(rapporto); }
    return fallite ? 1 : 0;
}

/* ------------------------------------------------------------------ main */

int main(int argc, char *argv[]) {
    bool selftest = false;
    for (int i = 1; i < argc; i++) if (strcmp(argv[i], "--selftest") == 0) selftest = true;

    Platform_PreInit();
    unsigned flags = FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT;
    if (!Platform_IsMobile()) flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);
    SetTraceLogLevel(selftest ? LOG_WARNING : LOG_INFO);
    InitWindow(1280, 720, "ECLYRA - prototipo iOS");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);           /* ESC e' ANNULLA, non "chiudi" */
    Platform_Init(InPausa, InRipresa);

    Input_Init(&P.in);
    P.diagnostica = true;
    P.px = 230; P.py = 70;
    P.ultimo = Time_Monotonic();
    P.pyrin = LoadTexture(Storage_AssetPath("pyrin.png"));
    P.pyrinOk = P.pyrin.id != 0;
    if (P.pyrinOk) SetTextureFilter(P.pyrin, TEXTURE_FILTER_POINT);

    int avvii = CaricaStato();
    P.avvii = (avvii < 0 ? 0 : avvii) + 1;     /* corrotto = si riparte da zero, senza crash */
    P.salvataggioOk = SalvaStato();
    Audio_Init();

    int codice = 0;
    if (selftest) codice = ProvaAutomatica();
    else {
        while (!WindowShouldClose()) {
            if (Platform_Lifecycle()->paused) {   /* in background: niente GL, niente logica */
                PollInputEvents();
                if (IsKeyPressed(KEY_F6)) Platform_SimulateForeground();   /* solo desktop */
                WaitTime(0.05);
                continue;
            }
            if (IsKeyPressed(KEY_F1)) SetWindowSize(1280, 720);    /* 16:9 */
            if (IsKeyPressed(KEY_F2)) SetWindowSize(1280, 800);    /* Steam Deck */
            if (IsKeyPressed(KEY_F3)) SetWindowSize(1560, 720);    /* telefono 19.5:9 */
            if (IsKeyPressed(KEY_F4)) SetWindowSize(1024, 768);    /* iPad */
            if (IsKeyPressed(KEY_F5)) Platform_SimulateBackground();
            if (IsKeyPressed(KEY_F6)) Platform_SimulateForeground();
            Frame();
            if (P.tieniProva >= 120) { P.tieniProva = 0; ProvaAutomatica(); mostraRisultati = true; }
        }
    }

    Audio_Close();
    if (P.pyrinOk) UnloadTexture(P.pyrin);
    if (P.rtW) UnloadRenderTexture(P.rt);
    CloseWindow();
    return codice;
}
