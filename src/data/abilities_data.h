/* abilities_data.h - Abilita' 001..046 in un'unica tabella.
 * Gli alias (034, 036, 041, 045) puntano all'abilita' originale con aliasDi. */
#ifndef ECLYRA_ABILITIES_DATA_H
#define ECLYRA_ABILITIES_DATA_H

#include "types.h"

#define NUM_ABILITA 46

typedef enum {
    AB_MOLT_TIPO_ATTACCO = 0,   /* mosse di tipo1 (e tipo2) x moltiplicatore */
    AB_MOLT_DANNO_SUBITO,       /* danni subiti di tipo1 (o categoria) x moltiplicatore */
    AB_MOLT_DANNO_FISICO_SUBITO,
    AB_IMMUNITA_STATUS,         /* param = StatusId */
    AB_IMMUNITA_TIPO,           /* immune al tipo1; +1 stadio param (BoostId) quando colpito */
    AB_CURA_FINE_TURNO,         /* cura moltiplicatore % HP */
    AB_MOLT_STAT,               /* statistica param x moltiplicatore */
    AB_CRITICO_RIDOTTO,         /* bonus critico avversario ridotto */
    AB_CRITICO_ALTO,            /* probabilita' di critico param % */
    AB_RESISTE_UN_COLPO,        /* una volta per battaglia resta a 1 HP */
    AB_STATUS_SU_ATTACCO,       /* attacchi di tipo1: param % di veleno */
    AB_IMMUNE_CALO_ATK,
    AB_EFFETTO_CAMPO,           /* effetti di campo speciali (Aurion) */
    NUM_EFFETTI_ABILITA
} EffettoAbilita;

typedef enum {
    COND_SEMPRE = 0, COND_HP_SOTTO_30, COND_HP_SOTTO_50, COND_HP_SOTTO_25,
    COND_SOLE, COND_PIOGGIA, COND_TEMPESTA, COND_NOTTE
} CondizioneAbilita;

typedef struct {
    int id;
    const char *nome;
    int aliasDi;                  /* 0 = abilita' originale */
    EffettoAbilita effetto;
    CondizioneAbilita condizione;
    ElementType tipo1, tipo2;
    float moltiplicatore;
    int param;
    const char *descrizione;
} AbilityData;

const AbilityData *GetAbility(int id);
/* Segue gli alias fino all'abilita' originale */
const AbilityData *GetAbilityResolved(int id);

#endif
