/* test_bilanciamento.c - Bilanciamento della prima lotta con Kael (Lv5 contro Lv5).
 *
 * Per ogni starter simula N lotte con due tipi di giocatore:
 *   - ESPERTO: usa sempre la mossa con il danno atteso piu' alto (mosse di stato escluse);
 *   - PRINCIPIANTE: sceglie a caso tra le mosse d'attacco, senza badare ai tipi.
 * Nessun oggetto in battaglia. Semi fissi: i risultati sono riproducibili.
 *
 * Uso: test_bilanciamento [N] [--confronto]
 *   --confronto stampa anche i risultati con la vecchia IA casuale (AI_SIMPLE). */
#include "battle.h"
#include "data/trainers_data.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { ESPERTO, PRINCIPIANTE } Strategia;

static const char *STARTER[3] = { "PYRIN", "AQUIL", "MOSSEN" };
static const char *SFIDA[3] = { "KAEL_INIZIALE_VS_PYRIN", "KAEL_INIZIALE_VS_AQUIL", "KAEL_INIZIALE_VS_MOSSEN" };

static Azione SceltaGiocatore(Lotta *L, Strategia s, Rng *r) {
    Azione a = { AZ_MOSSA, MOSSA_DIMENARSI, 0, 0 };
    if (Lotta_DeveDimenarsi(L)) return a;
    const Elyon *e = L->lato[LATO_GIOCATORE].e;
    int validi[MAX_MOSSE_ELYON], nv = 0, best = -1, bestDanno = -1;
    for (int k = 0; k < MAX_MOSSE_ELYON; k++) {
        const MoveData *m = GetMove(e->mosse[k]);
        if (!m || e->pp[k] <= 0) continue;
        if (m->categoria == CAT_STATO) continue;
        validi[nv++] = k;
        int d = Lotta_StimaDanno(L, LATO_GIOCATORE, m->id, 0.925f) * m->precisione / 100;
        if (d > bestDanno) { bestDanno = d; best = k; }
    }
    if (nv == 0) return a;
    a.param = (s == ESPERTO && best >= 0) ? best : validi[Rng_Range(r, nv)];
    return a;
}

typedef struct { int vittorie, sconfitte, turni, hpResiduoVinte, esitazioni; } Risultato;

static Risultato Simula(int indice, Strategia s, LivelloIA ia, int n) {
    static Collezione coll;
    static Inventario inv;
    Risultato res = { 0 };
    TrainerData t = *GetTrainer(SFIDA[indice]);
    t.ia = ia;
    Lotta *L = malloc(sizeof(Lotta));
    for (int i = 0; i < n; i++) {
        Rng r, rl;
        Rng_Init(&r, 77000u + (uint32_t)i * 2654435761u);
        Rng_Init(&rl, 99000u + (uint32_t)i * 40503u);
        Collezione_Azzera(&coll);
        Inventario_Azzera(&inv);
        Elyon mio;
        Elyon_Crea(&mio, GetSpeciesByName(STARTER[indice])->id, 5, &r);
        Collezione_Aggiungi(&coll, &mio);
        if (!Lotta_InitAllenatore(L, &coll, &inv, &rl, &t)) continue;
        Lotta_Inizia(L);
        int turni = 0;
        while (!Lotta_Finita(L) && turni < 200) {
            Lotta_Turno(L, SceltaGiocatore(L, s, &r));
            turni++;
            for (int k = 0; k < L->numEventi; k++) res.esitazioni += strstr(L->eventi[k].testo, " esita") != NULL;
        }
        res.turni += turni;
        if (L->esito == ESITO_VITTORIA) {
            res.vittorie++;
            res.hpResiduoVinte += coll.squadra[0].hp * 100 / coll.squadra[0].stat[STAT_HP];
        } else if (L->esito == ESITO_SCONFITTA) res.sconfitte++;
    }
    free(L);
    return res;
}

static double Perc(int a, int n) { return n ? 100.0 * a / n : 0; }

static void Tabella(LivelloIA ia, const char *nome, int n, double out[3][2]) {
    printf("\n  IA %s, %d lotte per riga\n", nome, n);
    printf("  %-8s %-12s %8s %8s %10s %9s\n", "starter", "giocatore", "vittorie", "turni", "HP rimasti", "esitazioni");
    for (int i = 0; i < 3; i++)
        for (int s = 0; s < 2; s++) {
            Risultato r = Simula(i, (Strategia)s, ia, n);
            out[i][s] = Perc(r.vittorie, n);
            printf("  %-8s %-12s %7.1f%% %8.1f %9d%% %8.0f%%\n", STARTER[i], s == ESPERTO ? "esperto" : "principiante",
                   out[i][s], (double)r.turni / n, r.vittorie ? r.hpResiduoVinte / r.vittorie : 0,
                   r.turni ? 100.0 * r.esitazioni / r.turni : 0.0);
        }
}

int main(int argc, char **argv) {
    int n = argc > 1 && argv[1][0] != '-' ? atoi(argv[1]) : 4000;
    bool confronto = false;
    for (int i = 1; i < argc; i++) confronto |= strcmp(argv[i], "--confronto") == 0;
    if (n < 100) n = 100;

    printf("Prima lotta con Kael: Lv5 contro Lv5, nessun oggetto\n");
    double prima[3][2], dopo[3][2];
    if (confronto) Tabella(AI_SIMPLE, "SIMPLE (Fase 4)", n, prima);

    /* I dati devono usare l'IA introduttiva per tutte e tre le varianti */
    for (int i = 0; i < 3; i++) VERIFICA(GetTrainer(SFIDA[i])->ia == AI_INTRO);
    Tabella(AI_INTRO, "INTRO", n, dopo);

    /* Obiettivi: introduttiva ma non regalata.
     *  - esperto: 70-95% con ogni starter
     *  - principiante: almeno 40% con ogni starter (la sconfitta resta possibile)
     *  - differenza tra starter (esperto) non oltre 20 punti */
    double min = 100, max = 0;
    for (int i = 0; i < 3; i++) {
        VERIFICA(dopo[i][ESPERTO] >= 70.0 && dopo[i][ESPERTO] <= 95.0);
        VERIFICA(dopo[i][PRINCIPIANTE] >= 40.0);
        if (dopo[i][ESPERTO] < min) min = dopo[i][ESPERTO];
        if (dopo[i][ESPERTO] > max) max = dopo[i][ESPERTO];
    }
    VERIFICA(max - min <= 20.0);
    return FINE_TEST();
}
