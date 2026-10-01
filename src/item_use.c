/* item_use.c */
#include "item_use.h"
#include "data/items_data.h"
#include <stdio.h>

bool Oggetto_SuElyon(int item) { return item >= ITEM_ERBA_VITALE && item <= ITEM_REVITA_MAX; }
bool Oggetto_RichiedeMossa(int item) { return item == ITEM_ELISIR || item == ITEM_ELISIR_MAX; }

float Oggetto_FattoreCore(int item) {
    switch (item) {
    case ITEM_CORE:      return 1.0f;
    case ITEM_CORE_PLUS: return 1.5f;
    case ITEM_CORE_MAX:  return 2.5f;
    case ITEM_CORE_VOID: return 3.5f;
    default:             return 0.0f;
    }
}

static bool CuraHp(Elyon *e, int quanti, char *msg, size_t dim) {
    if (Elyon_Esausto(e) || e->hp >= e->stat[STAT_HP]) return false;
    int prima = e->hp;
    e->hp = Utils_ClampInt(e->hp + quanti, 0, e->stat[STAT_HP]);
    snprintf(msg, dim, "%s recupera %d HP!", Elyon_Nome(e), e->hp - prima);
    return true;
}

static bool CuraStatus(Elyon *e, StatusId s1, StatusId s2, const char *testo, char *msg, size_t dim) {
    if (Elyon_Esausto(e) || (e->status != s1 && e->status != s2)) return false;
    e->status = STATUS_NESSUNO;
    e->turniSonno = 0;
    e->contatoreVelenoGrave = 0;
    snprintf(msg, dim, "%s %s", Elyon_Nome(e), testo);
    return true;
}

bool Oggetto_Usa(Elyon *e, int item, int slotMossa, char *msg, size_t dim) {
    if (!e || Elyon_Vuoto(e)) return false;
    switch (item) {
    case ITEM_ERBA_VITALE:   return CuraHp(e, 30, msg, dim);
    case ITEM_FRUTTO_LUMEN:  return CuraHp(e, 60, msg, dim);
    case ITEM_ELISIR:
    case ITEM_ELISIR_MAX: {
        if (Elyon_Esausto(e) || slotMossa < 0 || slotMossa >= MAX_MOSSE_ELYON) return false;
        const MoveData *m = GetMove(e->mosse[slotMossa]);
        if (!m || e->pp[slotMossa] >= m->pp) return false;
        int prima = e->pp[slotMossa];
        e->pp[slotMossa] = item == ITEM_ELISIR ? Utils_ClampInt(prima + 10, 0, m->pp) : m->pp;
        snprintf(msg, dim, "%s: %s recupera %d PP!", Elyon_Nome(e), m->nome, e->pp[slotMossa] - prima);
        return true;
    }
    case ITEM_ANTIDOTO:       return CuraStatus(e, STATUS_VELENO, STATUS_VELENO_GRAVE, "non e' piu' avvelenato!", msg, dim);
    case ITEM_STABILIZZATORE: return CuraStatus(e, STATUS_PARALISI, STATUS_PARALISI, "non e' piu' paralizzato!", msg, dim);
    case ITEM_RISVEGLIO:      return CuraStatus(e, STATUS_SONNO, STATUS_SONNO, "si e' svegliato!", msg, dim);
    case ITEM_FIAMMIFUGA:     return CuraStatus(e, STATUS_BRUCIATURA, STATUS_BRUCIATURA, "non e' piu' bruciato!", msg, dim);
    case ITEM_GELORISCALDO:   return CuraStatus(e, STATUS_GELO, STATUS_GELO, "si e' scongelato!", msg, dim);
    case ITEM_REVITA:
    case ITEM_REVITA_MAX: {
        if (!Elyon_Esausto(e)) return false;
        int max = e->stat[STAT_HP];
        e->hp = item == ITEM_REVITA ? (max * 30 + 99) / 100 : max;
        if (e->hp < 1) e->hp = 1;
        e->status = STATUS_NESSUNO;
        e->turniSonno = 0;
        e->contatoreVelenoGrave = 0;
        snprintf(msg, dim, "%s si e' ripreso!", Elyon_Nome(e));
        return true;
    }
    default:
        return false;
    }
}
