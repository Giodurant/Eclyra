/* common.h - Tipi condivisi tra logica e grafica (nessuna dipendenza). */
#ifndef ECLYRA_COMMON_H
#define ECLYRA_COMMON_H

typedef enum { DIR_GIU = 0, DIR_SU, DIR_SINISTRA, DIR_DESTRA } Direzione;

/* Spostamento di una tile nella direzione data */
static inline int Dir_DX(Direzione d) { return d == DIR_SINISTRA ? -1 : (d == DIR_DESTRA ? 1 : 0); }
static inline int Dir_DY(Direzione d) { return d == DIR_SU ? -1 : (d == DIR_GIU ? 1 : 0); }
static inline Direzione Dir_Opposta(Direzione d) {
    switch (d) {
        case DIR_GIU: return DIR_SU;
        case DIR_SU: return DIR_GIU;
        case DIR_SINISTRA: return DIR_DESTRA;
        default: return DIR_SINISTRA;
    }
}

#define NOME_GIOCATORE_DEFAULT "ARI"
#define NOME_RIVALE "KAEL"

#endif
