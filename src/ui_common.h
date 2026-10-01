/* ui_common.h - Elementi d'interfaccia condivisi da lotta e menu (barre, sprite, righe squadra). */
#ifndef ECLYRA_UI_COMMON_H
#define ECLYRA_UI_COMMON_H

#include "elyon.h"
#include "graphics.h"
#include "data/types.h"

Color UI_ColoreTipo(ElementType t);
/* Etichetta colorata del tipo (larghezza 44) */
void  UI_EtichettaTipo(ElementType t, int x, int y);
/* Barra HP: verde > 50%, gialla > 20%, rossa sotto */
void  UI_BarraHp(int x, int y, int w, float hp, int max);
void  UI_BarraXp(int x, int y, int w, float frazione);
/* Frazione della barra XP per il livello mostrato */
float UI_FrazioneXp(int specie, int livello, int32_t xp);
const char *UI_SiglaStatus(StatusId s);
void  UI_Status(StatusId s, int x, int y);
/* Sprite 64x64 di una specie (retro = vista da dietro). tinta WHITE = normale. */
void  UI_SpriteElyon(int specie, int x, int y, bool retro, float scala, Color tinta);
/* Riga della squadra: mini sprite, nome, livello, barra HP, stato */
void  UI_RigaSquadra(const Elyon *e, int x, int y, int w, bool selezionata, bool evidenziata);
/* Testo a capo automatico in una larghezza (ritorna le righe usate) */
int   UI_TestoACapo(const char *testo, int x, int y, int larghezza, int maxRighe, Color colore);

#endif
