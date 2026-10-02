/* battle.c - Motore di lotta. Vedi battle.h per il modello a eventi. */
#include "battle.h"
#include "item_use.h"
#include "data/abilities_data.h"
#include "data/items_data.h"
#include "data/types.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ eventi */

static EventoLotta *Accoda(Lotta *L, TipoEvento tipo, int lato, int a, int b, int c, const char *fmt, ...) {
    if (L->numEventi >= MAX_EVENTI_LOTTA) return NULL;   /* coda piena: si perdono solo gli eventi in eccesso */
    EventoLotta *ev = &L->eventi[L->numEventi++];
    memset(ev, 0, sizeof(*ev));
    ev->tipo = tipo;
    ev->lato = lato;
    ev->a = a; ev->b = b; ev->c = c;
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(ev->testo, sizeof(ev->testo), fmt, ap);
        va_end(ap);
    }
    return ev;
}

#define MSG(L, ...) Accoda((L), EV_MESSAGGIO, -1, 0, 0, 0, __VA_ARGS__)

/* Nome con cui il motore cita l'Elyon in campo */
static const char *NomeLato(const Lotta *L, int lato, char *buf, size_t dim) {
    const Elyon *e = L->lato[lato].e;
    if (lato == LATO_GIOCATORE) snprintf(buf, dim, "%s", Elyon_Nome(e));
    else if (L->tipo == LOTTA_SELVATICA) snprintf(buf, dim, "%s selvatico", Elyon_Nome(e));
    else snprintf(buf, dim, "%s avversario", Elyon_Nome(e));
    return buf;
}

static const char *NOMI_BOOST[NUM_BOOST] = {
    "ATTACCO", "DIFESA", "ATT. SPECIALE", "DIF. SPECIALE", "VELOCITÀ", "PRECISIONE", "ELUSIONE"
};

const char *Meteo_Nome(Meteo m) {
    static const char *n[NUM_METEO] = { "SERENO", "SOLE", "PIOGGIA", "NEVE", "TEMPESTA", "NEBBIA" };
    return (m >= 0 && m < NUM_METEO) ? n[m] : "?";
}

/* ------------------------------------------------------------------ calcoli */

float Lotta_MoltBoost(int stadi) {
    stadi = Utils_ClampInt(stadi, -BOOST_MAX, BOOST_MAX);
    return stadi >= 0 ? (2.0f + stadi) / 2.0f : 2.0f / (2.0f - stadi);
}

float Lotta_MoltMeteo(Meteo m, ElementType t) {
    switch (m) {
    case METEO_SOLE:     return t == TYPE_FIAMMA ? 1.2f : t == TYPE_ACQUA ? 0.8f : 1.0f;
    case METEO_PIOGGIA:  return t == TYPE_ACQUA ? 1.2f : t == TYPE_FIAMMA ? 0.8f : 1.0f;
    case METEO_NEVE:     return t == TYPE_GELO ? 1.2f : 1.0f;
    case METEO_TEMPESTA: return t == TYPE_IMPULSO ? 1.2f : t == TYPE_VENTO ? 1.1f : 1.0f;
    default:             return 1.0f;
    }
}

float Lotta_ProbCattura(const Elyon *e, float fattoreCore, bool evento) {
    const SpeciesData *s = GetSpecies(e->specie);
    if (!s || e->stat[STAT_HP] <= 0) return 0.0f;
    float base = s->catchRate;
    if (base <= 0.0f) {
        if (!evento) return 0.0f;          /* primordiale: solo durante l'evento */
        base = 0.015f;
    }
    float hp = (float)Utils_ClampInt(e->hp, 0, e->stat[STAT_HP]) / (float)e->stat[STAT_HP];
    float st = 1.0f;
    if (e->status == STATUS_SONNO || e->status == STATUS_GELO) st = 1.5f;
    else if (e->status != STATUS_NESSUNO) st = 1.25f;
    return Utils_ClampF(base * (1.0f + 2.0f * (1.0f - hp)) * st * fattoreCore, 0.02f, 0.95f);
}

float Lotta_ProbFuga(int speG, int speN, int tentativo) {
    if (speG >= speN) return 1.0f;
    float rapporto = (float)speG / (float)(speN > 0 ? speN : 1);
    return Utils_ClampF(0.25f + 0.5f * rapporto + 0.2f * (float)(tentativo - 1), 0.25f, 1.0f);
}

int32_t Lotta_XpGuadagnata(int livelloNemico, int baseYield, bool allenatore, int partecipanti) {
    if (partecipanti < 1) partecipanti = 1;
    int64_t xp = (int64_t)livelloNemico * baseYield * (allenatore ? 3 : 2) / (2 * (int64_t)partecipanti);
    if (xp < 1) xp = 1;
    if (xp > 1000000) xp = 1000000;
    return (int32_t)xp;
}

static const AbilityData *Abilita(const Elyon *e) {
    const SpeciesData *s = GetSpecies(e->specie);
    if (!s) return NULL;
    int id = s->abilita;
    if (e->forma) {
        const FormData *f = GetForm(e->specie, e->forma);
        if (f) id = f->abilita;
    }
    return GetAbilityResolved(id);
}

static bool CondizioneOk(const Lotta *L, const Elyon *e, CondizioneAbilita c) {
    int max = e->stat[STAT_HP] > 0 ? e->stat[STAT_HP] : 1;
    switch (c) {
    case COND_SEMPRE:      return true;
    case COND_HP_SOTTO_30: return e->hp * 100 < max * 30;
    case COND_HP_SOTTO_50: return e->hp * 100 < max * 50;
    case COND_HP_SOTTO_25: return e->hp * 100 < max * 25;
    case COND_SOLE:        return L->meteo == METEO_SOLE;
    case COND_PIOGGIA:     return L->meteo == METEO_PIOGGIA;
    case COND_TEMPESTA:    return L->meteo == METEO_TEMPESTA;
    case COND_NOTTE:       return false;   /* ciclo giorno/notte non ancora presente */
    }
    return false;
}

/* Moltiplicatore di abilita' sulla statistica s (AB_MOLT_STAT) */
static float MoltStatAbilita(const Lotta *L, const Elyon *e, StatId s) {
    const AbilityData *ab = Abilita(e);
    if (ab && ab->effetto == AB_MOLT_STAT && ab->param == (int)s && CondizioneOk(L, e, ab->condizione))
        return ab->moltiplicatore;
    return 1.0f;
}

static bool TipoAbilita(const AbilityData *ab, ElementType t) {
    return t != TYPE_NONE && (ab->tipo1 == t || ab->tipo2 == t);
}

static bool HaTipo(const Elyon *e, ElementType t) {
    const SpeciesData *s = GetSpecies(e->specie);
    return s && t != TYPE_NONE && (s->tipo1 == t || s->tipo2 == t);
}

static float EfficaciaSu(const Elyon *dif, ElementType t) {
    const SpeciesData *s = GetSpecies(dif->specie);
    if (!s) return 1.0f;
    return Type_EfficaciaDoppia(t, s->tipo1, s->tipo2);
}

float Lotta_EfficaciaContro(const Lotta *L, int latoDif, ElementType t) {
    return EfficaciaSu(L->lato[latoDif].e, t);
}

int Lotta_VelocitaEffettiva(const Lotta *L, int lato) {
    const Combattente *k = &L->lato[lato];
    float v = (float)k->e->stat[STAT_SPE] * Lotta_MoltBoost(k->boost[BOOST_SPE]) * MoltStatAbilita(L, k->e, STAT_SPE);
    if (k->e->status == STATUS_PARALISI) v *= 0.5f;
    return (int)v;
}

/* Potenza effettiva della mossa per chi attacca */
static int PotenzaMossa(const MoveData *m, const Elyon *att, bool colpitoPrima) {
    int p = m->potenza;
    switch (m->effetto) {
    case EFF_POTENZA_AMICIZIA:
        p = 20 + att->amicizia / 3;
        if (p > m->param1) p = m->param1;
        break;
    case EFF_DIMEZZATA_HP_BASSI:
        if (att->hp * 100 < att->stat[STAT_HP] * m->param1) p /= 2;
        break;
    case EFF_RADDOPPIA_SE_COLPITO:
        if (colpitoPrima) p *= 2;
        break;
    default: break;
    }
    return p;
}

/* Calcolo completo del danno tra due Elyon con i rispettivi boost (NULL = nessun boost).
 * crit: 0/1, casuale in [0.90, 1.00]. */
static int DannoTra(const Lotta *L, const Elyon *att, const int *boostAtt, int furia, bool colpitoPrima,
                    const Elyon *dif, const int *boostDif, const MoveData *m, bool crit, float casuale) {
    if (!m || m->categoria == CAT_STATO || Elyon_Vuoto(att) || Elyon_Vuoto(dif)) return 0;
    bool fisica = m->categoria == CAT_FISICA;
    bool ignoraAbilita = m->effetto == EFF_IGNORA_ABILITA_DIFESA;
    const AbilityData *abA = Abilita(att), *abD = ignoraAbilita ? NULL : Abilita(dif);

    if (abD && abD->effetto == AB_IMMUNITA_TIPO && TipoAbilita(abD, m->tipo)) return 0;
    float eff = EfficaciaSu(dif, m->tipo);
    if (eff <= 0.0f) return 0;

    StatId sa = fisica ? STAT_ATK : STAT_SPA, sd = fisica ? STAT_DEF : STAT_SPD;
    BoostId ba = fisica ? BOOST_ATK : BOOST_SPA, bd = fisica ? BOOST_DEF : BOOST_SPD;
    float A = (float)att->stat[sa] * (boostAtt ? Lotta_MoltBoost(boostAtt[ba]) : 1.0f) * MoltStatAbilita(L, att, sa);
    if (fisica && att->status == STATUS_BRUCIATURA) A *= 0.8f;
    float D = (float)dif->stat[sd] * MoltStatAbilita(L, dif, sd);
    if (boostDif && m->effetto != EFF_IGNORA_BOOST_DIFESA) D *= Lotta_MoltBoost(boostDif[bd]);
    if (sd == STAT_DEF && dif->oggetto == 33) D *= 1.1f;               /* PIETRA FERROSA */
    if (D < 1.0f) D = 1.0f;

    int pot = PotenzaMossa(m, att, colpitoPrima);
    double base = ((2.0 * att->livello / 5.0) + 2.0) * pot * A / D / 50.0 + 2.0;

    double mod = eff * casuale;
    if (HaTipo(att, m->tipo)) mod *= 1.25;                              /* STAB */
    if (crit) mod *= (abD && abD->effetto == AB_CRITICO_RIDOTTO) ? 1.0 + 0.5 * abD->moltiplicatore : 1.5;
    mod *= Lotta_MoltMeteo(L->meteo, m->tipo);
    if (abA && abA->effetto == AB_MOLT_TIPO_ATTACCO && TipoAbilita(abA, m->tipo) && CondizioneOk(L, att, abA->condizione))
        mod *= abA->moltiplicatore;
    if (abD && abD->effetto == AB_MOLT_DANNO_SUBITO && TipoAbilita(abD, m->tipo)) mod *= abD->moltiplicatore;
    if (abD && abD->effetto == AB_MOLT_DANNO_FISICO_SUBITO && fisica) mod *= abD->moltiplicatore;
    /* oggetti equipaggiati che potenziano un tipo */
    static const struct { int oggetto; ElementType tipo; } POT[] = {
        { 30, TYPE_FIAMMA }, { 31, TYPE_ACQUA }, { 34, TYPE_ASTRALE }, { 35, TYPE_OMBRA } };
    for (size_t i = 0; i < sizeof(POT) / sizeof(POT[0]); i++)
        if (att->oggetto == POT[i].oggetto && m->tipo == POT[i].tipo) mod *= 1.1;
    if (L->campoTurni > 0 && m->tipo == L->campoTipo && m->tipo != TYPE_NONE) mod *= 1.0 + L->campoBonus / 100.0;
    if (fisica && furia > 0) mod *= 1.0 + furia / 100.0;

    int danno = (int)floor(base * mod);
    return danno < 1 ? 1 : danno;
}

int Lotta_StimaDanno(const Lotta *L, int latoAtt, int mossa, float casuale) {
    const Combattente *a = &L->lato[latoAtt], *d = &L->lato[1 - latoAtt];
    const MoveData *m = GetMove(mossa);
    if (!m) return 0;
    int danno = DannoTra(L, a->e, a->boost, a->furia, a->colpitoNelTurno, d->e, d->boost, m, false, casuale);
    if (m->effetto == EFF_MULTICOLPO) danno *= m->param1;
    return danno;
}

/* ------------------------------------------------------------------ stato e boost */

static bool Prob(Lotta *L, int percento) { return percento >= 100 || Rng_Range(L->rng, 100) < percento; }

static void ApplicaBoost(Lotta *L, int lato, int stat, int stadi) {
    Combattente *k = &L->lato[lato];
    char n[48];
    NomeLato(L, lato, n, sizeof(n));
    if (stat < 0 || stat >= NUM_BOOST || stadi == 0) return;
    int prima = k->boost[stat];
    k->boost[stat] = Utils_ClampInt(prima + stadi, -BOOST_MAX, BOOST_MAX);
    int fatto = k->boost[stat] - prima;
    if (fatto == 0) {
        MSG(L, "%s: %s non può %s oltre!", n, NOMI_BOOST[stat], stadi > 0 ? "salire" : "scendere");
        return;
    }
    const char *come = fatto >= 2 ? "sale molto!" : fatto > 0 ? "sale!" : fatto <= -2 ? "scende molto!" : "scende!";
    Accoda(L, EV_BOOST, lato, stat, fatto, 0, "%s: %s %s", n, NOMI_BOOST[stat], come);
}

/* Infligge uno stato. Ritorna false se non e' possibile (gia' presente o immunita'). */
static bool InfliggiStatus(Lotta *L, int lato, StatusId s) {
    Combattente *k = &L->lato[lato];
    if (Elyon_Esausto(k->e)) return false;
    const AbilityData *ab = Abilita(k->e);
    if (ab && ab->effetto == AB_IMMUNITA_STATUS && ab->param == (int)s) return false;
    char n[48];
    NomeLato(L, lato, n, sizeof(n));
    if (s == STATUS_CONFUSIONE) {
        if (k->confusione > 0) return false;
        k->confusione = 1 + Rng_Range(L->rng, 3);
        Accoda(L, EV_STATUS, lato, STATUS_CONFUSIONE, 0, 0, "%s è confuso!", n);
        return true;
    }
    if (k->e->status != STATUS_NESSUNO || s == STATUS_NESSUNO) return false;
    k->e->status = s;
    const char *testo = "";
    switch (s) {
    case STATUS_VELENO:       testo = "è stato avvelenato!"; break;
    case STATUS_VELENO_GRAVE: testo = "è stato gravemente avvelenato!"; k->e->contatoreVelenoGrave = 1; break;
    case STATUS_BRUCIATURA:   testo = "è stato bruciato!"; break;
    case STATUS_PARALISI:     testo = "è paralizzato! Potrebbe non riuscire a muoversi."; break;
    case STATUS_SONNO:        testo = "si è addormentato!"; k->e->turniSonno = 1 + Rng_Range(L->rng, 3); break;
    case STATUS_GELO:         testo = "è stato congelato!"; break;
    default: break;
    }
    Accoda(L, EV_STATUS, lato, s, 0, 0, "%s %s", n, testo);
    return true;
}

static void ControllaInterruzione(Lotta *L);
static void GestisciKO(Lotta *L, int lato);

/* Toglie HP senza modificatori (stati, contraccolpo, autocolpo) */
static void PerdiHp(Lotta *L, int lato, int quanti, const char *fmt, ...) {
    Combattente *k = &L->lato[lato];
    if (Elyon_Esausto(k->e)) return;
    if (quanti < 1) quanti = 1;
    k->e->hp = Utils_ClampInt(k->e->hp - quanti, 0, k->e->stat[STAT_HP]);
    EventoLotta *ev = Accoda(L, EV_DANNO, lato, k->e->hp, 100, 0, NULL);
    if (ev && fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(ev->testo, sizeof(ev->testo), fmt, ap);
        va_end(ap);
    }
    if (lato == LATO_NEMICO) ControllaInterruzione(L);
    if (k->e->hp <= 0) GestisciKO(L, lato);
}

static void CuraHp(Lotta *L, int lato, int quanti, const char *fmt, ...) {
    Combattente *k = &L->lato[lato];
    if (Elyon_Esausto(k->e) || k->e->hp >= k->e->stat[STAT_HP]) return;
    if (quanti < 1) quanti = 1;
    k->e->hp = Utils_ClampInt(k->e->hp + quanti, 0, k->e->stat[STAT_HP]);
    EventoLotta *ev = Accoda(L, EV_CURA, lato, k->e->hp, 0, 0, NULL);
    if (ev && fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(ev->testo, sizeof(ev->testo), fmt, ap);
        va_end(ap);
    }
}

/* ------------------------------------------------------------------ XP */

static void AssegnaXp(Lotta *L, int i, int32_t xp) {
    Elyon *e = &L->coll->squadra[i];
    if (Elyon_Esausto(e) || e->livello >= LIVELLO_MAX) return;
    int prima = e->livello;
    int saliti = Elyon_AggiungiXp(e, xp);
    /* l'evento XP riporta l'XP al termine; i livelli seguono come eventi separati */
    Accoda(L, EV_XP, LATO_GIOCATORE, i, e->xp, 0, "%s guadagna %d punti esperienza!", Elyon_Nome(e), (int)xp);
    for (int lv = prima + 1; lv <= prima + saliti; lv++) {
        EventoLotta *ev = Accoda(L, EV_LIVELLO, LATO_GIOCATORE, i, lv, e->hp, "%s sale al livello %d!", Elyon_Nome(e), lv);
        if (ev) ev->d = e->stat[STAT_HP];
        int nuove[4];
        int n = Elyon_MosseAlLivello(e, lv, nuove, 4);
        for (int k = 0; k < n; k++) {
            const MoveData *m = GetMove(nuove[k]);
            if (!m) continue;
            bool conosciuta = false;
            for (int s = 0; s < MAX_MOSSE_ELYON; s++) if (e->mosse[s] == nuove[k]) conosciuta = true;
            if (conosciuta) continue;
            if (Elyon_ImparaMossa(e, nuove[k]))
                Accoda(L, EV_NUOVA_MOSSA, LATO_GIOCATORE, i, nuove[k], 0, "%s impara %s!", Elyon_Nome(e), m->nome);
            else
                Accoda(L, EV_RICHIESTA_MOSSA, LATO_GIOCATORE, i, nuove[k], 0, "%s vuole imparare %s...", Elyon_Nome(e), m->nome);
        }
    }
    if (saliti) L->livelloSalito[i] = true;
}

static void DistribuisciXp(Lotta *L, const Elyon *sconfitto) {
    const SpeciesData *s = GetSpecies(sconfitto->specie);
    if (!s) return;
    int part = 0;
    for (int i = 0; i < MAX_SQUADRA; i++)
        if (L->partecipanti[i] && !Elyon_Esausto(&L->coll->squadra[i])) part++;
    if (part == 0) return;
    int32_t xp = Lotta_XpGuadagnata(sconfitto->livello, s->baseYield, L->tipo == LOTTA_ALLENATORE, part);
    bool condiviso = Inventario_Quantita(L->inv, ITEM_NUCLEO_CONDIVISO) > 0;
    for (int i = 0; i < MAX_SQUADRA; i++) {
        if (Elyon_Esausto(&L->coll->squadra[i])) continue;
        if (L->partecipanti[i]) AssegnaXp(L, i, xp);
        else if (condiviso) AssegnaXp(L, i, xp / 2 > 0 ? xp / 2 : 1);
    }
}

/* ------------------------------------------------------------------ KO, entrata, fine */

static void GestisciKO(Lotta *L, int lato) {
    Combattente *k = &L->lato[lato];
    if (k->koAnnunciato || k->e->hp > 0) return;
    k->koAnnunciato = true;
    k->e->status = STATUS_NESSUNO;
    k->e->turniSonno = 0;
    k->e->contatoreVelenoGrave = 0;
    char n[48];
    Accoda(L, EV_KO, lato, 0, 0, 0, "%s è esausto!", NomeLato(L, lato, n, sizeof(n)));
    if (lato == LATO_NEMICO && L->esito == ESITO_IN_CORSO) DistribuisciXp(L, k->e);
}

static void ControllaInterruzione(Lotta *L) {
    if (!L->allenatore || !L->allenatore->interruzioneNarrativa || L->esito != ESITO_IN_CORSO) return;
    int vivi = 0;
    for (int i = 0; i < L->numNemici; i++) if (!Elyon_Esausto(&L->nemici[i])) vivi++;
    const Elyon *e = L->lato[LATO_NEMICO].e;
    bool ultimo = vivi == 0 || (vivi == 1 && !Elyon_Esausto(e));
    if (ultimo && e->hp * 2 <= e->stat[STAT_HP]) {
        L->esito = ESITO_INTERROTTA;
        MSG(L, "La lotta si interrompe all'improvviso!");
    }
}

static void Entra(Lotta *L, int lato, int indice, bool annuncia) {
    Combattente *k = &L->lato[lato];
    memset(k, 0, sizeof(*k));
    k->e = lato == LATO_GIOCATORE ? &L->coll->squadra[indice] : &L->nemici[indice];
    k->indice = indice;
    if (k->e->status == STATUS_VELENO_GRAVE) k->e->contatoreVelenoGrave = 1;
    if (lato == LATO_GIOCATORE) L->partecipanti[indice] = true;
    if (!annuncia) return;
    EventoLotta *ev;
    if (lato == LATO_GIOCATORE)
        ev = Accoda(L, EV_ENTRATA, lato, indice, k->e->hp, k->e->stat[STAT_HP], "Vai, %s!", Elyon_Nome(k->e));
    else if (L->tipo == LOTTA_SELVATICA)
        ev = Accoda(L, EV_ENTRATA, lato, indice, k->e->hp, k->e->stat[STAT_HP], "Appare un %s selvatico!", Elyon_Nome(k->e));
    else
        ev = Accoda(L, EV_ENTRATA, lato, indice, k->e->hp, k->e->stat[STAT_HP], "%s manda in campo %s!",
                    L->allenatore->nome, Elyon_Nome(k->e));
    if (ev) ev->d = k->e->livello;
}

static int PrimoVivoSquadra(const Collezione *c) {
    for (int i = 0; i < MAX_SQUADRA; i++) if (!Elyon_Esausto(&c->squadra[i])) return i;
    return -1;
}

static int NemiciVivi(const Lotta *L) {
    int n = 0;
    for (int i = 0; i < L->numNemici; i++) if (!Elyon_Esausto(&L->nemici[i])) n++;
    return n;
}

static void Termina(Lotta *L, EsitoLotta esito) {
    if (L->esito == ESITO_IN_CORSO) L->esito = esito;
    switch (L->esito) {
    case ESITO_VITTORIA:
        if (L->allenatore) {
            Inventario_AggiungiLumen(L->inv, L->allenatore->premio);
            MSG(L, "Hai sconfitto %s %s!", L->allenatore->classe, L->allenatore->nome);
            if (L->allenatore->premio > 0) MSG(L, "Ricevi %d Lumen come premio.", L->allenatore->premio);
        }
        break;
    case ESITO_SCONFITTA:
        MSG(L, "Non hai più Elyon in grado di lottare...");
        break;
    default: break;
    }
    Accoda(L, EV_FINE, -1, L->esito, 0, 0, NULL);
}

/* ------------------------------------------------------------------ init */

static void InitBase(Lotta *L, Collezione *c, Inventario *inv, Rng *rng) {
    memset(L, 0, sizeof(*L));
    L->introEsitaBase = -1;
    L->coll = c;
    L->inv = inv;
    L->rng = rng;
    L->campoTipo = TYPE_NONE;
    L->ultimoCambioIA = -10;
}

bool Lotta_InitSelvatica(Lotta *L, Collezione *c, Inventario *inv, Rng *rng, int specie, int livello) {
    InitBase(L, c, inv, rng);
    L->tipo = LOTTA_SELVATICA;
    Elyon_Crea(&L->nemici[0], specie, livello, rng);
    if (Elyon_Vuoto(&L->nemici[0])) return false;
    L->numNemici = 1;
    int p = PrimoVivoSquadra(c);
    if (p < 0) return false;
    Entra(L, LATO_GIOCATORE, p, false);
    Entra(L, LATO_NEMICO, 0, false);
    return true;
}

bool Lotta_InitAllenatore(Lotta *L, Collezione *c, Inventario *inv, Rng *rng, const TrainerData *t) {
    InitBase(L, c, inv, rng);
    if (!t || t->numMembri <= 0) return false;
    L->tipo = LOTTA_ALLENATORE;
    L->allenatore = t;
    for (int i = 0; i < t->numMembri && i < MAX_SQUADRA; i++) {
        Elyon *e = &L->nemici[L->numNemici];
        Elyon_Crea(e, t->squadra[i].specie, t->squadra[i].livello, rng);
        if (Elyon_Vuoto(e)) continue;
        e->varianteLumen = false;
        bool definite = false;
        for (int k = 0; k < 4; k++) if (t->squadra[i].mosse[k]) definite = true;
        if (definite) {
            memset(e->mosse, 0, sizeof(e->mosse));
            memset(e->pp, 0, sizeof(e->pp));
            for (int k = 0; k < 4; k++) if (t->squadra[i].mosse[k]) Elyon_ImparaMossa(e, t->squadra[i].mosse[k]);
        }
        L->numNemici++;
    }
    if (L->numNemici == 0) return false;
    if (t->ia == AI_BOSS) L->cureBossMax = Utils_ClampInt(L->numNemici / 2, 1, 3);
    int p = PrimoVivoSquadra(c);
    if (p < 0) return false;
    Entra(L, LATO_GIOCATORE, p, false);
    Entra(L, LATO_NEMICO, 0, false);
    return true;
}

void Lotta_Inizia(Lotta *L) {
    L->numEventi = 0;
    Combattente *g = &L->lato[LATO_GIOCATORE], *n = &L->lato[LATO_NEMICO];
    if (L->tipo == LOTTA_ALLENATORE)
        MSG(L, "%s %s ti sfida!", L->allenatore->classe, L->allenatore->nome);
    Entra(L, LATO_NEMICO, n->indice, true);
    Entra(L, LATO_GIOCATORE, g->indice, true);
    if (L->meteo != METEO_NESSUNO)
        Accoda(L, EV_METEO, -1, L->meteo, 0, 0, "Meteo: %s.", Meteo_Nome(L->meteo));
}

/* ------------------------------------------------------------------ validazione */

bool Lotta_Finita(const Lotta *L) { return L->esito != ESITO_IN_CORSO; }

bool Lotta_DeveDimenarsi(const Lotta *L) {
    const Elyon *e = L->lato[LATO_GIOCATORE].e;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) if (e->mosse[i] && e->pp[i] > 0) return false;
    return true;
}

static bool SpazioCattura(const Collezione *c) {
    return Collezione_NumSquadra(c) < MAX_SQUADRA || Collezione_NumBox(c) < NUM_BOX * POSTI_BOX;
}

bool Lotta_AzioneValida(const Lotta *L, Azione az, char *motivo, int dim) {
    char tmp[8];
    if (!motivo) { motivo = tmp; dim = sizeof(tmp); }
    motivo[0] = '\0';
    if (L->esito != ESITO_IN_CORSO) { snprintf(motivo, (size_t)dim, "La lotta è finita."); return false; }
    if (L->attesaCambio) { snprintf(motivo, (size_t)dim, "Scegli un Elyon da mandare in campo."); return false; }
    const Combattente *g = &L->lato[LATO_GIOCATORE];
    switch (az.tipo) {
    case AZ_MOSSA:
        if (az.param == MOSSA_DIMENARSI) {
            if (Lotta_DeveDimenarsi(L)) return true;
            snprintf(motivo, (size_t)dim, "Ci sono ancora mosse utilizzabili.");
            return false;
        }
        if (az.param < 0 || az.param >= MAX_MOSSE_ELYON || !GetMove(g->e->mosse[az.param])) {
            snprintf(motivo, (size_t)dim, "Mossa non valida.");
            return false;
        }
        if (g->e->pp[az.param] <= 0) { snprintf(motivo, (size_t)dim, "Non ci sono PP per questa mossa!"); return false; }
        return true;
    case AZ_CAMBIO: {
        if (az.param < 0 || az.param >= MAX_SQUADRA || Elyon_Vuoto(&L->coll->squadra[az.param])) {
            snprintf(motivo, (size_t)dim, "Nessun Elyon in quella posizione.");
            return false;
        }
        const Elyon *e = &L->coll->squadra[az.param];
        if (az.param == g->indice) { snprintf(motivo, (size_t)dim, "%s è già in campo!", Elyon_Nome(e)); return false; }
        if (Elyon_Esausto(e)) { snprintf(motivo, (size_t)dim, "%s non è in grado di lottare!", Elyon_Nome(e)); return false; }
        if (g->bloccoCambio > 0) { snprintf(motivo, (size_t)dim, "%s non può rientrare!", Elyon_Nome(g->e)); return false; }
        return true;
    }
    case AZ_OGGETTO: {
        if (!GetItem(az.param) || Inventario_Quantita(L->inv, az.param) <= 0) {
            snprintf(motivo, (size_t)dim, "Non hai questo oggetto.");
            return false;
        }
        float core = Oggetto_FattoreCore(az.param);
        if (core > 0.0f) {
            if (L->tipo != LOTTA_SELVATICA) {
                snprintf(motivo, (size_t)dim, "Non puoi catturare l'Elyon di un allenatore!");
                return false;
            }
            if (Lotta_ProbCattura(L->lato[LATO_NEMICO].e, core, L->catturaEvento) <= 0.0f) {
                snprintf(motivo, (size_t)dim, "Il Core non reagisce: questo Elyon non può essere catturato.");
                return false;
            }
            if (!SpazioCattura(L->coll)) { snprintf(motivo, (size_t)dim, "Non c'è più spazio né in squadra né nel box!"); return false; }
            return true;
        }
        if (!Oggetto_SuElyon(az.param)) { snprintf(motivo, (size_t)dim, "Non puoi usarlo ora."); return false; }
        if (az.bersaglio < 0 || az.bersaglio >= MAX_SQUADRA || Elyon_Vuoto(&L->coll->squadra[az.bersaglio])) {
            snprintf(motivo, (size_t)dim, "Scegli un Elyon.");
            return false;
        }
        Elyon prova = L->coll->squadra[az.bersaglio];
        char m[TESTO_EVENTO];
        if (!Oggetto_Usa(&prova, az.param, az.slotMossa, m, sizeof(m))) {
            snprintf(motivo, (size_t)dim, "Non avrebbe alcun effetto.");
            return false;
        }
        return true;
    }
    case AZ_FUGA:
        if (L->tipo != LOTTA_SELVATICA) { snprintf(motivo, (size_t)dim, "Non puoi fuggire da una lotta contro un allenatore!"); return false; }
        return true;
    }
    return false;
}

/* ------------------------------------------------------------------ esecuzione mosse */

static void ApplicaDanno(Lotta *L, int latoDif, int danno, bool fisico, int latoAtt) {
    Combattente *d = &L->lato[latoDif];
    if (Elyon_Esausto(d->e)) return;
    if (d->dimezzaProssimo) {
        danno = danno / 2 > 0 ? danno / 2 : 1;
        d->dimezzaProssimo = false;
    }
    bool scudo = false;
    if (danno >= d->e->hp && d->e->hp > 1 && !L->scudoUsato[latoDif][d->indice]) {
        const AbilityData *ab = Abilita(d->e);
        if (ab && ab->effetto == AB_RESISTE_UN_COLPO) {
            danno = d->e->hp - 1;
            L->scudoUsato[latoDif][d->indice] = true;
            scudo = true;
        }
    }
    d->e->hp = Utils_ClampInt(d->e->hp - danno, 0, d->e->stat[STAT_HP]);
    d->colpitoNelTurno = true;
    Accoda(L, EV_DANNO, latoDif, d->e->hp, 100, 0, NULL);
    if (scudo) {
        char n[48];
        MSG(L, "%s resiste grazie a SCUDO LUMEN!", NomeLato(L, latoDif, n, sizeof(n)));
    }
    /* RIFLESSIONE: parte del danno fisico torna all'attaccante */
    if (fisico && d->riflessioneTurni > 0 && danno > 0 && !Elyon_Esausto(L->lato[latoAtt].e)) {
        char n[48];
        PerdiHp(L, latoAtt, danno * d->riflessionePerc / 100, "%s subisce danni riflessi!", NomeLato(L, latoAtt, n, sizeof(n)));
    }
    if (latoDif == LATO_NEMICO) ControllaInterruzione(L);
    if (d->e->hp <= 0) GestisciKO(L, latoDif);
}

static void EffettoStato(Lotta *L, int lato, const MoveData *m) {
    Combattente *a = &L->lato[lato], *d = &L->lato[1 - lato];
    char na[48], nd[48];
    NomeLato(L, lato, na, sizeof(na));
    NomeLato(L, 1 - lato, nd, sizeof(nd));
    switch (m->effetto) {
    case EFF_STATUS:
        if (!Prob(L, m->prob)) { MSG(L, "Ma fallisce!"); break; }
        if (!InfliggiStatus(L, 1 - lato, (StatusId)m->param1)) MSG(L, "Non ha effetto su %s!", nd);
        break;
    case EFF_BOOST_SE:
        ApplicaBoost(L, lato, m->param1, m->param2);
        if (m->param4) ApplicaBoost(L, lato, m->param3, m->param4);
        break;
    case EFF_BOOST_NEMICO:
        if (!Prob(L, m->prob)) { MSG(L, "Ma fallisce!"); break; }
        ApplicaBoost(L, 1 - lato, m->param1, m->param2);
        break;
    case EFF_CURA_PERC:
        if (a->e->hp >= a->e->stat[STAT_HP]) MSG(L, "Gli HP di %s sono già al massimo!", na);
        else CuraHp(L, lato, a->e->stat[STAT_HP] * m->param1 / 100, "%s recupera energia!", na);
        break;
    case EFF_RADICAMENTO:
        if (a->radicamento) MSG(L, "Ma fallisce!");
        else { a->radicamento = m->param1 > 0 ? m->param1 : 8; MSG(L, "%s mette radici!", na); }
        break;
    case EFF_DIMEZZA_PROSSIMO:
        a->dimezzaProssimo = true;
        MSG(L, "%s si prepara ad attutire il prossimo colpo!", na);
        break;
    case EFF_STORDIMENTO_PROSSIMO:
        if (!Prob(L, m->prob)) { MSG(L, "Ma fallisce!"); break; }
        d->storditoProssimo = true;
        MSG(L, "%s è sotto controllo!", nd);
        break;
    case EFF_RIFLESSIONE:
        a->riflessioneTurni = m->param2 + 1;   /* +1: il turno corrente si chiude subito */
        a->riflessionePerc = m->param1;
        MSG(L, "%s si ricopre di una barriera riflettente!", na);
        break;
    case EFF_FURIA:
        if (a->furia >= m->param2) { MSG(L, "La furia di %s è già al massimo!", na); break; }
        a->furia = Utils_ClampInt(a->furia + m->param1, 0, m->param2);
        MSG(L, "%s si carica di furia!", na);
        break;
    case EFF_BLOCCA_CAMBIO:
        d->bloccoCambio = m->param1 + 1;
        MSG(L, "%s non può più rientrare!", nd);
        break;
    case EFF_CAMPO_TIPO:
        L->campoTipo = (ElementType)m->param1;
        L->campoBonus = m->param2;
        L->campoTurni = m->param3 + 1;
        MSG(L, "Il campo si carica di energia %s!", Type_Nome(L->campoTipo));
        break;
    case EFF_EQUILIBRIO: {
        int media = (a->e->hp + d->e->hp) / 2;
        int ha = Utils_ClampInt(media, 1, a->e->stat[STAT_HP]);
        int hd = Utils_ClampInt(media, 1, d->e->stat[STAT_HP]);
        a->e->hp = ha;
        d->e->hp = hd;
        Accoda(L, EV_CURA, lato, ha, 0, 0, NULL);
        Accoda(L, EV_CURA, 1 - lato, hd, 0, 0, "Gli HP dei due Elyon si equilibrano!");
        if (1 - lato == LATO_NEMICO) ControllaInterruzione(L);
        break;
    }
    default:
        MSG(L, "Ma non succede nulla...");
        break;
    }
}

/* Ritorna true se l'Elyon riesce ad agire (gestisce sonno, gelo, stordimento, paralisi, confusione) */
static bool PuoAgire(Lotta *L, int lato) {
    Combattente *a = &L->lato[lato];
    char n[48];
    NomeLato(L, lato, n, sizeof(n));
    if (a->e->status == STATUS_SONNO) {
        if (a->e->turniSonno > 0) {
            a->e->turniSonno--;
            MSG(L, "%s dorme profondamente.", n);
            return false;
        }
        a->e->status = STATUS_NESSUNO;
        Accoda(L, EV_STATUS, lato, STATUS_NESSUNO, 0, 0, "%s si sveglia!", n);
    }
    if (a->e->status == STATUS_GELO) {
        if (Rng_Range(L->rng, 100) < 20) {
            a->e->status = STATUS_NESSUNO;
            Accoda(L, EV_STATUS, lato, STATUS_NESSUNO, 0, 0, "%s si è scongelato!", n);
        } else {
            MSG(L, "%s è congelato!", n);
            return false;
        }
    }
    if (a->stordito) {
        a->stordito = false;
        MSG(L, "%s è stordito e non riesce ad agire!", n);
        return false;
    }
    if (a->e->status == STATUS_PARALISI && Rng_Range(L->rng, 100) < 25) {
        MSG(L, "%s è paralizzato! Non riesce a muoversi!", n);
        return false;
    }
    if (a->confusione > 0) {
        a->confusione--;
        if (a->confusione == 0) {
            Accoda(L, EV_STATUS, lato, STATUS_NESSUNO, 1, 0, "%s non è più confuso!", n);
        } else {
            MSG(L, "%s è confuso...", n);
            if (Rng_Range(L->rng, 100) < 40) {
                /* autocolpo: potenza 40, fisico, senza tipo, senza critico */
                static const MoveData AUTOCOLPO = { 0, "AUTOCOLPO", TYPE_NONE, CAT_FISICA, 40, 100, 1, 0,
                                                    EFF_NESSUNO, 0, 0, 0, 0, 0, 0, "" };
                float cas = 0.90f + (float)Rng_Range(L->rng, 11) / 100.0f;
                int danno = DannoTra(L, a->e, a->boost, 0, false, a->e, a->boost, &AUTOCOLPO, false, cas);
                PerdiHp(L, lato, danno, "Si colpisce da solo nella confusione!");
                return false;
            }
        }
    }
    return true;
}

static void UsaMossa(Lotta *L, int lato, int slot) {
    Combattente *a = &L->lato[lato], *d = &L->lato[1 - lato];
    char na[48], nd[48];
    NomeLato(L, lato, na, sizeof(na));
    if (!PuoAgire(L, lato) || Elyon_Esausto(a->e)) return;

    bool dimenarsi = slot == MOSSA_DIMENARSI;
    const MoveData *m;
    if (dimenarsi) {
        m = GetMove(ID_DIMENARSI);
        MSG(L, "%s non ha più PP e si dimena!", na);
    } else {
        m = GetMove(a->e->mosse[slot]);
        if (!m || a->e->pp[slot] <= 0) return;
        a->e->pp[slot]--;
    }
    Accoda(L, EV_ATTACCO, lato, m->id, 0, 0, "%s usa %s!", na, m->nome);

    if (Elyon_Esausto(d->e) && m->categoria != CAT_STATO) { MSG(L, "Ma non c'è nessun bersaglio!"); return; }
    NomeLato(L, 1 - lato, nd, sizeof(nd));

    /* precisione */
    if (!(m->flag & MOSSA_PRECISIONE_AUTOMATICA)) {
        int stadi = Utils_ClampInt(a->boost[BOOST_PRECISIONE] - d->boost[BOOST_EVASIONE], -BOOST_MAX, BOOST_MAX);
        float prec = (float)m->precisione * Lotta_MoltBoost(stadi);
        if (L->meteo == METEO_NEBBIA) prec *= 0.9f;
        if ((float)Rng_Range(L->rng, 100) >= prec) {
            MSG(L, "%s manca il bersaglio!", na);
            return;
        }
    }

    if (m->categoria == CAT_STATO) { EffettoStato(L, lato, m); return; }

    bool fisica = m->categoria == CAT_FISICA;
    const AbilityData *abD = m->effetto == EFF_IGNORA_ABILITA_DIFESA ? NULL : Abilita(d->e);
    if (abD && abD->effetto == AB_IMMUNITA_TIPO && TipoAbilita(abD, m->tipo)) {
        MSG(L, "Non ha effetto su %s!", nd);
        ApplicaBoost(L, 1 - lato, abD->param, 1);
        return;
    }
    float eff = EfficaciaSu(d->e, m->tipo);
    if (eff <= 0.0f) { MSG(L, "Non ha effetto su %s!", nd); return; }

    int colpi = m->effetto == EFF_MULTICOLPO ? Utils_ClampInt(m->param1, 1, 5) : 1, fatti = 0;
    const AbilityData *abA = Abilita(a->e);
    int critPerc = 6;
    if (m->effetto == EFF_CRITICO_ALTO) critPerc = m->param1;
    if (abA && abA->effetto == AB_CRITICO_ALTO && CondizioneOk(L, a->e, abA->condizione) && abA->param > critPerc)
        critPerc = abA->param;
    bool colpitoPrima = a->colpitoNelTurno;
    for (int c = 0; c < colpi && !Elyon_Esausto(d->e); c++) {
        bool crit = Rng_Range(L->rng, 100) < critPerc;
        float cas = 0.90f + (float)Rng_Range(L->rng, 11) / 100.0f;
        int danno = DannoTra(L, a->e, a->boost, a->furia, colpitoPrima, d->e, d->boost, m, crit, cas);
        int evPrima = L->numEventi;
        ApplicaDanno(L, 1 - lato, danno, fisica, lato);
        if (evPrima < L->numEventi && L->eventi[evPrima].tipo == EV_DANNO) {
            L->eventi[evPrima].b = (int)(eff * 100.0f + 0.5f);
            L->eventi[evPrima].c = crit;
        }
        if (crit) MSG(L, "Brutto colpo!");
        fatti++;
    }
    if (eff > 1.0f) MSG(L, "È superefficace!");
    else if (eff < 1.0f) MSG(L, "Non è molto efficace...");
    if (colpi > 1) MSG(L, "Colpito %d volte!", fatti);
    if (fisica) a->furia = 0;
    if (L->esito != ESITO_IN_CORSO) return;

    /* effetti secondari sul bersaglio (solo se ancora in piedi) */
    bool difVivo = !Elyon_Esausto(d->e);
    switch (m->effetto) {
    case EFF_STATUS:
        if (difVivo && Prob(L, m->prob)) InfliggiStatus(L, 1 - lato, (StatusId)m->param1);
        break;
    case EFF_STORDIMENTO:
        if (difVivo && !d->haAgito && Prob(L, m->prob)) d->stordito = true;
        break;
    case EFF_BOOST_NEMICO:
        if (difVivo && Prob(L, m->prob)) ApplicaBoost(L, 1 - lato, m->param1, m->param2);
        break;
    case EFF_BOOST_SE:
        if (Prob(L, m->prob)) {
            ApplicaBoost(L, lato, m->param1, m->param2);
            if (m->param4) ApplicaBoost(L, lato, m->param3, m->param4);
        }
        break;
    case EFF_CONTRACCOLPO:
        PerdiHp(L, lato, a->e->stat[STAT_HP] * m->param1 / 100, "%s subisce il contraccolpo!", na);
        break;
    default: break;
    }
    if (abA && abA->effetto == AB_STATUS_SU_ATTACCO && TipoAbilita(abA, m->tipo) && difVivo && !Elyon_Esausto(d->e)
        && Prob(L, abA->param))
        InfliggiStatus(L, 1 - lato, STATUS_VELENO);
    if (dimenarsi) PerdiHp(L, lato, a->e->stat[STAT_HP] / 4, "%s subisce il contraccolpo!", na);
}

/* ------------------------------------------------------------------ altre azioni */

static void Cattura(Lotta *L, int item) {
    Inventario_Rimuovi(L->inv, item, 1);
    const ItemData *it = GetItem(item);
    Accoda(L, EV_LANCIO_CORE, LATO_GIOCATORE, item, 0, 0, "Lanci un %s!", it ? it->nome : "CORE");
    Elyon *e = L->lato[LATO_NEMICO].e;
    float p = Lotta_ProbCattura(e, Oggetto_FattoreCore(item), L->catturaEvento);
    /* tre scosse indipendenti con probabilita' p^(1/3): la cattura riesce con probabilita' p */
    float q = cbrtf(p);
    int scosse = 0;
    while (scosse < 3 && Rng_Float(L->rng) < q) {
        scosse++;
        Accoda(L, EV_SCOSSA, LATO_NEMICO, scosse, 0, 0, NULL);
    }
    if (scosse < 3) {
        static const char *testi[3] = { "Oh no! L'Elyon si è liberato!", "Ci eri quasi!", "Accidenti, per un soffio!" };
        Accoda(L, EV_CATTURA_FALLITA, LATO_NEMICO, scosse, 0, 0, "%s", testi[scosse]);
        return;
    }
    Elyon preso = *e;
    int dove = Collezione_Aggiungi(L->coll, &preso);
    Accoda(L, EV_CATTURA_OK, LATO_NEMICO, dove, 0, 0, "Hai catturato %s!", Elyon_Nome(&preso));
    if (dove == 1) MSG(L, "%s è stato mandato nel box.", Elyon_Nome(&preso));
    L->esito = ESITO_CATTURA;
}

static void UsaOggetto(Lotta *L, const Azione *az) {
    Elyon *e = &L->coll->squadra[az->bersaglio];
    int hpPrima = e->hp;
    StatusId stPrima = e->status;
    char msg[TESTO_EVENTO];
    const ItemData *it = GetItem(az->param);
    MSG(L, "Usi %s.", it ? it->nome : "?");
    if (!Oggetto_Usa(e, az->param, az->slotMossa, msg, sizeof(msg))) { MSG(L, "Non ha effetto."); return; }
    Inventario_Rimuovi(L->inv, az->param, 1);
    bool inCampo = az->bersaglio == L->lato[LATO_GIOCATORE].indice;
    if (inCampo && e->hp != hpPrima) Accoda(L, EV_CURA, LATO_GIOCATORE, e->hp, 0, 0, NULL);
    if (inCampo && e->status != stPrima) Accoda(L, EV_STATUS, LATO_GIOCATORE, e->status, 0, 0, NULL);
    MSG(L, "%s", msg);
}

static void Fuga(Lotta *L) {
    L->tentativiFuga++;
    float p = Lotta_ProbFuga(Lotta_VelocitaEffettiva(L, LATO_GIOCATORE), Lotta_VelocitaEffettiva(L, LATO_NEMICO),
                             L->tentativiFuga);
    if (p >= 1.0f || Rng_Float(L->rng) < p) {
        MSG(L, "Fuga riuscita!");
        L->esito = ESITO_FUGA;
    } else {
        MSG(L, "Non riesci a fuggire!");
    }
}

static void Cambio(Lotta *L, int lato, int indice) {
    char n[48];
    NomeLato(L, lato, n, sizeof(n));
    if (lato == LATO_GIOCATORE) Accoda(L, EV_RITIRO, lato, 0, 0, 0, "%s, rientra!", Elyon_Nome(L->lato[lato].e));
    else Accoda(L, EV_RITIRO, lato, 0, 0, 0, "%s ritira %s!", L->allenatore->nome, Elyon_Nome(L->lato[lato].e));
    Entra(L, lato, indice, true);
}

/* Cura d'emergenza dei boss (azione IA con param = -1) */
static void CuraBoss(Lotta *L) {
    L->cureBoss++;
    MSG(L, "%s usa una cura d'emergenza!", L->allenatore ? L->allenatore->nome : "L'avversario");
    char n[48];
    CuraHp(L, LATO_NEMICO, L->lato[LATO_NEMICO].e->stat[STAT_HP] / 2, "%s recupera energia!",
           NomeLato(L, LATO_NEMICO, n, sizeof(n)));
}

static void EseguiAzione(Lotta *L, int lato, const Azione *az) {
    Combattente *k = &L->lato[lato];
    if (L->esito != ESITO_IN_CORSO || Elyon_Esausto(k->e)) return;
    switch (az->tipo) {
    case AZ_MOSSA:
        if (az->param == MOSSA_ESITA) {
            char n[48];
            MSG(L, "%s esita: non è ancora abituato ai comandi!", NomeLato(L, lato, n, sizeof(n)));
        } else UsaMossa(L, lato, az->param);
        break;
    case AZ_CAMBIO:  Cambio(L, lato, az->param); break;
    case AZ_FUGA:    Fuga(L); break;
    case AZ_OGGETTO:
        if (lato == LATO_NEMICO) CuraBoss(L);
        else if (Oggetto_FattoreCore(az->param) > 0.0f) Cattura(L, az->param);
        else UsaOggetto(L, az);
        break;
    }
    L->lato[lato].haAgito = true;
}

/* ------------------------------------------------------------------ IA */

static int MigliorDannoTra(const Lotta *L, const Elyon *att, const Elyon *dif) {
    int best = 0;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) {
        const MoveData *m = GetMove(att->mosse[i]);
        if (!m || att->pp[i] <= 0 || m->categoria == CAT_STATO) continue;
        int dn = DannoTra(L, att, NULL, 0, false, dif, NULL, m, false, 0.95f) * m->precisione / 100;
        if (m->effetto == EFF_MULTICOLPO) dn *= m->param1;
        if (dn > best) best = dn;
    }
    return best;
}

/* Sostituto del nemico: in ordine (SIMPLE/STANDARD) o miglior confronto (SMART/BOSS) */
static int ScegliSostituto(const Lotta *L) {
    LivelloIA ia = L->allenatore ? L->allenatore->ia : AI_SIMPLE;
    int scelto = -1, migliore = -1000000;
    const Elyon *avv = L->lato[LATO_GIOCATORE].e;
    for (int i = 0; i < L->numNemici; i++) {
        if (Elyon_Esausto(&L->nemici[i]) || i == L->lato[LATO_NEMICO].indice) continue;
        if (ia < AI_SMART) return i;
        int punti = Elyon_Esausto(avv) ? L->nemici[i].livello
                  : MigliorDannoTra(L, &L->nemici[i], avv) - MigliorDannoTra(L, avv, &L->nemici[i]);
        if (punti > migliore) { migliore = punti; scelto = i; }
    }
    return scelto;
}

/* Valore di una mossa di stato per l'IA (0 = inutile ora) */
static int ValoreStato(const Lotta *L, const MoveData *m) {
    const Combattente *a = &L->lato[LATO_NEMICO], *d = &L->lato[LATO_GIOCATORE];
    int base = d->e->stat[STAT_HP] / 6;
    switch (m->effetto) {
    case EFF_STATUS:
        if (m->param1 == STATUS_CONFUSIONE) return d->confusione ? 0 : base;
        return d->e->status ? 0 : base * m->prob / 50;
    case EFF_BOOST_SE:       return a->boost[m->param1] >= 2 ? 0 : base;
    case EFF_BOOST_NEMICO:   return d->boost[m->param1] <= -2 ? 0 : base * 2 / 3;
    case EFF_CURA_PERC:      return a->e->hp * 2 < a->e->stat[STAT_HP] ? a->e->stat[STAT_HP] / 2 : 0;
    case EFF_RADICAMENTO:    return a->radicamento ? 0 : base / 2;
    case EFF_FURIA:          return a->furia >= m->param2 ? 0 : base / 2;
    case EFF_DIMEZZA_PROSSIMO: return a->dimezzaProssimo ? 0 : base / 2;
    case EFF_RIFLESSIONE:    return a->riflessioneTurni ? 0 : base / 2;
    case EFF_CAMPO_TIPO:     return L->campoTurni ? 0 : base / 2;
    case EFF_EQUILIBRIO:     return a->e->hp < d->e->hp ? (d->e->hp - a->e->hp) / 2 : 0;
    case EFF_STORDIMENTO_PROSSIMO: return d->storditoProssimo ? 0 : base / 3;
    case EFF_BLOCCA_CAMBIO:  return d->bloccoCambio ? 0 : base / 4;
    default:                 return 0;
    }
}

/* IA introduttiva (prima sfida di Kael). Kael ha appena ricevuto il suo Elyon: la lotta fa da tutorial.
 * Non bara: danni, precisione e statistiche restano quelli normali; cambia solo cosa sceglie.
 *  1) Mai mosse che infliggono stati o calano le statistiche del giocatore.
 *  2) Mosse super efficaci solo nel INTRO_PROB_FORTE% dei casi, altrimenti una mossa neutra.
 *  3) Handicap equo: a inizio lotta stima quanti turni servono a ciascuno per vincere (miglior colpo
 *     neutro contro HP) e fissa una probabilita' di esitazione tale che Kael abbia bisogno di almeno
 *     INTRO_MARGINE% dei turni del giocatore. Cosi' ogni starter parte alla pari, qualunque sia
 *     l'incontro, senza numeri scritti a mano per ciascuna coppia.
 *  4) Elastico: se Kael e' in vantaggio di HP esita un po' di piu', se e' in svantaggio molto meno.
 *  5) Una volta per lotta rinuncia al colpo che manderebbe KO il giocatore.
 * La sconfitta resta possibile (giocate a caso, mosse di stato inutili, colpi critici). */
#ifndef INTRO_PROB_FORTE
#define INTRO_PROB_FORTE 20
#endif
#ifndef INTRO_MARGINE
#define INTRO_MARGINE 140
#endif
#ifndef INTRO_ELASTICO
#define INTRO_ELASTICO 50          /* punti di esitazione per 100 punti di vantaggio in HP% */
#endif
#define INTRO_ESITA_MAX 75

static int PercHp(const Elyon *e) { return e->hp * 100 / (e->stat[STAT_HP] > 0 ? e->stat[STAT_HP] : 1); }
static int MigliorDannoTra(const Lotta *L, const Elyon *att, const Elyon *dif);

/* Danno atteso per turno con le stesse regole di scelta di SceltaIntro (senza esitazione):
 * media delle mosse "altre" (le mosse di stato innocue contano 0) e delle super efficaci */
static int DannoAttesoIntro(const Lotta *L) {
    const Elyon *k = L->lato[LATO_NEMICO].e;
    int sommaF = 0, nf = 0, sommaA = 0, na = 0;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) {
        const MoveData *m = GetMove(k->mosse[i]);
        if (!m || k->pp[i] <= 0) continue;
        if (m->categoria == CAT_STATO) {
            if (m->effetto != EFF_STATUS && m->effetto != EFF_BOOST_NEMICO) na++;
            continue;
        }
        int d = Lotta_StimaDanno(L, LATO_NEMICO, m->id, 0.95f) * m->precisione / 100;
        if (Lotta_EfficaciaContro(L, LATO_GIOCATORE, m->tipo) > 1.0f) { sommaF += d; nf++; }
        else { sommaA += d; na++; }
    }
    if (nf == 0) return na ? sommaA / na : 0;
    if (na == 0) return sommaF / nf;
    return (sommaA * (100 - INTRO_PROB_FORTE) / na + sommaF * INTRO_PROB_FORTE / nf) / 100;
}

/* Cura per turno data dall'abilita' (es. RADICE_VIVA): riduce il danno netto dell'avversario */
static int CuraTurnoAbilita(const Lotta *L, const Elyon *e) {
    const AbilityData *ab = Abilita(e);
    if (!ab || ab->effetto != AB_CURA_FINE_TURNO || !CondizioneOk(L, e, ab->condizione)) return 0;
    return (int)((float)e->stat[STAT_HP] * ab->moltiplicatore / 100.0f);
}

static int EsitazioneBaseIntro(const Lotta *L) {
    const Elyon *k = L->lato[LATO_NEMICO].e, *g = L->lato[LATO_GIOCATORE].e;
    int dannoG = MigliorDannoTra(L, g, k) - CuraTurnoAbilita(L, k);
    int dannoK = DannoAttesoIntro(L) - CuraTurnoAbilita(L, g);
    if (dannoG < 1) dannoG = 1;
    if (dannoK < 1) dannoK = 1;
    /* turni per vincere, x100 per lavorare con interi */
    int turniG = (k->hp * 100 + dannoG - 1) / dannoG;
    int turniK = (g->hp * 100 + dannoK - 1) / dannoK;
    int obiettivo = turniG * INTRO_MARGINE / 100;           /* turni che Kael deve "spendere" */
    if (turniK >= obiettivo) return 0;
    return Utils_ClampInt(100 - turniK * 100 / obiettivo, 0, INTRO_ESITA_MAX);
}

static Azione SceltaIntro(Lotta *L, const int *validi, int nv) {
    Azione az = { AZ_MOSSA, validi[0], 0, 0 };
    const Combattente *k = &L->lato[LATO_NEMICO], *g = &L->lato[LATO_GIOCATORE];
    if (L->introEsitaBase < 0) L->introEsitaBase = EsitazioneBaseIntro(L);

    int vantaggio = PercHp(k->e) - PercHp(g->e);
    int esita = L->introEsitaBase + vantaggio * INTRO_ELASTICO / 100;
    esita = Utils_ClampInt(esita, 0, INTRO_ESITA_MAX);
    if (Rng_Range(L->rng, 100) < esita) { az.param = MOSSA_ESITA; return az; }

    int forti[MAX_MOSSE_ELYON], nf = 0, altre[MAX_MOSSE_ELYON], na = 0;
    for (int v = 0; v < nv; v++) {
        const MoveData *m = GetMove(k->e->mosse[validi[v]]);
        if (m->categoria == CAT_STATO) {
            if (m->effetto == EFF_STATUS || m->effetto == EFF_BOOST_NEMICO) continue;
            altre[na++] = validi[v];
        } else if (Lotta_EfficaciaContro(L, LATO_GIOCATORE, m->tipo) > 1.0f) forti[nf++] = validi[v];
        else altre[na++] = validi[v];
    }
    if (nf + na == 0) { az.param = validi[Rng_Range(L->rng, nv)]; return az; }
    if (nf > 0 && (na == 0 || Rng_Range(L->rng, 100) < INTRO_PROB_FORTE)) az.param = forti[Rng_Range(L->rng, nf)];
    else az.param = altre[Rng_Range(L->rng, na)];

    const MoveData *m = GetMove(k->e->mosse[az.param]);
    if (!L->graziaIntro && m->categoria != CAT_STATO && Lotta_StimaDanno(L, LATO_NEMICO, m->id, 0.85f) >= g->e->hp) {
        L->graziaIntro = true;
        az.param = MOSSA_ESITA;
    }
    return az;
}

Azione Lotta_SceltaIA(Lotta *L) {
    Azione az = { AZ_MOSSA, MOSSA_DIMENARSI, 0, 0 };
    Combattente *k = &L->lato[LATO_NEMICO], *g = &L->lato[LATO_GIOCATORE];
    int validi[MAX_MOSSE_ELYON], nv = 0;
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) if (GetMove(k->e->mosse[i]) && k->e->pp[i] > 0) validi[nv++] = i;
    if (nv == 0) return az;
    LivelloIA ia = L->allenatore ? L->allenatore->ia : AI_SIMPLE;

    if (ia == AI_INTRO) return SceltaIntro(L, validi, nv);
    if (ia == AI_SIMPLE) { az.param = validi[Rng_Range(L->rng, nv)]; return az; }

    /* BOSS: cure limitate sotto il 30% */
    if (ia == AI_BOSS && L->cureBoss < L->cureBossMax && k->e->hp * 10 < k->e->stat[STAT_HP] * 3) {
        az.tipo = AZ_OGGETTO;
        az.param = -1;
        return az;
    }

    int punteggio[MAX_MOSSE_ELYON] = { 0 };
    int migliore = validi[0];
    for (int v = 0; v < nv; v++) {
        int i = validi[v];
        const MoveData *m = GetMove(k->e->mosse[i]);
        if (m->categoria == CAT_STATO) punteggio[i] = ValoreStato(L, m);
        else punteggio[i] = Lotta_StimaDanno(L, LATO_NEMICO, m->id, 0.95f) * m->precisione / 100;
        if (punteggio[i] > punteggio[migliore]) migliore = i;
    }

    if (ia >= AI_SMART) {
        /* 1) colpo da KO: la mossa piu' precisa che manda KO anche col casuale minimo */
        int ko = -1;
        for (int v = 0; v < nv; v++) {
            int i = validi[v];
            const MoveData *m = GetMove(k->e->mosse[i]);
            if (m->categoria == CAT_STATO || Lotta_StimaDanno(L, LATO_NEMICO, m->id, 0.90f) < g->e->hp) continue;
            if (ko < 0 || m->precisione > GetMove(k->e->mosse[ko])->precisione) ko = i;
        }
        if (ko >= 0) { az.param = ko; return az; }
        /* 2) cambio se il giocatore manda KO l'Elyon attuale e un compagno regge meglio */
        if (L->tipo == LOTTA_ALLENATORE && k->bloccoCambio == 0 && L->turno - L->ultimoCambioIA >= 3 &&
            MigliorDannoTra(L, g->e, k->e) >= k->e->hp) {
            int s = ScegliSostituto(L);
            if (s >= 0 && MigliorDannoTra(L, g->e, &L->nemici[s]) * 2 < L->nemici[s].hp) {
                L->ultimoCambioIA = L->turno;
                az.tipo = AZ_CAMBIO;
                az.param = s;
                return az;
            }
        }
    } else if (Rng_Range(L->rng, 100) < 15) {
        /* STANDARD: un po' di imprevedibilita' */
        az.param = validi[Rng_Range(L->rng, nv)];
        return az;
    }
    az.param = migliore;
    return az;
}

/* ------------------------------------------------------------------ turno */

static int PrioritaAzione(const Lotta *L, int lato, const Azione *az) {
    if (az->tipo != AZ_MOSSA) return 10;      /* cambi, oggetti e fuga prima delle mosse */
    if (az->param == MOSSA_DIMENARSI || az->param == MOSSA_ESITA) return 0;
    const MoveData *m = GetMove(L->lato[lato].e->mosse[az->param]);
    return m ? m->priorita : 0;
}

static void FineTurnoLato(Lotta *L, int lato) {
    Combattente *k = &L->lato[lato];
    if (Elyon_Esausto(k->e) || L->esito != ESITO_IN_CORSO) return;
    char n[48];
    NomeLato(L, lato, n, sizeof(n));
    int max = k->e->stat[STAT_HP];
    switch (k->e->status) {
    case STATUS_VELENO:
        PerdiHp(L, lato, max / 8, "%s soffre per il veleno!", n);
        break;
    case STATUS_VELENO_GRAVE: {
        int c = Utils_ClampInt(k->e->contatoreVelenoGrave, 1, 15);
        PerdiHp(L, lato, max * c / 16, "%s soffre per il veleno!", n);
        k->e->contatoreVelenoGrave = c + 1;
        break;
    }
    case STATUS_BRUCIATURA:
        PerdiHp(L, lato, max / 16, "%s soffre per la bruciatura!", n);
        break;
    default: break;
    }
    if (Elyon_Esausto(k->e) || L->esito != ESITO_IN_CORSO) return;
    if (k->radicamento) CuraHp(L, lato, max / k->radicamento, "%s assorbe energia dalle radici!", n);
    const AbilityData *ab = Abilita(k->e);
    if (ab && ab->effetto == AB_CURA_FINE_TURNO && CondizioneOk(L, k->e, ab->condizione))
        CuraHp(L, lato, (int)((float)max * ab->moltiplicatore / 100.0f), "%s recupera energia (%s).", n, ab->nome);
    if (k->e->oggetto == 32) CuraHp(L, lato, max / 100, "%s recupera energia grazie al SEME VITALE.", n);
    if (k->riflessioneTurni > 0 && --k->riflessioneTurni == 0) MSG(L, "La barriera di %s svanisce.", n);
    if (k->bloccoCambio > 0) k->bloccoCambio--;
}

static void FineTurno(Lotta *L) {
    int primo = Lotta_VelocitaEffettiva(L, LATO_GIOCATORE) >= Lotta_VelocitaEffettiva(L, LATO_NEMICO) ? 0 : 1;
    FineTurnoLato(L, primo);
    FineTurnoLato(L, 1 - primo);

    if (L->campoTurni > 0 && --L->campoTurni == 0) {
        MSG(L, "L'energia del campo si dissolve.");
        L->campoTipo = TYPE_NONE;
    }
    if (L->turniMeteo > 0 && --L->turniMeteo == 0) {
        L->meteo = METEO_NESSUNO;
        Accoda(L, EV_METEO, -1, METEO_NESSUNO, 0, 0, "Il tempo torna sereno.");
    }
    /* abilita' di campo (Aurion): ogni N turni cambia il meteo */
    for (int lato = 0; lato < 2; lato++) {
        const Elyon *e = L->lato[lato].e;
        const AbilityData *ab = Elyon_Esausto(e) ? NULL : Abilita(e);
        if (!ab || ab->effetto != AB_EFFETTO_CAMPO || ab->param <= 0 || L->turno % ab->param != 0) continue;
        Meteo nuovo = (Meteo)(1 + Rng_Range(L->rng, NUM_METEO - 1));
        if (nuovo == L->meteo) nuovo = (Meteo)(1 + (nuovo % (NUM_METEO - 1)));
        L->meteo = nuovo;
        L->turniMeteo = 5;
        Accoda(L, EV_METEO, -1, nuovo, 0, 0, "%s altera il campo! Meteo: %s.", ab->nome, Meteo_Nome(nuovo));
    }
}

/* Dopo le azioni: sostituzioni, vittoria o sconfitta */
static void RisolviFine(Lotta *L) {
    if (L->esito != ESITO_IN_CORSO) { Termina(L, L->esito); return; }
    bool giocatoreKo = Elyon_Esausto(L->lato[LATO_GIOCATORE].e);
    bool nemicoKo = Elyon_Esausto(L->lato[LATO_NEMICO].e);
    if (giocatoreKo && Collezione_SquadraViva(L->coll) == 0) { Termina(L, ESITO_SCONFITTA); return; }
    if (nemicoKo) {
        if (NemiciVivi(L) == 0) { Termina(L, ESITO_VITTORIA); return; }
        int s = ScegliSostituto(L);
        Entra(L, LATO_NEMICO, s, true);
        memset(L->partecipanti, 0, sizeof(L->partecipanti));
        if (!giocatoreKo) L->partecipanti[L->lato[LATO_GIOCATORE].indice] = true;
    }
    if (giocatoreKo) {
        L->attesaCambio = true;
        Accoda(L, EV_RICHIESTA_CAMBIO, LATO_GIOCATORE, 0, 0, 0, "Chi mandi in campo?");
    }
}

bool Lotta_Turno(Lotta *L, Azione az) {
    L->numEventi = 0;
    char motivo[TESTO_EVENTO];
    if (!Lotta_AzioneValida(L, az, motivo, sizeof(motivo))) {
        MSG(L, "%s", motivo);
        return false;
    }
    Azione ia = Lotta_SceltaIA(L);
    L->turno++;
    for (int i = 0; i < 2; i++) {
        Combattente *k = &L->lato[i];
        k->haAgito = false;
        k->colpitoNelTurno = false;
        k->stordito = k->storditoProssimo;
        k->storditoProssimo = false;
    }
    Azione azioni[2] = { az, ia };
    int pg = PrioritaAzione(L, LATO_GIOCATORE, &azioni[0]), pn = PrioritaAzione(L, LATO_NEMICO, &azioni[1]);
    int primo;
    if (pg != pn) primo = pg > pn ? LATO_GIOCATORE : LATO_NEMICO;
    else {
        int vg = Lotta_VelocitaEffettiva(L, LATO_GIOCATORE), vn = Lotta_VelocitaEffettiva(L, LATO_NEMICO);
        primo = vg != vn ? (vg > vn ? LATO_GIOCATORE : LATO_NEMICO) : Rng_Range(L->rng, 2);
    }
    /* un Elyon KO non agisce; le sostituzioni avvengono solo a fine turno */
    EseguiAzione(L, primo, &azioni[primo]);
    EseguiAzione(L, 1 - primo, &azioni[1 - primo]);
    if (L->esito == ESITO_IN_CORSO) FineTurno(L);
    RisolviFine(L);
    return true;
}

bool Lotta_CambioForzato(Lotta *L, int indice) {
    L->numEventi = 0;
    if (!L->attesaCambio || indice < 0 || indice >= MAX_SQUADRA || Elyon_Esausto(&L->coll->squadra[indice])) {
        MSG(L, "Scegli un Elyon in grado di lottare.");
        return false;
    }
    L->attesaCambio = false;
    Entra(L, LATO_GIOCATORE, indice, true);
    return true;
}

/* ------------------------------------------------------------------ incontri */

bool Lotta_GeneraIncontro(const HabitatData *h, Rng *rng, int *specie, int *livello) {
    if (!h || h->numVoci <= 0) return false;
    int totale = 0;
    for (int i = 0; i < h->numVoci; i++) totale += h->voci[i].percentuale;
    if (totale <= 0) return false;
    int r = Rng_Range(rng, totale), acc = 0;
    *specie = h->voci[h->numVoci - 1].specie;
    for (int i = 0; i < h->numVoci; i++) {
        acc += h->voci[i].percentuale;
        if (r < acc) { *specie = h->voci[i].specie; break; }
    }
    *livello = h->livelloMin + Rng_Range(rng, h->livelloMax - h->livelloMin + 1);
    return true;
}
