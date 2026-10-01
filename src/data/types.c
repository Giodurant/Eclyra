/* types.c - Matrice dei tipi costruita dalle regole del documento di design.
 *
 * SUPER EFFICACIA (attaccante -> difensori colpiti x2)
 * RESISTENZE (difensore -> attacchi che subisce x0,5)
 * Se un attacco e' sia super efficace sia resistito torna x1. Nessuna immunita'. */
#include "types.h"
#include <stdbool.h>

static const char *NOMI[TYPE_COUNT] = {
    "NEUTRO", "FIAMMA", "ACQUA", "NATURA", "IMPULSO", "VENTO", "ROCCIA",
    "GELO", "OMBRA", "ASTRALE", "TOSSICO", "PSICHE", "METALLO",
};

#define F TYPE_FIAMMA
#define A TYPE_ACQUA
#define N TYPE_NATURA
#define I TYPE_IMPULSO
#define V TYPE_VENTO
#define R TYPE_ROCCIA
#define G TYPE_GELO
#define O TYPE_OMBRA
#define S TYPE_ASTRALE
#define T TYPE_TOSSICO
#define P TYPE_PSICHE
#define M TYPE_METALLO
#define FINE TYPE_NONE

/* riga = attaccante; elenco dei difensori colpiti in modo super efficace */
static const ElementType SUPER[TYPE_COUNT][5] = {
    [F] = { N, G, M, FINE },
    [A] = { F, R, FINE },
    [N] = { A, R, FINE },
    [I] = { A, V, FINE },
    [V] = { N, T, FINE },
    [R] = { F, G, V, FINE },
    [G] = { V, N, FINE },
    [O] = { P, S, FINE },
    [S] = { O, T, FINE },
    [T] = { N, FINE },
    [P] = { T, I, FINE },
    [M] = { G, R, S, FINE },
};

/* riga = difensore; elenco dei tipi di attacco che resiste */
static const ElementType RESISTE[TYPE_COUNT][5] = {
    [F] = { F, N, G, M, FINE },
    [A] = { A, F, G, FINE },
    [N] = { N, A, I, FINE },
    [I] = { I, V, FINE },
    [V] = { N, V, FINE },
    [R] = { F, V, FINE },
    [G] = { G, FINE },
    [O] = { O, P, FINE },
    [S] = { S, FINE },
    [T] = { N, T, FINE },
    [P] = { P, FINE },
    [M] = { M, R, G, FINE },
};

static bool Contiene(const ElementType *elenco, ElementType t) {
    for (int i = 0; i < 5 && elenco[i] != FINE; i++)
        if (elenco[i] == t) return true;
    return false;
}

const char *Type_Nome(ElementType t) {
    return (t >= 0 && t < TYPE_COUNT) ? NOMI[t] : "?";
}

float Type_Efficacia(ElementType attacco, ElementType difesa) {
    if (attacco <= TYPE_NONE || attacco >= TYPE_COUNT || difesa <= TYPE_NONE || difesa >= TYPE_COUNT) return 1.0f;
    bool super = Contiene(SUPER[attacco], difesa);
    bool resistito = Contiene(RESISTE[difesa], attacco);
    if (super && resistito) return 1.0f;
    if (super) return 2.0f;
    if (resistito) return 0.5f;
    return 1.0f;
}

float Type_EfficaciaDoppia(ElementType attacco, ElementType difesa1, ElementType difesa2) {
    float e = Type_Efficacia(attacco, difesa1);
    if (difesa2 != TYPE_NONE && difesa2 != difesa1) e *= Type_Efficacia(attacco, difesa2);
    return e;
}
