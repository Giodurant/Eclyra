/* game.c */
#include "game.h"
#include "graphics.h"
#include "raylib.h"
#include "ui_common.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
    g->lotta = calloc(1, sizeof(BattleUI));
    if (!g->lotta) return false;
    Rng_Init(&g->rng, (uint32_t)time(NULL));
    g->opzioni.velocitaTesto = TESTO_MEDIO;

    Menu_Init(&g->menuTitolo, VOCI_TITOLO, 4);
    /* CONTINUA si abilita quando esiste un salvataggio (sistema di salvataggio: fase successiva) */
    g->menuTitolo.abilitata[1] = FileExists("saves/eclyra.sav");
    AggiornaTestiOpzioni(g);
    Menu_Init(&g->menuOpzioni, VOCI_OPZIONI, 3);
    Menu_Init(&g->menuGioco, VOCI_GIOCO, 6);

    char errore[200];
    if (!DialogoDb_CaricaFile(&g->dialoghi, TextFormat("%s/text/dialoghi.txt", Gfx_CartellaAsset()), errore, sizeof(errore)))
        TraceLog(LOG_WARNING, "ECLYRA: dialoghi non caricati (%s)", errore);

    g->scena = SCENA_TITOLO;
    g->dissolvenza = 1.0f;
    return true;
}

void Game_Shutdown(Game *g) {
    DialogoDb_Libera(&g->dialoghi);
    free(g->lotta);
    g->lotta = NULL;
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
            Partita_Nuova(&g->partita, Rng_Next(&g->rng));
            World_Init(&g->world, &g->partita, &g->dialoghi);
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

static float CaratteriPerPasso(const Game *g) {
    switch (g->opzioni.velocitaTesto) {
        case TESTO_LENTO:  return 0.25f;   /* 15 caratteri al secondo */
        case TESTO_MEDIO:  return 0.6f;    /* 36 caratteri al secondo */
        default:           return 2.0f;    /* quasi istantaneo */
    }
}

static void AvviaLottaSelvatica(Game *g, int specie, int livello) {
    if (BattleUI_AvviaSelvatica(g->lotta, &g->partita, &g->world.rng, specie, livello)) g->scena = SCENA_LOTTA;
    else g->world.richiesta = RICH_NESSUNA;
}

static void AvviaLottaAllenatore(Game *g, const TrainerData *t) {
    if (t && BattleUI_AvviaAllenatore(g->lotta, &g->partita, &g->world.rng, t)) g->scena = SCENA_LOTTA;
    else g->world.richiesta = RICH_NESSUNA;
}

#ifdef ECLYRA_DEBUG
/* Tasti di debug (solo build Debug): F1 +1000 Lumen, F2 cura, F3 +1 livello al primo Elyon,
 * F4 incontro selvatico, F6 tutti i Sigilli, F7 teletrasporto, F8 lotta con un boss (Lira) */
static void DebugTasti(Game *g) {
    if (!World_Libero(&g->world)) return;
    Partita *p = &g->partita;
    Collezione *c = &p->collezione;
    if (IsKeyPressed(KEY_F1)) {
        Inventario_AggiungiLumen(&p->inventario, 1000);
        World_Messaggio(&g->world, "[DEBUG] +1000 Lumen.");
    }
    if (IsKeyPressed(KEY_F2)) {
        Collezione_CuraSquadra(c);
        World_Messaggio(&g->world, "[DEBUG] Squadra curata.");
    }
    if (IsKeyPressed(KEY_F3) && Collezione_NumSquadra(c) > 0) {
        Elyon *e = &c->squadra[0];
        Elyon_AggiungiXp(e, Elyon_XpPerLivello(GetSpecies(e->specie), e->livello + 1) - e->xp);
        int evo = Elyon_PuoEvolvere(e);
        if (evo) Elyon_Evolvi(e, evo);
        World_Messaggio(&g->world, TextFormat("[DEBUG] %s sale al livello %d.", Elyon_Nome(e), e->livello));
    }
    if (IsKeyPressed(KEY_F4)) {
        if (Collezione_SquadraViva(c) == 0) World_Messaggio(&g->world, "[DEBUG] Serve almeno un Elyon in grado di lottare.");
        else {
            const HabitatData *h = GetHabitat(g->world.mappa.habitat[0] ? g->world.mappa.habitat : "ROUTE1");
            int specie, livello;
            if (Lotta_GeneraIncontro(h, &g->world.rng, &specie, &livello)) AvviaLottaSelvatica(g, specie, livello);
        }
    }
    if (IsKeyPressed(KEY_F6)) {
        for (int i = 1; i <= 7; i++) Flags_Imposta(&p->flags, TextFormat("SIGILLO_%d", i));
        World_Messaggio(&g->world, "[DEBUG] Sette Sigilli impostati (solo flag: la storia arriva nelle fasi 5-6).");
    }
    if (IsKeyPressed(KEY_F7)) {
        /* F7: teletrasporto ciclico tra tutte le mappe (allo SPAWN di ciascuna) */
        g->debugMappa = (g->debugMappa + 1) % NUM_MAPPE_GIOCO;
        World_Teletrasporta(&g->world, MAPPE_GIOCO[g->debugMappa], -1, -1, DIR_GIU);
    }
    if (IsKeyPressed(KEY_F8)) {
        if (Collezione_SquadraViva(c) == 0) World_Messaggio(&g->world, "[DEBUG] Serve almeno un Elyon in grado di lottare.");
        else AvviaLottaAllenatore(g, GetTrainer("GUARDIANA_LIRA"));
    }
}
#endif

static void GestisciRichiesta(Game *g) {
    World *w = &g->world;
    switch (w->richiesta) {
    case RICH_SELVATICO:
        AvviaLottaSelvatica(g, w->richSpecie, w->richLivello);
        break;
    case RICH_ALLENATORE:
        AvviaLottaAllenatore(g, w->richAllenatore);
        break;
    case RICH_PC:
        w->richiesta = RICH_NESSUNA;
        Schermate_Apri(&g->schermata, SCH_PC, &g->partita);
        g->schermataAperta = true;
        break;
    default:
        break;
    }
}

static void PassoMondo(Game *g) {
    Input *in = &g->input;
    if (g->avviso) {
        if (in->conferma || in->indietro) g->avviso = NULL;
        return;
    }
    if (g->schermataAperta) {
        if (!Schermate_Aggiorna(&g->schermata, in)) g->schermataAperta = false;
        return;
    }
    if (g->menuGiocoAperto) {
        int scelta = Menu_Aggiorna(&g->menuGioco, in);
        if (scelta == -2 || scelta == 5 || in->menu) g->menuGiocoAperto = false;
        else if (scelta >= 0 && scelta <= 2) {
            static const TipoSchermata tipi[3] = { SCH_SQUADRA, SCH_ZAINO, SCH_SCHEDA };
            Schermate_Apri(&g->schermata, tipi[scelta], &g->partita);
            g->schermataAperta = true;
        }
        else if (scelta == 3) g->avviso = "Il salvataggio arriverà nella Fase 5 dello sviluppo.";
        else if (scelta == 4) { g->menuGiocoAperto = false; ApriOpzioni(g); }
        return;
    }
    if ((in->menu || in->indietro) && World_Libero(&g->world)) {
        g->menuGiocoAperto = true;
        g->menuGioco.cursore = 0;
        return;
    }
#ifdef ECLYRA_DEBUG
    DebugTasti(g);
    if (g->scena != SCENA_MONDO) return;
#endif
    World_Aggiorna(&g->world, in, CaratteriPerPasso(g));
    GestisciRichiesta(g);
}

static void PassoLotta(Game *g) {
    BattleUI_Aggiorna(g->lotta, &g->input, CaratteriPerPasso(g));
    if (BattleUI_Finita(g->lotta)) {
        World_DopoLotta(&g->world, g->lotta->esito);
        g->scena = SCENA_MONDO;
        g->dissolvenza = 1.0f;
    }
}

void Game_Passo(Game *g) {
    if (g->dissolvenza > 0.0f) g->dissolvenza -= 1.0f / 30.0f;
    switch (g->scena) {
        case SCENA_TITOLO:  PassoTitolo(g); break;
        case SCENA_OPZIONI: PassoOpzioni(g); break;
        case SCENA_MONDO:   PassoMondo(g); break;
        case SCENA_LOTTA:   PassoLotta(g); break;
    }
    if ((g->scena == SCENA_MONDO || g->scena == SCENA_LOTTA) && ++g->passiTempo >= 60) {
        g->passiTempo = 0;
        g->partita.secondiGioco++;
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
    if (g->schermataAperta) { Schermate_Disegna(&g->schermata); return; }
    if (g->menuGiocoAperto) Menu_Disegna(&g->menuGioco, LOGICAL_W - 92, 4, 88);
    if (g->avviso) {
        Gfx_Finestra(4, LOGICAL_H - 48, LOGICAL_W - 8, 44);
        UI_TestoACapo(g->avviso, 14, LOGICAL_H - 38, LOGICAL_W - 30, 2, COL_TESTO);
    }
}

void Game_Disegna(Game *g) {
    Gfx_InizioFrame();
    switch (g->scena) {
        case SCENA_TITOLO:  DisegnaTitolo(g); break;
        case SCENA_OPZIONI: DisegnaOpzioni(g); break;
        case SCENA_MONDO:   DisegnaMondo(g); break;
        case SCENA_LOTTA:   BattleUI_Disegna(g->lotta); break;
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
