/* map.h - Mappe a tile 16x16 caricate da file di testo (assets/maps/NOME.map).
 *
 * Questo modulo non usa raylib: viene compilato in eclyra_core e testato.
 * Il disegno sta in map_render.c.
 *
 * Formato del file (una direttiva per riga, '#' = commento):
 *   NOME Lunaria                        nome mostrato al giocatore
 *   TIPO ESTERNO | INTERNO
 *   DIMENSIONI 40 30
 *   TILES                               seguono DIMENSIONI.h righe di caratteri
 *   ....
 *   PORTA x y DEST dx dy DIR            si attiva provando ad entrare nella tile (porte degli edifici)
 *   USCITA x y w h DEST dx dy DIR       si attiva arrivando su una tile del rettangolo;
 *                                       la destinazione si sposta con la stessa colonna/riga
 *   NPC id sprite x y DIR MOV dialogo Nome visualizzato
 *   CARTELLO x y dialogo                testo leggibile premendo Z davanti alla tile
 *   OGGETTO x y idOggetto quantita flag oggetto raccoglibile (sparisce quando il flag e' attivo)
 *   SPAWN x y DIR                       punto di partenza della mappa
 *   HABITAT ID                          tabella degli incontri nell'erba alta (encounters_data)
 *   ALLENATORE npc ID_ALLENATORE flag   l'NPC sfida il giocatore (una volta: il flag segna la vittoria);
 *                                       "{STARTER}" nell'ID diventa PYRIN / AQUIL / MOSSEN
 *   CURATORE npc                        dopo il dialogo l'NPC cura la squadra
 *   PC x y                              terminale del box degli Elyon
 *   STARTER x y SPECIE                  capsula per scegliere il primo Elyon (provvisoria, Fase 4)
 */
#ifndef ECLYRA_MAP_H
#define ECLYRA_MAP_H

#include "common.h"
#include <stdbool.h>
#include <stdint.h>

#define MAP_MAX_W 80
#define MAP_MAX_H 60
#define MAP_MAX_WARP 24
#define MAP_MAX_NPC 16
#define MAP_MAX_CARTELLI 24
#define MAP_MAX_OGGETTI 8
#define MAP_NOME_ID 24
#define DIALOGO_ID 32
#define MAP_MAX_ALLENATORI 8
#define MAP_MAX_PC 4
#define MAP_MAX_STARTER 3

typedef enum {
    /* esterni */
    T_ERBA = 0, T_ERBA_ALTA, T_SENTIERO, T_ALBERO, T_ACQUA, T_FIORI, T_CARTELLO,
    T_CASA, T_CASA_BLU, T_NEGOZIO, T_LAB,
    /* interni */
    T_PAVIMENTO, T_MURO, T_TAPPETINO, T_TAPPETO, T_SEDIA,
    T_LETTO_A, T_LETTO_B, T_LIBRERIA_A, T_LIBRERIA_B, T_MACCHINA_A, T_MACCHINA_B,
    T_PIANTA, T_TV, T_BANCONE, T_SCAFFALE_A, T_SCAFFALE_B, T_TAVOLO, T_PC,
    T_COUNT
} TileTipo;

typedef enum { MAPPA_ESTERNO, MAPPA_INTERNO } TipoMappa;
typedef enum { MOV_FERMO, MOV_GIRA, MOV_VAGA } MovimentoNpc;

typedef struct {
    int x, y, w, h;
    char dest[MAP_NOME_ID];
    int dx, dy;
    Direzione dir;
    bool suUrto;            /* true = PORTA, false = USCITA */
} Warp;

typedef struct {
    char id[24];
    char nome[24];
    int sprite;
    int x, y;
    Direzione dir;
    MovimentoNpc mov;
    char dialogo[DIALOGO_ID];
} NpcDef;

typedef struct {
    int x, y;
    char dialogo[DIALOGO_ID];
} Cartello;

typedef struct {
    int x, y;
    int oggetto, quantita;
    char flag[32];
} OggettoMappa;

typedef struct {
    char npc[24];
    char allenatore[40];
    char flag[32];
} AllenatoreMappa;

typedef struct {
    int x, y;
    int specie;
} StarterMappa;

typedef struct {
    char id[MAP_NOME_ID];       /* nome del file senza estensione, es. "lunaria" */
    char nome[32];              /* nome mostrato, es. "LUNARIA" */
    TipoMappa tipo;
    int w, h;
    uint8_t tile[MAP_MAX_H][MAP_MAX_W];
    Warp warp[MAP_MAX_WARP];
    int numWarp;
    NpcDef npc[MAP_MAX_NPC];
    int numNpc;
    Cartello cartello[MAP_MAX_CARTELLI];
    int numCartelli;
    OggettoMappa oggetto[MAP_MAX_OGGETTI];
    int numOggetti;
    int spawnX, spawnY;
    Direzione spawnDir;
    char habitat[24];           /* "" = nessun incontro */
    AllenatoreMappa allenatore[MAP_MAX_ALLENATORI];
    int numAllenatori;
    char curatore[24];          /* id NPC ("" = nessuno) */
    int pcX[MAP_MAX_PC], pcY[MAP_MAX_PC];
    int numPc;
    StarterMappa starter[MAP_MAX_STARTER];
    int numStarter;
} Map;

/* Dimensioni (in tile) degli edifici; la porta e' al centro della riga in basso */
void Map_DimensioniEdificio(TileTipo t, int *w, int *h);
bool Map_IsEdificio(TileTipo t);

/* Carica da testo. Ritorna false e scrive il motivo in errore in caso di problemi. */
bool Map_CaricaDaTesto(Map *m, const char *id, const char *testo, char *errore, int dimErrore);
/* Carica dal file <cartella>/<id>.map */
bool Map_CaricaFile(Map *m, const char *cartella, const char *id, char *errore, int dimErrore);

TileTipo Map_Tile(const Map *m, int x, int y);   /* fuori mappa: albero (esterni) o muro (interni) */
bool     Map_TileBloccata(TileTipo t);
bool     Map_Bloccata(const Map *m, int x, int y);
bool     Map_Dentro(const Map *m, int x, int y);

/* Warp che contiene la tile (x, y), oppure NULL */
const Warp *Map_WarpIn(const Map *m, int x, int y);
/* Destinazione di un warp per la tile (x, y) dentro il suo rettangolo */
void Map_DestinazioneWarp(const Warp *w, int x, int y, int *dx, int *dy);

const Cartello *Map_CartelloIn(const Map *m, int x, int y);
const AllenatoreMappa *Map_AllenatoreDi(const Map *m, const char *npcId);
bool Map_PcIn(const Map *m, int x, int y);
const StarterMappa *Map_StarterIn(const Map *m, int x, int y);

/* Elenco delle mappe esistenti (per test e teletrasporto di debug) */
extern const char *const MAPPE_GIOCO[];
extern const int NUM_MAPPE_GIOCO;

#endif
