/* test_elyon.c - Formule delle statistiche, nature, curva XP, mosse, evoluzioni, squadra e box. */
#include "elyon.h"
#include "item_use.h"
#include "data/items_data.h"
#include "data/evolution_data.h"
#include "test_framework.h"
#include <stdlib.h>
#include <string.h>

static void TestFormule(void) {
    /* HP = base + 3L + floor(L/2) + IV */
    VERIFICA(Elyon_StatFormula(STAT_HP, 45, 5, 10, NATURA_NEUTRO) == 45 + 15 + 2 + 10);
    VERIFICA(Elyon_StatFormula(STAT_HP, 100, 100, 15, NATURA_CORAGGIOSO) == 100 + 300 + 50 + 15);
    /* altre = floor((base + 2L + IV) * natura) */
    VERIFICA(Elyon_StatFormula(STAT_ATK, 50, 10, 5, NATURA_NEUTRO) == 75);
    VERIFICA(Elyon_StatFormula(STAT_ATK, 50, 10, 5, NATURA_CORAGGIOSO) == 82);   /* 75*1.1 = 82.5 */
    VERIFICA(Elyon_StatFormula(STAT_SPA, 50, 10, 5, NATURA_CORAGGIOSO) == 67);   /* 75*0.9 = 67.5 */
    VERIFICA(Elyon_StatFormula(STAT_DEF, 50, 10, 5, NATURA_CORAGGIOSO) == 75);
    /* nature: ognuna (tranne NEUTRO) alza una stat e ne abbassa un'altra, mai gli HP */
    for (int n = 0; n < NUM_NATURE; n++) {
        int su = 0, giu = 0;
        VERIFICA(Natura_Percento((Natura)n, STAT_HP) == 100);
        for (int s = STAT_ATK; s < NUM_STAT; s++) {
            int p = Natura_Percento((Natura)n, (StatId)s);
            VERIFICA(p == 90 || p == 100 || p == 110);
            su += p == 110;
            giu += p == 90;
        }
        VERIFICA(su == (n == NATURA_NEUTRO ? 0 : 1));
        VERIFICA(giu == (n == NATURA_NEUTRO ? 0 : 1));
        VERIFICA(strcmp(Natura_Nome((Natura)n), "?") != 0);
    }
    VERIFICA(strcmp(Natura_Nome(NUM_NATURE), "?") == 0);
}

static void TestCurvaXp(void) {
    for (int id = 1; id <= 100; id++) {
        const SpeciesData *s = GetSpecies(id);
        VERIFICA(Elyon_XpPerLivello(s, 1) == 0);
        int32_t prec = 0;
        bool crescente = true;
        for (int l = 2; l <= LIVELLO_MAX; l++) {
            int32_t x = Elyon_XpPerLivello(s, l);
            if (x <= prec) crescente = false;
            prec = x;
        }
        VERIFICA(crescente);
        /* nessun overflow: crescita massima 1.2 * 100^3 = 1.200.000 */
        VERIFICA(Elyon_XpPerLivello(s, LIVELLO_MAX) > 0 && Elyon_XpPerLivello(s, LIVELLO_MAX) <= 1200000);
        VERIFICA(Elyon_XpPerLivello(s, 500) == Elyon_XpPerLivello(s, LIVELLO_MAX));
    }
    const SpeciesData *pyrin = GetSpeciesByName("PYRIN");
    VERIFICA(pyrin && Elyon_XpPerLivello(pyrin, 10) == 1000);      /* crescita 1.0 */
    VERIFICA(Elyon_XpPerLivello(NULL, 10) == 0);
}

static void TestCreazione(void) {
    Rng rng;
    Rng_Init(&rng, 1234);
    for (int id = 1; id <= 100; id++) {
        for (int l = 1; l <= 100; l += 33) {
            Elyon e;
            Elyon_Crea(&e, id, l, &rng);
            const SpeciesData *s = GetSpecies(id);
            VERIFICA(e.specie == id && e.livello == l);
            VERIFICA(e.hp == e.stat[STAT_HP] && e.hp > 0);
            VERIFICA(e.xp == Elyon_XpPerLivello(s, l));
            bool ivOk = true;
            for (int k = 0; k < NUM_STAT; k++) if (e.iv[k] < 0 || e.iv[k] > IV_MAX) ivOk = false;
            VERIFICA(ivOk);
            VERIFICA(e.natura >= 0 && e.natura < NUM_NATURE);
            VERIFICA(Elyon_NumMosse(&e) >= 1 && Elyon_NumMosse(&e) <= 4);
            /* tutte le mosse conosciute sono nel learnset entro il livello, con PP pieni */
            bool mosseOk = true;
            for (int k = 0; k < MAX_MOSSE_ELYON; k++) {
                if (!e.mosse[k]) continue;
                bool trovata = false;
                for (int j = 0; j < s->numLearnset; j++)
                    if (s->learnset[j].mossa == e.mosse[k] && s->learnset[j].livello <= l) trovata = true;
                if (!trovata || e.pp[k] != GetMove(e.mosse[k])->pp) mosseOk = false;
            }
            VERIFICA(mosseOk);
        }
    }
    Elyon v;
    Elyon_Crea(&v, 0, 5, &rng);
    VERIFICA(Elyon_Vuoto(&v));
    Elyon_Crea(&v, 101, 5, &rng);
    VERIFICA(Elyon_Vuoto(&v));
    Elyon_Crea(&v, 1, 0, &rng);
    VERIFICA(v.livello == 1);
    Elyon_Crea(&v, 1, 250, &rng);
    VERIFICA(v.livello == LIVELLO_MAX);

    /* variante Lumen ~1/256: su 51200 creazioni attese ~200 */
    int lumen = 0;
    for (int i = 0; i < 51200; i++) {
        Elyon e;
        Elyon_Crea(&e, 10, 5, &rng);
        lumen += e.varianteLumen;
    }
    VERIFICA(lumen > 120 && lumen < 300);
    printf("  varianti Lumen: %d su 51200\n", lumen);
}

static void TestXpLivelli(void) {
    Rng rng;
    Rng_Init(&rng, 99);
    Elyon e;
    Elyon_Crea(&e, GetSpeciesByName("PYRIN")->id, 5, &rng);
    int hpMaxPrima = e.stat[STAT_HP];
    e.hp = hpMaxPrima / 2;
    int amiciziaPrima = e.amicizia;
    int saliti = Elyon_AggiungiXp(&e, Elyon_XpPerLivello(GetSpecies(e.specie), 8) - e.xp);
    VERIFICA(saliti == 3 && e.livello == 8);
    VERIFICA(e.stat[STAT_HP] > hpMaxPrima);
    /* HP in proporzione: chi era a meta' resta circa a meta' */
    VERIFICA(e.hp * 2 >= e.stat[STAT_HP] - 1 && e.hp * 2 <= e.stat[STAT_HP] + 2);
    VERIFICA(e.amicizia == amiciziaPrima + 9);
    VERIFICA(Elyon_AggiungiXp(&e, 0) == 0 && Elyon_AggiungiXp(&e, -50) == 0);
    /* XP enorme: si ferma al 100 senza overflow */
    Elyon_AggiungiXp(&e, 2000000000);
    VERIFICA(e.livello == LIVELLO_MAX);
    VERIFICA(e.xp == Elyon_XpPerLivello(GetSpecies(e.specie), LIVELLO_MAX));
    VERIFICA(Elyon_AggiungiXp(&e, 1000) == 0);
    VERIFICA(e.amicizia <= AMICIZIA_MAX);

    /* un Elyon esausto resta esausto quando sale di livello */
    Elyon k;
    Elyon_Crea(&k, 10, 5, &rng);
    k.hp = 0;
    Elyon_AggiungiXp(&k, 5000);
    VERIFICA(k.hp == 0);

    /* mosse al livello */
    Elyon p;
    Elyon_Crea(&p, GetSpeciesByName("PYRIN")->id, 1, &rng);
    int m[4];
    const SpeciesData *s = GetSpecies(p.specie);
    int attese = 0;
    for (int i = 0; i < s->numLearnset; i++) attese += s->learnset[i].livello == 9;
    VERIFICA(Elyon_MosseAlLivello(&p, 9, m, 4) == attese);
    VERIFICA(Elyon_MosseAlLivello(&p, 2, m, 4) == 0 || s->learnset[0].livello == 2);
}

static void TestMosse(void) {
    Elyon e;
    memset(&e, 0, sizeof(e));
    e.specie = 1;
    VERIFICA(Elyon_ImparaMossa(&e, 1));
    VERIFICA(Elyon_ImparaMossa(&e, 1));                  /* gia' conosciuta: nessun doppione */
    VERIFICA(Elyon_NumMosse(&e) == 1);
    VERIFICA(Elyon_ImparaMossa(&e, 2) && Elyon_ImparaMossa(&e, 3) && Elyon_ImparaMossa(&e, 4));
    VERIFICA(!Elyon_ImparaMossa(&e, 5));                  /* quattro mosse: serve sostituire */
    VERIFICA(!Elyon_ImparaMossa(&e, 999));
    Elyon_SostituisciMossa(&e, 0, 5);
    VERIFICA(e.mosse[0] == 5 && e.pp[0] == GetMove(5)->pp);
    Elyon_SostituisciMossa(&e, 7, 6);                     /* slot fuori range ignorato */
    Elyon_SostituisciMossa(&e, 1, 0);                     /* mossa inesistente ignorata */
    VERIFICA(e.mosse[1] == 2);
}

static void TestEvoluzioni(void) {
    Rng rng;
    Rng_Init(&rng, 7);
    for (int i = 0; i < NumEvoluzioni(); i++) {
        const EvolutionData *ev = GetEvolutionIndex(i);
        Elyon e;
        Elyon_Crea(&e, ev->da, ev->livello - 1, &rng);
        VERIFICA(Elyon_PuoEvolvere(&e) == 0);             /* mai prima del livello definito */
        Elyon_AggiungiXp(&e, Elyon_XpPerLivello(GetSpecies(ev->da), ev->livello) - e.xp);
        VERIFICA(e.livello == ev->livello);
        VERIFICA(Elyon_PuoEvolvere(&e) == ev->a);
        int hp = e.stat[STAT_HP];
        Elyon_Evolvi(&e, ev->a);
        VERIFICA(e.specie == ev->a && e.livello == ev->livello);
        VERIFICA(e.stat[STAT_HP] >= hp);
        /* la famiglia condivide la curva: l'XP resta coerente col livello */
        VERIFICA(e.xp >= Elyon_XpPerLivello(GetSpecies(ev->a), e.livello));
    }
    Elyon e;
    Elyon_Crea(&e, 100, 50, &rng);
    VERIFICA(Elyon_PuoEvolvere(&e) == 0);
}

static void TestCollezione(void) {
    Rng rng;
    Rng_Init(&rng, 3);
    Collezione *c = malloc(sizeof(Collezione));
    Collezione_Azzera(c);
    VERIFICA(Collezione_NumSquadra(c) == 0 && Collezione_NumBox(c) == 0 && Collezione_SquadraViva(c) == 0);
    Elyon e;
    Elyon_Crea(&e, 10, 5, &rng);
    for (int i = 0; i < MAX_SQUADRA; i++) VERIFICA(Collezione_Aggiungi(c, &e) == 0);
    VERIFICA(Collezione_NumSquadra(c) == 6);
    for (int i = 0; i < NUM_BOX * POSTI_BOX; i++) VERIFICA(Collezione_Aggiungi(c, &e) == 1);
    VERIFICA(Collezione_NumBox(c) == 100);
    VERIFICA(Collezione_Aggiungi(c, &e) == -1);          /* tutto pieno */
    VERIFICA(!Collezione_Deposita(c, 0));                 /* box pieno */

    /* deposita e ritira */
    memset(&c->box[2][5], 0, sizeof(Elyon));
    VERIFICA(Collezione_Deposita(c, 3));
    VERIFICA(Collezione_NumSquadra(c) == 5 && Collezione_NumBox(c) == 100);
    VERIFICA(!Elyon_Vuoto(&c->squadra[4]) && Elyon_Vuoto(&c->squadra[5]));   /* compattata */
    VERIFICA(Collezione_Ritira(c, 0, 0));
    VERIFICA(Collezione_NumSquadra(c) == 6);
    VERIFICA(!Collezione_Ritira(c, 0, 1));                /* squadra piena */
    VERIFICA(!Collezione_Ritira(c, 9, 0) && !Collezione_Ritira(c, 0, 99) && !Collezione_Ritira(c, 0, 0));

    /* la squadra non resta mai vuota */
    Collezione_Azzera(c);
    Collezione_Aggiungi(c, &e);
    VERIFICA(!Collezione_Deposita(c, 0));
    VERIFICA(!Collezione_Deposita(c, 4) && !Collezione_Deposita(c, -1));

    /* cura e squadra viva */
    c->squadra[0].hp = 0;
    c->squadra[0].status = STATUS_VELENO;
    VERIFICA(Collezione_SquadraViva(c) == 0);
    Collezione_CuraSquadra(c);
    VERIFICA(Collezione_SquadraViva(c) == 1 && c->squadra[0].status == STATUS_NESSUNO);
    VERIFICA(c->squadra[0].hp == c->squadra[0].stat[STAT_HP]);
    free(c);
}

static void TestOggetti(void) {
    Rng rng;
    Rng_Init(&rng, 5);
    Elyon e;
    Elyon_Crea(&e, 10, 30, &rng);
    char msg[96];
    VERIFICA(!Oggetto_Usa(&e, ITEM_ERBA_VITALE, 0, msg, sizeof(msg)));   /* HP pieni */
    e.hp = 10;
    VERIFICA(Oggetto_Usa(&e, ITEM_ERBA_VITALE, 0, msg, sizeof(msg)) && e.hp == 40);
    e.hp = e.stat[STAT_HP] - 5;
    VERIFICA(Oggetto_Usa(&e, ITEM_FRUTTO_LUMEN, 0, msg, sizeof(msg)) && e.hp == e.stat[STAT_HP]);
    VERIFICA(!Oggetto_Usa(&e, ITEM_ANTIDOTO, 0, msg, sizeof(msg)));
    e.status = STATUS_VELENO_GRAVE;
    VERIFICA(Oggetto_Usa(&e, ITEM_ANTIDOTO, 0, msg, sizeof(msg)) && e.status == STATUS_NESSUNO);
    e.status = STATUS_SONNO;
    VERIFICA(!Oggetto_Usa(&e, ITEM_FIAMMIFUGA, 0, msg, sizeof(msg)));
    VERIFICA(Oggetto_Usa(&e, ITEM_RISVEGLIO, 0, msg, sizeof(msg)));
    e.pp[0] = 0;
    VERIFICA(Oggetto_Usa(&e, ITEM_ELISIR, 0, msg, sizeof(msg)) && e.pp[0] == Utils_ClampInt(10, 0, GetMove(e.mosse[0])->pp));
    VERIFICA(Oggetto_Usa(&e, ITEM_ELISIR_MAX, 0, msg, sizeof(msg)) || e.pp[0] == GetMove(e.mosse[0])->pp);
    VERIFICA(e.pp[0] == GetMove(e.mosse[0])->pp);
    VERIFICA(!Oggetto_Usa(&e, ITEM_ELISIR, 9, msg, sizeof(msg)));
    /* rianimazione */
    VERIFICA(!Oggetto_Usa(&e, ITEM_REVITA, 0, msg, sizeof(msg)));
    e.hp = 0;
    VERIFICA(!Oggetto_Usa(&e, ITEM_ERBA_VITALE, 0, msg, sizeof(msg)));     /* niente cure agli esausti */
    VERIFICA(Oggetto_Usa(&e, ITEM_REVITA, 0, msg, sizeof(msg)) && e.hp >= e.stat[STAT_HP] * 30 / 100 && e.hp > 0);
    e.hp = 0;
    VERIFICA(Oggetto_Usa(&e, ITEM_REVITA_MAX, 0, msg, sizeof(msg)) && e.hp == e.stat[STAT_HP]);
    VERIFICA(!Oggetto_Usa(&e, ITEM_CORE, 0, msg, sizeof(msg)));
    VERIFICA(Oggetto_FattoreCore(ITEM_CORE) == 1.0f && Oggetto_FattoreCore(ITEM_CORE_PLUS) == 1.5f);
    VERIFICA(Oggetto_FattoreCore(ITEM_CORE_MAX) == 2.5f && Oggetto_FattoreCore(ITEM_CORE_VOID) == 3.5f);
    VERIFICA(Oggetto_FattoreCore(ITEM_ERBA_VITALE) == 0.0f);
}

int main(void) {
    printf("test_elyon\n");
    TestFormule();
    TestCurvaXp();
    TestCreazione();
    TestXpLivelli();
    TestMosse();
    TestEvoluzioni();
    TestCollezione();
    TestOggetti();
    return FINE_TEST();
}
