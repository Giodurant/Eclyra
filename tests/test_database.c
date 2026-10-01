/* test_database.c - Controlli di consistenza del database (Fase 3).
 * Le tabelle di riferimento sono ricopiate qui dal documento di design, in modo
 * indipendente dal generatore: se il database compilato diverge, il test fallisce. */
#include "data/abilities_data.h"
#include "data/encounters_data.h"
#include "data/evolution_data.h"
#include "data/items_data.h"
#include "data/moves_data.h"
#include "data/species_data.h"
#include "data/trainers_data.h"
#include "data/types.h"
#include "test_framework.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int Uguale(float a, float b) { return fabsf(a - b) < 0.0001f; }

static ElementType TipoDaNome(const char *n) {
    for (int t = 1; t < TYPE_COUNT; t++)
        if (strcmp(Type_Nome((ElementType)t), n) == 0) return (ElementType)t;
    return TYPE_NONE;
}

/* ---------------------------------------------------------------- tipi */
static const char *SUPER_TXT =
    "FIAMMA:NATURA,GELO,METALLO;ACQUA:FIAMMA,ROCCIA;NATURA:ACQUA,ROCCIA;IMPULSO:ACQUA,VENTO;"
    "VENTO:NATURA,TOSSICO;ROCCIA:FIAMMA,GELO,VENTO;GELO:VENTO,NATURA;OMBRA:PSICHE,ASTRALE;"
    "ASTRALE:OMBRA,TOSSICO;TOSSICO:NATURA;PSICHE:TOSSICO,IMPULSO;METALLO:GELO,ROCCIA,ASTRALE";
static const char *RESISTE_TXT =
    "FIAMMA:FIAMMA,NATURA,GELO,METALLO;ACQUA:ACQUA,FIAMMA,GELO;NATURA:NATURA,ACQUA,IMPULSO;"
    "IMPULSO:IMPULSO,VENTO;VENTO:NATURA,VENTO;ROCCIA:FIAMMA,VENTO;GELO:GELO;OMBRA:OMBRA,PSICHE;"
    "ASTRALE:ASTRALE;TOSSICO:NATURA,TOSSICO;PSICHE:PSICHE;METALLO:METALLO,ROCCIA,GELO";

/* Riempie m[a][d] = 1 per ogni coppia "A:D1,D2;..." */
static void LeggiCoppie(const char *txt, int m[TYPE_COUNT][TYPE_COUNT], int inverti) {
    char buf[1024];
    strncpy(buf, txt, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    for (char *gruppo = strtok(buf, ";"); gruppo; gruppo = strtok(NULL, ";")) {
        char *due = strchr(gruppo, ':');
        *due = '\0';
        ElementType a = TipoDaNome(gruppo);
        char *resto = due + 1;
        char *ctx = NULL;
        (void)ctx;
        char copia[256];
        strncpy(copia, resto, sizeof(copia) - 1);
        copia[sizeof(copia) - 1] = '\0';
        char *p = copia;
        while (p && *p) {
            char *virgola = strchr(p, ',');
            if (virgola) *virgola = '\0';
            ElementType d = TipoDaNome(p);
            if (inverti) m[d][a] = 1; else m[a][d] = 1;   /* RESISTE: riga = difensore */
            p = virgola ? virgola + 1 : NULL;
        }
    }
}

static void TestTipi(void) {
    /* 12 tipi, nessun TERRA, NEUTRO = TYPE_NONE */
    VERIFICA(TYPE_COUNT - 1 == NUM_TIPI_ELEMENTALI);
    for (int t = 0; t < TYPE_COUNT; t++) VERIFICA(strcmp(Type_Nome((ElementType)t), "TERRA") != 0);
    VERIFICA(strcmp(Type_Nome(TYPE_NONE), "NEUTRO") == 0);

    static int sup[TYPE_COUNT][TYPE_COUNT], res[TYPE_COUNT][TYPE_COUNT];
    LeggiCoppie(SUPER_TXT, sup, 0);
    LeggiCoppie(RESISTE_TXT, res, 1);   /* RESISTE: "DIF:ATT" -> res[att][dif] */
    int errori = 0, minimo2 = 0, conflitti = 0;
    for (int a = 1; a < TYPE_COUNT; a++) {
        for (int d = 1; d < TYPE_COUNT; d++) {
            float atteso = 1.0f;
            if (sup[a][d] && res[a][d]) { atteso = 1.0f; conflitti++; }
            else if (sup[a][d]) atteso = 2.0f;
            else if (res[a][d]) atteso = 0.5f;
            float e = Type_Efficacia((ElementType)a, (ElementType)d);
            if (!Uguale(e, atteso)) { errori++; printf("  matrice %s->%s = %.2f (atteso %.2f)\n", Type_Nome(a), Type_Nome(d), e, atteso); }
            if (e < 0.5f) minimo2++;
        }
    }
    VERIFICA(errori == 0);
    VERIFICA(minimo2 == 0);   /* nessuna immunita' */
    printf("  matrice 12x12 controllata (%d coppie super efficaci e resistite insieme -> x1)\n", conflitti);
    /* NEUTRO */
    for (int t = 0; t < TYPE_COUNT; t++) {
        VERIFICA(Uguale(Type_Efficacia(TYPE_NONE, (ElementType)t), 1.0f));
        VERIFICA(Uguale(Type_Efficacia((ElementType)t, TYPE_NONE), 1.0f));
    }
    /* casi espliciti */
    VERIFICA(Uguale(Type_Efficacia(TYPE_FIAMMA, TYPE_NATURA), 2.0f));
    VERIFICA(Uguale(Type_Efficacia(TYPE_NATURA, TYPE_FIAMMA), 0.5f));
    VERIFICA(Uguale(Type_Efficacia(TYPE_ACQUA, TYPE_FIAMMA), 2.0f));
    VERIFICA(Uguale(Type_Efficacia(TYPE_FIAMMA, TYPE_ACQUA), 0.5f));
    /* doppio tipo: moltiplicazione (ACQUA su NATURA/ROCCIA = 0.5 x 2 = 1) */
    VERIFICA(Uguale(Type_EfficaciaDoppia(TYPE_ACQUA, TYPE_NATURA, TYPE_ROCCIA), 1.0f));
    VERIFICA(Uguale(Type_EfficaciaDoppia(TYPE_GELO, TYPE_VENTO, TYPE_NATURA), 4.0f));
    VERIFICA(Uguale(Type_EfficaciaDoppia(TYPE_FIAMMA, TYPE_FIAMMA, TYPE_ACQUA), 0.25f));
    VERIFICA(Uguale(Type_EfficaciaDoppia(TYPE_FIAMMA, TYPE_METALLO, TYPE_NONE), 2.0f));
    VERIFICA(Uguale(Type_Efficacia((ElementType)99, TYPE_FIAMMA), 1.0f));   /* valore fuori range */
}

/* ---------------------------------------------------------------- specie */
static void TestSpecie(void) {
    VERIFICA(GetSpecies(0) == NULL && GetSpecies(101) == NULL && GetSpecies(-5) == NULL);
    int nomiDuplicati = 0;
    for (int id = 1; id <= NUM_SPECIE; id++) {
        const SpeciesData *s = GetSpecies(id);
        VERIFICA(s != NULL);
        if (!s) continue;
        VERIFICA(s->id == id);
        VERIFICA(s->nome && s->nome[0]);
        VERIFICA(s->tipo1 > TYPE_NONE && s->tipo1 < TYPE_COUNT);
        VERIFICA(s->tipo2 >= TYPE_NONE && s->tipo2 < TYPE_COUNT && s->tipo2 != s->tipo1);
        for (int k = 0; k < NUM_STAT; k++) VERIFICA(s->base[k] >= 1 && s->base[k] <= 255);
        VERIFICA(GetAbility(s->abilita) != NULL);
        VERIFICA(s->descrizione && strlen(s->descrizione) > 20);
        VERIFICA(s->baseYield >= 4 && s->baseYield <= 30);
        VERIFICA(Uguale(s->crescita, 0.8f) || Uguale(s->crescita, 1.0f) || Uguale(s->crescita, 1.2f));
        float cr[] = { 0.20f, 0.08f, 0.015f, 0.0f };
        VERIFICA(Uguale(s->catchRate, cr[s->rarita]));
        VERIFICA(GetSpeciesByName(s->nome) == s);
        for (int j = id + 1; j <= NUM_SPECIE; j++)
            if (strcmp(GetSpecies(j)->nome, s->nome) == 0) nomiDuplicati++;
        /* learnset: ordinato, mosse esistenti, niente mosse esclusive, almeno una mossa al Lv 1 */
        VERIFICA(s->numLearnset >= 2 && s->numLearnset <= MAX_LEARNSET);
        int haLv1 = 0, ordinato = 1, esclusive = 0, valide = 1;
        for (int k = 0; k < s->numLearnset; k++) {
            const MoveData *m = GetMove(s->learnset[k].mossa);
            if (!m) { valide = 0; continue; }
            if (s->learnset[k].livello == 1) haLv1 = 1;
            if (k > 0 && s->learnset[k].livello < s->learnset[k - 1].livello) ordinato = 0;
            if ((m->flag & MOSSA_SOLO_EVENTO) || ((m->flag & MOSSA_SOLO_AURION) && id != 100)) esclusive = 1;
        }
        VERIFICA(valide && haLv1 && ordinato && !esclusive);
        /* crescita uguale in tutta la famiglia evolutiva */
        const EvolutionData *e = GetEvolution(id);
        if (e) VERIFICA(Uguale(GetSpecies(e->a)->crescita, s->crescita));
    }
    VERIFICA(nomiDuplicati == 0);

    /* nomi normalizzati e numeri autoritativi */
    const char *normalizzati[] = { "SHADEVOR", "THORVAN", "VENOROK", "ASTRION", "CYRIEL", "ZEPHRAX", "FERRADON", "AERION" };
    for (int i = 0; i < 8; i++) VERIFICA(GetSpeciesByName(normalizzati[i]) != NULL);
    VERIFICA(GetSpeciesByName("SHADAVOR") == NULL && GetSpeciesByName("THORN VAN") == NULL && GetSpeciesByName("ASTR AION") == NULL);
    VERIFICA(GetSpeciesByName("THORVAN")->id == 52 && GetSpeciesByName("NIGHTOR")->id == 54);
    VERIFICA(GetSpeciesByName("VENOROK")->id == 56 && GetSpeciesByName("ROOTAN")->id == 60);
    VERIFICA(GetSpecies(100)->rarita == RARITA_PRIMORDIALE && Uguale(GetSpecies(100)->catchRate, 0.0f));
    VERIFICA(GetSpecies(1)->tipo1 == TYPE_FIAMMA && GetSpecies(3)->tipo2 == TYPE_METALLO);
    VERIFICA(GetSpecies(9)->tipo2 == TYPE_ROCCIA);   /* VERDANTOR NATURA/ROCCIA */
    const int b[6] = { 160, 150, 140, 165, 145, 130 };
    for (int k = 0; k < 6; k++) VERIFICA(GetSpecies(100)->base[k] == b[k]);
    /* forme di AURION: non occupano un ID specie */
    VERIFICA(NumForme() == 2 && GetForm(100, 1) && GetForm(100, 2) && GetForm(100, 3) == NULL);
    VERIFICA(GetAbility(GetForm(100, 1)->abilita) != NULL);
    /* starter allo stesso ritmo di crescita */
    VERIFICA(Uguale(GetSpecies(1)->crescita, GetSpecies(4)->crescita) && Uguale(GetSpecies(4)->crescita, GetSpecies(7)->crescita));

    /* learnset esplicito di PYRIN */
    const char *pyrin[] = { "BRACIAVIVA", "GRAFFIO", "MARCHIO DEL FUOCO", "FIAMMATA", "RAPIDITÀ",
                            "ARTIGLIO METALLICO", "INFERNO CREMISI", "NUCLEO ARDENTE" };
    const int livelli[] = { 1, 5, 9, 13, 17, 22, 28, 35 };
    const SpeciesData *p = GetSpecies(1);
    VERIFICA(p->numLearnset == 8);
    for (int i = 0; i < 8 && i < p->numLearnset; i++) {
        VERIFICA(p->learnset[i].livello == livelli[i]);
        VERIFICA(strcmp(GetMove(p->learnset[i].mossa)->nome, pyrin[i]) == 0);
    }
}

/* ---------------------------------------------------------------- mosse */
static void TestMosse(void) {
    VERIFICA(GetMove(0) == NULL && GetMove(101) == NULL);
    for (int id = 1; id <= NUM_MOSSE; id++) {
        const MoveData *m = GetMove(id);
        VERIFICA(m && m->id == id && m->nome && m->nome[0]);
        if (!m) continue;
        VERIFICA(m->tipo >= TYPE_NONE && m->tipo < TYPE_COUNT);
        VERIFICA(m->precisione >= 1 && m->precisione <= 100);
        VERIFICA(m->pp >= 1 && m->pp <= 35);
        VERIFICA(m->effetto >= EFF_NESSUNO && m->effetto < NUM_EFFETTI);
        VERIFICA(m->prob >= 0 && m->prob <= 100);
        if (m->categoria == CAT_STATO) VERIFICA(m->potenza == 0);
        else VERIFICA(m->potenza > 0 || m->effetto == EFF_POTENZA_AMICIZIA);
        if (m->effetto == EFF_STATUS) VERIFICA(m->param1 > STATUS_NESSUNO && m->param1 <= STATUS_CONFUSIONE);
        if (m->effetto == EFF_BOOST_SE || m->effetto == EFF_BOOST_NEMICO) {
            VERIFICA(m->param1 >= 0 && m->param1 < NUM_BOOST && m->param2 != 0 && abs(m->param2) <= 2);
        }
        if (m->flag & MOSSA_PRECISIONE_AUTOMATICA) VERIFICA(m->categoria == CAT_STATO);
        VERIFICA(GetMoveByName(m->nome) == m);
    }
    VERIFICA(GetMoveByName("AZIONE RAPIDA")->priorita == 1);
    VERIFICA(GetMoveByName("RITORNO")->effetto == EFF_POTENZA_AMICIZIA && GetMoveByName("RITORNO")->param1 == 105);
    VERIFICA(GetMoveByName("FRATTURA DEL VUOTO")->flag & MOSSA_SOLO_AURION);
    VERIFICA(GetMove(99)->flag & MOSSA_SOLO_EVENTO);
    VERIFICA(GetMove(100)->flag & MOSSA_SOLO_EVENTO);
    VERIFICA(GetMoveByName("SOVRACCARICO")->effetto == EFF_BOOST_SE && GetMoveByName("SOVRACCARICO")->param2 == -2);
    VERIFICA(GetMoveByName("VELENO PROFONDO")->param1 == STATUS_VELENO_GRAVE);
    VERIFICA(GetMoveByName("COLPO")->tipo == TYPE_NONE);   /* NEUTRO */
    VERIFICA(GetMoveByName("SUPERNOVA")->effetto == EFF_CONTRACCOLPO && GetMoveByName("SUPERNOVA")->param1 == 25);
    VERIFICA(GetMoveByName("CORAZZA DI PIETRA")->flag & MOSSA_PRECISIONE_AUTOMATICA);
    VERIFICA(!(GetMoveByName("PAURA")->flag & MOSSA_PRECISIONE_AUTOMATICA));   /* contro il nemico */
}

/* ---------------------------------------------------------------- evoluzioni */
static const char *EVO_TXT =
    "001>002@16;002>003@34;004>005@16;005>006@34;007>008@16;008>009@34;010>011@18;011>012@34;"
    "013>014@18;014>015@35;016>017@20;018>019@20;020>021@20;022>023@22;024>025@22;026>027@24;"
    "028>029@24;030>031@23;032>033@25;034>035@25;037>093@27;038>060@18;039>052@32;040>095@26;"
    "041>061@30;042>096@30;043>097@28;044>098@30;045>056@29;046>089@30;047>090@30;048>049@28;050>091@34";

static void TestEvoluzioni(void) {
    int n = 0;
    const char *p = EVO_TXT;
    while (*p) {
        int da, a, liv, letti = 0;
        if (sscanf(p, "%d>%d@%d%n", &da, &a, &liv, &letti) != 3) break;
        const EvolutionData *e = GetEvolution(da);
        VERIFICA(e && e->a == a && e->livello == liv);
        VERIFICA(GetPreEvolution(a) == e);
        VERIFICA(Evolution_LivelloMinimo(a) == liv);
        n++;
        p += letti;
        if (*p == ';') p++;
    }
    VERIFICA(n == 33 && NumEvoluzioni() == 33);
    for (int i = 0; i < NumEvoluzioni(); i++) {
        const EvolutionData *e = GetEvolutionIndex(i);
        VERIFICA(GetSpecies(e->da) && GetSpecies(e->a) && e->livello > 1);
        const EvolutionData *succ = GetEvolution(e->a);
        if (succ) VERIFICA(succ->livello > e->livello);   /* catene a livelli crescenti */
        int passi = 0, s = e->da;
        while (GetEvolution(s) && passi < 10) { s = GetEvolution(s)->a; passi++; }
        VERIFICA(passi < 10);   /* nessun ciclo */
    }
    VERIFICA(Evolution_LivelloMinimo(1) == 1 && Evolution_LivelloMinimo(3) == 34);
    VERIFICA(GetEvolution(100) == NULL && GetEvolution(74) == NULL);   /* leggendari non evolvono */
    VERIFICA(GetEvolution(8)->a == 9 && GetEvolution(8)->livello == 34);   /* VERDANTOR al 34 */
}

/* ---------------------------------------------------------------- abilita' */
static const char *ECC_ABILITA =
    "#003 014, #006 031, #009 023, #012 032, #015 010, #017 011, #019 004, #021 033, #023 005, #025 006, #027 007, "
    "#029 009, #031 015, #033 008, #035 012, #049 038, #052 004, #053 006, #054 035, #055 008, #056 015, #057 012, "
    "#058 027, #059 010, #060 003, #061 005, #062 006, #063 028, #064 009, #065 016, #066 040, #067 019, #068 024, "
    "#069 018, #070 027, #071 007, #072 039, #073 023, #074 014, #075 025, #076 004, #077 006, #078 019, #079 024, "
    "#080 026, #081 042, #082 043, #083 022, #084 004, #085 044, #086 016, #087 013, #088 015, #089 008, #090 012, "
    "#091 017, #092 021, #093 018, #094 023, #095 019, #096 006, #097 035, #098 009, #099 020, #100 030";

static void TestAbilita(void) {
    VERIFICA(GetAbility(0) == NULL && GetAbility(47) == NULL);
    for (int id = 1; id <= NUM_ABILITA; id++) {
        const AbilityData *a = GetAbility(id);
        VERIFICA(a && a->id == id && a->nome && a->descrizione);
        const AbilityData *r = GetAbilityResolved(id);
        VERIFICA(r && r->aliasDi == 0);
    }
    VERIFICA(GetAbility(34)->aliasDi == 28 && GetAbility(36)->aliasDi == 27);
    VERIFICA(GetAbility(41)->aliasDi == 24 && GetAbility(45)->aliasDi == 20);
    VERIFICA(GetAbilityResolved(36)->id == 27);

    /* abilita' delle specie: eccezioni esplicite, altrimenti quella del tipo primario */
    int eccezione[NUM_SPECIE + 1] = { 0 };
    const char *p = ECC_ABILITA;
    while (*p) {
        int s, ab, letti = 0;
        if (sscanf(p, " #%d %d%n", &s, &ab, &letti) != 2) break;
        eccezione[s] = ab;
        p += letti;
        if (*p == ',') p++;
    }
    const int perTipo[TYPE_COUNT] = { 0, 1, 2, 3, 4, 33, 5, 6, 7, 9, 15, 8, 12 };
    int errori = 0;
    for (int id = 1; id <= NUM_SPECIE; id++) {
        const SpeciesData *s = GetSpecies(id);
        int atteso = eccezione[id] ? eccezione[id] : perTipo[s->tipo1];
        if (s->abilita != atteso) { errori++; printf("  %s: abilita %d invece di %d\n", s->nome, s->abilita, atteso); }
    }
    VERIFICA(errori == 0);
}

/* ---------------------------------------------------------------- allenatori e habitat */
static void TestAllenatori(void) {
    VERIFICA(NumAllenatori() >= 25);
    for (int i = 0; i < NumAllenatori(); i++) {
        const TrainerData *t = GetTrainerIndex(i);
        VERIFICA(t->numMembri >= 1 && t->numMembri <= 6);
        int sotto = 0;
        for (int k = 0; k < t->numMembri; k++) {
            const MembroSquadra *m = &t->squadra[k];
            VERIFICA(GetSpecies(m->specie) != NULL && m->livello >= 1 && m->livello <= 100);
            if (m->livello < Evolution_LivelloMinimo(m->specie)) sotto = 1;
            for (int j = 0; j < 4; j++) VERIFICA(m->mosse[j] == 0 || GetMove(m->mosse[j]) != NULL);
        }
        /* un Elyon sotto il livello di evoluzione solo con eccezione esplicita (e motivata) */
        VERIFICA(sotto == t->eccezioneLivello);
        if (t->eccezioneLivello) VERIFICA(t->nota && strlen(t->nota) > 10);
    }
    const TrainerData *lira = GetTrainer("GUARDIANA_LIRA");
    VERIFICA(lira && strcmp(lira->ricompensa, "SIGILLO_VERDE") == 0 && lira->premio == 2100);
    VERIFICA(lira->squadra[0].specie == 17 && lira->squadra[1].specie == 8);   /* LEAFORA, MOSSKA */
    for (int k = 0; k < lira->numMembri; k++) VERIFICA(lira->squadra[k].specie != 9);   /* niente VERDANTOR */
    VERIFICA(GetTrainer("VAEL_INTERMEZZO")->interruzioneNarrativa);
    const TrainerData *vael = GetTrainer("VAEL_FINALE");
    for (int k = 0; k < vael->numMembri; k++) VERIFICA(vael->squadra[k].livello >= 60 && vael->squadra[k].livello <= 64);
    const TrainerData *aur = GetTrainer("AURION_FASE1");
    VERIFICA(aur->squadra[0].specie == 100 && aur->squadra[0].livello == 65);
    VERIFICA(strcmp(GetMove(aur->squadra[0].mosse[2])->nome, "ROTTURA LUMEN") == 0);
    VERIFICA(GetTrainer("KAEL_INIZIALE_VS_PYRIN")->squadra[0].specie == 4);   /* AQUIL */
    VERIFICA(GetTrainer("KAEL_INIZIALE_VS_AQUIL")->squadra[0].specie == 7);   /* MOSSEN */
    VERIFICA(GetTrainer("KAEL_INIZIALE_VS_MOSSEN")->squadra[0].specie == 1);  /* PYRIN */
    VERIFICA(GetTrainer("CAMPIONE_KAEL_VS_PYRIN")->numMembri == 6);
    VERIFICA(GetTrainer("INESISTENTE") == NULL);
}

static void TestHabitat(void) {
    VERIFICA(NumHabitat() == 8);
    for (int i = 0; i < NumHabitat(); i++) {
        const HabitatData *h = GetHabitatIndex(i);
        int tot = 0;
        for (int k = 0; k < h->numVoci; k++) {
            VERIFICA(GetSpecies(h->voci[k].specie) != NULL);
            tot += h->voci[k].percentuale;
            /* la specie deve poter esistere entro il livello massimo dell'habitat */
            VERIFICA(Evolution_LivelloMinimo(h->voci[k].specie) <= h->livelloMax);
        }
        if (h->numVoci > 0) VERIFICA(tot == 100 && h->livelloMin <= h->livelloMax);
    }
    VERIFICA(GetHabitat("LUNARIA")->numVoci == 0);
    const HabitatData *r1 = GetHabitat("ROUTE1");
    VERIFICA(r1->livelloMin == 2 && r1->livelloMax == 7 && r1->numVoci == 5);
    for (int k = 0; k < r1->numVoci; k++) VERIFICA(r1->voci[k].specie > 9);   /* nessuno starter */
}

static void TestOggetti(void) {
    for (int id = 1; id <= 15; id++) VERIFICA(GetItem(id) != NULL);
    for (int id = 16; id <= 19; id++) VERIFICA(GetItem(id) == NULL);
    for (int id = 20; id <= 35; id++) VERIFICA(GetItem(id) != NULL);
    VERIFICA(GetItem(1)->prezzo == 150 && GetItem(12)->prezzo == 200 && GetItem(15)->prezzo == 3500);
    VERIFICA(GetItem(36) == NULL && GetItem(0) == NULL);
}

int main(void) {
    TestTipi();
    TestSpecie();
    TestMosse();
    TestEvoluzioni();
    TestAbilita();
    TestAllenatori();
    TestHabitat();
    TestOggetti();
    return FINE_TEST();
}
