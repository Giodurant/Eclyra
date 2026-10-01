/* game.c */
#include "game.h"
#include "graphics.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

static const char *VOCI_TITOLO[] = { "NUOVA PARTITA", "CONTINUA", "OPZIONI", "ESCI" };
static const char *VOCI_GIOCO[]  = { "ELYON", "ZAINO", "SCHEDA", "SALVA", "OPZIONI", "CHIUDI" };
static const char *NOMI_VELOCITA[] = { "LENTA", "MEDIA", "VELOCE" };
static char vociOpzioni[3][40];
static const char *VOCI_OPZIONI[3] = { vociOpzioni[0], vociOpzioni[1], vociOpzioni[2] };

static void AggiornaTestiOpzioni(Game *g) {
    snprintf(vociOpzioni[0], sizeof(vociOpzioni[0]), "TESTO: %s", NOMI_VELOCITA[g->opzioni.velocitaTesto]);
    snprintf(vociOpzioni[1], sizeof(vociOpzioni[1]), "SCHERMO INTERO: %s", g->opzioni.schermoIntero ? "SÌ" : "NO");
    snprintf(vociOpzioni[2], sizeof(vociOpzioni[2]), "INDIETRO");
}

bool Game_Init(Game *g) {
    *g = (Game){ 0 };
    if (!Gfx_Init("ECLYRA - Le Cronache dell'Equilibrio")) return false;
    Rng_Init(&g->rng, (uint32_t)time(NULL));
    g->opzioni.velocitaTesto = TESTO_MEDIO;

    Menu_Init(&g->menuTitolo, VOCI_TITOLO, 4);
    /* CONTINUA si abilita quando esiste un salvataggio (sistema di salvataggio: fase successiva) */
    g->menuTitolo.abilitata[1] = FileExists("saves/eclyra.sav");
    AggiornaTestiOpzioni(g);
    Menu_Init(&g->menuOpzioni, VOCI_OPZIONI, 3);
    Menu_Init(&g->menuGioco, VOCI_GIOCO, 6);

    g->scena = SCENA_TITOLO;
    g->dissolvenza = 1.0f;
    return true;
}

void Game_Shutdown(Game *g) {
    (void)g;
    Gfx_Shutdown();
}

/* ------------------------------------------------------------------
 * Logica (60 passi al secondo)
 * ------------------------------------------------------------------ */
static void ApriOpzioni(Game *g) {
    g->scenaPrimaOpzioni = g->scena;
    g->scena = SCENA_OPZIONI;
    g->menuOpzioni.cursore = 0;
}

static void PassoTitolo(Game *g) {
    g->frameTitolo++;
    if (g->dissolvenza > 0.0f) return;
    switch (Menu_Aggiorna(&g->menuTitolo, &g->input)) {
        case 0:   /* NUOVA PARTITA */
            World_Init(&g->world);
            g->scena = SCENA_MONDO;
            g->dissolvenza = 1.0f;
            break;
        case 1:   /* CONTINUA: disabilitato finche' non esiste un salvataggio */
            break;
        case 2:
            ApriOpzioni(g);
            break;
        case 3:
            g->esci = true;
            break;
        default:
            break;
    }
}

static void PassoOpzioni(Game *g) {
    Input *in = &g->input;
    int scelta = Menu_Aggiorna(&g->menuOpzioni, in);
    int cur = g->menuOpzioni.cursore;
    if (cur == 0 && (in->sinistraP || in->destraP || scelta == 0)) {
        int d = in->sinistraP ? -1 : 1;
        g->opzioni.velocitaTesto = (VelocitaTesto)((g->opzioni.velocitaTesto + d + 3) % 3);
    }
    if (cur == 1 && (in->sinistraP || in->destraP || scelta == 1)) {
        g->opzioni.schermoIntero = !g->opzioni.schermoIntero;
        Gfx_SchermoIntero(g->opzioni.schermoIntero);
    }
    AggiornaTestiOpzioni(g);
    if (scelta == 2 || scelta == -2) g->scena = g->scenaPrimaOpzioni;
}

static void PassoMondo(Game *g) {
    Input *in = &g->input;
    if (g->avviso) {
        if (in->conferma || in->indietro) g->avviso = NULL;
        return;
    }
    if (g->menuGiocoAperto) {
        int scelta = Menu_Aggiorna(&g->menuGioco, in);
        if (scelta == -2 || scelta == 5 || in->menu) g->menuGiocoAperto = false;
        else if (scelta == 4) { g->menuGiocoAperto = false; ApriOpzioni(g); }
        else if (scelta >= 0) g->avviso = "Questa funzione arriverà in una fase successiva dello sviluppo.";
        return;
    }
    if ((in->menu || in->indietro) && !g->world.player.inMovimento && !g->world.messaggio) {
        g->menuGiocoAperto = true;
        g->menuGioco.cursore = 0;
        return;
    }
    World_Aggiorna(&g->world, in);
}

void Game_Passo(Game *g) {
    if (g->dissolvenza > 0.0f) g->dissolvenza -= 1.0f / 30.0f;
    switch (g->scena) {
        case SCENA_TITOLO:  PassoTitolo(g); break;
        case SCENA_OPZIONI: PassoOpzioni(g); break;
        case SCENA_MONDO:   PassoMondo(g); break;
    }
    Input_ConsumaEventi(&g->input);
}

/* ------------------------------------------------------------------
 * Disegno
 * ------------------------------------------------------------------ */
static void DisegnaSfondoTitolo(const Game *g) {
    /* cielo notturno a fasce */
    static const Color fasce[6] = { {16, 16, 40, 255}, {20, 22, 56, 255}, {28, 30, 72, 255},
                                    {40, 40, 92, 255}, {56, 52, 112, 255}, {76, 64, 128, 255} };
    for (int i = 0; i < 6; i++) DrawRectangle(0, i * 30, LOGICAL_W, 30, fasce[i]);

    /* stelle che scintillano (posizioni fisse pseudo-casuali) */
    for (int i = 0; i < 60; i++) {
        unsigned h = (unsigned)(i * 2654435761u);
        int x = (int)(h % LOGICAL_W), y = (int)((h >> 9) % 120);
        if (((g->frameTitolo / 8) + i) % 9 != 0) DrawPixel(x, y, (Color){ 220, 220, 255, 255 });
    }
    /* particelle di Lumen che salgono */
    for (int i = 0; i < 24; i++) {
        unsigned h = (unsigned)(i * 40503u + 977u);
        int x = (int)(h % LOGICAL_W);
        int y = LOGICAL_H - (int)((g->frameTitolo * (1 + i % 3) / 2 + (int)(h % 400)) % (LOGICAL_H + 20));
        Color c = (i % 2) ? (Color){ 120, 240, 200, 200 } : (Color){ 180, 220, 255, 200 };
        DrawRectangle(x, y, 2, 2, c);
    }
    /* sagoma di colline */
    for (int x = 0; x < LOGICAL_W; x++) {
        int h = 150 + (int)(8 * sinf(x * 0.03f) + 5 * sinf(x * 0.071f + 1.3f));
        DrawLine(x, h, x, LOGICAL_H, (Color){ 20, 28, 44, 255 });
    }
}

static void DisegnaSigilli(const Game *g, int cx, int cy) {
    /* i sette Sigilli: cerchio di sette luci attorno al logo */
    for (int i = 0; i < 7; i++) {
        float a = (float)i / 7.0f * 6.2831853f + g->frameTitolo * 0.004f;
        int x = cx + (int)(cosf(a) * 92), y = cy + (int)(sinf(a) * 26);
        float pulse = 0.5f + 0.5f * sinf(g->frameTitolo * 0.05f + i);
        DrawCircle(x, y, 3, (Color){ 120, 240, 210, (unsigned char)(60 + 80 * pulse) });
        DrawRectangle(x, y, 1, 1, (Color){ 240, 255, 250, 255 });
    }
}

static void DisegnaTitolo(const Game *g) {
    DisegnaSfondoTitolo(g);
    DisegnaSigilli(g, LOGICAL_W / 2, 48);
    /* logo: ombra, contorno e testo */
    const char *logo = "ECLYRA";
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            Gfx_TestoCentrato(logo, LOGICAL_W / 2 + dx * 2, 28 + dy * 2, 40, (Color){ 24, 32, 72, 255 });
    Gfx_TestoCentrato(logo, LOGICAL_W / 2, 28, 40, (Color){ 248, 232, 160, 255 });
    Gfx_TestoCentrato("LE CRONACHE DELL'EQUILIBRIO", LOGICAL_W / 2, 74, 10, (Color){ 180, 220, 255, 255 });

    Menu_Disegna(&g->menuTitolo, LOGICAL_W / 2 - 60, 94, 120);
    DrawText("v" ECLYRA_VERSIONE, 4, LOGICAL_H - 10, 10, (Color){ 120, 120, 160, 255 });
}

static void DisegnaOpzioni(const Game *g) {
    if (g->scenaPrimaOpzioni == SCENA_MONDO) World_Disegna(&g->world);
    else DisegnaSfondoTitolo(g);
    Gfx_Finestra(40, 20, LOGICAL_W - 80, 24);
    Gfx_TestoCentrato("OPZIONI", LOGICAL_W / 2, 28, 10, COL_TESTO);
    Menu_Disegna(&g->menuOpzioni, 40, 52, LOGICAL_W - 80);
    Gfx_TestoCentrato("Frecce: cambia  -  Z: conferma  -  X: indietro", LOGICAL_W / 2, LOGICAL_H - 16, 10, COL_BIANCO_UI);
}

static void DisegnaMondo(const Game *g) {
    World_Disegna(&g->world);
    if (g->menuGiocoAperto) Menu_Disegna(&g->menuGioco, LOGICAL_W - 92, 4, 88);
    if (g->avviso) {
        Gfx_Finestra(4, LOGICAL_H - 48, LOGICAL_W - 8, 44);
        Gfx_Testo("Questa funzione arriverà in una fase\nsuccessiva dello sviluppo.", 14, LOGICAL_H - 38, COL_TESTO);
    }
}

void Game_Disegna(Game *g) {
    Gfx_InizioFrame();
    switch (g->scena) {
        case SCENA_TITOLO:  DisegnaTitolo(g); break;
        case SCENA_OPZIONI: DisegnaOpzioni(g); break;
        case SCENA_MONDO:   DisegnaMondo(g); break;
    }
    Gfx_Dissolvenza(g->dissolvenza);
    Gfx_FineFrame();
}

void Game_Frame(Game *g) {
    Input_Aggiorna(&g->input);
    g->accumulatore += GetFrameTime();
    if (g->accumulatore > 0.25f) g->accumulatore = 0.25f;
    while (g->accumulatore >= PASSO_LOGICO) {
        Game_Passo(g);
        g->accumulatore -= PASSO_LOGICO;
    }
    Game_Disegna(g);
    if (WindowShouldClose()) g->esci = true;
}
