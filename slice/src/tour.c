/* tour.c - Foto automatiche della vertical slice per il controllo qualita' visivo.
 *
 * Gioca la slice da sola (input iniettati, un passo logico per frame) e salva il canvas logico di
 * ogni schermata a tre proporzioni: 16:9 (PC), 19,5:9 (telefono, con i controlli touch) e
 * 16:10 (Steam Deck). Le foto finiscono in CARTELLA/<formato>/NN_nome.png. */
#include "slice.h"
#include "gfx.h"
#include "data/items_data.h"
#include <stdio.h>
#include <string.h>

static const char *dir;
static char sotto[64];
static int numFoto, errori;

static void Frames(Slice *s, int n) { for (int i = 0; i < n && !WindowShouldClose(); i++) Slice_Frame(s); }

static void Premi(Slice *s, AzioneIn a) { Azioni_Inietta(&s->az, a); Frames(s, 2); }

static void Foto(Slice *s, const char *nome) {
    Frames(s, 1);
    Image im = LoadImageFromTexture(s->rt.texture);
    ImageFlipVertical(&im);
    char p[700];
    snprintf(p, sizeof(p), "%s/%s/%s.png", dir, sotto, nome);
    if (!ExportImage(im, p)) errori++;
    UnloadImage(im);
    numFoto++;
}

/* Avanza la lotta confermando i messaggi finche' non arriva lo stato voluto (o finisce) */
static bool FinoA(Slice *s, StatoBattleUI voluto, int max) {
    for (int i = 0; i < max; i++) {
        if (s->scena != SC_LOTTA) return false;
        if (s->lotta->stato == voluto) return true;
        if (s->lotta->stato == BUI_EVENTI || s->lotta->stato == BUI_AVVISO || s->lotta->stato == BUI_IMPARA_SINO) {
            if (i % 12 == 0) Azioni_Inietta(&s->az, AZ_CONFERMA);
        }
        if (s->lotta->stato == BUI_EVOLUZIONE && i % 12 == 0) Azioni_Inietta(&s->az, AZ_CONFERMA);
        Frames(s, 1);
    }
    return s->scena == SC_LOTTA && s->lotta->stato == voluto;
}

static void FinoAlMondo(Slice *s, int max) {
    for (int i = 0; i < max && s->scena != SC_MONDO; i++) {
        if (i % 12 == 0) Azioni_Inietta(&s->az, AZ_CONFERMA);
        if (s->scena == SC_LOTTA && s->lotta->stato == BUI_MENU) s->lotta->cursoreMenu = 0;
        if (s->scena == SC_LOTTA && (s->lotta->stato == BUI_MENU || s->lotta->stato == BUI_MOSSE) && i % 12 == 6) Azioni_Inietta(&s->az, AZ_CONFERMA);
        Frames(s, 1);
    }
}

static void ChiudiDialogo(Slice *s) {
    for (int i = 0; i < 400 && s->world.dialogo.attivo; i++) { if (i % 10 == 0) Azioni_Inietta(&s->az, AZ_CONFERMA); Frames(s, 1); }
}

static void Percorso(Slice *s, int starterIdx, bool telefono) {
    static const int sp[3] = { 1, 4, 7 };
    s->scena = SC_TITOLO; s->cursoreTitolo = 0; s->dissolvenza = 1.0f; s->pannello = PAN_NESSUNO;
    Frames(s, 50);
    Foto(s, "01_titolo");
    Premi(s, AZ_CONFERMA);
    for (int i = 0; i < starterIdx; i++) Premi(s, AZ_DESTRA);
    Frames(s, 20);
    Foto(s, "02_starter");
    Premi(s, AZ_CONFERMA);
    (void)sp;
    Frames(s, 40);
    Foto(s, "03_lunaria_arrivo");
    Frames(s, 140);
    if (telefono) { s->az.touchVisibile = true; }
    Foto(s, "04_lunaria");
    /* dialogo con Bruno (NPC del gioco, testo dal database dei dialoghi) */
    World_Teletrasporta(&s->world, "lunaria", 13, 13, DIR_SU);
    s->world.timerNomeMappa = 0;
    s->world.npc[2].tx = 13; s->world.npc[2].ty = 12; s->world.npc[2].px = 13 * 16; s->world.npc[2].py = 12 * 16;
    Frames(s, 4);
    Premi(s, AZ_CONFERMA);
    Frames(s, 150);
    Foto(s, "05_dialogo");
    ChiudiDialogo(s);
    /* Route 1 ed erba alta */
    World_Teletrasporta(&s->world, "route1", 29, 19, DIR_SU);
    s->world.timerNomeMappa = 150;
    Frames(s, 30);
    Foto(s, "06_route1_arrivo");
    World_Teletrasporta(&s->world, "route1", 12, 6, DIR_DESTRA);
    s->world.timerNomeMappa = 0;
    Frames(s, 20);
    Foto(s, "07_route1_erba_alta");
    /* lotta selvatica: menu, mosse, colpo, cattura */
    Slice_AvviaSelvatica(s, 7, 4);
    Frames(s, 30);
    Foto(s, "08_lotta_ingresso");
    if (FinoA(s, BUI_MENU, 900)) {
        Foto(s, "09_lotta_rosa");
        Premi(s, AZ_CONFERMA);
        Frames(s, 6);
        Foto(s, "10_lotta_mosse");
        Premi(s, AZ_DESTRA);
        Frames(s, 4);
        Foto(s, "11_lotta_mosse_dettaglio");
        Premi(s, AZ_SINISTRA);
        Premi(s, AZ_CONFERMA);
        Frames(s, 34);
        Foto(s, "12_lotta_colpo");
        if (FinoA(s, BUI_MENU, 900)) {
            /* cattura con un Core */
            Premi(s, AZ_SINISTRA);
            Foto(s, "13_lotta_zaino_cursore");
            Premi(s, AZ_CONFERMA);
            Frames(s, 4);
            Foto(s, "14_lotta_zaino");
            for (int i = 0; i < 6 && s->lotta->stato == BUI_ZAINO; i++) {
                int lista = s->lotta->cursoreZaino;
                (void)lista;
                if (Inventario_Quantita(&s->partita.inventario, ITEM_CORE) > 0) break;
            }
            /* il Core e' il secondo oggetto della lista (Erba vitale, Core) */
            Premi(s, AZ_GIU);
            Premi(s, AZ_CONFERMA);
            Frames(s, 60);
            Foto(s, "15_cattura");
        }
    }
    FinoAlMondo(s, 2500);
    /* lotta con Kael */
    const char *st = World_Starter(&s->world);
    if (st) {
        Collezione_CuraSquadra(&s->partita.collezione);
        Slice_AvviaAllenatore(s, GetTrainer(TextFormat("KAEL_INIZIALE_VS_%s", st)));
        if (FinoA(s, BUI_MENU, 900)) Foto(s, "16_lotta_kael");
        int turni0 = s->lotta->lotta.turno;
        FinoAlMondo(s, 6000);
        printf("  prima lotta con Kael (%s, giocatore automatico: sempre la prima mossa): %s in %d turni\n", st,
               s->lotta->esito == ESITO_VITTORIA ? "VITTORIA" : s->lotta->esito == ESITO_SCONFITTA ? "SCONFITTA" : "altro",
               s->lotta->lotta.turno - turni0);
    }
    /* menu e schermate (davanti alla piazza di Lunaria) */
    ChiudiDialogo(s);
    World_Teletrasporta(&s->world, "lunaria", 20, 16, DIR_GIU);
    s->world.timerNomeMappa = 0;
    Frames(s, 40);
    Foto(s, "16b_lunaria_piazza");
    Pannelli_Apri(s, PAN_MENU); s->cursoreMenu = 0;
    Frames(s, 4);
    Foto(s, "17_menu");
    Premi(s, AZ_CONFERMA); Frames(s, 4);
    Foto(s, "18_squadra");
    Premi(s, AZ_CONFERMA); Frames(s, 4);
    Foto(s, "19_scheda_statistiche");
    Premi(s, AZ_DESTRA); Frames(s, 4);
    Foto(s, "20_scheda_mosse");
    Premi(s, AZ_DESTRA); Frames(s, 4);
    Foto(s, "21_scheda_info");
    s->pannello = PAN_MENU; s->cursoreMenu = 1;
    Premi(s, AZ_CONFERMA); Frames(s, 4);
    Foto(s, "22_zaino");
    s->pannello = PAN_MENU; s->cursoreMenu = 2;
    Premi(s, AZ_CONFERMA); Frames(s, 4);
    Foto(s, "23_bestiario");
    for (int i = 0; i < 3; i++) Premi(s, AZ_GIU);
    Frames(s, 4);
    Foto(s, "24_bestiario_voce");
    Premi(s, AZ_DESTRA); Frames(s, 4);
    Premi(s, AZ_CONFERMA); Frames(s, 30);
    Foto(s, "25_bestiario_dettaglio");
    s->pannello = PAN_MENU; s->cursoreMenu = 3;
    Premi(s, AZ_CONFERMA); Frames(s, 4);
    Foto(s, "26_mappa");
    s->pannello = PAN_MENU; s->cursoreMenu = 4;
    Premi(s, AZ_CONFERMA); Frames(s, 4);
    Foto(s, "27_scheda_allenatore");
    s->pannello = PAN_NESSUNO;
    /* evoluzione */
    Slice_AvviaEvoluzione(s, s->partita.collezione.squadra[0].specie);
    Frames(s, 50);
    Foto(s, "28_evoluzione_annuncio");
    Premi(s, AZ_CONFERMA);
    Frames(s, 120);
    Foto(s, "29_evoluzione_luce");
    Frames(s, 110);
    Foto(s, "30_evoluzione_finale");
    Premi(s, AZ_CONFERMA);
    Frames(s, 10);
}

int Tour_Esegui(Slice *s, const char *cartella) {
    dir = cartella;
    s->passoFisso = true;
    MakeDirectory(cartella);
    struct { const char *nome; int w, h; bool touch; int starter; } formati[3] = {
        { "16x9_pc", 1280, 720, false, 0 }, { "19x9_telefono", 1560, 720, true, 1 }, { "16x10_steamdeck", 1280, 800, false, 2 } };
    for (int f = 0; f < 3; f++) {
        snprintf(sotto, sizeof(sotto), "%s", formati[f].nome);
        char p[700];
        snprintf(p, sizeof(p), "%s/%s", cartella, sotto);
        MakeDirectory(p);
        SetWindowSize(formati[f].w, formati[f].h);
        s->az.forzaTouch = formati[f].touch;
        s->az.touchVisibile = formati[f].touch;
        s->az.ultimo = formati[f].touch ? DISP_TOUCH : DISP_TASTIERA;   /* i prompt dei tasti spariscono sul telefono */
        Frames(s, 10);
        printf("tour %s: canvas %dx%d x%d\n", sotto, s->canvas.w, s->canvas.h, s->canvas.scale);
        Percorso(s, formati[f].starter, formati[f].touch);
    }
    printf("tour: %d foto, %d errori\n", numFoto, errori);
    return errori ? 1 : 0;
}
