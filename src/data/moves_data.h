/* moves_data.h - 100 mosse con ID stabili 1..100.
 * Gli effetti testuali del documento sono tradotti in campi strutturati
 * (effetto, probabilita', parametri) che il motore di lotta usera' nella Fase 4. */
#ifndef ECLYRA_MOVES_DATA_H
#define ECLYRA_MOVES_DATA_H

#include "types.h"
#include <stdbool.h>

#define NUM_MOSSE 100

typedef enum { CAT_FISICA = 0, CAT_SPECIALE, CAT_STATO } CategoriaMossa;

/* Stati alterati (persistenti) e confusione (temporanea) */
typedef enum {
    STATUS_NESSUNO = 0, STATUS_VELENO, STATUS_VELENO_GRAVE, STATUS_BRUCIATURA,
    STATUS_PARALISI, STATUS_SONNO, STATUS_GELO, STATUS_CONFUSIONE
} StatusId;

/* Statistiche modificabili in lotta (stadi -6..+6) */
typedef enum {
    BOOST_ATK = 0, BOOST_DEF, BOOST_SPA, BOOST_SPD, BOOST_SPE, BOOST_PRECISIONE, BOOST_EVASIONE, NUM_BOOST
} BoostId;

typedef enum {
    EFF_NESSUNO = 0,
    EFF_STATUS,                 /* param1 = StatusId, prob */
    EFF_STORDIMENTO,            /* il bersaglio salta il turno se agisce dopo; prob */
    EFF_BOOST_SE,               /* param1 = BoostId, param2 = stadi; param3/param4 secondo boost (param4 = 0 se assente) */
    EFF_BOOST_NEMICO,           /* param1 = BoostId, param2 = stadi (negativi), prob */
    EFF_CURA_PERC,              /* cura param1 % degli HP massimi */
    EFF_RADICAMENTO,            /* cura 1/param1 degli HP a fine turno */
    EFF_CRITICO_ALTO,           /* probabilita' di critico param1 % */
    EFF_MULTICOLPO,             /* colpisce param1 volte */
    EFF_CONTRACCOLPO,           /* subisce param1 % dei propri HP massimi */
    EFF_POTENZA_AMICIZIA,       /* potenza = 20 + amicizia/3, massimo param1 */
    EFF_DIMEZZATA_HP_BASSI,     /* potenza dimezzata sotto param1 % HP */
    EFF_CAMPO_TIPO,             /* tipo param1 +param2 % per param3 turni */
    EFF_RADDOPPIA_SE_COLPITO,   /* potenza doppia se colpito prima nel turno */
    EFF_DIMEZZA_PROSSIMO,       /* il prossimo colpo subito e' dimezzato */
    EFF_STORDIMENTO_PROSSIMO,   /* prob % di stordire il bersaglio nel turno successivo */
    EFF_RIFLESSIONE,            /* riflette param1 % dei danni fisici per param2 turni */
    EFF_FURIA,                  /* prossima mossa fisica +param1 %, cumulabile fino a +param2 % */
    EFF_BLOCCA_CAMBIO,          /* il bersaglio non puo' cambiare per param1 turni */
    EFF_IGNORA_BOOST_DIFESA,
    EFF_IGNORA_ABILITA_DIFESA,
    EFF_EQUILIBRIO,             /* HP di entrambi = media, limitata al proprio massimo */
    EFF_MULTI_BERSAGLIO,        /* in lotta singola colpisce l'unico avversario */
    NUM_EFFETTI
} EffettoMossa;

/* Flag */
#define MOSSA_SOLO_AURION  0x01
#define MOSSA_SOLO_EVENTO  0x02
#define MOSSA_PRECISIONE_AUTOMATICA 0x04   /* mosse di stato su se stessi */

typedef struct {
    int id;
    const char *nome;
    ElementType tipo;             /* TYPE_NONE = NEUTRO */
    CategoriaMossa categoria;
    int potenza;                  /* 0 per mosse di stato e per RITORNO (variabile) */
    int precisione;               /* 1..100 */
    int pp;
    int priorita;
    EffettoMossa effetto;
    int prob;                     /* % di attivazione dell'effetto (100 = sempre) */
    int param1, param2, param3, param4;
    int flag;
    const char *testoEffetto;     /* testo originale del documento */
} MoveData;

const MoveData *GetMove(int id);
const MoveData *GetMoveByName(const char *nome);
const char *Categoria_Nome(CategoriaMossa c);

#endif
