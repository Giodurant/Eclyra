/* partita.c */
#include "partita.h"
#include "utils.h"
#include <string.h>

void Partita_Nuova(Partita *p, uint32_t seme) {
    memset(p, 0, sizeof(*p));
    Utils_StrCopy(p->nomeGiocatore, sizeof(p->nomeGiocatore), NOME_GIOCATORE_DEFAULT);
    Flags_Azzera(&p->flags);
    Inventario_Azzera(&p->inventario);
    /* si comincia in camera, al primo piano della casa del giocatore (vedi casa_giocatore.map) */
    Utils_StrCopy(p->mappa, sizeof(p->mappa), "casa_giocatore");
    p->x = -1;   /* -1 = usa lo SPAWN della mappa */
    p->y = -1;
    p->dir = DIR_GIU;
    p->semeRng = seme;
}
