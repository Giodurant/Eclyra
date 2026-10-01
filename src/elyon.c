/* elyon.c */
#include "elyon.h"
#include "data/evolution_data.h"
#include <string.h>

/* Nature (documento): aumenta / diminuisce */
static const struct { const char *nome; StatId su, giu; } NATURE[NUM_NATURE] = {
    [NATURA_NEUTRO]     = { "NEUTRO",     STAT_HP,  STAT_HP },
    [NATURA_CORAGGIOSO] = { "CORAGGIOSO", STAT_ATK, STAT_SPA },
    [NATURA_MENTALE]    = { "MENTALE",    STAT_SPA, STAT_ATK },
    [NATURA_FERMO]      = { "FERMO",      STAT_DEF, STAT_SPE },
    [NATURA_RAPIDO]     = { "RAPIDO",     STAT_SPE, STAT_DEF },
    [NATURA_TENACE]     = { "TENACE",     STAT_SPD, STAT_SPA },
    [NATURA_IMPULSIVO]  = { "IMPULSIVO",  STAT_SPE, STAT_SPD },
    [NATURA_CALMO]      = { "CALMO",      STAT_SPD, STAT_ATK },
    [NATURA_ARDENTE]    = { "ARDENTE",    STAT_ATK, STAT_DEF },
    [NATURA_SERENO]     = { "SERENO",     STAT_SPA, STAT_SPE },
};

const char *Natura_Nome(Natura n) { return (n >= 0 && n < NUM_NATURE) ? NATURE[n].nome : "?"; }

int Natura_Percento(Natura n, StatId s) {
    if (n <= NATURA_NEUTRO || n >= NUM_NATURE || s == STAT_HP) return 100;
    if (s == NATURE[n].su) return 110;
    if (s == NATURE[n].giu) return 90;
    return 100;
}

int32_t Elyon_XpPerLivello(const SpeciesData *s, int livello) {
    if (!s || livello <= 1) return 0;
    if (livello > LIVELLO_MAX) livello = LIVELLO_MAX;
    /* crescita come frazione intera (0.8 -> 8/10) per evitare errori di arrotondamento */
    int64_t decimi = (int64_t)(s->crescita * 10.0f + 0.5f);
    int64_t l3 = (int64_t)livello * livello * livello;
    return (int32_t)(l3 * decimi / 10);
}

int Elyon_StatFormula(StatId s, int base, int livello, int iv, Natura n) {
    if (s == STAT_HP) return base + 3 * livello + livello / 2 + iv;
    return (base + 2 * livello + iv) * Natura_Percento(n, s) / 100;
}

bool Elyon_Vuoto(const Elyon *e) { return e->specie == 0 || GetSpecies(e->specie) == NULL; }
bool Elyon_Esausto(const Elyon *e) { return Elyon_Vuoto(e) || e->hp <= 0; }

const char *Elyon_Nome(const Elyon *e) {
    if (e->soprannome[0]) return e->soprannome;
    const SpeciesData *s = GetSpecies(e->specie);
    return s ? s->nome : "???";
}

int Elyon_NumMosse(const Elyon *e) {
    int n = 0;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) if (e->mosse[i]) n++;
    return n;
}

void Elyon_RicalcolaStat(Elyon *e) {
    const SpeciesData *s = GetSpecies(e->specie);
    if (!s) return;
    int vecchioMax = e->stat[STAT_HP];
    for (int k = 0; k < NUM_STAT; k++) e->stat[k] = Elyon_StatFormula((StatId)k, s->base[k], e->livello, e->iv[k], e->natura);
    if (vecchioMax <= 0) e->hp = e->stat[STAT_HP];
    else if (e->hp > 0) {
        /* HP preservati in proporzione (arrotondati per eccesso: chi era vivo resta vivo) */
        int64_t nuovo = ((int64_t)e->hp * e->stat[STAT_HP] + vecchioMax - 1) / vecchioMax;
        e->hp = (int)(nuovo > e->stat[STAT_HP] ? e->stat[STAT_HP] : nuovo);
    }
}

bool Elyon_ImparaMossa(Elyon *e, int mossa) {
    const MoveData *m = GetMove(mossa);
    if (!m) return false;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) if (e->mosse[i] == mossa) return true;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) {
        if (e->mosse[i] == 0) { e->mosse[i] = mossa; e->pp[i] = m->pp; return true; }
    }
    return false;
}

void Elyon_SostituisciMossa(Elyon *e, int slot, int mossa) {
    const MoveData *m = GetMove(mossa);
    if (!m || slot < 0 || slot >= MAX_MOSSE_ELYON) return;
    e->mosse[slot] = mossa;
    e->pp[slot] = m->pp;
}

void Elyon_Crea(Elyon *e, int specie, int livello, Rng *rng) {
    memset(e, 0, sizeof(*e));
    const SpeciesData *s = GetSpecies(specie);
    if (!s) return;
    livello = Utils_ClampInt(livello, 1, LIVELLO_MAX);
    e->specie = specie;
    e->livello = livello;
    e->xp = Elyon_XpPerLivello(s, livello);
    for (int k = 0; k < NUM_STAT; k++) e->iv[k] = Rng_Range(rng, IV_MAX + 1);
    e->natura = (Natura)Rng_Range(rng, NUM_NATURE);
    e->amicizia = 70;
    e->varianteLumen = Rng_Range(rng, 256) == 0;
    Elyon_RicalcolaStat(e);
    e->hp = e->stat[STAT_HP];
    /* le ultime 4 mosse apprese entro il livello */
    int scelte[MAX_LEARNSET], n = 0;
    for (int i = 0; i < s->numLearnset; i++) if (s->learnset[i].livello <= livello) scelte[n++] = s->learnset[i].mossa;
    for (int i = n > MAX_MOSSE_ELYON ? n - MAX_MOSSE_ELYON : 0; i < n; i++) Elyon_ImparaMossa(e, scelte[i]);
}

void Elyon_Cura(Elyon *e) {
    if (Elyon_Vuoto(e)) return;
    e->hp = e->stat[STAT_HP];
    e->status = STATUS_NESSUNO;
    e->turniSonno = 0;
    e->contatoreVelenoGrave = 0;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) {
        const MoveData *m = GetMove(e->mosse[i]);
        if (m) e->pp[i] = m->pp;
    }
}

int Elyon_AggiungiXp(Elyon *e, int32_t xp) {
    const SpeciesData *s = GetSpecies(e->specie);
    if (!s || xp <= 0 || e->livello >= LIVELLO_MAX) return 0;
    int32_t massimo = Elyon_XpPerLivello(s, LIVELLO_MAX);
    int64_t nuovo = (int64_t)e->xp + xp;
    e->xp = (int32_t)(nuovo > massimo ? massimo : nuovo);
    int guadagnati = 0;
    while (e->livello < LIVELLO_MAX && e->xp >= Elyon_XpPerLivello(s, e->livello + 1)) {
        e->livello++;
        guadagnati++;
        e->amicizia = Utils_ClampInt(e->amicizia + 3, 0, AMICIZIA_MAX);
    }
    if (guadagnati) Elyon_RicalcolaStat(e);
    return guadagnati;
}

int Elyon_MosseAlLivello(const Elyon *e, int livello, int *mosse, int max) {
    const SpeciesData *s = GetSpecies(e->specie);
    int n = 0;
    if (!s) return 0;
    for (int i = 0; i < s->numLearnset && n < max; i++)
        if (s->learnset[i].livello == livello) mosse[n++] = s->learnset[i].mossa;
    return n;
}

int Elyon_PuoEvolvere(const Elyon *e) {
    const EvolutionData *ev = GetEvolution(e->specie);
    if (!ev || e->livello < ev->livello) return 0;
    return ev->a;
}

void Elyon_Evolvi(Elyon *e, int nuovaSpecie) {
    const SpeciesData *n = GetSpecies(nuovaSpecie);
    if (!n) return;
    e->specie = nuovaSpecie;
    /* XP: la curva e' la stessa in tutta la famiglia, quindi il livello non cambia */
    Elyon_RicalcolaStat(e);
}

/* ------------------------------------------------------------------ */

void Collezione_Azzera(Collezione *c) { memset(c, 0, sizeof(*c)); }

int Collezione_NumSquadra(const Collezione *c) {
    int n = 0;
    for (int i = 0; i < MAX_SQUADRA; i++) if (!Elyon_Vuoto(&c->squadra[i])) n++;
    return n;
}

int Collezione_SquadraViva(const Collezione *c) {
    int n = 0;
    for (int i = 0; i < MAX_SQUADRA; i++) if (!Elyon_Esausto(&c->squadra[i])) n++;
    return n;
}

int Collezione_NumBox(const Collezione *c) {
    int n = 0;
    for (int b = 0; b < NUM_BOX; b++)
        for (int p = 0; p < POSTI_BOX; p++) if (!Elyon_Vuoto(&c->box[b][p])) n++;
    return n;
}

void Collezione_Compatta(Collezione *c) {
    int o = 0;
    for (int i = 0; i < MAX_SQUADRA; i++) {
        if (Elyon_Vuoto(&c->squadra[i])) continue;
        if (o != i) { c->squadra[o] = c->squadra[i]; memset(&c->squadra[i], 0, sizeof(Elyon)); }
        o++;
    }
}

int Collezione_Aggiungi(Collezione *c, const Elyon *e) {
    for (int i = 0; i < MAX_SQUADRA; i++)
        if (Elyon_Vuoto(&c->squadra[i])) { c->squadra[i] = *e; return 0; }
    for (int b = 0; b < NUM_BOX; b++)
        for (int p = 0; p < POSTI_BOX; p++)
            if (Elyon_Vuoto(&c->box[b][p])) { c->box[b][p] = *e; return 1; }
    return -1;
}

bool Collezione_Deposita(Collezione *c, int i) {
    if (i < 0 || i >= MAX_SQUADRA || Elyon_Vuoto(&c->squadra[i]) || Collezione_NumSquadra(c) <= 1) return false;
    for (int b = 0; b < NUM_BOX; b++)
        for (int p = 0; p < POSTI_BOX; p++)
            if (Elyon_Vuoto(&c->box[b][p])) {
                c->box[b][p] = c->squadra[i];
                memset(&c->squadra[i], 0, sizeof(Elyon));
                Collezione_Compatta(c);
                return true;
            }
    return false;
}

bool Collezione_Ritira(Collezione *c, int b, int p) {
    if (b < 0 || b >= NUM_BOX || p < 0 || p >= POSTI_BOX || Elyon_Vuoto(&c->box[b][p])) return false;
    for (int i = 0; i < MAX_SQUADRA; i++)
        if (Elyon_Vuoto(&c->squadra[i])) {
            c->squadra[i] = c->box[b][p];
            memset(&c->box[b][p], 0, sizeof(Elyon));
            return true;
        }
    return false;
}

void Collezione_CuraSquadra(Collezione *c) {
    for (int i = 0; i < MAX_SQUADRA; i++) Elyon_Cura(&c->squadra[i]);
}
