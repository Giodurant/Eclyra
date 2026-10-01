/* dialogue.c */
#include "dialogue.h"
#include "common.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void DialogoDb_Libera(DialogoDb *db) {
    free(db->testo);
    memset(db, 0, sizeof(*db));
}

static bool RigaDa(const char **p, char *riga, size_t dim) {
    if (**p == '\0') return false;
    size_t n = 0;
    while (**p && **p != '\n') {
        if (**p != '\r' && n + 1 < dim) riga[n++] = **p;
        (*p)++;
    }
    if (**p == '\n') (*p)++;
    riga[n] = '\0';
    return true;
}

/* Toglie le righe vuote in fondo al testo della voce */
static void ChiudiVoce(DialogoDb *db, VoceDialogo *v) {
    while (db->dimTesto > v->inizio &&
           (db->testo[db->dimTesto - 1] == '\f' || db->testo[db->dimTesto - 1] == '\n'))
        db->dimTesto--;
    v->lunghezza = db->dimTesto - v->inizio;
}

bool DialogoDb_CaricaDaTesto(DialogoDb *db, const char *sorgente, char *errore, int dimErrore) {
    memset(db, 0, sizeof(*db));
    if (!sorgente) return false;
    size_t cap = strlen(sorgente) + 1;
    db->testo = (char *)malloc(cap);
    if (!db->testo) return false;

    const char *p = sorgente;
    char riga[512];
    VoceDialogo *corrente = NULL;
    bool rigaVuotaPrima = false, primaRiga = false;
    int numRiga = 0;

    while (RigaDa(&p, riga, sizeof(riga))) {
        numRiga++;
        if (riga[0] == '#') continue;
        if (riga[0] == '@') {
            if (corrente) ChiudiVoce(db, corrente);
            if (db->numVoci >= DIALOGO_MAX_VOCI) {
                snprintf(errore, (size_t)dimErrore, "riga %d: troppe voci", numRiga);
                return false;
            }
            corrente = &db->voci[db->numVoci++];
            memset(corrente, 0, sizeof(*corrente));
            char *q = strchr(riga + 1, '?');
            if (q) { *q = '\0'; Utils_StrCopy(corrente->condizione, sizeof(corrente->condizione), q + 1); }
            Utils_StrCopy(corrente->id, sizeof(corrente->id), riga + 1);
            corrente->inizio = db->dimTesto;
            rigaVuotaPrima = false;
            primaRiga = true;
            continue;
        }
        if (!corrente) continue;
        if (riga[0] == '\0') { rigaVuotaPrima = true; continue; }
        /* "Nome: testo" sulla prima riga = chi parla */
        if (primaRiga) {
            primaRiga = false;
            char *dp = strstr(riga, ": ");
            if (dp && dp - riga < 22 && riga[0] >= 'A' && riga[0] <= 'Z') {
                *dp = '\0';
                Utils_StrCopy(corrente->parlante, sizeof(corrente->parlante), riga);
                memmove(riga, dp + 2, strlen(dp + 2) + 1);
            }
        }
        if (db->dimTesto > corrente->inizio)
            db->testo[db->dimTesto++] = rigaVuotaPrima ? '\f' : '\n';
        rigaVuotaPrima = false;
        size_t n = strlen(riga);
        memcpy(db->testo + db->dimTesto, riga, n);
        db->dimTesto += (int)n;
    }
    if (corrente) ChiudiVoce(db, corrente);
    db->testo[db->dimTesto] = '\0';
    return true;
}

bool DialogoDb_CaricaFile(DialogoDb *db, const char *percorso, char *errore, int dimErrore) {
    char *s = Utils_LeggiFile(percorso);
    if (!s) {
        snprintf(errore, (size_t)dimErrore, "impossibile aprire %s", percorso);
        memset(db, 0, sizeof(*db));
        return false;
    }
    bool ok = DialogoDb_CaricaDaTesto(db, s, errore, dimErrore);
    free(s);
    return ok;
}

const VoceDialogo *DialogoDb_Trova(const DialogoDb *db, const char *id, const Flags *flags) {
    const VoceDialogo *base = NULL;
    /* la variante con condizione soddisfatta piu' in basso nel file vince */
    const VoceDialogo *variante = NULL;
    for (int i = 0; i < db->numVoci; i++) {
        const VoceDialogo *v = &db->voci[i];
        if (strcmp(v->id, id) != 0) continue;
        if (v->condizione[0] == '\0') base = v;
        else if (flags && Flags_Attivo(flags, v->condizione)) variante = v;
    }
    return variante ? variante : base;
}

/* ------------------------------------------------------------------ */

/* Sostituisce {GIOCATORE} e {RIVALE} */
static void Espandi(const char *src, int len, char *dst, int dim, const char *giocatore) {
    int o = 0;
    for (int i = 0; i < len && o + 1 < dim; ) {
        const char *seg = NULL;
        int salta = 0;
        if (strncmp(src + i, "{GIOCATORE}", 11) == 0) { seg = giocatore; salta = 11; }
        else if (strncmp(src + i, "{RIVALE}", 8) == 0) { seg = NOME_RIVALE; salta = 8; }
        if (seg) {
            for (int k = 0; seg[k] && o + 1 < dim; k++) dst[o++] = seg[k];
            i += salta;
        } else {
            dst[o++] = src[i++];
        }
    }
    dst[o] = '\0';
}

static void DividiPagine(Dialogo *d, const char *testo) {
    d->numPagine = 0;
    const char *inizio = testo;
    for (const char *c = testo; ; c++) {
        if (*c == '\f' || *c == '\0') {
            if (d->numPagine < DIALOGO_MAX_PAGINE) {
                int n = (int)(c - inizio);
                if (n >= DIALOGO_PAGINA) n = DIALOGO_PAGINA - 1;
                memcpy(d->pagine[d->numPagine], inizio, (size_t)n);
                d->pagine[d->numPagine][n] = '\0';
                d->numPagine++;
            }
            if (*c == '\0') break;
            inizio = c + 1;
        }
    }
    d->pagina = 0;
    d->caratteri = 0;
    d->lunghezzaPagina = (int)strlen(d->pagine[0]);
    d->attivo = true;
}

void Dialogo_Avvia(Dialogo *d, const DialogoDb *db, const char *id, const Flags *flags,
                   const char *nomeGiocatore, const char *parlanteDefault) {
    memset(d, 0, sizeof(*d));
    const VoceDialogo *v = db ? DialogoDb_Trova(db, id, flags) : NULL;
    char buf[DIALOGO_PAGINA * DIALOGO_MAX_PAGINE];
    if (!v) {
        snprintf(buf, sizeof(buf), "[Testo mancante: %s]", id ? id : "?");
        Utils_StrCopy(d->parlante, sizeof(d->parlante), parlanteDefault);
    } else {
        Espandi(db->testo + v->inizio, v->lunghezza, buf, (int)sizeof(buf),
                nomeGiocatore ? nomeGiocatore : NOME_GIOCATORE_DEFAULT);
        Utils_StrCopy(d->parlante, sizeof(d->parlante), v->parlante[0] ? v->parlante : parlanteDefault);
    }
    DividiPagine(d, buf);
}

void Dialogo_AvviaTesto(Dialogo *d, const char *testo, const char *parlante) {
    memset(d, 0, sizeof(*d));
    char buf[DIALOGO_PAGINA * DIALOGO_MAX_PAGINE];
    int o = 0;
    for (int i = 0; testo[i] && o + 1 < (int)sizeof(buf); i++) {
        if (testo[i] == '\n' && testo[i + 1] == '\n') { buf[o++] = '\f'; i++; }
        else buf[o++] = testo[i];
    }
    buf[o] = '\0';
    Utils_StrCopy(d->parlante, sizeof(d->parlante), parlante);
    DividiPagine(d, buf);
}

bool Dialogo_PaginaCompleta(const Dialogo *d) {
    return (int)d->caratteri >= d->lunghezzaPagina;
}

bool Dialogo_Aggiorna(Dialogo *d, bool conferma, float caratteriPerPasso) {
    if (!d->attivo) return false;
    if (!Dialogo_PaginaCompleta(d)) {
        if (conferma) d->caratteri = (float)d->lunghezzaPagina;   /* salta l'animazione */
        else d->caratteri += caratteriPerPasso;
        return true;
    }
    if (conferma) {
        d->pagina++;
        if (d->pagina >= d->numPagine) {
            d->attivo = false;
            return false;
        }
        d->caratteri = 0;
        d->lunghezzaPagina = (int)strlen(d->pagine[d->pagina]);
    }
    return true;
}

void Dialogo_TestoVisibile(const Dialogo *d, char *buf, int dim) {
    int n = (int)d->caratteri;
    if (n > d->lunghezzaPagina) n = d->lunghezzaPagina;
    /* non spezzare un carattere UTF-8 a meta' (byte di continuazione 10xxxxxx) */
    while (n > 0 && n < d->lunghezzaPagina && ((unsigned char)d->pagine[d->pagina][n] & 0xC0) == 0x80) n--;
    if (n >= dim) n = dim - 1;
    memcpy(buf, d->pagine[d->pagina], (size_t)n);
    buf[n] = '\0';
}
