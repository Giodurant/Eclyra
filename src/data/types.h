/* types.h - I 12 tipi elementali di Asterra e la matrice di efficacia 12x12.
 * TYPE_NONE non e' un tredicesimo tipo: e' la categoria tecnica NEUTRO
 * (mosse senza affinita' elementale, STAB 1, efficacia 1). */
#ifndef ECLYRA_TYPES_H
#define ECLYRA_TYPES_H

typedef enum {
    TYPE_NONE = 0,
    TYPE_FIAMMA, TYPE_ACQUA, TYPE_NATURA, TYPE_IMPULSO, TYPE_VENTO, TYPE_ROCCIA,
    TYPE_GELO, TYPE_OMBRA, TYPE_ASTRALE, TYPE_TOSSICO, TYPE_PSICHE, TYPE_METALLO,
    TYPE_COUNT
} ElementType;

#define NUM_TIPI_ELEMENTALI 12

const char *Type_Nome(ElementType t);
/* 2.0 super efficace, 0.5 resistito, 1.0 normale (anche se super efficace E resistito) */
float Type_Efficacia(ElementType attacco, ElementType difesa);
/* Due tipi del difensore: i moltiplicatori si moltiplicano */
float Type_EfficaciaDoppia(ElementType attacco, ElementType difesa1, ElementType difesa2);

#endif
