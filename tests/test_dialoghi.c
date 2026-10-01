/* test_dialoghi.c - Database dei testi, varianti con flag, pagine, segnaposto,
 * avanzamento del testo progressivo e riferimenti dalle mappe.
 * Uso: test_dialoghi <cartella assets> */
#include "dialogue.h"
#include "map.h"
#include "test_framework.h"
#include <string.h>

int main(int argc, char **argv) {
    const char *assets = argc > 1 ? argv[1] : "assets";
    char err[200], percorso[512];

    /* 1) parsing di un testo di prova */
    DialogoDb db;
    const char *src =
        "# commento\n"
        "@SALUTO\n"
        "Elian: Ciao {GIOCATORE}!\n"
        "Seconda riga.\n"
        "\n"
        "Pagina due con {RIVALE}.\n"
        "@SALUTO?FATTO\n"
        "Elian: Di nuovo tu!\n"
        "@ACCENTI\n"
        "Città è già là.\n";
    VERIFICA(DialogoDb_CaricaDaTesto(&db, src, err, sizeof(err)));
    VERIFICA(db.numVoci == 3);
    Flags flags;
    Flags_Azzera(&flags);
    const VoceDialogo *v = DialogoDb_Trova(&db, "SALUTO", &flags);
    VERIFICA(v && strcmp(v->parlante, "Elian") == 0 && v->condizione[0] == '\0');
    Flags_Imposta(&flags, "FATTO");
    v = DialogoDb_Trova(&db, "SALUTO", &flags);
    VERIFICA(v && strcmp(v->condizione, "FATTO") == 0);
    VERIFICA(DialogoDb_Trova(&db, "NON_ESISTE", &flags) == NULL);
    Flags_Rimuovi(&flags, "FATTO");
    VERIFICA(!Flags_Attivo(&flags, "FATTO"));

    /* 2) dialogo: segnaposto, pagine, testo progressivo e salto */
    Dialogo d;
    Dialogo_Avvia(&d, &db, "SALUTO", &flags, "ARI", "");
    VERIFICA(d.attivo && d.numPagine == 2);
    VERIFICA(strcmp(d.pagine[0], "Ciao ARI!\nSeconda riga.") == 0);
    VERIFICA(strcmp(d.pagine[1], "Pagina due con KAEL.") == 0);
    VERIFICA(strcmp(d.parlante, "Elian") == 0);
    char vis[256];
    Dialogo_TestoVisibile(&d, vis, sizeof(vis));
    VERIFICA(vis[0] == '\0');
    for (int i = 0; i < 4; i++) Dialogo_Aggiorna(&d, false, 1.0f);
    Dialogo_TestoVisibile(&d, vis, sizeof(vis));
    VERIFICA(strcmp(vis, "Ciao") == 0);
    Dialogo_Aggiorna(&d, true, 1.0f);              /* Z durante la scrittura: mostra tutto */
    VERIFICA(Dialogo_PaginaCompleta(&d) && d.pagina == 0);
    Dialogo_Aggiorna(&d, true, 1.0f);              /* Z a pagina completa: pagina successiva */
    VERIFICA(d.pagina == 1 && !Dialogo_PaginaCompleta(&d));
    Dialogo_Aggiorna(&d, true, 1.0f);
    VERIFICA(Dialogo_Aggiorna(&d, true, 1.0f) == false && !d.attivo);   /* fine */

    /* 3) i caratteri accentati (UTF-8 a 2 byte) non vengono spezzati */
    Dialogo_Avvia(&d, &db, "ACCENTI", &flags, "ARI", "");
    for (int i = 0; i < 4; i++) Dialogo_Aggiorna(&d, false, 1.0f);   /* "Citt" + primo byte di "à" */
    Dialogo_Aggiorna(&d, false, 1.0f);
    Dialogo_TestoVisibile(&d, vis, sizeof(vis));
    VERIFICA(strcmp(vis, "Citt") == 0);
    Dialogo_Aggiorna(&d, false, 1.0f);
    Dialogo_TestoVisibile(&d, vis, sizeof(vis));
    VERIFICA(strcmp(vis, "Città") == 0);

    /* 4) ID mancante: messaggio leggibile, nessun crash */
    Dialogo_Avvia(&d, &db, "NIENTE", &flags, "ARI", "");
    VERIFICA(d.attivo && strstr(d.pagine[0], "NIENTE") != NULL);
    Dialogo_Avvia(&d, NULL, "NIENTE", NULL, NULL, NULL);
    VERIFICA(d.attivo);
    DialogoDb_Libera(&db);

    /* 5) il file del gioco si carica e contiene tutti i testi usati dalle mappe */
    snprintf(percorso, sizeof(percorso), "%s/text/dialoghi.txt", assets);
    VERIFICA(DialogoDb_CaricaFile(&db, percorso, err, sizeof(err)));
    VERIFICA(db.numVoci >= 30);
    VERIFICA(DialogoDb_Trova(&db, "ELIAN_LAB", NULL) != NULL);
    const VoceDialogo *elian = DialogoDb_Trova(&db, "ELIAN_LAB", NULL);
    VERIFICA(elian && strstr(db.testo + elian->inizio,
             "Il Lumen non è semplicemente energia. È ciò che collega ogni cosa vivente.") != NULL);

    char cartella[512];
    snprintf(cartella, sizeof(cartella), "%s/maps", assets);
    static Map m;
    int riferimenti = 0, mancanti = 0, npcTotali = 0;
    for (int i = 0; i < NUM_MAPPE_GIOCO; i++) {
        if (!Map_CaricaFile(&m, cartella, MAPPE_GIOCO[i], err, sizeof(err))) { VERIFICA(0); continue; }
        npcTotali += m.numNpc;
        for (int k = 0; k < m.numNpc; k++) {
            riferimenti++;
            if (!DialogoDb_Trova(&db, m.npc[k].dialogo, NULL)) { mancanti++; printf("  manca %s\n", m.npc[k].dialogo); }
        }
        for (int k = 0; k < m.numCartelli; k++) {
            riferimenti++;
            if (!DialogoDb_Trova(&db, m.cartello[k].dialogo, NULL)) { mancanti++; printf("  manca %s\n", m.cartello[k].dialogo); }
        }
    }
    VERIFICA(riferimenti > 30);
    VERIFICA(mancanti == 0);
    printf("NPC nelle mappe: %d, testi referenziati: %d\n", npcTotali, riferimenti);
    DialogoDb_Libera(&db);

    return FINE_TEST();
}
