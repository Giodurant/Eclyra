/* battle_ui.c - Vedi battle_ui.h */
#include "battle_ui.h"
#include "graphics.h"
#include "item_use.h"
#include "ui_common.h"
#include "data/items_data.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define EV_UI_EVOLUZIONE ((TipoEvento)(EV_FINE + 1))   /* evento interno della UI: a = indice squadra */

enum { ANIM_NESSUNA = 0, ANIM_ENTRATA, ANIM_ATTACCO, ANIM_COLPITO, ANIM_KO, ANIM_RITIRO, ANIM_CATTURATO, ANIM_RIAPPARE };
static const int DURATA_ANIM[] = { 0, 22, 14, 18, 22, 16, 16, 12 };

/* posizioni (angolo in alto a sinistra degli sprite 64x64) */
#define NEMICO_X 204
#define NEMICO_Y 16
#define MIO_X 34
#define MIO_Y 54
#define SCALA_RETRO 1.25f     /* l'Elyon del giocatore e' piu' vicino alla camera */
#define MSG_Y 134

/* ------------------------------------------------------------------ coda */

static void CaricaCoda(BattleUI *u) {
    const Lotta *L = &u->lotta;
    u->numCoda = L->numEventi < BUI_MAX_CODA ? L->numEventi : BUI_MAX_CODA;
    memcpy(u->coda, L->eventi, sizeof(EventoLotta) * (size_t)u->numCoda);
    u->pos = 0;
    u->eventoIniziato = false;
    u->stato = BUI_EVENTI;
}

static void Accoda(BattleUI *u, TipoEvento t, int a, int b, const char *testo) {
    if (u->numCoda >= BUI_MAX_CODA) return;
    EventoLotta *e = &u->coda[u->numCoda++];
    memset(e, 0, sizeof(*e));
    e->tipo = t;
    e->lato = LATO_GIOCATORE;
    e->a = a;
    e->b = b;
    if (testo) snprintf(e->testo, sizeof(e->testo), "%s", testo);
}

/* Inserisce un messaggio subito dopo l'evento corrente */
static void InserisciMessaggio(BattleUI *u, const char *testo) {
    if (u->numCoda >= BUI_MAX_CODA) return;
    memmove(&u->coda[u->pos + 2], &u->coda[u->pos + 1], sizeof(EventoLotta) * (size_t)(u->numCoda - u->pos - 1));
    u->numCoda++;
    EventoLotta *e = &u->coda[u->pos + 1];
    memset(e, 0, sizeof(*e));
    e->tipo = EV_MESSAGGIO;
    e->lato = -1;
    snprintf(e->testo, sizeof(e->testo), "%s", testo);
}

static void Testo(BattleUI *u, const char *t) {
    snprintf(u->testo, sizeof(u->testo), "%s", t ? t : "");
    u->caratteri = 0;
    u->attesaTesto = 0;
}

static bool TestoCompleto(const BattleUI *u) { return (int)u->caratteri >= (int)strlen(u->testo); }

/* Avanza il testo; true quando e' stato letto (conferma o attesa automatica) */
static bool AggiornaTesto(BattleUI *u, const Input *in, bool attendiConferma) {
    if (!u->testo[0]) return true;
    if (!TestoCompleto(u)) {
        u->caratteri += u->caratteriPerPasso;
        if (in->conferma || in->indietro) u->caratteri = (float)strlen(u->testo);
        return false;
    }
    u->attesaTesto++;
    if (in->conferma || in->indietro) return true;
    return !attendiConferma && u->attesaTesto > 70;
}

/* ------------------------------------------------------------------ avvio */

static void ImpostaLato(BattleUI *u, int lato, int indice, int hp, int max, int livello) {
    LatoUI *l = &u->lato[lato];
    const Elyon *e = lato == LATO_GIOCATORE ? &u->lotta.coll->squadra[indice] : &u->lotta.nemici[indice];
    l->visibile = true;
    l->indice = indice;
    l->specie = e->specie;
    l->livello = livello;
    l->hp = (float)hp;
    l->hpTarget = hp;
    l->hpMax = max;
    l->status = e->status;
    snprintf(l->nome, sizeof(l->nome), "%s", Elyon_Nome(e));
    l->anim = ANIM_ENTRATA;
    l->animTimer = 0;
    if (lato == LATO_GIOCATORE) {
        u->xpMostrato = (float)e->xp;
        u->xpTarget = e->xp;
    }
}

static void Inizia(BattleUI *u, Partita *p) {
    u->partita = p;
    u->stato = BUI_TRANSIZIONE;
    u->timer = 0;
    u->caratteriPerPasso = 0.6f;
    Lotta_Inizia(&u->lotta);
    u->meteo = u->lotta.meteo;
    /* la coda viene caricata alla fine della transizione */
}

bool BattleUI_AvviaSelvatica(BattleUI *u, Partita *p, Rng *rng, int specie, int livello) {
    memset(u, 0, sizeof(*u));
    if (!Lotta_InitSelvatica(&u->lotta, &p->collezione, &p->inventario, rng, specie, livello)) return false;
    Inizia(u, p);
    return true;
}

bool BattleUI_AvviaAllenatore(BattleUI *u, Partita *p, Rng *rng, const TrainerData *t) {
    memset(u, 0, sizeof(*u));
    if (!Lotta_InitAllenatore(&u->lotta, &p->collezione, &p->inventario, rng, t)) return false;
    Inizia(u, p);
    return true;
}

bool BattleUI_Finita(const BattleUI *u) { return u->finita; }

/* ------------------------------------------------------------------ eventi */

static void IniziaEvento(BattleUI *u, const EventoLotta *ev) {
    u->timer = 0;
    Testo(u, ev->testo);
    int lato = ev->lato;
    LatoUI *l = (lato == 0 || lato == 1) ? &u->lato[lato] : NULL;
    switch ((int)ev->tipo) {
    case EV_ENTRATA:
        ImpostaLato(u, lato, ev->a, ev->b, ev->c, ev->d);
        break;
    case EV_RITIRO:
        if (l) { l->anim = ANIM_RITIRO; l->animTimer = 0; }
        break;
    case EV_ATTACCO:
        if (l) { l->anim = ANIM_ATTACCO; l->animTimer = 0; }
        {
            const MoveData *m = GetMove(ev->a);
            u->effettoTipo = m ? m->tipo : TYPE_NONE;
            u->effettoLato = (m && m->categoria == CAT_STATO && (m->flag & MOSSA_PRECISIONE_AUTOMATICA)) ? lato : 1 - lato;
            u->effettoTimer = -8;   /* parte dopo l'affondo */
        }
        break;
    case EV_DANNO:
        if (l) { l->hpTarget = ev->a; l->anim = ANIM_COLPITO; l->animTimer = 0; }
        break;
    case EV_CURA:
        if (l) l->hpTarget = ev->a;
        break;
    case EV_STATUS:
        if (l && !(ev->a == STATUS_NESSUNO && ev->b == 1) && ev->a != STATUS_CONFUSIONE) l->status = (StatusId)ev->a;
        break;
    case EV_BOOST:
        u->boostTimer = 24;
        u->boostLato = lato;
        u->boostDir = ev->b > 0 ? -1 : 1;
        break;
    case EV_KO:
        if (l) { l->anim = ANIM_KO; l->animTimer = 0; l->hpTarget = 0; }
        break;
    case EV_LANCIO_CORE:
        u->coreVisibile = true;
        u->coreAnim = 1;
        u->coreTimer = 0;
        break;
    case EV_SCOSSA:
        u->coreAnim = 2;
        u->coreTimer = 0;
        break;
    case EV_CATTURA_FALLITA:
        u->coreVisibile = false;
        u->lato[LATO_NEMICO].visibile = true;
        u->lato[LATO_NEMICO].anim = ANIM_RIAPPARE;
        u->lato[LATO_NEMICO].animTimer = 0;
        break;
    case EV_CATTURA_OK:
        u->coreAnim = 3;
        u->coreTimer = 0;
        break;
    case EV_XP:
        if (ev->a == u->lato[0].indice) u->xpTarget = ev->b;
        break;
    case EV_LIVELLO:
        if (ev->a == u->lato[0].indice) {
            LatoUI *g = &u->lato[0];
            g->livello = ev->b;
            g->hp = (float)ev->c;
            g->hpTarget = ev->c;
            g->hpMax = ev->d;
            const SpeciesData *s = GetSpecies(g->specie);
            float base = (float)Elyon_XpPerLivello(s, ev->b);
            if (u->xpMostrato < base) u->xpMostrato = base;
        }
        break;
    case EV_METEO:
        u->meteo = (Meteo)ev->a;
        break;
    case EV_FINE:
        u->esito = (EsitoLotta)ev->a;
        /* evoluzioni a fine lotta (mai dopo una sconfitta) */
        if (u->esito != ESITO_SCONFITTA)
            for (int i = 0; i < MAX_SQUADRA; i++) {
                const Elyon *e = &u->lotta.coll->squadra[i];
                if (u->lotta.livelloSalito[i] && !Elyon_Esausto(e) && Elyon_PuoEvolvere(e)) Accoda(u, EV_UI_EVOLUZIONE, i, 0, NULL);
            }
        break;
    case EV_UI_EVOLUZIONE: {
        const Elyon *e = &u->lotta.coll->squadra[ev->a];
        u->evoIndice = ev->a;
        u->evoDa = e->specie;
        u->evoA = Elyon_PuoEvolvere(e);
        u->evoFase = 0;
        u->evoTimer = 0;
        snprintf(u->evoNome, sizeof(u->evoNome), "%s", Elyon_Nome(e));
        char t[96];
        snprintf(t, sizeof(t), "Cosa? %s si sta evolvendo!", u->evoNome);
        Testo(u, t);
        u->stato = BUI_EVOLUZIONE;
        break;
    }
    default:
        break;
    }
}

static bool AnimazioniFinite(const BattleUI *u) {
    for (int i = 0; i < 2; i++) {
        const LatoUI *l = &u->lato[i];
        if (l->anim && l->animTimer < DURATA_ANIM[l->anim]) return false;
        if (fabsf(l->hp - (float)l->hpTarget) > 0.01f) return false;
    }
    if (u->coreAnim == 1 && u->coreTimer < 30) return false;
    if (u->coreAnim == 2 && u->coreTimer < 34) return false;
    if (u->effettoTimer < 14) return false;
    return true;
}

static void AggiornaAnimazioni(BattleUI *u) {
    for (int i = 0; i < 2; i++) {
        LatoUI *l = &u->lato[i];
        if (l->anim) {
            l->animTimer++;
            if (l->animTimer >= DURATA_ANIM[l->anim]) {
                if (l->anim == ANIM_KO || l->anim == ANIM_RITIRO || l->anim == ANIM_CATTURATO) l->visibile = false;
                if (l->anim != ANIM_KO && l->anim != ANIM_RITIRO && l->anim != ANIM_CATTURATO) l->anim = ANIM_NESSUNA;
            }
        }
        /* barra HP: velocita' proporzionale ai PV massimi (circa un secondo per l'intera barra) */
        float passo = (float)(l->hpMax > 0 ? l->hpMax : 1) / 48.0f;
        if (passo < 0.5f) passo = 0.5f;
        if (l->hp > (float)l->hpTarget) l->hp = fmaxf((float)l->hpTarget, l->hp - passo);
        else if (l->hp < (float)l->hpTarget) l->hp = fminf((float)l->hpTarget, l->hp + passo);
    }
    /* XP fino alla soglia del livello mostrato */
    const SpeciesData *s = GetSpecies(u->lato[0].specie);
    if (s && u->xpMostrato < (float)u->xpTarget) {
        float soglia = (float)Elyon_XpPerLivello(s, u->lato[0].livello + 1);
        float limite = fminf((float)u->xpTarget, u->lato[0].livello >= LIVELLO_MAX ? (float)u->xpTarget : soglia);
        float passo = fmaxf(1.0f, (soglia - (float)Elyon_XpPerLivello(s, u->lato[0].livello)) / 40.0f);
        u->xpMostrato = fminf(limite, u->xpMostrato + passo);
    }
    if (u->effettoTimer < 14) u->effettoTimer++;
    if (u->boostTimer > 0) u->boostTimer--;
    if (u->coreAnim) {
        u->coreTimer++;
        if (u->coreAnim == 1 && u->coreTimer == 30) {
            u->lato[LATO_NEMICO].anim = ANIM_CATTURATO;
            u->lato[LATO_NEMICO].animTimer = 0;
        }
    }
}

static void DopoEventi(BattleUI *u) {
    const Lotta *L = &u->lotta;
    if (Lotta_Finita(L)) { u->stato = BUI_FINE; u->finita = true; return; }
    if (L->attesaCambio) {
        u->stato = BUI_SQUADRA;
        u->modoSquadra = SQ_FORZATO;
        u->cursoreSquadra = 0;
        return;
    }
    u->stato = BUI_MENU;
    char t[96];
    snprintf(t, sizeof(t), "Cosa deve fare %s?", Elyon_Nome(L->lato[LATO_GIOCATORE].e));
    Testo(u, t);
    u->caratteri = (float)strlen(u->testo);
}

static void AggiornaEventi(BattleUI *u, const Input *in) {
    if (u->pos >= u->numCoda) { DopoEventi(u); return; }
    EventoLotta *ev = &u->coda[u->pos];
    if (!u->eventoIniziato) {
        u->eventoIniziato = true;
        IniziaEvento(u, ev);
        if (u->stato != BUI_EVENTI) return;      /* l'evento ha aperto un'altra schermata */
    }
    u->timer++;
    bool anim = AnimazioniFinite(u);
    /* le scosse e le richieste aspettano la conferma; gli altri messaggi avanzano anche da soli */
    bool conferma = ev->tipo == EV_RICHIESTA_MOSSA || ev->tipo == EV_CATTURA_OK || ev->tipo == EV_LIVELLO ||
                    ev->tipo == EV_FINE;
    bool letto = anim ? AggiornaTesto(u, in, conferma) : false;
    if (!anim && u->testo[0] && !TestoCompleto(u)) u->caratteri += u->caratteriPerPasso;
    if (!anim || !letto) return;
    if (ev->tipo == EV_XP || ev->tipo == EV_LIVELLO) {
        /* aspetta che la barra XP raggiunga la soglia o il valore finale */
        const SpeciesData *s = GetSpecies(u->lato[0].specie);
        if (ev->a == u->lato[0].indice && s && u->xpMostrato < (float)u->xpTarget &&
            u->xpMostrato < (float)Elyon_XpPerLivello(s, u->lato[0].livello + 1))
            return;
    }
    if (ev->tipo == EV_RICHIESTA_MOSSA) {
        u->imparaIndice = ev->a;
        u->imparaMossa = ev->b;
        u->stato = BUI_IMPARA_SINO;
        u->cursoreSiNo = 0;
        char t[128];
        const MoveData *m = GetMove(ev->b);
        snprintf(t, sizeof(t), "Vuoi che %s dimentichi una mossa per imparare %s?",
                 Elyon_Nome(&u->lotta.coll->squadra[ev->a]), m ? m->nome : "?");
        Testo(u, t);
        u->caratteri = (float)strlen(u->testo);
        return;
    }
    u->pos++;
    u->eventoIniziato = false;
}

/* ------------------------------------------------------------------ azioni */

static void EseguiAzione(BattleUI *u, Azione az) {
    char motivo[TESTO_EVENTO];
    if (!Lotta_AzioneValida(&u->lotta, az, motivo, sizeof(motivo))) {
        snprintf(u->avviso, sizeof(u->avviso), "%s", motivo);
        u->statoPrimaAvviso = u->stato;
        u->stato = BUI_AVVISO;
        Testo(u, motivo);
        return;
    }
    Lotta_Turno(&u->lotta, az);
    CaricaCoda(u);
}

static int OggettiLotta(const BattleUI *u, int *lista) {
    int n = 0;
    for (int id = 1; id < ITEM_ID_MAX; id++) {
        const ItemData *it = GetItem(id);
        if (!it || Inventario_Quantita(u->lotta.inv, id) <= 0) continue;
        if (it->categoria == CAT_CURA || it->categoria == CAT_CORE) lista[n++] = id;
    }
    return n;
}

static void AggiornaMenu(BattleUI *u, const Input *in) {
    int c = u->cursoreMenu;
    if (in->sinistraP || in->destraP) c ^= 1;
    if (in->suP || in->giuP) c ^= 2;
    u->cursoreMenu = c;
    if (!in->conferma) return;
    switch (c) {
    case 0:
        if (Lotta_DeveDimenarsi(&u->lotta)) EseguiAzione(u, (Azione){ AZ_MOSSA, MOSSA_DIMENARSI, 0, 0 });
        else u->stato = BUI_MOSSE;
        break;
    case 1:
        u->stato = BUI_SQUADRA;
        u->modoSquadra = SQ_CAMBIO;
        u->cursoreSquadra = u->lotta.lato[LATO_GIOCATORE].indice;
        break;
    case 2:
        u->stato = BUI_ZAINO;
        u->cursoreZaino = 0;
        break;
    case 3:
        EseguiAzione(u, (Azione){ AZ_FUGA, 0, 0, 0 });
        break;
    }
}

static void AggiornaMosse(BattleUI *u, const Input *in) {
    const Elyon *e = u->lotta.lato[LATO_GIOCATORE].e;
    int c = u->cursoreMossa;
    if (in->sinistraP || in->destraP) c ^= 1;
    if (in->suP || in->giuP) c ^= 2;
    if (e->mosse[c]) u->cursoreMossa = c;
    if (in->indietro) { u->stato = BUI_MENU; return; }
    if (in->conferma) EseguiAzione(u, (Azione){ AZ_MOSSA, u->cursoreMossa, 0, 0 });
}

static void AggiornaSquadra(BattleUI *u, const Input *in) {
    int n = Collezione_NumSquadra(u->lotta.coll);
    if (n <= 0) return;
    if (in->suP) u->cursoreSquadra = (u->cursoreSquadra + n - 1) % n;
    if (in->giuP) u->cursoreSquadra = (u->cursoreSquadra + 1) % n;
    if (in->indietro && u->modoSquadra != SQ_FORZATO) {
        u->stato = u->modoSquadra == SQ_OGGETTO ? BUI_ZAINO : BUI_MENU;
        return;
    }
    if (!in->conferma) return;
    int i = u->cursoreSquadra;
    if (u->modoSquadra == SQ_FORZATO) {
        if (Lotta_CambioForzato(&u->lotta, i)) CaricaCoda(u);
        else {
            u->statoPrimaAvviso = BUI_SQUADRA;
            u->stato = BUI_AVVISO;
            Testo(u, Elyon_Esausto(&u->lotta.coll->squadra[i]) ? "Non è in grado di lottare!" : "Scegli un altro Elyon.");
        }
    } else if (u->modoSquadra == SQ_CAMBIO) {
        EseguiAzione(u, (Azione){ AZ_CAMBIO, i, 0, 0 });
    } else {
        u->bersaglioOggetto = i;
        if (Oggetto_RichiedeMossa(u->oggettoScelto)) {
            u->stato = BUI_ELISIR;
            u->cursoreElisir = 0;
        } else {
            EseguiAzione(u, (Azione){ AZ_OGGETTO, u->oggettoScelto, i, 0 });
        }
    }
}

static void AggiornaZaino(BattleUI *u, const Input *in) {
    int lista[ITEM_ID_MAX];
    int n = OggettiLotta(u, lista);
    if (in->indietro) { u->stato = BUI_MENU; return; }
    if (n == 0) return;
    if (u->cursoreZaino >= n) u->cursoreZaino = n - 1;
    if (in->suP) u->cursoreZaino = (u->cursoreZaino + n - 1) % n;
    if (in->giuP) u->cursoreZaino = (u->cursoreZaino + 1) % n;
    if (!in->conferma) return;
    int id = lista[u->cursoreZaino];
    u->oggettoScelto = id;
    if (Oggetto_FattoreCore(id) > 0.0f) {
        EseguiAzione(u, (Azione){ AZ_OGGETTO, id, 0, 0 });
    } else {
        u->stato = BUI_SQUADRA;
        u->modoSquadra = SQ_OGGETTO;
        u->cursoreSquadra = u->lotta.lato[LATO_GIOCATORE].indice;
    }
}

static void AggiornaElisir(BattleUI *u, const Input *in) {
    const Elyon *e = &u->lotta.coll->squadra[u->bersaglioOggetto];
    int n = Elyon_NumMosse(e);
    if (in->indietro) { u->stato = BUI_SQUADRA; return; }
    if (n <= 0) return;
    if (in->suP) u->cursoreElisir = (u->cursoreElisir + n - 1) % n;
    if (in->giuP) u->cursoreElisir = (u->cursoreElisir + 1) % n;
    if (in->conferma) EseguiAzione(u, (Azione){ AZ_OGGETTO, u->oggettoScelto, u->bersaglioOggetto, u->cursoreElisir });
}

static void FineImpara(BattleUI *u, const char *messaggio) {
    InserisciMessaggio(u, messaggio);
    u->stato = BUI_EVENTI;
    u->pos++;
    u->eventoIniziato = false;
}

static void AggiornaImparaSiNo(BattleUI *u, const Input *in) {
    if (in->suP || in->giuP) u->cursoreSiNo ^= 1;
    Elyon *e = &u->lotta.coll->squadra[u->imparaIndice];
    const MoveData *m = GetMove(u->imparaMossa);
    char t[128];
    if (in->conferma && u->cursoreSiNo == 0) {
        u->stato = BUI_IMPARA_SCELTA;
        u->cursoreImpara = 0;
        snprintf(t, sizeof(t), "Quale mossa deve dimenticare %s?", Elyon_Nome(e));
        Testo(u, t);
        u->caratteri = (float)strlen(u->testo);
    } else if (in->indietro || (in->conferma && u->cursoreSiNo == 1)) {
        snprintf(t, sizeof(t), "%s non ha imparato %s.", Elyon_Nome(e), m ? m->nome : "?");
        FineImpara(u, t);
    }
}

static void AggiornaImparaScelta(BattleUI *u, const Input *in) {
    if (in->suP) u->cursoreImpara = (u->cursoreImpara + 4) % 5;
    if (in->giuP) u->cursoreImpara = (u->cursoreImpara + 1) % 5;
    Elyon *e = &u->lotta.coll->squadra[u->imparaIndice];
    const MoveData *m = GetMove(u->imparaMossa);
    char t[160];
    if (in->indietro || (in->conferma && u->cursoreImpara == 4)) {
        snprintf(t, sizeof(t), "%s non ha imparato %s.", Elyon_Nome(e), m ? m->nome : "?");
        FineImpara(u, t);
    } else if (in->conferma) {
        const MoveData *vecchia = GetMove(e->mosse[u->cursoreImpara]);
        snprintf(t, sizeof(t), "1, 2 e... puff! %s ha dimenticato %s e ha imparato %s!", Elyon_Nome(e),
                 vecchia ? vecchia->nome : "?", m ? m->nome : "?");
        Elyon_SostituisciMossa(e, u->cursoreImpara, u->imparaMossa);
        FineImpara(u, t);
    }
}

static void AggiornaEvoluzione(BattleUI *u, const Input *in) {
    Elyon *e = &u->lotta.coll->squadra[u->evoIndice];
    char t[128];
    switch (u->evoFase) {
    case 0:   /* annuncio */
        if (AggiornaTesto(u, in, true)) { u->evoFase = 1; u->evoTimer = 0; Testo(u, ""); }
        break;
    case 1:   /* animazione: X la interrompe */
        u->evoTimer++;
        if (in->indietro) {
            u->evoFase = 3;
            snprintf(t, sizeof(t), "%s ha smesso di evolversi.", u->evoNome);
            Testo(u, t);
        } else if (u->evoTimer >= 200) {
            Elyon_Evolvi(e, u->evoA);
            u->evoFase = 2;
            snprintf(t, sizeof(t), "Evviva! %s si è evoluto in %s!", u->evoNome, GetSpecies(u->evoA)->nome);
            Testo(u, t);
            /* mosse della nuova specie al livello attuale */
            int nuove[4];
            int n = Elyon_MosseAlLivello(e, e->livello, nuove, 4);
            int dove = u->pos + 1;
            for (int k = n - 1; k >= 0; k--) {
                bool conosciuta = false;
                for (int s = 0; s < MAX_MOSSE_ELYON; s++) if (e->mosse[s] == nuove[k]) conosciuta = true;
                if (conosciuta || u->numCoda >= BUI_MAX_CODA) continue;
                memmove(&u->coda[dove + 1], &u->coda[dove], sizeof(EventoLotta) * (size_t)(u->numCoda - dove));
                u->numCoda++;
                EventoLotta *ev = &u->coda[dove];
                memset(ev, 0, sizeof(*ev));
                ev->a = u->evoIndice;
                ev->b = nuove[k];
                const MoveData *m = GetMove(nuove[k]);
                if (Elyon_ImparaMossa(e, nuove[k])) {
                    ev->tipo = EV_NUOVA_MOSSA;
                    snprintf(ev->testo, sizeof(ev->testo), "%s impara %s!", Elyon_Nome(e), m->nome);
                } else {
                    ev->tipo = EV_RICHIESTA_MOSSA;
                    snprintf(ev->testo, sizeof(ev->testo), "%s vuole imparare %s...", Elyon_Nome(e), m->nome);
                }
            }
        }
        break;
    default:  /* 2 = evoluto, 3 = annullata */
        if (AggiornaTesto(u, in, true)) {
            u->stato = BUI_EVENTI;
            u->pos++;
            u->eventoIniziato = false;
        }
        break;
    }
}

void BattleUI_Aggiorna(BattleUI *u, const Input *in, float caratteriPerPasso) {
    u->frame++;
    u->caratteriPerPasso = caratteriPerPasso;
    AggiornaAnimazioni(u);
    switch (u->stato) {
    case BUI_TRANSIZIONE:
        if (++u->timer >= 48) CaricaCoda(u);
        break;
    case BUI_EVENTI:        AggiornaEventi(u, in); break;
    case BUI_MENU:          AggiornaMenu(u, in); break;
    case BUI_MOSSE:         AggiornaMosse(u, in); break;
    case BUI_SQUADRA:       AggiornaSquadra(u, in); break;
    case BUI_ZAINO:         AggiornaZaino(u, in); break;
    case BUI_ELISIR:        AggiornaElisir(u, in); break;
    case BUI_IMPARA_SINO:   AggiornaImparaSiNo(u, in); break;
    case BUI_IMPARA_SCELTA: AggiornaImparaScelta(u, in); break;
    case BUI_EVOLUZIONE:    AggiornaEvoluzione(u, in); break;
    case BUI_AVVISO:
        if (AggiornaTesto(u, in, true)) {
            StatoBattleUI prima = u->statoPrimaAvviso;
            DopoEventi(u);                       /* ripristina il testo del menu */
            if (u->stato == BUI_MENU && prima != BUI_MENU) u->stato = prima;
        }
        break;
    case BUI_FINE:
        u->finita = true;
        break;
    }
}

/* ------------------------------------------------------------------ disegno */

static void DisegnaSfondo(const BattleUI *u) {
    static const Color cielo[4] = { { 120, 184, 232, 255 }, { 144, 200, 240, 255 }, { 168, 216, 244, 255 }, { 196, 228, 248, 255 } };
    Color tinta = WHITE;
    if (u->meteo == METEO_PIOGGIA || u->meteo == METEO_TEMPESTA) tinta = (Color){ 150, 160, 180, 255 };
    for (int i = 0; i < 4; i++) {
        Color c = cielo[i];
        c.r = (unsigned char)(c.r * tinta.r / 255); c.g = (unsigned char)(c.g * tinta.g / 255); c.b = (unsigned char)(c.b * tinta.b / 255);
        DrawRectangle(0, i * 18, LOGICAL_W, 18, c);
    }
    DrawRectangle(0, 72, LOGICAL_W, MSG_Y - 72, (Color){ 168, 216, 128, 255 });
    for (int x = 0; x < LOGICAL_W; x += 8) DrawRectangle(x, 72 + (x * 7) % 5, 2, 1, (Color){ 136, 192, 104, 255 });
    /* colline lontane */
    for (int x = 0; x < LOGICAL_W; x++) {
        int h = 64 + (int)(5 * sinf(x * 0.05f) + 3 * sinf(x * 0.13f));
        DrawLine(x, h, x, 72, (Color){ 120, 176, 120, 255 });
    }
    /* pedane */
    DrawEllipse(NEMICO_X + 32, NEMICO_Y + 62, 46, 10, (Color){ 128, 184, 96, 255 });
    DrawEllipse(NEMICO_X + 32, NEMICO_Y + 62, 40, 7, (Color){ 152, 204, 112, 255 });
    DrawEllipse(MIO_X + 40, MIO_Y + 78, 58, 12, (Color){ 128, 184, 96, 255 });
    DrawEllipse(MIO_X + 40, MIO_Y + 78, 50, 8, (Color){ 152, 204, 112, 255 });
    /* meteo */
    for (int i = 0; i < 40 && u->meteo != METEO_NESSUNO && u->meteo != METEO_SOLE; i++) {
        int x = (i * 53 + u->frame * (u->meteo == METEO_NEVE ? 1 : 4)) % LOGICAL_W;
        int y = (i * 37 + u->frame * (u->meteo == METEO_NEVE ? 1 : 6)) % MSG_Y;
        if (u->meteo == METEO_PIOGGIA || u->meteo == METEO_TEMPESTA) DrawLine(x, y, x - 2, y + 5, (Color){ 200, 220, 255, 200 });
        else if (u->meteo == METEO_NEVE) DrawRectangle(x, y, 2, 2, WHITE);
        else DrawRectangle(x, y, 6, 2, (Color){ 240, 240, 240, 90 });
    }
    if (u->meteo == METEO_SOLE) DrawCircle(30, 18, 10, (Color){ 255, 240, 160, 200 });
}

static void DisegnaSpriteLato(const BattleUI *u, int lato) {
    const LatoUI *l = &u->lato[lato];
    if (!l->visibile) return;
    float x = lato ? NEMICO_X : MIO_X, y = lato ? NEMICO_Y : MIO_Y;
    float t = l->anim ? (float)l->animTimer / (float)DURATA_ANIM[l->anim] : 1.0f;
    float scala = 1.0f;
    Color tinta = WHITE;
    switch (l->anim) {
    case ANIM_ENTRATA: x += (lato ? 1 : -1) * (1.0f - t) * 150.0f; break;
    case ANIM_ATTACCO: x += (lato ? -1 : 1) * (t < 0.5f ? t : 1.0f - t) * 16.0f; y += (lato ? 1 : -1) * (t < 0.5f ? t : 1.0f - t) * 6.0f; break;
    case ANIM_COLPITO: if ((l->animTimer / 3) % 2 == 0) return; break;
    case ANIM_KO: y += t * 40.0f; tinta.a = (unsigned char)(255 * (1.0f - t)); break;
    case ANIM_RITIRO: x += (lato ? 1 : -1) * t * 150.0f; break;
    case ANIM_CATTURATO: scala = 1.0f - t; x += 32 * t; y += 48 * t; tinta = (Color){ 255, 120, 120, 255 }; break;
    case ANIM_RIAPPARE: scala = t; x += 32 * (1 - t); y += 48 * (1 - t); break;
    default: break;
    }
    if (scala <= 0.02f) return;
    if (lato == LATO_GIOCATORE) scala *= SCALA_RETRO;
    UI_SpriteElyon(l->specie, (int)x, (int)y, lato == LATO_GIOCATORE, scala, tinta);
}

static void DisegnaRiquadro(const BattleUI *u, int lato) {
    const LatoUI *l = &u->lato[lato];
    if (!l->visibile && l->anim != ANIM_KO) return;
    int x = lato ? 10 : 172, y = lato ? 12 : 88, w = lato ? 132 : 140, h = lato ? 30 : 42;
    DrawRectangle(x + 2, y + 2, w, h, (Color){ 40, 48, 64, 120 });
    DrawRectangle(x, y, w, h, (Color){ 248, 248, 232, 255 });
    DrawRectangleLines(x, y, w, h, (Color){ 64, 72, 88, 255 });
    DrawRectangle(x, y + h - 3, w, 3, (Color){ 96, 112, 128, 255 });
    Gfx_Testo(l->nome, x + 6, y + 4, COL_TESTO);
    Gfx_Testo(TextFormat("Lv%d", l->livello), x + w - 34, y + 4, COL_TESTO);
    UI_BarraHp(x + w - 72, y + 17, 64, l->hp, l->hpMax);
    DrawText("HP", x + w - 88, y + 15, 10, (Color){ 224, 160, 48, 255 });
    UI_Status(l->status, x + 6, y + 16);
    if (lato == LATO_GIOCATORE) {
        DrawText(TextFormat("%d/%d", (int)ceilf(l->hp), l->hpMax), x + w - 60, y + 24, 10, COL_TESTO);
        UI_BarraXp(x + 22, y + h - 6, w - 30, UI_FrazioneXp(l->specie, l->livello, (int32_t)u->xpMostrato));
        DrawText("XP", x + 6, y + h - 11, 10, (Color){ 72, 136, 216, 255 });
    }
}

static void DisegnaEffetti(const BattleUI *u) {
    if (u->effettoTimer >= 0 && u->effettoTimer < 14) {
        int lato = u->effettoLato;
        int cx = lato ? NEMICO_X + 32 : MIO_X + 40, cy = lato ? NEMICO_Y + 36 : MIO_Y + 50;
        Color c = UI_ColoreTipo(u->effettoTipo);
        float r = 6.0f + u->effettoTimer * 2.2f;
        c.a = (unsigned char)(220 - u->effettoTimer * 14);
        DrawCircleLines(cx, cy, r, c);
        DrawCircleLines(cx, cy, r * 0.6f, c);
        for (int i = 0; i < 6; i++) {
            float a = i * 1.047f + u->effettoTimer * 0.1f;
            DrawRectangle(cx + (int)(cosf(a) * r), cy + (int)(sinf(a) * r), 3, 3, c);
        }
    }
    if (u->boostTimer > 0) {
        int lato = u->boostLato;
        int bx = (lato ? NEMICO_X : MIO_X) + 14, by = (lato ? NEMICO_Y + 40 : MIO_Y + 54);
        Color c = u->boostDir < 0 ? (Color){ 248, 120, 72, 220 } : (Color){ 88, 136, 248, 220 };
        for (int i = 0; i < 4; i++) {
            int x = bx + i * 12, y = by + ((u->boostTimer * 2 + i * 9) % 30) * u->boostDir;
            DrawTriangle((Vector2){ (float)x, (float)y + (u->boostDir < 0 ? 0 : 5) }, (Vector2){ (float)x - 3, (float)y + (u->boostDir < 0 ? 5 : 0) },
                         (Vector2){ (float)x + 3, (float)y + (u->boostDir < 0 ? 5 : 0) }, c);
        }
    }
    if (u->coreVisibile) {
        float x = NEMICO_X + 32, y = NEMICO_Y + 52;
        if (u->coreAnim == 1 && u->coreTimer < 30) {
            float t = (float)u->coreTimer / 30.0f;
            x = 70 + (NEMICO_X + 32 - 70) * t;
            y = 120 + (NEMICO_Y + 52 - 120) * t - sinf(t * 3.14159f) * 50;
        }
        if (u->coreAnim == 2 && u->coreTimer < 34) x += sinf(u->coreTimer * 0.6f) * 3.0f * (u->coreTimer < 24);
        DrawTriangle((Vector2){ x, y - 7 }, (Vector2){ x - 5, y }, (Vector2){ x + 5, y }, (Color){ 120, 232, 208, 255 });
        DrawTriangle((Vector2){ x - 5, y }, (Vector2){ x, y + 6 }, (Vector2){ x + 5, y }, (Color){ 56, 152, 168, 255 });
        DrawPixel((int)x - 1, (int)y - 3, WHITE);
        if (u->coreAnim == 3)
            for (int i = 0; i < 5; i++) {
                float a = i * 1.256f + u->coreTimer * 0.05f;
                int r = 10 + (u->coreTimer % 20);
                DrawRectangle((int)(x + cosf(a) * r), (int)(y - 4 + sinf(a) * r * 0.6f), 2, 2, (Color){ 255, 240, 120, 255 });
            }
    }
}

static void DisegnaMessaggio(const BattleUI *u, int larghezza) {
    Gfx_Finestra(0, MSG_Y, larghezza, LOGICAL_H - MSG_Y);
    char vis[200];
    int n = (int)u->caratteri;
    if (n > (int)sizeof(vis) - 1) n = (int)sizeof(vis) - 1;
    snprintf(vis, sizeof(vis), "%.*s", n, u->testo);
    UI_TestoACapo(vis, 10, MSG_Y + 8, larghezza - 22, 3, COL_TESTO);
    if (u->stato == BUI_EVENTI && TestoCompleto(u) && u->testo[0] && (u->frame / 16) % 2)
        Gfx_Cursore(larghezza - 14, LOGICAL_H - 13);
}

static void DisegnaMenu(const BattleUI *u) {
    DisegnaMessaggio(u, 176);
    Gfx_Finestra(176, MSG_Y, 144, LOGICAL_H - MSG_Y);
    static const char *voci[4] = { "COMBATTI", "ELYON", "ZAINO", "FUGA" };
    for (int i = 0; i < 4; i++) {
        int x = 192 + (i % 2) * 66, y = MSG_Y + 10 + (i / 2) * 16;
        Gfx_Testo(voci[i], x, y, COL_TESTO);
        if (u->cursoreMenu == i) Gfx_Cursore(x - 8, y + 1);
    }
}

static void DisegnaMosse(const BattleUI *u) {
    const Elyon *e = u->lotta.lato[LATO_GIOCATORE].e;
    Gfx_Finestra(0, MSG_Y, 204, LOGICAL_H - MSG_Y);
    for (int i = 0; i < 4; i++) {
        int x = 16 + (i % 2) * 94, y = MSG_Y + 10 + (i / 2) * 16;
        const MoveData *m = GetMove(e->mosse[i]);
        Gfx_Testo(m ? m->nome : "-", x, y, m && e->pp[i] > 0 ? COL_TESTO : (Color){ 160, 160, 168, 255 });
        if (u->cursoreMossa == i) Gfx_Cursore(x - 8, y + 1);
    }
    Gfx_Finestra(204, MSG_Y, 116, LOGICAL_H - MSG_Y);
    const MoveData *m = GetMove(e->mosse[u->cursoreMossa]);
    if (!m) return;
    Gfx_Testo(TextFormat("PP %d/%d", e->pp[u->cursoreMossa], m->pp), 214, MSG_Y + 7, COL_TESTO);
    UI_EtichettaTipo(m->tipo, 214, MSG_Y + 19);
    DrawText(m->categoria == CAT_FISICA ? "FIS" : m->categoria == CAT_SPECIALE ? "SPE" : "STA", 262, MSG_Y + 20, 10, COL_TESTO);
    if (m->categoria != CAT_STATO) {
        float eff = Lotta_EfficaciaContro(&u->lotta, LATO_NEMICO, m->tipo);
        const char *t = eff > 1.0f ? "SUPEREFF. x2" : eff < 1.0f ? "POCO EFFICACE" : "NORMALE";
        Color c = eff > 1.0f ? (Color){ 200, 64, 48, 255 } : eff < 1.0f ? (Color){ 96, 96, 112, 255 } : COL_TESTO;
        DrawText(t, 214, MSG_Y + 33, 10, c);
    }
}

static void DisegnaSquadra(const BattleUI *u) {
    Gfx_Finestra(0, 0, LOGICAL_W, LOGICAL_H);
    const char *titolo = u->modoSquadra == SQ_FORZATO ? "Chi mandi in campo?" :
                         u->modoSquadra == SQ_OGGETTO ? "Su quale Elyon usarlo?" : "Scegli un Elyon";
    Gfx_Testo(titolo, 10, 6, (Color){ 40, 88, 152, 255 });
    for (int i = 0; i < MAX_SQUADRA; i++) {
        const Elyon *e = &u->lotta.coll->squadra[i];
        if (Elyon_Vuoto(e)) continue;
        UI_RigaSquadra(e, 8, 20 + i * 25, LOGICAL_W - 16, i == u->cursoreSquadra, i == u->lotta.lato[LATO_GIOCATORE].indice);
    }
    if (u->modoSquadra != SQ_FORZATO) DrawText("X: indietro", LOGICAL_W - 70, 6, 10, (Color){ 140, 140, 150, 255 });
}

static void DisegnaZaino(const BattleUI *u) {
    int lista[ITEM_ID_MAX];
    int n = OggettiLotta(u, lista);
    Gfx_Finestra(150, 4, 166, 128);
    Gfx_Testo("ZAINO", 162, 10, (Color){ 40, 88, 152, 255 });
    if (n == 0) Gfx_Testo("Niente da usare.", 162, 28, (Color){ 140, 140, 150, 255 });
    int inizio = u->cursoreZaino > 6 ? u->cursoreZaino - 6 : 0;
    for (int i = inizio; i < n && i < inizio + 7; i++) {
        const ItemData *it = GetItem(lista[i]);
        int y = 26 + (i - inizio) * 14;
        Gfx_Testo(it->nome, 168, y, COL_TESTO);
        Gfx_Testo(TextFormat("x%d", Inventario_Quantita(u->lotta.inv, lista[i])), 286, y, COL_TESTO);
        if (i == u->cursoreZaino) Gfx_Cursore(160, y + 1);
    }
    Gfx_Finestra(0, MSG_Y, LOGICAL_W, LOGICAL_H - MSG_Y);
    if (n > 0) UI_TestoACapo(GetItem(lista[u->cursoreZaino < n ? u->cursoreZaino : 0])->descrizione, 10, MSG_Y + 8, 296, 3, COL_TESTO);
}

static void DisegnaElisir(const BattleUI *u) {
    const Elyon *e = &u->lotta.coll->squadra[u->bersaglioOggetto];
    Gfx_Finestra(150, 30, 166, 80);
    Gfx_Testo("Quale mossa?", 162, 36, (Color){ 40, 88, 152, 255 });
    for (int i = 0; i < Elyon_NumMosse(e); i++) {
        const MoveData *m = GetMove(e->mosse[i]);
        int y = 52 + i * 13;
        Gfx_Testo(m->nome, 168, y, COL_TESTO);
        Gfx_Testo(TextFormat("%d/%d", e->pp[i], m->pp), 276, y, COL_TESTO);
        if (i == u->cursoreElisir) Gfx_Cursore(160, y + 1);
    }
}

static void DisegnaEvoluzione(const BattleUI *u) {
    DrawRectangle(0, 0, LOGICAL_W, LOGICAL_H, (Color){ 24, 24, 48, 255 });
    for (int i = 0; i < 30; i++) {
        int x = (i * 71 + u->frame) % LOGICAL_W, y = MSG_Y - ((i * 29 + u->frame * (1 + i % 3)) % MSG_Y);
        DrawRectangle(x, y, 2, 2, (Color){ 120, 240, 210, 180 });
    }
    int specie = u->evoDa;
    Color tinta = WHITE;
    if (u->evoFase == 1) {
        /* alternanza sempre piu' rapida tra le due sagome bianche */
        int periodo = 30 - u->evoTimer / 8;
        if (periodo < 3) periodo = 3;
        specie = (u->evoTimer / periodo) % 2 ? u->evoA : u->evoDa;
        tinta = (Color){ 255, 255, 255, 255 };
        float lampo = fminf(1.0f, u->evoTimer / 60.0f);
        BeginBlendMode(BLEND_ADDITIVE);
        UI_SpriteElyon(specie, LOGICAL_W / 2 - 48, 20, false, 1.5f, tinta);
        UI_SpriteElyon(specie, LOGICAL_W / 2 - 48, 20, false, 1.5f, (Color){ 255, 255, 255, (unsigned char)(200 * lampo) });
        EndBlendMode();
    } else {
        if (u->evoFase == 2) specie = u->evoA;
        UI_SpriteElyon(specie, LOGICAL_W / 2 - 48, 20, false, 1.5f, tinta);
    }
    Gfx_Finestra(0, MSG_Y, LOGICAL_W, LOGICAL_H - MSG_Y);
    char vis[200];
    snprintf(vis, sizeof(vis), "%.*s", (int)u->caratteri, u->testo);
    UI_TestoACapo(vis, 10, MSG_Y + 8, 296, 3, COL_TESTO);
    if (u->evoFase == 1) DrawText("X: interrompi", LOGICAL_W - 84, 4, 10, (Color){ 200, 200, 220, 255 });
}

static void DisegnaImpara(const BattleUI *u) {
    const Elyon *e = &u->lotta.coll->squadra[u->imparaIndice];
    if (u->stato == BUI_IMPARA_SINO) {
        DisegnaMessaggio(u, LOGICAL_W);
        Gfx_Finestra(256, 96, 56, 36);
        Gfx_Testo("SÌ", 274, 102, COL_TESTO);
        Gfx_Testo("NO", 274, 116, COL_TESTO);
        Gfx_Cursore(266, 103 + u->cursoreSiNo * 14);
        return;
    }
    Gfx_Finestra(150, 4, 166, 100);
    for (int i = 0; i < 5; i++) {
        int y = 12 + i * 15;
        const MoveData *m = i < 4 ? GetMove(e->mosse[i]) : NULL;
        Gfx_Testo(i < 4 ? (m ? m->nome : "-") : "NON IMPARARE", 168, y, i < 4 ? COL_TESTO : (Color){ 200, 72, 64, 255 });
        if (i == u->cursoreImpara) Gfx_Cursore(160, y + 1);
    }
    const MoveData *nuova = GetMove(u->imparaMossa);
    Gfx_Finestra(0, MSG_Y, LOGICAL_W, LOGICAL_H - MSG_Y);
    Gfx_Testo(u->testo, 10, MSG_Y + 8, COL_TESTO);
    if (nuova) {
        Gfx_Testo(TextFormat("Nuova: %s", nuova->nome), 10, MSG_Y + 22, COL_TESTO);
        UI_EtichettaTipo(nuova->tipo, 180, MSG_Y + 22);
        DrawText(TextFormat("POT %d", nuova->potenza), 232, MSG_Y + 23, 10, COL_TESTO);
    }
}

void BattleUI_Disegna(const BattleUI *u) {
    if (u->stato == BUI_EVOLUZIONE) { DisegnaEvoluzione(u); return; }
    DisegnaSfondo(u);
    if (u->stato != BUI_TRANSIZIONE) {
        DisegnaSpriteLato(u, LATO_NEMICO);
        DisegnaSpriteLato(u, LATO_GIOCATORE);
        DisegnaEffetti(u);
        DisegnaRiquadro(u, LATO_NEMICO);
        DisegnaRiquadro(u, LATO_GIOCATORE);
    }
    switch (u->stato) {
    case BUI_MENU:          DisegnaMenu(u); break;
    case BUI_MOSSE:         DisegnaMosse(u); break;
    case BUI_SQUADRA:       DisegnaSquadra(u); break;
    case BUI_ZAINO:         DisegnaZaino(u); break;
    case BUI_ELISIR:        DisegnaSquadra(u); DisegnaElisir(u); break;
    case BUI_IMPARA_SINO:
    case BUI_IMPARA_SCELTA: DisegnaImpara(u); break;
    default:                DisegnaMessaggio(u, LOGICAL_W); break;
    }
    if (u->stato == BUI_TRANSIZIONE) {
        /* lampi e bande nere che si aprono */
        int t = u->timer;
        if (t < 18 && (t / 3) % 2 == 0) DrawRectangle(0, 0, LOGICAL_W, LOGICAL_H, (Color){ 255, 255, 255, 200 });
        int h = t < 18 ? LOGICAL_H / 2 : (int)((float)(LOGICAL_H / 2) * (1.0f - (float)(t - 18) / 30.0f));
        if (h > 0) {
            DrawRectangle(0, 0, LOGICAL_W, h, BLACK);
            DrawRectangle(0, LOGICAL_H - h, LOGICAL_W, h, BLACK);
        }
    }
}
