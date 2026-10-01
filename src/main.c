/* main.c - ECLYRA: Le Cronache dell'Equilibrio
 *
 * Uso:  eclyra                    avvia il gioco
 *       eclyra --screenshot-test  (solo build di debug) percorre title e mondo
 *                                 con input simulato e salva screenshot PNG
 */
#include "game.h"
#include "graphics.h"
#include "items.h"
#include <stdio.h>
#include <string.h>

#ifdef ECLYRA_DEBUG
static void Scatta(const char *nome) {
    Gfx_EsportaFrame(nome);   /* la render texture logica 320x180 */
}

static void Attendi(Game *g, int passi) {
    for (int i = 0; i < passi; i++) Game_Passo(g);
}

static void PremiConferma(Game *g) { g->input.conferma = true; Game_Passo(g); }

/* Tiene premuta una direzione per un certo numero di passi logici */
static void Tieni(Game *g, Direzione d, int passi) {
    for (int i = 0; i < passi; i++) {
        g->input.su = d == DIR_SU; g->input.giu = d == DIR_GIU;
        g->input.sinistra = d == DIR_SINISTRA; g->input.destra = d == DIR_DESTRA;
        Game_Passo(g);
    }
    g->input.su = g->input.giu = g->input.sinistra = g->input.destra = false;
}

static void Foto(Game *g, const char *nome) {
    char copia[128];   /* TextFormat usa buffer a rotazione: il disegno li riusa */
    snprintf(copia, sizeof(copia), "%s", nome);
    Game_Disegna(g);
    Scatta(copia);
}

static int MossaMigliore(const Lotta *L) {
    int best = 0, valore = -1;
    for (int k = 0; k < MAX_MOSSE_ELYON; k++) {
        const MoveData *m = GetMove(L->lato[LATO_GIOCATORE].e->mosse[k]);
        if (!m || L->lato[LATO_GIOCATORE].e->pp[k] <= 0) continue;
        int v = Lotta_StimaDanno(L, LATO_GIOCATORE, m->id, 0.95f);
        if (v > valore) { valore = v; best = k; }
    }
    return best;
}

/* Gioca una lotta scegliendo sempre la mossa migliore; foto al primo menu e al primo colpo */
static void GiocaLotta(Game *g, const char *prefisso, int maxPassi) {
    bool fotoMenu = false, fotoMosse = false, fotoColpo = false;
    for (int i = 0; i < maxPassi && g->scena == SCENA_LOTTA; i++) {
        BattleUI *u = g->lotta;
        switch (u->stato) {
        case BUI_MENU:
            if (!fotoMenu) { Foto(g, TextFormat("%s_menu.png", prefisso)); fotoMenu = true; }
            u->cursoreMenu = 0;
            PremiConferma(g);
            break;
        case BUI_MOSSE:
            u->cursoreMossa = MossaMigliore(&u->lotta);
            if (!fotoMosse) { Game_Passo(g); Foto(g, TextFormat("%s_mosse.png", prefisso)); fotoMosse = true; }
            PremiConferma(g);
            break;
        case BUI_ZAINO:
        case BUI_ELISIR:
            g->input.indietro = true;
            Game_Passo(g);
            break;
        case BUI_SQUADRA:
            if (u->modoSquadra != SQ_FORZATO) { g->input.indietro = true; Game_Passo(g); break; }
            for (int k = 0; k < MAX_SQUADRA; k++)
                if (!Elyon_Esausto(&g->partita.collezione.squadra[k])) { u->cursoreSquadra = k; break; }
            PremiConferma(g);
            break;
        case BUI_IMPARA_SINO:
            u->cursoreSiNo = 1;
            PremiConferma(g);
            break;
        case BUI_EVENTI:
            if (!fotoColpo && u->pos < u->numCoda && u->coda[u->pos].tipo == EV_DANNO && u->timer == 6) {
                Foto(g, TextFormat("%s_colpo.png", prefisso));
                fotoColpo = true;
            }
            if (i % 8 == 0) PremiConferma(g); else Game_Passo(g);
            break;
        default:
            if (i % 8 == 0) PremiConferma(g); else Game_Passo(g);
            break;
        }
    }
}

static int TestScreenshot(Game *g) {
    SetTargetFPS(0);
    g->opzioni.velocitaTesto = TESTO_VELOCE;
    /* 1) titolo */
    Attendi(g, 90);
    Foto(g, "test_01_titolo.png");
    /* 2) nuova partita: si comincia in casa */
    PremiConferma(g);
    Attendi(g, 40);
    Foto(g, "test_02_casa.png");
    /* 3) parlo con la mamma */
    World_Teletrasporta(&g->world, "casa_giocatore", 7, 5, DIR_DESTRA);
    PremiConferma(g);
    Attendi(g, 60);
    Foto(g, "test_03_dialogo_mamma.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    /* 4) esco dalla porta: il tappetino porta a Lunaria */
    World_Teletrasporta(&g->world, "casa_giocatore", 5, 6, DIR_GIU);
    Tieni(g, DIR_GIU, 20);
    Attendi(g, 60);
    Foto(g, "test_04_lunaria.png");
    TraceLog(LOG_INFO, "ECLYRA TEST: dopo l'uscita sono in %s (%d,%d)", g->world.mappa.id, g->world.player.tx, g->world.player.ty);
    /* 5) entro nel laboratorio urtando la porta e parlo con Elian */
    World_Teletrasporta(&g->world, "lunaria", 31, 13, DIR_SU);
    Tieni(g, DIR_SU, 4);
    Attendi(g, 60);
    TraceLog(LOG_INFO, "ECLYRA TEST: dopo la porta sono in %s (%d,%d)", g->world.mappa.id, g->world.player.tx, g->world.player.ty);
    World_Teletrasporta(&g->world, "laboratorio", 7, 4, DIR_SINISTRA);
    PremiConferma(g);
    Attendi(g, 80);
    Foto(g, "test_05_elian.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    /* 6) Route 1: raccolgo il CORE vicino al lago */
    World_Teletrasporta(&g->world, "route1", 44, 14, DIR_DESTRA);
    Attendi(g, 10);
    PremiConferma(g);
    Attendi(g, 40);
    Foto(g, "test_06_core.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    TraceLog(LOG_INFO, "ECLYRA TEST: CORE nello zaino = %d", Inventario_Quantita(&g->partita.inventario, ITEM_CORE));
    /* 7) Kael davanti al laboratorio e menu ZAINO */
    World_Teletrasporta(&g->world, "lunaria", 30, 15, DIR_SU);
    PremiConferma(g);
    Attendi(g, 80);
    Foto(g, "test_07_kael.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 30); }
    g->input.menu = true; Game_Passo(g);
    g->input.giuP = true; Game_Passo(g);
    PremiConferma(g);
    Foto(g, "test_08_zaino.png");
    g->input.indietro = true; Game_Passo(g);
    g->input.indietro = true; Game_Passo(g);

    /* 8) laboratorio: scelta (provvisoria) dello starter dalla capsula di PYRIN */
    int esito = 0;
    World_Teletrasporta(&g->world, "laboratorio", 5, 6, DIR_SU);
    PremiConferma(g);
    Attendi(g, 40);
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 20); }
    Foto(g, "test_09_capsula.png");
    PremiConferma(g);                                  /* SI */
    Attendi(g, 40);
    Foto(g, "test_10_starter.png");
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 20); }
    TraceLog(LOG_INFO, "ECLYRA TEST: squadra = %d, starter = %s", Collezione_NumSquadra(&g->partita.collezione),
             World_Starter(&g->world) ? World_Starter(&g->world) : "nessuno");
    if (Collezione_NumSquadra(&g->partita.collezione) != 1) esito = 1;

    /* 9) Kael sfida il giocatore: lotta vera fino alla fine */
    World_Teletrasporta(&g->world, "lunaria", 30, 15, DIR_SU);
    PremiConferma(g);
    Attendi(g, 30);
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 20); }
    Game_Passo(g);
    TraceLog(LOG_INFO, "ECLYRA TEST: scena dopo il dialogo con Kael = %d (3 = lotta)", g->scena);
    if (g->scena != SCENA_LOTTA) esito = 1;
    Attendi(g, 20);
    Foto(g, "test_11_lotta_transizione.png");
    Attendi(g, 40);
    Foto(g, "test_12_lotta_entrata.png");
    GiocaLotta(g, "test_13_kael", 20000);
    TraceLog(LOG_INFO, "ECLYRA TEST: lotta con Kael finita, esito %d, flag vittoria %d, Lumen %d", g->lotta->esito,
             Flags_Attivo(&g->partita.flags, "SCONFITTO_KAEL_1"), g->partita.inventario.lumen);
    if (g->scena != SCENA_MONDO) esito = 1;
    Attendi(g, 40);
    while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 20); }

    /* 10) Route 1: erba alta fino a un incontro, poi un CORE */
    World_Teletrasporta(&g->world, "route1", 37, 18, DIR_DESTRA);
    Collezione_CuraSquadra(&g->partita.collezione);
    for (int passi = 0; passi < 400 && g->scena != SCENA_LOTTA; passi++) {
        Tieni(g, (passi / 6) % 2 ? DIR_SINISTRA : DIR_DESTRA, 9);
        Game_Passo(g);
    }
    TraceLog(LOG_INFO, "ECLYRA TEST: incontro nell'erba alta = %s", g->scena == SCENA_LOTTA ? "si" : "no");
    if (g->scena != SCENA_LOTTA) esito = 1;
    else {
        Attendi(g, 60);
        for (int i = 0; i < 2000 && g->lotta->stato != BUI_MENU; i++) { if (i % 8 == 0) PremiConferma(g); else Game_Passo(g); }
        Foto(g, "test_14_selvatico.png");
        g->lotta->cursoreMenu = 2;                     /* ZAINO */
        PremiConferma(g);
        Foto(g, "test_15_zaino_lotta.png");
        /* nella lista gli oggetti sono in ordine di ID: prima le cure, poi i Core */
        int indiceCore = 0;
        for (int id = 1; id < ITEM_CORE; id++) indiceCore += Inventario_Quantita(&g->partita.inventario, id) > 0;
        g->lotta->cursoreZaino = indiceCore;
        PremiConferma(g);                              /* CORE */
        for (int i = 0; i < 200; i++) {
            Game_Passo(g);
            if (i == 40) Foto(g, "test_16_cattura.png");
        }
        GiocaLotta(g, "test_17_selvatico", 20000);
        TraceLog(LOG_INFO, "ECLYRA TEST: lotta selvatica finita, esito %d, squadra %d", g->lotta->esito,
                 Collezione_NumSquadra(&g->partita.collezione));
        Attendi(g, 40);
        while (g->world.dialogo.attivo) { PremiConferma(g); Attendi(g, 20); }
    }

    /* 11) menu ELYON (riepilogo), SCHEDA e PC */
    g->input.menu = true; Game_Passo(g);
    PremiConferma(g);
    Foto(g, "test_18_squadra.png");
    PremiConferma(g);
    PremiConferma(g);
    Foto(g, "test_19_riepilogo.png");
    g->input.indietro = true; Game_Passo(g);
    g->input.indietro = true; Game_Passo(g);
    g->input.giuP = true; Game_Passo(g);
    g->input.giuP = true; Game_Passo(g);
    PremiConferma(g);
    Foto(g, "test_20_scheda.png");
    PremiConferma(g);
    g->input.indietro = true; Game_Passo(g);
    TraceLog(LOG_INFO, "ECLYRA TEST: menu aperto %d, schermata %d", g->menuGiocoAperto, g->schermataAperta);
    World_Teletrasporta(&g->world, "laboratorio", 2, 7, DIR_SINISTRA);
    PremiConferma(g);
    Game_Passo(g);
    Foto(g, "test_21_pc.png");
    g->input.indietro = true; Game_Passo(g);

    /* 12) livelli, nuova mossa con sostituzione ed evoluzione (PYRIN Lv15 quasi al 16) */
    {
        Collezione *c = &g->partita.collezione;
        Elyon *e = &c->squadra[0];
        Elyon_Crea(e, 1, 15, &g->world.rng);
        memset(e->mosse, 0, sizeof(e->mosse));
        static const char *mosse[4] = { "BRACIAVIVA", "GRAFFIO", "MARCHIO DEL FUOCO", "FIAMMATA" };
        for (int k = 0; k < 4; k++) Elyon_ImparaMossa(e, GetMoveByName(mosse[k])->id);
        e->xp = Elyon_XpPerLivello(GetSpecies(1), 16) - 1;
        if (BattleUI_AvviaSelvatica(g->lotta, &g->partita, &g->world.rng, 74, 100)) {
            g->scena = SCENA_LOTTA;
            g->lotta->lotta.nemici[0].hp = 1;
            g->lotta->lotta.nemici[0].stat[STAT_SPE] = 1;
            bool fotoImpara = false, fotoEvo = false, fotoScelta = false;
            for (int i = 0; i < 20000 && g->scena == SCENA_LOTTA; i++) {
                BattleUI *u = g->lotta;
                if (u->stato == BUI_MENU) { u->cursoreMenu = 0; PremiConferma(g); }
                else if (u->stato == BUI_MOSSE) { u->cursoreMossa = MossaMigliore(&u->lotta); PremiConferma(g); }
                else if (u->stato == BUI_IMPARA_SINO) {
                    if (!fotoImpara) { Foto(g, "test_22_impara.png"); fotoImpara = true; }
                    u->cursoreSiNo = 0;
                    PremiConferma(g);
                } else if (u->stato == BUI_IMPARA_SCELTA) {
                    u->cursoreImpara = 1;                  /* dimentica GRAFFIO */
                    if (!fotoScelta) { Foto(g, "test_23_impara_scelta.png"); fotoScelta = true; }
                    PremiConferma(g);
                } else if (u->stato == BUI_EVOLUZIONE && u->evoFase == 1) {
                    if (!fotoEvo && u->evoTimer == 120) { Foto(g, "test_24_evoluzione.png"); fotoEvo = true; }
                    Game_Passo(g);
                } else if (i % 8 == 0) PremiConferma(g);
                else Game_Passo(g);
            }
            const Elyon *dopo = &c->squadra[0];
            bool haRapidita = false, haGraffio = false;
            for (int k = 0; k < 4; k++) {
                haRapidita |= dopo->mosse[k] == GetMoveByName("RAPIDITÀ")->id;
                haGraffio |= dopo->mosse[k] == GetMoveByName("GRAFFIO")->id;
            }
            TraceLog(LOG_INFO, "ECLYRA TEST: dopo la lotta: %s Lv%d, RAPIDITA %d, GRAFFIO %d", GetSpecies(dopo->specie)->nome,
                     dopo->livello, haRapidita, haGraffio);
            if (dopo->specie != 2 || dopo->livello != 17 || !haRapidita || haGraffio) esito = 1;
            Attendi(g, 30);
        }
    }

    /* 13) sconfitta: ritorno sicuro a casa con la squadra curata */
    for (int k = 0; k < MAX_SQUADRA; k++) if (!Elyon_Vuoto(&g->partita.collezione.squadra[k])) g->partita.collezione.squadra[k].hp = 1;
    if (BattleUI_AvviaAllenatore(g->lotta, &g->partita, &g->world.rng, GetTrainer("GUARDIANA_LIRA"))) {
        g->scena = SCENA_LOTTA;
        GiocaLotta(g, "test_25_lira", 20000);
        Attendi(g, 30);
        Foto(g, "test_26_sconfitta_casa.png");
        TraceLog(LOG_INFO, "ECLYRA TEST: dopo la sconfitta sono in %s, squadra viva %d", g->world.mappa.id,
                 Collezione_SquadraViva(&g->partita.collezione));
        if (strcmp(g->world.mappa.id, "casa_giocatore") != 0) esito = 1;
    }
    TraceLog(LOG_INFO, "ECLYRA TEST: %s", esito == 0 ? "COMPLETATO" : "FALLITO");
    return esito;
}
#endif

int main(int argc, char **argv) {
    static Game game;   /* static: la mappa e' grande, meglio non sullo stack */
    if (!Game_Init(&game)) return 1;

#ifdef ECLYRA_DEBUG
    if (argc > 1 && strcmp(argv[1], "--screenshot-test") == 0) {
        int r = TestScreenshot(&game);
        Game_Shutdown(&game);
        return r;
    }
#else
    (void)argc; (void)argv;
#endif

    SetTargetFPS(60);
    while (!game.esci) Game_Frame(&game);
    Game_Shutdown(&game);
    return 0;
}
