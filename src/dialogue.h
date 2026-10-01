/* dialogue.h - Database dei testi e dialogo a schermo con testo progressivo.
 *
 * File assets/text/dialoghi.txt:
 *   @ID                    inizio di un testo
 *   @ID?FLAG               variante usata al posto di @ID quando il FLAG e' attivo
 *   Nome: riga             (facoltativo, prima riga) nome di chi parla
 *   righe di testo...
 *   (riga vuota)           nuova pagina
 * Segnaposto: {GIOCATORE} {RIVALE}
 *
 * Nessuna dipendenza da raylib: il disegno e' in dialogue_ui.c.
 */
#ifndef ECLYRA_DIALOGUE_H
#define ECLYRA_DIALOGUE_H

#include "events.h"
#include <stdbool.h>

#define DIALOGO_MAX_VOCI 512
#define DIALOGO_MAX_PAGINE 16
#define DIALOGO_PAGINA 256

typedef struct {
    char id[32];
    char condizione[32];     /* flag richiesto ("" = nessuno) */
    char parlante[24];
    int inizio, lunghezza;   /* posizione nel buffer di testo */
} VoceDialogo;

typedef struct {
    VoceDialogo voci[DIALOGO_MAX_VOCI];
    int numVoci;
    char *testo;             /* pagine separate da '\f' */
    int dimTesto;
} DialogoDb;

bool DialogoDb_CaricaDaTesto(DialogoDb *db, const char *sorgente, char *errore, int dimErrore);
bool DialogoDb_CaricaFile(DialogoDb *db, const char *percorso, char *errore, int dimErrore);
void DialogoDb_Libera(DialogoDb *db);
/* Sceglie la variante giusta in base ai flag; NULL se l'ID non esiste */
const VoceDialogo *DialogoDb_Trova(const DialogoDb *db, const char *id, const Flags *flags);

/* ------------------------------------------------------------------ */

typedef struct {
    bool attivo;
    char parlante[24];
    char pagine[DIALOGO_MAX_PAGINE][DIALOGO_PAGINA];
    int numPagine;
    int pagina;
    float caratteri;         /* caratteri (UTF-8 byte) mostrati finora */
    int lunghezzaPagina;
} Dialogo;

/* Avvia un testo dal database (con sostituzione dei segnaposto).
 * Se l'ID manca mostra un messaggio di errore leggibile invece di bloccarsi. */
void Dialogo_Avvia(Dialogo *d, const DialogoDb *db, const char *id, const Flags *flags,
                   const char *nomeGiocatore, const char *parlanteDefault);
/* Avvia un testo libero (pagine separate da riga vuota "\n\n") */
void Dialogo_AvviaTesto(Dialogo *d, const char *testo, const char *parlante);
/* caratteriPerPasso: velocita' del testo. conferma: Z/X premuto.
 * Ritorna true finche' il dialogo e' aperto. */
bool Dialogo_Aggiorna(Dialogo *d, bool conferma, float caratteriPerPasso);
/* Testo visibile della pagina corrente (copiato in buf) */
void Dialogo_TestoVisibile(const Dialogo *d, char *buf, int dim);
bool Dialogo_PaginaCompleta(const Dialogo *d);

#endif
