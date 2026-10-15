/* ECLYRA - Vertical slice visiva (Design System v2, STYLE BIBLE, architettura multipiattaforma)
 *
 *   ./eclyra_slice                 gioca la slice
 *   ./eclyra_slice --touch         mostra sempre i controlli touch (il mouse fa da dito)
 *   ./eclyra_slice --tour CARTELLA foto automatiche di tutte le schermate a 16:9, 19,5:9 e 16:10
 *
 * Tasti di prova (desktop): F1 16:9  F2 Steam Deck 16:10  F3 telefono 19,5:9  F4 iPad 4:3
 * F5 lotta selvatica  F6 lotta con Kael  F7 evoluzione  F8 cura  F9 controlli touch  F12 foto.
 * Il codice del gioco principale non e' modificato: la slice ne usa la logica (vedi legacy_shim.c). */
#include "slice.h"
#include "gfx.h"
#include "vista_dialogo.h"
#include "data/encounters_data.h"
#include "data/items_data.h"
#include "data/evolution_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

Slice S;
void Shim_ImpostaCartellaGioco(const char *c);
int Tour_Esegui(Slice *s, const char *cartella);

#define PASSO (1.0f / 60.0f)
#define CARATTERI_PER_PASSO 0.6f

/* ------------------------------------------------------------------ sostituzioni della slice */

/* La slice mostra solo i 9 Elyon di riferimento: gli incontri e le squadre di Route 1 usano
 * quelle specie (stesso livello), finche' gli altri sprite non saranno prodotti. */
static int SpecieSlice(int specie) {
    if (G_HaSprite(specie)) return specie;
    static const int base[3] = { 7, 4, 1 };   /* MOSSEN, AQUIL, PYRIN */
    return base[specie % 3];
}

static TrainerData sostituti[4];

static const TrainerData *AllenatoreSlice(const TrainerData *t) {
    bool tutti = true;
    for (int i = 0; i < t->numMembri; i++) if (!G_HaSprite(t->squadra[i].specie)) tutti = false;
    if (tutti) return t;
    static int prossimo;
    TrainerData *c = &sostituti[prossimo++ % 4];
    *c = *t;
    for (int i = 0; i < c->numMembri; i++) {
        c->squadra[i].specie = SpecieSlice(c->squadra[i].specie + i);
        memset(c->squadra[i].mosse, 0, sizeof(c->squadra[i].mosse));
    }
    return c;
}

static bool MappaSlice(const char *id) { return strcmp(id, "lunaria") == 0 || strcmp(id, "route1") == 0; }

/* Gli interni non fanno parte della slice: la porta mostra un avviso invece di entrare */
static bool BloccaPorte(Slice *s) {
    World *w = &s->world;
    if (!World_Libero(w)) return false;
    Direzione d;
    if (s->in.su) d = DIR_SU; else if (s->in.giu) d = DIR_GIU; else if (s->in.sinistra) d = DIR_SINISTRA;
    else if (s->in.destra) d = DIR_DESTRA; else return false;
    int x = w->player.tx + Dir_DX(d), y = w->player.ty + Dir_DY(d);
    const Warp *wp = Map_WarpIn(&w->mappa, x, y);
    if (!wp || MappaSlice(wp->dest)) return false;
    if (w->player.dir != d) return false;   /* prima si gira verso la porta */
    World_Messaggio(w, "Gli interni non fanno parte della vertical slice: qui si provano Lunaria e Route 1.");
    return true;
}

static void AggiornaBestiario(Slice *s) {
    const Collezione *c = &s->partita.collezione;
    for (int i = 0; i < MAX_SQUADRA; i++) if (!Elyon_Vuoto(&c->squadra[i])) s->catturato[c->squadra[i].specie] = s->visto[c->squadra[i].specie] = true;
    for (int b = 0; b < NUM_BOX; b++)
        for (int p = 0; p < POSTI_BOX; p++)
            if (!Elyon_Vuoto(&c->box[b][p])) s->catturato[c->box[b][p].specie] = s->visto[c->box[b][p].specie] = true;
}

/* ------------------------------------------------------------------ scene */

void Slice_NuovaPartita(Slice *s, int starter) {
    Partita_Nuova(&s->partita, Rng_Next(&s->rng));
    strncpy(s->partita.mappa, "lunaria", sizeof(s->partita.mappa) - 1);
    s->partita.x = 10; s->partita.y = 23; s->partita.dir = DIR_SU;
    Elyon e;
    Elyon_Crea(&e, starter, 5, &s->rng);
    Collezione_Aggiungi(&s->partita.collezione, &e);
    Flags_Imposta(&s->partita.flags, FLAG_HA_ELYON);
    Flags_Imposta(&s->partita.flags, TextFormat("STARTER_%s", GetSpecies(starter)->nome));
    Inventario_Aggiungi(&s->partita.inventario, ITEM_CORE, 5);
    Inventario_Aggiungi(&s->partita.inventario, ITEM_ERBA_VITALE, 2);
    Inventario_AggiungiLumen(&s->partita.inventario, 500);
    memset(s->visto, 0, sizeof(s->visto));
    memset(s->catturato, 0, sizeof(s->catturato));
    memset(s->dove, 0, sizeof(s->dove));
    snprintf(s->dove[starter], sizeof(s->dove[starter]), "Laboratorio di Lunaria");
    World_Init(&s->world, &s->partita, &s->dialoghi);
    s->world.timerNomeMappa = 150;
    AggiornaBestiario(s);
    s->scena = SC_MONDO;
    s->pannello = PAN_NESSUNO;
    s->dissolvenza = 1.0f;
}

void Slice_AvviaSelvatica(Slice *s, int specie, int livello) {
    specie = SpecieSlice(specie);
    if (BattleUI_AvviaSelvatica(s->lotta, &s->partita, &s->world.rng, specie, livello)) {
        s->visto[specie] = true;
        if (!s->dove[specie][0]) snprintf(s->dove[specie], sizeof(s->dove[specie]), "%s", s->world.mappa.nome);
        s->scena = SC_LOTTA;
    }
    s->world.richiesta = RICH_NESSUNA;
}

void Slice_AvviaAllenatore(Slice *s, const TrainerData *t) {
    t = AllenatoreSlice(t);
    if (BattleUI_AvviaAllenatore(s->lotta, &s->partita, &s->world.rng, t)) {
        for (int i = 0; i < t->numMembri; i++) {
            int sp = t->squadra[i].specie;
            s->visto[sp] = true;
            if (!s->dove[sp][0]) snprintf(s->dove[sp], sizeof(s->dove[sp]), "%s", s->world.mappa.nome);
        }
        s->scena = SC_LOTTA;
    }
    s->world.richiesta = RICH_NESSUNA;
}

void Slice_AvviaEvoluzione(Slice *s, int da) {
    const EvolutionData *ev = GetEvolution(da);
    if (!ev) return;
    s->evo = (EvoVetrina){ da, ev->a, 0, 0, "" };
    snprintf(s->evo.nome, sizeof(s->evo.nome), "%s", GetSpecies(da)->nome);
    s->scenaDopoEvo = s->scena;
    s->scena = SC_EVOLUZIONE;
}

static void PassoTitolo(Slice *s) {
    Input *in = &s->in;
    if (in->suP) s->cursoreTitolo = s->cursoreTitolo == 0 ? 3 : 0;
    if (in->giuP) s->cursoreTitolo = s->cursoreTitolo == 3 ? 0 : 3;
    for (int i = 0; i < 4; i++) {
        int mx = s->canvas.w / 2 - 72, my = (s->canvas.h - 180) / 2 + 82 + 8 + i * 13 - 2;
        if ((i == 0 || i == 3) && Azioni_Tap(&s->az, (Rectangle){ (float)mx, (float)my, 104, 12 })) { s->cursoreTitolo = i; in->conferma = true; }
    }
    if (in->conferma) {
        if (s->cursoreTitolo == 0) { s->scena = SC_STARTER; s->cursoreStarter = 0; }
        else {
#ifdef __EMSCRIPTEN__
            /* nel browser "Esci" torna alla schermata della pagina (ed esce dallo schermo intero) */
            EM_ASM({ if (window.EclyraEsci) window.EclyraEsci(); });
            s->cursoreTitolo = 0;
#else
            s->esci = true;
#endif
        }
    }
}

static void PassoStarter(Slice *s) {
    Input *in = &s->in;
    if (in->sinistraP) s->cursoreStarter = (s->cursoreStarter + 2) % 3;
    if (in->destraP) s->cursoreStarter = (s->cursoreStarter + 1) % 3;
    int ox = (s->canvas.w - 320) / 2, oy = (s->canvas.h - 180) / 2;
    for (int i = 0; i < 3; i++)
        if (Azioni_Tap(&s->az, (Rectangle){ (float)(ox + 10 + i * 102), (float)(oy + 30), 96, 118 })) {
            if (s->cursoreStarter == i) in->conferma = true;
            s->cursoreStarter = i;
        }
    if (in->indietro) s->scena = SC_TITOLO;
    if (in->conferma) { static const int sp[3] = { 1, 4, 7 }; Slice_NuovaPartita(s, sp[s->cursoreStarter]); }
}

static void PassoMondo(Slice *s) {
    World *w = &s->world;
    if (s->pannello != PAN_NESSUNO) { Pannelli_Aggiorna(s); return; }
    if ((s->in.menu || s->in.indietro) && World_Libero(w)) { Pannelli_Apri(s, PAN_MENU); return; }
    if (BloccaPorte(s)) return;
    World_Aggiorna(w, &s->in, CARATTERI_PER_PASSO);
    switch (w->richiesta) {
    case RICH_SELVATICO:  Slice_AvviaSelvatica(s, w->richSpecie, w->richLivello); break;
    case RICH_ALLENATORE: if (w->richAllenatore) Slice_AvviaAllenatore(s, w->richAllenatore); else w->richiesta = RICH_NESSUNA; break;
    case RICH_PC:         w->richiesta = RICH_NESSUNA; World_Messaggio(w, "Il PC del box non fa parte della vertical slice."); break;
    default: break;
    }
}

static void PassoLotta(Slice *s) {
    VistaLotta_PreparaInput(s);
    BattleUI_Aggiorna(s->lotta, &s->in, CARATTERI_PER_PASSO);
    if (BattleUI_Finita(s->lotta)) {
        World_DopoLotta(&s->world, s->lotta->esito);
        if (!MappaSlice(s->world.mappa.id)) {
            /* il gioco riporta a casa (interno): nella slice si torna davanti alla porta di casa */
            Dialogo salvato = s->world.dialogo;
            World_Teletrasporta(&s->world, "lunaria", 10, 22, DIR_GIU);
            s->world.dialogo = salvato;
        }
        AggiornaBestiario(s);
        s->scena = SC_MONDO;
        s->dissolvenza = 1.0f;
    }
}

static void PassoEvoluzione(Slice *s) {
    EvoVetrina *e = &s->evo;
    switch (e->fase) {
    case 0: if (++e->timer > 40 && s->in.conferma) { e->fase = 1; e->timer = 0; } break;
    case 1: if (++e->timer >= 200) { e->fase = 2; e->timer = 0; } if (s->in.indietro) { e->fase = 3; e->timer = 0; } break;
    default: if (++e->timer > 30 && s->in.conferma) s->scena = s->scenaDopoEvo; break;
    }
}

static void Debug(Slice *s) {
#ifndef __EMSCRIPTEN__
    if (!s->mobile) {
        if (IsKeyPressed(KEY_F1)) SetWindowSize(1280, 720);
        if (IsKeyPressed(KEY_F2)) SetWindowSize(1280, 800);
        if (IsKeyPressed(KEY_F3)) SetWindowSize(1560, 720);
        if (IsKeyPressed(KEY_F4)) SetWindowSize(1024, 768);
    }
#endif
    if (IsKeyPressed(KEY_F9)) { s->az.forzaTouch = !s->az.forzaTouch; s->az.touchVisibile = s->az.forzaTouch; }
    if (s->scena != SC_MONDO || s->pannello != PAN_NESSUNO || !World_Libero(&s->world)) return;
    if (IsKeyPressed(KEY_F5)) {
        int sp, lv;
        if (Lotta_GeneraIncontro(GetHabitat("ROUTE1"), &s->world.rng, &sp, &lv)) Slice_AvviaSelvatica(s, sp, lv);
    }
    if (IsKeyPressed(KEY_F6)) {
        const char *st = World_Starter(&s->world);
        if (st) Slice_AvviaAllenatore(s, GetTrainer(TextFormat("KAEL_INIZIALE_VS_%s", st)));
    }
    if (IsKeyPressed(KEY_F7)) Slice_AvviaEvoluzione(s, s->partita.collezione.squadra[0].specie);
    if (IsKeyPressed(KEY_F8)) { Collezione_CuraSquadra(&s->partita.collezione); World_Messaggio(&s->world, "Squadra curata (tasto di prova)."); }
}

void Slice_Passo(Slice *s) {
    if (s->dissolvenza > 0) s->dissolvenza -= 1.0f / 24.0f;
    switch (s->scena) {
    case SC_TITOLO:     PassoTitolo(s); break;
    case SC_STARTER:    PassoStarter(s); break;
    case SC_MONDO:      PassoMondo(s); break;
    case SC_LOTTA:      PassoLotta(s); break;
    case SC_EVOLUZIONE: PassoEvoluzione(s); break;
    }
    Input_ConsumaEventi(&s->in);
}

/* ------------------------------------------------------------------ disegno */

static void AggiornaContesto(Slice *s) {
    ContestoTouch c = TOUCH_ESPLORA;
    if (s->scena == SC_TITOLO || s->scena == SC_STARTER || s->scena == SC_EVOLUZIONE) c = TOUCH_DIALOGO;
    else if (s->scena == SC_LOTTA) c = TOUCH_LOTTA;
    else if (s->pannello != PAN_NESSUNO) c = TOUCH_MENU;
    else if (s->world.dialogo.attivo) c = TOUCH_DIALOGO;
    s->az.contesto = c;
}

void Slice_DisegnaCanvas(Slice *s) {
    BeginTextureMode(s->rt);
    ClearBackground(PAL_NOTTE[0]);
    switch (s->scena) {
    case SC_TITOLO:  VistaTitolo_Disegna(s); break;
    case SC_STARTER: VistaStarter_Disegna(s); break;
    case SC_MONDO:
        VistaMondo_Disegna(s);
        if (s->pannello != PAN_NESSUNO) Pannelli_Disegna(s);
        break;
    case SC_LOTTA: VistaLotta_Disegna(s); break;
    case SC_EVOLUZIONE: {
        const EvoVetrina *e = &s->evo;
        char t[96];
        if (e->fase == 0) snprintf(t, sizeof(t), "Cosa? %s si sta evolvendo!", e->nome);
        else if (e->fase == 1) t[0] = 0;
        else if (e->fase == 2) snprintf(t, sizeof(t), "Evviva! %s si è evoluto in %s!", e->nome, GetSpecies(e->a)->nome);
        else snprintf(t, sizeof(t), "%s ha smesso di evolversi.", e->nome);
        VistaEvoluzione_Disegna(&s->canvas, e->da, e->fase == 3 ? e->da : e->a, e->timer, e->fase, t, s->frame);
        break;
    }
    }
    if (s->dissolvenza > 0) G_Oscura(0, 0, s->canvas.w, s->canvas.h, (unsigned char)(255 * (s->dissolvenza > 1 ? 1 : s->dissolvenza)));
    Azioni_DisegnaTouch(&s->az, &s->canvas, s->frame);
    EndTextureMode();
}

void Slice_Frame(Slice *s) {
    int dw = GetRenderWidth(), dh = GetRenderHeight();
    SafeInsets nessuno = { 0, 0, 0, 0 };
    s->canvas = Canvas_Compute(dw, dh, nessuno);
    if (s->canvas.w != s->rtW || s->canvas.h != s->rtH) {
        if (s->rtW) UnloadRenderTexture(s->rt);
        s->rt = LoadRenderTexture(s->canvas.w, s->canvas.h);
        SetTextureFilter(s->rt.texture, TEXTURE_FILTER_POINT);
        s->rtW = s->canvas.w; s->rtH = s->canvas.h;
    }
    AggiornaContesto(s);
    Azioni_Aggiorna(&s->az, &s->canvas);
    Azioni_VersoInput(&s->az, &s->in);
    Debug(s);
    if (s->passoFisso) { s->frame++; Slice_Passo(s); }
    else {
        s->accumulatore += GetFrameTime();
        if (s->accumulatore > 0.1f) s->accumulatore = 0.1f;
        while (s->accumulatore >= PASSO) {
            s->frame++;
            Slice_Passo(s);
            s->accumulatore -= PASSO;
        }
    }
    Slice_DisegnaCanvas(s);
    BeginDrawing();
    ClearBackground(BLACK);
    const Canvas *c = &s->canvas;
    DrawTexturePro(s->rt.texture, (Rectangle){ 0, 0, (float)c->w, -(float)c->h },
                   (Rectangle){ (float)c->offX, (float)c->offY, (float)(c->w * c->scale), (float)(c->h * c->scale) },
                   (Vector2){ 0, 0 }, 0, WHITE);
    EndDrawing();
#ifndef __EMSCRIPTEN__
    if (IsKeyPressed(KEY_F12)) {
        static int n;
        Image im = LoadImageFromTexture(s->rt.texture);
        ImageFlipVertical(&im);
        ExportImage(im, TextFormat("slice_%02d.png", n++));
        UnloadImage(im);
    }
#endif
}

/* ------------------------------------------------------------------ versione web */
#ifdef __EMSCRIPTEN__
/* Nel browser la finestra e' il canvas della pagina: segue la grandezza della finestra (anche
 * quando il telefono ruota). Il telefono si riconosce dal puntatore "grossolano" (dito). */
static bool WebMobile(void) {
    return EM_ASM_INT({
        var q = new URLSearchParams(window.location.search);
        if (q.get('touch') === '0') return 0;
        if (q.get('touch') === '1') return 1;
        var dito = window.matchMedia && window.matchMedia('(pointer: coarse)').matches;
        return (dito && navigator.maxTouchPoints > 0) ? 1 : 0;
    }) != 0;
}

static void FrameWeb(void) {
    int w = EM_ASM_INT({ return window.innerWidth; }), h = EM_ASM_INT({ return window.innerHeight; });
    if (w > 0 && h > 0 && (w != GetScreenWidth() || h != GetScreenHeight())) SetWindowSize(w, h);
    Slice_Frame(&S);
}
#endif

/* ------------------------------------------------------------------ main */

static const char *CartellaAccanto(const char *nome) {
    static char p[600];
    snprintf(p, sizeof(p), "%s%s", GetApplicationDirectory(), nome);
    if (DirectoryExists(p)) return p;
    return nome;
}

static int Avvia(int argc, char **argv) {
    const char *tour = NULL;
    bool touch = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tour") == 0 && i + 1 < argc) tour = argv[++i];
        else if (strcmp(argv[i], "--touch") == 0) touch = true;
    }
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
#ifdef __EMSCRIPTEN__
    SetTraceLogLevel(LOG_WARNING);
#else
    SetTraceLogLevel(tour ? LOG_WARNING : LOG_INFO);
#endif
    InitWindow(1280, 720, "ECLYRA - Vertical slice visiva");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    memset(&S, 0, sizeof(S));
#ifdef __EMSCRIPTEN__
    S.mobile = WebMobile();
#else
    S.mobile = false;
#endif
    Shim_ImpostaCartellaGioco(CartellaAccanto("assets"));
    char cs[620];
    snprintf(cs, sizeof(cs), "%s/", CartellaAccanto("slice_assets"));
    if (!Gfx_Carica(cs)) { TraceLog(LOG_ERROR, "SLICE: asset della slice non trovati in %s", cs); CloseWindow(); return 1; }
    char errore[200];
    if (!DialogoDb_CaricaFile(&S.dialoghi, TextFormat("%s/text/dialoghi.txt", CartellaAccanto("assets")), errore, sizeof(errore)))
        TraceLog(LOG_WARNING, "SLICE: dialoghi non caricati (%s)", errore);
    S.lotta = calloc(1, sizeof(BattleUI));
    Rng_Init(&S.rng, tour ? 20261002u : (uint32_t)time(NULL));
    Azioni_Init(&S.az, S.mobile);
    S.az.forzaTouch = touch;
    S.scena = SC_TITOLO;
    S.dissolvenza = 1.0f;

    int codice = 0;
#ifdef __EMSCRIPTEN__
    (void)tour;
    EM_ASM({ if (window.EclyraPronta) window.EclyraPronta(); });
    emscripten_set_main_loop(FrameWeb, 0, 1);   /* non ritorna */
#endif
    if (tour) codice = Tour_Esegui(&S, tour);
    else while (!WindowShouldClose() && !S.esci) Slice_Frame(&S);

    Mappa_Libera();
    Gfx_Libera();
    DialogoDb_Libera(&S.dialoghi);
    free(S.lotta);
    if (S.rtW) UnloadRenderTexture(S.rt);
    CloseWindow();
    return codice;
}

#ifdef __EMSCRIPTEN__
int main(void) { return Avvia(0, NULL); }   /* nel browser non ci sono argomenti (?touch=1 nell'indirizzo) */
#else
int main(int argc, char **argv) { return Avvia(argc, argv); }
#endif
