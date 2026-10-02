/* slice.h - Stato della vertical slice visiva di ECLYRA.
 *
 * La slice usa la logica VERA del gioco (mondo, NPC, dialoghi, motore di lotta, IA, XP, evoluzioni)
 * e sostituisce solo il rendering e l'input con quelli del Design System v2 e dell'architettura
 * multipiattaforma. Contenuti: Lunaria, Route 1, lotte, menu, Bestiario, Mappa, dialoghi,
 * Scheda Elyon, Evoluzione. Solo i 9 Elyon di riferimento della STYLE BIBLE. */
#ifndef SLICE_H
#define SLICE_H

#include "azioni.h"
#include "battle_ui.h"
#include "canvas.h"
#include "dialogue.h"
#include "partita.h"
#include "world.h"
#include <stdbool.h>

#define SLICE_VERSIONE "0.1"

typedef enum { SC_TITOLO = 0, SC_STARTER, SC_MONDO, SC_LOTTA, SC_EVOLUZIONE } ScenaSlice;
typedef enum {
    PAN_NESSUNO = 0, PAN_MENU, PAN_SQUADRA, PAN_SCHEDA_ELYON, PAN_ZAINO, PAN_ALLENATORE,
    PAN_BESTIARIO, PAN_MAPPA, PAN_AVVISO
} Pannello;

/* Evoluzione mostrata fuori dalla lotta (vetrina e prove): stessa vista di quella in lotta */
typedef struct {
    int da, a, timer, fase;      /* fase 0 annuncio, 1 animazione, 2 evoluto */
    char nome[24];
} EvoVetrina;

typedef struct {
    /* piattaforma */
    Canvas canvas;
    RenderTexture2D rt;
    int rtW, rtH;
    bool mobile;
    Azioni az;
    Input in;
    int frame;
    float accumulatore;
    bool passoFisso;             /* tour automatico: un passo logico per frame, riproducibile */

    /* gioco */
    Partita partita;
    DialogoDb dialoghi;
    World world;
    BattleUI *lotta;
    Rng rng;
    ScenaSlice scena;
    float dissolvenza;           /* 1 = nero */
    bool esci;

    /* titolo e scelta dello starter */
    int cursoreTitolo, cursoreStarter;

    /* pannelli del menu */
    Pannello pannello, pannelloPrima;
    int cursoreMenu, cursoreLista, cursoreScheda, paginaScheda;
    int cursoreBest;
    int cursoreMappa;
    char avviso[160];
    Pannello pannelloDopoAvviso;

    /* Bestiario della slice: VISTO quando compare in lotta, CATTURATO quando e' in squadra o nel box */
    bool visto[101];
    bool catturato[101];
    char dove[101][40];          /* luogo in cui e' stato visto (habitat) */

    EvoVetrina evo;
    ScenaSlice scenaDopoEvo;
} Slice;

extern Slice S;

/* viste */
void VistaMondo_Disegna(const Slice *s);
void VistaMondo_Banner(const Slice *s);
void VistaMondo_Suggerimento(const Slice *s);
void VistaMondo_Vignetta(const Canvas *c);
void VistaLotta_PreparaInput(Slice *s);        /* rosa dei comandi, carte, tocchi diretti */
void VistaLotta_Disegna(const Slice *s);
void VistaEvoluzione_Disegna(const Canvas *c, int da, int a, int timer, int fase, const char *testo, int frame);
void VistaTitolo_Disegna(const Slice *s);
void VistaStarter_Disegna(const Slice *s);

void Pannelli_Apri(Slice *s, Pannello p);
bool Pannelli_Aggiorna(Slice *s);             /* false quando tutti i pannelli sono chiusi */
void Pannelli_Disegna(const Slice *s);
void Slice_Avviso(Slice *s, const char *testo);
void Slice_Frame(Slice *s);
void Slice_NuovaPartita(Slice *s, int starter);
void Slice_AvviaSelvatica(Slice *s, int specie, int livello);
void Slice_AvviaAllenatore(Slice *s, const TrainerData *t);
void Slice_AvviaEvoluzione(Slice *s, int da);

/* bestiario */
void Bestiario_Aggiorna(Slice *s);
void Bestiario_Disegna(const Slice *s);
void Mappa_Aggiorna(Slice *s);
void Mappa_Disegna(const Slice *s);
void Mappa_Libera(void);

#endif
