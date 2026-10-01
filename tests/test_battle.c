/* test_battle.c - Motore di lotta: formule, ordine dei turni, stati, cattura, fuga, XP, IA
 * e migliaia di lotte simulate con RNG a seme fisso. */
#include "battle.h"
#include "item_use.h"
#include "data/abilities_data.h"
#include "test_framework.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static Collezione coll;
static Inventario inv;
static Rng rng;

/* Elyon "di laboratorio": statistiche scelte a mano, abilita' della specie */
static void Prepara(Elyon *e, int specie, int livello, int hp, int atk, int def, int spa, int spd, int spe) {
    memset(e, 0, sizeof(*e));
    e->specie = specie;
    e->livello = livello;
    e->stat[STAT_HP] = hp;
    e->hp = hp;
    e->stat[STAT_ATK] = atk; e->stat[STAT_DEF] = def;
    e->stat[STAT_SPA] = spa; e->stat[STAT_SPD] = spd; e->stat[STAT_SPE] = spe;
    e->amicizia = 70;
}

static int Specie(const char *n) { return GetSpeciesByName(n)->id; }
static int Mossa(const char *n) { return GetMoveByName(n)->id; }

/* Lotta selvatica 1 contro 1 con Elyon di laboratorio */
static void Duello(Lotta *L, const Elyon *mio, const Elyon *suo) {
    Collezione_Azzera(&coll);
    Collezione_Aggiungi(&coll, mio);
    Lotta_InitSelvatica(L, &coll, &inv, &rng, suo->specie, suo->livello);
    L->nemici[0] = *suo;
}

static bool CercaTesto(const Lotta *L, const char *frammento) {
    for (int i = 0; i < L->numEventi; i++) if (strstr(L->eventi[i].testo, frammento)) return true;
    return false;
}

/* Slot della mossa d'attacco piu' potente con PP */
static int SlotDannoso(const Elyon *e) {
    int best = 0, pot = -1;
    for (int k = 0; k < MAX_MOSSE_ELYON; k++) {
        const MoveData *m = GetMove(e->mosse[k]);
        if (m && e->pp[k] > 0 && m->categoria != CAT_STATO && m->potenza > pot) { pot = m->potenza; best = k; }
    }
    return best;
}

static Azione Attacca(const Lotta *L) {
    Azione a = { AZ_MOSSA, SlotDannoso(L->lato[LATO_GIOCATORE].e), 0, 0 };
    if (Lotta_DeveDimenarsi(L)) a.param = MOSSA_DIMENARSI;
    return a;
}

static int NemiciVivi_Test(const Lotta *L) {
    int n = 0;
    for (int i = 0; i < L->numNemici; i++) n += !Elyon_Esausto(&L->nemici[i]);
    return n;
}

static int ContaEventi(const Lotta *L, TipoEvento t) {
    int n = 0;
    for (int i = 0; i < L->numEventi; i++) n += L->eventi[i].tipo == t;
    return n;
}

static void TestFormule(void) {
    VERIFICA(Lotta_MoltBoost(0) == 1.0f);
    VERIFICA(Lotta_MoltBoost(1) == 1.5f && Lotta_MoltBoost(2) == 2.0f && Lotta_MoltBoost(6) == 4.0f);
    VERIFICA(fabsf(Lotta_MoltBoost(-1) - 2.0f / 3.0f) < 1e-6f && Lotta_MoltBoost(-2) == 0.5f && Lotta_MoltBoost(-6) == 0.25f);
    VERIFICA(Lotta_MoltBoost(9) == 4.0f && Lotta_MoltBoost(-9) == 0.25f);   /* limitati a +-6 */

    VERIFICA(Lotta_MoltMeteo(METEO_SOLE, TYPE_FIAMMA) == 1.2f && Lotta_MoltMeteo(METEO_SOLE, TYPE_ACQUA) == 0.8f);
    VERIFICA(Lotta_MoltMeteo(METEO_PIOGGIA, TYPE_ACQUA) == 1.2f && Lotta_MoltMeteo(METEO_PIOGGIA, TYPE_FIAMMA) == 0.8f);
    VERIFICA(Lotta_MoltMeteo(METEO_NEVE, TYPE_GELO) == 1.2f);
    VERIFICA(Lotta_MoltMeteo(METEO_TEMPESTA, TYPE_IMPULSO) == 1.2f && Lotta_MoltMeteo(METEO_TEMPESTA, TYPE_VENTO) == 1.1f);
    VERIFICA(Lotta_MoltMeteo(METEO_NEBBIA, TYPE_FIAMMA) == 1.0f && Lotta_MoltMeteo(METEO_NESSUNO, TYPE_NONE) == 1.0f);

    /* fuga */
    VERIFICA(Lotta_ProbFuga(50, 50, 1) == 1.0f && Lotta_ProbFuga(80, 50, 1) == 1.0f);
    VERIFICA(fabsf(Lotta_ProbFuga(25, 50, 1) - 0.5f) < 1e-5f);
    VERIFICA(fabsf(Lotta_ProbFuga(25, 50, 2) - 0.7f) < 1e-5f);
    VERIFICA(Lotta_ProbFuga(0, 50, 1) == 0.25f && Lotta_ProbFuga(1, 0, 1) == 1.0f);
    VERIFICA(Lotta_ProbFuga(10, 100, 50) == 1.0f);

    /* XP: max(1, floor(L * resa * mod / partecipanti)) */
    VERIFICA(Lotta_XpGuadagnata(5, 6, false, 1) == 30);
    VERIFICA(Lotta_XpGuadagnata(5, 6, true, 1) == 45);
    VERIFICA(Lotta_XpGuadagnata(5, 6, false, 2) == 15);
    VERIFICA(Lotta_XpGuadagnata(5, 6, false, 0) == 30);          /* divisione per zero protetta */
    VERIFICA(Lotta_XpGuadagnata(1, 1, false, 6) == 1);           /* minimo 1 */
    VERIFICA(Lotta_XpGuadagnata(100, 2000000000, true, 1) == 1000000);   /* niente overflow */
}

static void TestCatturaFormula(void) {
    Rng_Init(&rng, 11);
    Elyon e;
    Elyon_Crea(&e, Specie("FLARO"), 5, &rng);      /* comune: 0.20 */
    VERIFICA(fabsf(Lotta_ProbCattura(&e, 1.0f, false) - 0.20f) < 1e-5f);
    e.hp = 0;
    VERIFICA(fabsf(Lotta_ProbCattura(&e, 1.0f, false) - 0.60f) < 1e-5f);
    e.hp = e.stat[STAT_HP];
    e.status = STATUS_SONNO;
    VERIFICA(fabsf(Lotta_ProbCattura(&e, 1.0f, false) - 0.30f) < 1e-5f);
    e.status = STATUS_VELENO;
    VERIFICA(fabsf(Lotta_ProbCattura(&e, 1.0f, false) - 0.25f) < 1e-5f);
    e.status = STATUS_NESSUNO;
    VERIFICA(fabsf(Lotta_ProbCattura(&e, 1.5f, false) - 0.30f) < 1e-5f);
    e.hp = 1;
    VERIFICA(Lotta_ProbCattura(&e, 3.5f, false) == 0.95f);       /* tetto */
    /* leggendario a HP pieni con Core normale: minimo 0.02 */
    Elyon leg;
    Elyon_Crea(&leg, 74, 50, &rng);
    VERIFICA(GetSpecies(74)->rarita == RARITA_LEGGENDARIO);
    VERIFICA(Lotta_ProbCattura(&leg, 1.0f, false) == 0.02f);
    /* primordiale: impossibile senza evento, anche con Core VOID e 1 HP */
    Elyon pri;
    Elyon_Crea(&pri, 100, 65, &rng);
    pri.hp = 1;
    pri.status = STATUS_SONNO;
    VERIFICA(Lotta_ProbCattura(&pri, 3.5f, false) == 0.0f);
    VERIFICA(Lotta_ProbCattura(&pri, 1.0f, true) > 0.0f);
    /* statistica HP a zero: nessuna divisione per zero */
    Elyon z;
    memset(&z, 0, sizeof(z));
    z.specie = 10;
    VERIFICA(Lotta_ProbCattura(&z, 1.0f, false) == 0.0f);
}

static void TestDanni(void) {
    Rng_Init(&rng, 21);
    Lotta *L = malloc(sizeof(Lotta));
    Elyon a, d;
    int flaro = Specie("FLARO"), nerubi = Specie("NERUBI");
    /* COLPO (40, NEUTRO, fisica): ((2*10/5+2)*40*30/30)/50 + 2 = 6.8 -> 6 */
    Prepara(&a, flaro, 10, 50, 30, 30, 30, 30, 30);
    Prepara(&d, flaro, 10, 50, 30, 30, 30, 30, 30);
    Duello(L, &a, &d);
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("COLPO"), 1.0f) == 6);
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("COLPO"), 0.90f) == 6);   /* 6.12 */

    /* potenza e livello piu' alti: valori grandi, verifica precisa */
    Prepara(&a, flaro, 50, 200, 120, 80, 100, 80, 90);
    Prepara(&d, flaro, 50, 200, 90, 100, 90, 110, 70);
    Duello(L, &a, &d);
    /* IMPATTO 60 neutro: (22*60*120/100)/50+2 = 33.68 */
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("IMPATTO"), 1.0f) == 33);
    /* STAB: FIAMMATA 70 speciale Fiamma su Fiamma (resiste x0.5):
       (22*70*100/110)/50+2 = 30 ; *1.25 *0.5 = 18.75 -> 18 */
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("FIAMMATA"), 1.0f) == 18);

    /* superefficace: Fiamma su Natura */
    int leafin = Specie("LEAFIN");
    VERIFICA(GetSpecies(leafin)->tipo1 == TYPE_NATURA);
    Prepara(&d, leafin, 50, 200, 90, 100, 90, 110, 70);
    Duello(L, &a, &d);
    int se = Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("FIAMMATA"), 1.0f);
    VERIFICA(se == (int)floor(30.0 * 1.25 * 2.0));
    VERIFICA(Lotta_EfficaciaContro(L, LATO_NEMICO, TYPE_FIAMMA) == 2.0f);
    /* boost +2 SPA raddoppia l'attacco effettivo */
    L->lato[LATO_GIOCATORE].boost[BOOST_SPA] = 2;
    int boost = Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("FIAMMATA"), 1.0f);
    VERIFICA(boost == (int)floor(((22.0 * 70 * 200 / 110) / 50 + 2) * 1.25 * 2.0));
    L->lato[LATO_GIOCATORE].boost[BOOST_SPA] = 0;
    /* ROTTURA LUMEN ignora i boost difensivi */
    L->lato[LATO_NEMICO].boost[BOOST_SPD] = 6;
    int conBoost = Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("FIAMMATA"), 1.0f);
    VERIFICA(conBoost < se);
    int r1 = Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("ROTTURA LUMEN"), 1.0f);
    L->lato[LATO_NEMICO].boost[BOOST_SPD] = 0;
    VERIFICA(r1 == Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("ROTTURA LUMEN"), 1.0f));
    /* sole: Fiamma x1.2 */
    L->meteo = METEO_SOLE;
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("FIAMMATA"), 1.0f) == (int)floor(30.0 * 1.25 * 2.0 * 1.2));
    L->meteo = METEO_PIOGGIA;
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("FIAMMATA"), 1.0f) == (int)floor(30.0 * 1.25 * 2.0 * 0.8));
    L->meteo = METEO_NESSUNO;
    /* bruciatura: attacco fisico x0.8 */
    int prima = Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("IMPATTO"), 1.0f);
    L->coll->squadra[0].status = STATUS_BRUCIATURA;
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("IMPATTO"), 1.0f) < prima);
    L->coll->squadra[0].status = STATUS_NESSUNO;

    /* danno minimo 1 e difesa zero senza divisioni per zero */
    Prepara(&a, flaro, 1, 10, 1, 1, 1, 1, 1);
    Prepara(&d, flaro, 100, 999, 999, 999, 999, 999, 999);
    Duello(L, &a, &d);
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("COLPO"), 0.9f) >= 1);
    L->nemici[0].stat[STAT_DEF] = 0;
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("COLPO"), 0.9f) >= 1);
    /* mosse di stato: nessun danno */
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("RAPIDITÀ"), 1.0f) == 0);

    /* abilita': PELLE_UMIDA (Acqua) riduce i danni Fiamma a x0.7 */
    VERIFICA(GetAbilityResolved(GetSpecies(nerubi)->abilita)->id == 2);
    Prepara(&a, flaro, 50, 200, 120, 80, 100, 80, 90);
    Prepara(&d, nerubi, 50, 200, 90, 100, 90, 110, 70);
    Duello(L, &a, &d);
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("FIAMMATA"), 1.0f) == (int)floor(30.0 * 1.25 * 0.5 * 0.7));

    /* CONDUTTORE (Impulso): immune a Impulso */
    int zappi = Specie("ZAPPI");
    VERIFICA(GetAbilityResolved(GetSpecies(zappi)->abilita)->effetto == AB_IMMUNITA_TIPO);
    Prepara(&d, zappi, 50, 200, 90, 100, 90, 110, 70);
    Duello(L, &a, &d);
    VERIFICA(Lotta_StimaDanno(L, LATO_GIOCATORE, Mossa("SCINTILLA"), 1.0f) == 0);
    free(L);
}

static void TestTurni(void) {
    Rng_Init(&rng, 31);
    Inventario_Azzera(&inv);
    Lotta *L = malloc(sizeof(Lotta));
    Elyon a, d;
    int flaro = Specie("FLARO");
    /* il nemico e' piu' veloce ma AZIONE RAPIDA (priorita' +1) colpisce prima */
    Prepara(&a, flaro, 20, 300, 50, 50, 50, 50, 10);
    Prepara(&d, flaro, 20, 300, 50, 50, 50, 50, 200);
    Elyon_ImparaMossa(&a, Mossa("AZIONE RAPIDA"));
    Elyon_ImparaMossa(&a, Mossa("COLPO"));
    Elyon_ImparaMossa(&d, Mossa("COLPO"));
    Duello(L, &a, &d);
    Lotta_Inizia(L);
    VERIFICA(ContaEventi(L, EV_ENTRATA) == 2);
    VERIFICA(Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 }));
    int primoAttacco = -1;
    for (int i = 0; i < L->numEventi; i++) if (L->eventi[i].tipo == EV_ATTACCO) { primoAttacco = L->eventi[i].lato; break; }
    VERIFICA(primoAttacco == LATO_GIOCATORE);
    VERIFICA(L->coll->squadra[0].pp[0] == GetMove(Mossa("AZIONE RAPIDA"))->pp - 1);
    /* senza priorita' agisce prima il piu' veloce */
    VERIFICA(Lotta_Turno(L, (Azione){ AZ_MOSSA, 1, 0, 0 }));
    for (int i = 0; i < L->numEventi; i++) if (L->eventi[i].tipo == EV_ATTACCO) { primoAttacco = L->eventi[i].lato; break; }
    VERIFICA(primoAttacco == LATO_NEMICO);
    /* paralisi: velocita' dimezzata */
    int v = Lotta_VelocitaEffettiva(L, LATO_NEMICO);
    L->nemici[0].status = STATUS_PARALISI;
    VERIFICA(Lotta_VelocitaEffettiva(L, LATO_NEMICO) == v / 2 || Lotta_VelocitaEffettiva(L, LATO_NEMICO) == (v + 1) / 2);
    L->nemici[0].status = STATUS_NESSUNO;

    /* azioni non valide: il turno non passa */
    int turno = L->turno;
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_MOSSA, 3, 0, 0 }));          /* slot vuoto */
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_MOSSA, MOSSA_DIMENARSI, 0, 0 }));
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_CAMBIO, 0, 0, 0 }));         /* gia' in campo */
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_CAMBIO, 4, 0, 0 }));         /* vuoto */
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_OGGETTO, ITEM_CORE, 0, 0 })); /* non ne hai */
    VERIFICA(L->turno == turno && L->numEventi == 1);

    /* PP esauriti: ci si dimena (senza consumare PP, con contraccolpo) */
    L->coll->squadra[0].pp[0] = 0;
    L->coll->squadra[0].pp[1] = 0;
    VERIFICA(Lotta_DeveDimenarsi(L));
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 }));
    int hp = L->coll->squadra[0].hp;
    VERIFICA(Lotta_Turno(L, (Azione){ AZ_MOSSA, MOSSA_DIMENARSI, 0, 0 }));
    VERIFICA(CercaTesto(L, "si dimena"));
    VERIFICA(L->coll->squadra[0].hp < hp);
    free(L);
}

static void TestStati(void) {
    Rng_Init(&rng, 41);
    Inventario_Azzera(&inv);
    Lotta *L = malloc(sizeof(Lotta));
    Elyon a, d;
    int flaro = Specie("FLARO");
    Prepara(&a, flaro, 20, 160, 50, 50, 50, 50, 100);
    Prepara(&d, flaro, 20, 160, 50, 50, 50, 50, 10);
    Elyon_ImparaMossa(&a, Mossa("RAPIDITÀ"));
    Elyon_ImparaMossa(&d, Mossa("RAPIDITÀ"));
    /* veleno: 1/8 degli HP massimi a fine turno */
    d.status = STATUS_VELENO;
    Duello(L, &a, &d);
    Lotta_Inizia(L);
    Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->nemici[0].hp == 160 - 20);
    VERIFICA(L->lato[LATO_GIOCATORE].boost[BOOST_SPE] == 1);
    /* boost: massimo +6 */
    Prepara(&d, flaro, 20, 160, 50, 50, 50, 50, 10);
    Elyon_ImparaMossa(&d, Mossa("RAPIDITÀ"));
    Duello(L, &a, &d);
    for (int i = 0; i < 8; i++) Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->lato[LATO_GIOCATORE].boost[BOOST_SPE] == 6);
    VERIFICA(CercaTesto(L, "non può salire oltre"));

    /* veleno grave: n/16 crescente */
    Prepara(&d, flaro, 20, 160, 50, 50, 50, 50, 10);
    Elyon_ImparaMossa(&d, Mossa("RAPIDITÀ"));
    d.status = STATUS_VELENO_GRAVE;
    Duello(L, &a, &d);
    Lotta_Inizia(L);
    Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->nemici[0].hp == 160 - 10);
    Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->nemici[0].hp == 160 - 10 - 20);
    Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->nemici[0].hp == 160 - 10 - 20 - 30);

    /* bruciatura 1/16 */
    Prepara(&d, flaro, 20, 160, 50, 50, 50, 50, 10);
    Elyon_ImparaMossa(&d, Mossa("RAPIDITÀ"));
    d.status = STATUS_BRUCIATURA;
    Duello(L, &a, &d);
    Lotta_Inizia(L);
    Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->nemici[0].hp == 150);

    /* sonno: dorme da 1 a 3 turni, poi si sveglia */
    int dormiti[4] = { 0 };
    for (int prova = 0; prova < 300; prova++) {
        Prepara(&d, flaro, 20, 160, 50, 50, 50, 50, 10);
        Elyon_ImparaMossa(&d, Mossa("RAPIDITÀ"));
        Duello(L, &a, &d);
        L->nemici[0].status = STATUS_SONNO;
        L->nemici[0].turniSonno = 1 + Rng_Range(&rng, 3);
        int turni = 0;
        for (int t = 0; t < 6 && L->nemici[0].status == STATUS_SONNO; t++) {
            Elyon_Cura(&L->coll->squadra[0]);
            L->nemici[0].pp[0] = 30;
            Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
            if (CercaTesto(L, "dorme profondamente")) turni++;
        }
        VERIFICA(L->nemici[0].status == STATUS_NESSUNO);
        if (turni >= 1 && turni <= 3) dormiti[turni]++;
        else dormiti[0]++;
    }
    VERIFICA(dormiti[0] == 0 && dormiti[1] > 0 && dormiti[2] > 0 && dormiti[3] > 0);

    /* gelo: si scongela in media in 5 turni (20%) */
    int scongelati = 0, tentativi = 0;
    for (int prova = 0; prova < 200; prova++) {
        Prepara(&d, flaro, 20, 160, 50, 50, 50, 50, 10);
        Elyon_ImparaMossa(&d, Mossa("RAPIDITÀ"));
        Duello(L, &a, &d);
        L->nemici[0].status = STATUS_GELO;
        for (int t = 0; t < 200 && L->nemici[0].status == STATUS_GELO; t++) {
            Elyon_Cura(&L->coll->squadra[0]);
            L->nemici[0].pp[0] = 30;
            Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
            tentativi++;
        }
        scongelati += L->nemici[0].status == STATUS_NESSUNO;
    }
    VERIFICA(scongelati == 200);
    printf("  gelo: %.2f turni medi prima dello scongelamento\n", (double)tentativi / 200.0);
    VERIFICA(tentativi > 200 * 3 && tentativi < 200 * 7);

    /* immunita' di abilita': GELO_ETERNO non si congela */
    int specieGelo = 0;
    for (int id = 1; id <= 100 && !specieGelo; id++)
        if (GetAbilityResolved(GetSpecies(id)->abilita)->id == 6) specieGelo = id;
    VERIFICA(specieGelo > 0);
    Prepara(&a, flaro, 20, 160, 50, 50, 50, 50, 100);
    Elyon_ImparaMossa(&a, Mossa("GELOPUNTA"));
    Prepara(&d, specieGelo, 20, 9999, 50, 500, 50, 500, 10);
    Elyon_ImparaMossa(&d, Mossa("RAPIDITÀ"));
    Duello(L, &a, &d);
    for (int t = 0; t < 200; t++) {
        L->coll->squadra[0].pp[0] = 30;
        Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    }
    VERIFICA(L->nemici[0].status != STATUS_GELO);
    free(L);
}

static void TestCatturaFugaXp(void) {
    Rng_Init(&rng, 51);
    Inventario_Azzera(&inv);
    Lotta *L = malloc(sizeof(Lotta));
    Collezione_Azzera(&coll);
    Elyon mio;
    Elyon_Crea(&mio, Specie("PYRIN"), 30, &rng);
    Collezione_Aggiungi(&coll, &mio);

    /* allenatore: niente cattura e niente fuga */
    const TrainerData *kael = GetTrainer("KAEL_INIZIALE_VS_PYRIN");
    VERIFICA(Lotta_InitAllenatore(L, &coll, &inv, &rng, kael));
    Lotta_Inizia(L);
    VERIFICA(CercaTesto(L, "ti sfida"));
    Inventario_Aggiungi(&inv, ITEM_CORE, 5);
    char motivo[96];
    VERIFICA(!Lotta_AzioneValida(L, (Azione){ AZ_OGGETTO, ITEM_CORE, 0, 0 }, motivo, sizeof(motivo)));
    VERIFICA(strstr(motivo, "allenatore") != NULL);
    VERIFICA(!Lotta_AzioneValida(L, (Azione){ AZ_FUGA, 0, 0, 0 }, motivo, sizeof(motivo)));
    VERIFICA(Inventario_Quantita(&inv, ITEM_CORE) == 5);
    /* un Lv30 batte Kael Lv5: vittoria, premio e XP x1.5 */
    int lumen = inv.lumen;
    int32_t xpPrima = coll.squadra[0].xp;
    for (int t = 0; t < 50 && !Lotta_Finita(L); t++) Lotta_Turno(L, Attacca(L));
    VERIFICA(L->esito == ESITO_VITTORIA);
    VERIFICA(inv.lumen == lumen + kael->premio);
    int32_t atteso = Lotta_XpGuadagnata(kael->squadra[0].livello, GetSpecies(kael->squadra[0].specie)->baseYield, true, 1);
    VERIFICA(coll.squadra[0].xp == xpPrima + atteso);
    VERIFICA(L->eventi[L->numEventi - 1].tipo == EV_FINE);

    /* selvatico: la fuga riesce sempre se si e' piu' veloci */
    Collezione_CuraSquadra(&coll);
    Lotta_InitSelvatica(L, &coll, &inv, &rng, Specie("FLARO"), 2);
    VERIFICA(Lotta_VelocitaEffettiva(L, LATO_GIOCATORE) >= Lotta_VelocitaEffettiva(L, LATO_NEMICO));
    VERIFICA(Lotta_Turno(L, (Azione){ AZ_FUGA, 0, 0, 0 }));
    VERIFICA(L->esito == ESITO_FUGA);

    /* cattura: prima o poi riesce, il Core viene consumato, l'Elyon entra in squadra */
    int catture = 0;
    for (int prova = 0; prova < 50; prova++) {
        Collezione_Azzera(&coll);
        Collezione_Aggiungi(&coll, &mio);
        Inventario_Azzera(&inv);
        Inventario_Aggiungi(&inv, ITEM_CORE, 99);
        Lotta_InitSelvatica(L, &coll, &inv, &rng, Specie("FLARO"), 3);
        int lanci = 0;
        for (int t = 0; t < 99 && !Lotta_Finita(L); t++) {
            Elyon_Cura(&coll.squadra[0]);    /* conta solo la cattura, non la resistenza */
            lanci += Lotta_Turno(L, (Azione){ AZ_OGGETTO, ITEM_CORE, 0, 0 });
        }
        VERIFICA(L->esito == ESITO_CATTURA);
        catture += L->esito == ESITO_CATTURA;
        VERIFICA(Collezione_NumSquadra(&coll) == 2 && coll.squadra[1].specie == Specie("FLARO"));
        VERIFICA(Inventario_Quantita(&inv, ITEM_CORE) == 99 - lanci);
        VERIFICA(ContaEventi(L, EV_SCOSSA) == 3 && ContaEventi(L, EV_CATTURA_OK) == 1);
    }
    VERIFICA(catture == 50);

    /* squadra e box pieni: la cattura non e' permessa */
    Collezione_Azzera(&coll);
    for (int i = 0; i < MAX_SQUADRA + NUM_BOX * POSTI_BOX; i++) Collezione_Aggiungi(&coll, &mio);
    Lotta_InitSelvatica(L, &coll, &inv, &rng, Specie("FLARO"), 3);
    VERIFICA(!Lotta_AzioneValida(L, (Azione){ AZ_OGGETTO, ITEM_CORE, 0, 0 }, motivo, sizeof(motivo)));

    /* primordiale: il Core non reagisce (e non viene consumato) */
    Collezione_Azzera(&coll);
    Collezione_Aggiungi(&coll, &mio);
    Inventario_Aggiungi(&inv, ITEM_CORE_VOID, 3);
    Lotta_InitSelvatica(L, &coll, &inv, &rng, 100, 65);
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_OGGETTO, ITEM_CORE_VOID, 0, 0 }));
    VERIFICA(Inventario_Quantita(&inv, ITEM_CORE_VOID) == 3);
    L->catturaEvento = true;
    VERIFICA(Lotta_AzioneValida(L, (Azione){ AZ_OGGETTO, ITEM_CORE_VOID, 0, 0 }, NULL, 0));

    /* NUCLEO CONDIVISO: chi non lotta riceve meta' XP */
    Collezione_Azzera(&coll);
    Elyon panchina;
    Elyon_Crea(&panchina, Specie("LEAFIN"), 10, &rng);
    Collezione_Aggiungi(&coll, &mio);
    Collezione_Aggiungi(&coll, &panchina);
    Inventario_Azzera(&inv);
    Lotta_InitSelvatica(L, &coll, &inv, &rng, Specie("FLARO"), 5);
    for (int t = 0; t < 30 && !Lotta_Finita(L); t++) Lotta_Turno(L, Attacca(L));
    VERIFICA(L->esito == ESITO_VITTORIA && coll.squadra[1].xp == panchina.xp);   /* senza Nucleo: nulla */
    Collezione_CuraSquadra(&coll);
    Inventario_Aggiungi(&inv, ITEM_NUCLEO_CONDIVISO, 1);
    int32_t xpPanchina = coll.squadra[1].xp, xpTitolare = coll.squadra[0].xp;
    Lotta_InitSelvatica(L, &coll, &inv, &rng, Specie("FLARO"), 5);
    for (int t = 0; t < 30 && !Lotta_Finita(L); t++) Lotta_Turno(L, Attacca(L));
    VERIFICA(L->esito == ESITO_VITTORIA);
    int32_t pieno = coll.squadra[0].xp - xpTitolare;
    VERIFICA(pieno > 0 && coll.squadra[1].xp - xpPanchina == pieno / 2);
    free(L);
}

static void TestCambioEKo(void) {
    Rng_Init(&rng, 61);
    Inventario_Azzera(&inv);
    Lotta *L = malloc(sizeof(Lotta));
    Collezione_Azzera(&coll);
    Elyon debole, forte;
    Elyon_Crea(&debole, Specie("LUMI"), 2, &rng);
    Elyon_Crea(&forte, Specie("PYRIN"), 40, &rng);
    debole.hp = 1;
    Collezione_Aggiungi(&coll, &debole);
    Collezione_Aggiungi(&coll, &forte);
    const TrainerData *t = GetTrainer("NEXUS_RECLUTA_01");
    VERIFICA(Lotta_InitAllenatore(L, &coll, &inv, &rng, t));
    VERIFICA(L->numNemici == t->numMembri);
    Lotta_Inizia(L);
    /* il debole va KO: si chiede il cambio e il turno successivo non parte */
    for (int i = 0; i < 10 && !L->attesaCambio; i++) Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->attesaCambio);
    VERIFICA(ContaEventi(L, EV_RICHIESTA_CAMBIO) == 1);
    VERIFICA(!Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 }));
    VERIFICA(!Lotta_CambioForzato(L, 0));         /* esausto */
    VERIFICA(Lotta_CambioForzato(L, 1));
    VERIFICA(L->lato[LATO_GIOCATORE].indice == 1 && !L->attesaCambio);
    for (int i = 0; i < 40 && !Lotta_Finita(L); i++) {
        if (L->attesaCambio) break;
        Lotta_Turno(L, Attacca(L));
    }
    VERIFICA(L->esito == ESITO_VITTORIA);

    /* sconfitta: nessun Elyon in grado di lottare */
    Collezione_Azzera(&coll);
    Elyon_Crea(&debole, Specie("LUMI"), 2, &rng);
    Collezione_Aggiungi(&coll, &debole);
    VERIFICA(Lotta_InitAllenatore(L, &coll, &inv, &rng, GetTrainer("GUARDIANA_LIRA")));
    for (int i = 0; i < 40 && !Lotta_Finita(L); i++) Lotta_Turno(L, (Azione){ AZ_MOSSA, 0, 0, 0 });
    VERIFICA(L->esito == ESITO_SCONFITTA);
    VERIFICA(Collezione_SquadraViva(&coll) == 0);
    /* con squadra tutta esausta non si puo' iniziare */
    VERIFICA(!Lotta_InitSelvatica(L, &coll, &inv, &rng, 10, 3));

    /* Vael (intermezzo): la lotta si interrompe a meta' HP dell'ultimo Elyon, mai sconfitta forzata */
    const TrainerData *vael = GetTrainer("VAEL_INTERMEZZO");
    VERIFICA(vael && vael->interruzioneNarrativa);
    int interrotte = 0;
    for (int prova = 0; prova < 20; prova++) {
        Collezione_Azzera(&coll);
        for (int i = 0; i < 6; i++) {
            Elyon e;
            Elyon_Crea(&e, Specie("PYROVAR"), 100, &rng);
            Collezione_Aggiungi(&coll, &e);
        }
        Lotta_InitAllenatore(L, &coll, &inv, &rng, vael);
        int lumen = inv.lumen;
        for (int i = 0; i < 200 && !Lotta_Finita(L); i++) {
            if (L->attesaCambio) {
                for (int k = 0; k < MAX_SQUADRA; k++) if (Lotta_CambioForzato(L, k)) break;
                continue;
            }
            Lotta_Turno(L, Attacca(L));
        }
        VERIFICA(L->esito == ESITO_INTERROTTA);
        interrotte += L->esito == ESITO_INTERROTTA;
        VERIFICA(inv.lumen == lumen);
        /* l'ultimo Elyon di Vael si e' fermato a meta' HP o meno */
        VERIFICA(NemiciVivi_Test(L) <= 1);
        const Elyon *ultimo = L->lato[LATO_NEMICO].e;
        VERIFICA(ultimo->hp * 2 <= ultimo->stat[STAT_HP]);
    }
    VERIFICA(interrotte == 20);
    free(L);
}

static void TestIA(void) {
    Rng_Init(&rng, 71);
    Inventario_Azzera(&inv);
    Lotta *L = malloc(sizeof(Lotta));
    Collezione_Azzera(&coll);
    Elyon mio;
    Elyon_Crea(&mio, Specie("LEAFIN"), 20, &rng);
    Collezione_Aggiungi(&coll, &mio);

    /* SMART: sceglie il colpo che manda KO */
    const TrainerData *smart = GetTrainer("NEXUS_COMMANDER");
    VERIFICA(smart->ia == AI_SMART);
    Lotta_InitAllenatore(L, &coll, &inv, &rng, smart);
    Elyon *n = &L->nemici[0];
    memset(n->mosse, 0, sizeof(n->mosse));
    Elyon_ImparaMossa(n, Mossa("RAPIDITÀ"));
    Elyon_ImparaMossa(n, Mossa("COLPO"));
    Elyon_ImparaMossa(n, Mossa("INFERNO CREMISI"));
    coll.squadra[0].hp = 5;
    for (int i = 0; i < 20; i++) {
        Azione a = Lotta_SceltaIA(L);
        VERIFICA(a.tipo == AZ_MOSSA && (n->mosse[a.param] == Mossa("COLPO") || n->mosse[a.param] == Mossa("INFERNO CREMISI")));
        /* COLPO e' piu' preciso (100 contro 85): se basta per il KO va preferito */
        if (Lotta_StimaDanno(L, LATO_NEMICO, Mossa("COLPO"), 0.9f) >= 5) VERIFICA(n->mosse[a.param] == Mossa("COLPO"));
    }

    /* STANDARD: preferisce la mossa superefficace (con un po' di casualita') */
    const TrainerData *std = GetTrainer("GUARDIANA_LIRA");
    VERIFICA(std->ia == AI_STANDARD);
    Collezione_CuraSquadra(&coll);
    Lotta_InitAllenatore(L, &coll, &inv, &rng, std);
    n = &L->nemici[0];
    memset(n->mosse, 0, sizeof(n->mosse));
    Elyon_ImparaMossa(n, Mossa("GETTOBLU"));          /* Acqua su Natura: resistita */
    Elyon_ImparaMossa(n, Mossa("FIAMMATA"));          /* Fiamma su Natura: superefficace */
    int fuoco = 0;
    for (int i = 0; i < 200; i++) fuoco += n->mosse[Lotta_SceltaIA(L).param] == Mossa("FIAMMATA");
    VERIFICA(fuoco > 150);

    /* SIMPLE: solo mosse con PP */
    Lotta_InitSelvatica(L, &coll, &inv, &rng, Specie("FLARO"), 5);
    n = &L->nemici[0];
    for (int k = 0; k < MAX_MOSSE_ELYON; k++) if (n->mosse[k]) n->pp[k] = 0;
    n->pp[0] = 3;
    for (int i = 0; i < 50; i++) VERIFICA(Lotta_SceltaIA(L).param == 0);
    n->pp[0] = 0;
    VERIFICA(Lotta_SceltaIA(L).param == MOSSA_DIMENARSI);

    /* BOSS: cure limitate (1-3) sotto il 30% */
    const TrainerData *boss = GetTrainer("CAMPIONE_KAEL_VS_PYRIN");
    VERIFICA(boss->ia == AI_BOSS);
    Collezione_Azzera(&coll);
    Elyon_Crea(&mio, Specie("PYROVAR"), 100, &rng);
    Collezione_Aggiungi(&coll, &mio);
    Lotta_InitAllenatore(L, &coll, &inv, &rng, boss);
    VERIFICA(L->cureBossMax >= 1 && L->cureBossMax <= 3);
    int cure = 0;
    for (int t = 0; t < 60 && !Lotta_Finita(L); t++) {
        L->lato[LATO_NEMICO].e->hp = L->lato[LATO_NEMICO].e->stat[STAT_HP] / 5;
        coll.squadra[0].hp = coll.squadra[0].stat[STAT_HP];
        Azione a = Lotta_SceltaIA(L);
        if (a.tipo == AZ_OGGETTO) {
            cure++;
            L->cureBoss++;    /* simula l'esecuzione */
        }
    }
    VERIFICA(cure == L->cureBossMax);
    free(L);
}

/* ---------------------------------------------------------------- simulazione */

static uint32_t hashEventi;

static void Mescola(const Lotta *L) {
    for (int i = 0; i < L->numEventi; i++) {
        const EventoLotta *e = &L->eventi[i];
        uint32_t v[5] = { (uint32_t)e->tipo, (uint32_t)e->lato, (uint32_t)e->a, (uint32_t)e->b, (uint32_t)e->c };
        for (int k = 0; k < 5; k++) hashEventi = (hashEventi ^ v[k]) * 16777619u;
        for (const char *c = e->testo; *c; c++) hashEventi = (hashEventi ^ (uint8_t)*c) * 16777619u;
    }
}

static int invariantiRotte = 0;

static void ControllaInvarianti(const Lotta *L) {
    bool ok = L->numEventi <= MAX_EVENTI_LOTTA;
    for (int i = 0; i < MAX_SQUADRA; i++) {
        const Elyon *e = &L->coll->squadra[i];
        if (!Elyon_Vuoto(e) && (e->hp < 0 || e->hp > e->stat[STAT_HP])) ok = false;
        for (int k = 0; k < MAX_MOSSE_ELYON; k++) if (e->pp[k] < 0) ok = false;
    }
    for (int i = 0; i < L->numNemici; i++)
        if (L->nemici[i].hp < 0 || L->nemici[i].hp > L->nemici[i].stat[STAT_HP]) ok = false;
    for (int s = 0; s < 2; s++)
        for (int b = 0; b < NUM_BOOST; b++)
            if (L->lato[s].boost[b] < -BOOST_MAX || L->lato[s].boost[b] > BOOST_MAX) ok = false;
    if (!Lotta_Finita(L) && !L->attesaCambio) {
        if (Elyon_Esausto(L->lato[0].e) || Elyon_Esausto(L->lato[1].e)) ok = false;
    }
    bool fine = L->numEventi > 0 && L->eventi[L->numEventi - 1].tipo == EV_FINE;
    if (Lotta_Finita(L) != fine && L->numEventi > 0) ok = false;
    if (!ok) invariantiRotte++;
}

static void SquadraCasuale(Collezione *c, Rng *r) {
    Collezione_Azzera(c);
    int n = 1 + Rng_Range(r, 6);
    for (int i = 0; i < n; i++) {
        Elyon e;
        Elyon_Crea(&e, 1 + Rng_Range(r, 100), 1 + Rng_Range(r, 100), r);
        /* meta' delle volte mosse completamente casuali: copre tutti gli effetti */
        if (Rng_Range(r, 2)) {
            memset(e.mosse, 0, sizeof(e.mosse));
            for (int k = 0; k < 4; k++) Elyon_ImparaMossa(&e, 1 + Rng_Range(r, 100));
        }
        e.status = Rng_Range(r, 8) == 0 ? (StatusId)(1 + Rng_Range(r, 6)) : STATUS_NESSUNO;
        if (e.status == STATUS_SONNO) e.turniSonno = 2;
        e.oggetto = Rng_Range(r, 4) == 0 ? 30 + Rng_Range(r, 6) : 0;
        Collezione_Aggiungi(c, &e);
    }
}

static Azione AzioneCasuale(const Lotta *L, Rng *r) {
    for (int prova = 0; prova < 30; prova++) {
        Azione a = { AZ_MOSSA, 0, 0, 0 };
        int tipo = Rng_Range(r, 10);
        if (tipo < 6) a.param = Rng_Range(r, 4);
        else if (tipo < 8) { a.tipo = AZ_CAMBIO; a.param = Rng_Range(r, MAX_SQUADRA); }
        else if (tipo < 9) {
            static const int oggetti[] = { ITEM_ERBA_VITALE, ITEM_FRUTTO_LUMEN, ITEM_ELISIR, ITEM_ANTIDOTO,
                                           ITEM_RISVEGLIO, ITEM_REVITA, ITEM_CORE, ITEM_CORE_MAX, ITEM_CORE_VOID };
            a.tipo = AZ_OGGETTO;
            a.param = oggetti[Rng_Range(r, 9)];
            a.bersaglio = Rng_Range(r, MAX_SQUADRA);
            a.slotMossa = Rng_Range(r, 4);
        } else a.tipo = AZ_FUGA;
        if (Lotta_AzioneValida(L, a, NULL, 0)) return a;
    }
    Azione d = { AZ_MOSSA, MOSSA_DIMENARSI, 0, 0 };
    if (Lotta_DeveDimenarsi(L)) return d;
    for (int k = 0; k < 4; k++) {
        Azione a = { AZ_MOSSA, k, 0, 0 };
        if (Lotta_AzioneValida(L, a, NULL, 0)) return a;
    }
    return d;
}

/* Una lotta completa con scelte casuali; ritorna i turni giocati (-1 se non termina) */
static int Simula(Lotta *L, uint32_t seme, int *esito) {
    Rng r, rl;
    Rng_Init(&r, seme);
    Rng_Init(&rl, seme ^ 0x9E3779B9u);
    SquadraCasuale(&coll, &r);
    Inventario_Azzera(&inv);
    for (int id = 1; id < ITEM_ID_MAX; id++) if (GetItem(id)) Inventario_Aggiungi(&inv, id, Rng_Range(&r, 4));
    bool ok;
    if (Rng_Range(&r, 2)) {
        ok = Lotta_InitAllenatore(L, &coll, &inv, &rl, GetTrainerIndex(Rng_Range(&r, NumAllenatori())));
    } else {
        ok = Lotta_InitSelvatica(L, &coll, &inv, &rl, 1 + Rng_Range(&r, 100), 1 + Rng_Range(&r, 100));
        L->catturaEvento = Rng_Range(&r, 4) == 0;
    }
    if (!ok) return -1;
    L->meteo = (Meteo)Rng_Range(&r, NUM_METEO);
    L->turniMeteo = L->meteo ? 5 : 0;
    Lotta_Inizia(L);
    Mescola(L);
    for (int t = 0; t < 3000; t++) {
        if (Lotta_Finita(L)) { *esito = L->esito; return t; }
        if (L->attesaCambio) {
            int k = Rng_Range(&r, MAX_SQUADRA);
            while (!Lotta_CambioForzato(L, k)) k = (k + 1) % MAX_SQUADRA;
        } else {
            Lotta_Turno(L, AzioneCasuale(L, &r));
        }
        Mescola(L);
        ControllaInvarianti(L);
    }
    return -1;
}

static void TestSimulazioni(void) {
    Lotta *L = malloc(sizeof(Lotta));
    int esiti[6] = { 0 }, nonFinite = 0, turniMax = 0;
    long turniTot = 0;
    const int N = 4000;
    for (int i = 0; i < N; i++) {
        int esito = 0;
        int t = Simula(L, 1000u + (uint32_t)i * 7919u, &esito);
        if (t < 0) { nonFinite++; continue; }
        esiti[esito]++;
        turniTot += t;
        if (t > turniMax) turniMax = t;
    }
    printf("  %d lotte simulate: vittorie %d, sconfitte %d, fughe %d, catture %d, interrotte %d, non finite %d\n",
           N, esiti[ESITO_VITTORIA], esiti[ESITO_SCONFITTA], esiti[ESITO_FUGA], esiti[ESITO_CATTURA],
           esiti[ESITO_INTERROTTA], nonFinite);
    printf("  turni medi %.1f, massimo %d\n", (double)turniTot / (N - nonFinite > 0 ? N - nonFinite : 1), turniMax);
    VERIFICA(nonFinite == 0);
    VERIFICA(invariantiRotte == 0);
    VERIFICA(esiti[ESITO_IN_CORSO] == 0);
    VERIFICA(esiti[ESITO_VITTORIA] > 0 && esiti[ESITO_SCONFITTA] > 0 && esiti[ESITO_FUGA] > 0 && esiti[ESITO_CATTURA] > 0);

    /* determinismo: stesso seme, stessa identica sequenza di eventi */
    uint32_t h[2];
    for (int volta = 0; volta < 2; volta++) {
        hashEventi = 2166136261u;
        for (int i = 0; i < 200; i++) { int e; Simula(L, 424242u + (uint32_t)i, &e); }
        h[volta] = hashEventi;
    }
    VERIFICA(h[0] == h[1]);
    hashEventi = 2166136261u;
    for (int i = 0; i < 200; i++) { int e; Simula(L, 525252u + (uint32_t)i, &e); }
    VERIFICA(hashEventi != h[0]);
    free(L);
}

static void TestIncontri(void) {
    Rng_Init(&rng, 81);
    const HabitatData *h = GetHabitat("ROUTE1");
    VERIFICA(h != NULL);
    int conteggio[101] = { 0 };
    for (int i = 0; i < 20000; i++) {
        int s = 0, l = 0;
        VERIFICA(Lotta_GeneraIncontro(h, &rng, &s, &l) || 0);
        if (s >= 1 && s <= 100) conteggio[s]++;
        if (l < h->livelloMin || l > h->livelloMax) VERIFICA(0);
    }
    for (int i = 0; i < h->numVoci; i++) {
        double atteso = 20000.0 * h->voci[i].percentuale / 100.0;
        double c = conteggio[h->voci[i].specie];
        VERIFICA(c > atteso * 0.85 && c < atteso * 1.15);
    }
    int s, l;
    VERIFICA(!Lotta_GeneraIncontro(GetHabitat("LUNARIA"), &rng, &s, &l));
    VERIFICA(!Lotta_GeneraIncontro(NULL, &rng, &s, &l));
}

int main(void) {
    printf("test_battle\n");
    TestFormule();
    TestCatturaFormula();
    TestDanni();
    TestTurni();
    TestStati();
    TestCatturaFugaXp();
    TestCambioEKo();
    TestIA();
    TestIncontri();
    TestSimulazioni();
    return FINE_TEST();
}
