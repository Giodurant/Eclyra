/* items_data.c */
#include "items_data.h"
#include <stddef.h>

static const ItemData ITEMS[] = {
    {  1, "ERBA VITALE",      CAT_CURA,  150, "Ridà 30 HP a un Elyon." },
    {  2, "FRUTTO LUMEN",     CAT_CURA,  350, "Ridà 60 HP a un Elyon." },
    {  3, "ELISIR",           CAT_CURA,  500, "Ridà 10 PP a una mossa." },
    {  4, "ELISIR MAX",       CAT_CURA, 1200, "Ripristina tutti i PP di una mossa." },
    {  5, "ANTIDOTO",         CAT_CURA,  100, "Cura l'avvelenamento." },
    {  6, "STABILIZZATORE",   CAT_CURA,  100, "Cura la paralisi." },
    {  7, "RISVEGLIO",        CAT_CURA,  100, "Sveglia un Elyon addormentato." },
    {  8, "FIAMMIFUGA",       CAT_CURA,  100, "Cura la bruciatura." },
    {  9, "GELORISCALDO",     CAT_CURA,  100, "Scongela un Elyon." },
    { 10, "REVITA",           CAT_CURA, 1500, "Rianima un Elyon esausto con il 30% degli HP." },
    { 11, "REVITA MAX",       CAT_CURA, 3000, "Rianima un Elyon esausto con tutti gli HP." },
    { 12, "CORE",             CAT_CORE,  200, "Nucleo di Lumen per catturare Elyon selvatici." },
    { 13, "CORE PLUS",        CAT_CORE,  600, "Core potenziato: cattura più facile (x1,5)." },
    { 14, "CORE MAX",         CAT_CORE, 1500, "Core di alta qualità (x2,5)." },
    { 15, "CORE VOID",        CAT_CORE, 3500, "Core instabile legato al Vuoto (x3,5). Post-game." },
    { 20, "PIETRA FIAMMA",    CAT_EVOLUZIONE, 0, "Pietra calda che pulsa di Lumen." },
    { 21, "PIETRA ACQUA",     CAT_EVOLUZIONE, 0, "Pietra fresca come una sorgente." },
    { 22, "PIETRA NATURA",    CAT_EVOLUZIONE, 0, "Pietra coperta di muschio vivo." },
    { 23, "PIETRA LUMEN",     CAT_EVOLUZIONE, 0, "Pietra che brilla di luce propria." },
    { 24, "FRAMMENTO I",      CAT_CHIAVE, 0, "Frammento di una tavola dei Custodi." },
    { 25, "FRAMMENTO II",     CAT_CHIAVE, 0, "Frammento di una tavola dei Custodi." },
    { 26, "FRAMMENTO III",    CAT_CHIAVE, 0, "Frammento di una tavola dei Custodi." },
    { 27, "FRAMMENTO IV",     CAT_CHIAVE, 0, "Frammento di una tavola dei Custodi." },
    { 28, "FRAMMENTO V",      CAT_CHIAVE, 0, "Frammento di una tavola dei Custodi." },
    { 29, "NUCLEO CONDIVISO", CAT_CHIAVE, 0, "Dà il 50% dell'XP anche agli Elyon che non lottano." },
    { 30, "NUCLEO CALDO",     CAT_EQUIPAGGIAMENTO, 0, "Equipaggiato: mosse Fiamma x1,1." },
    { 31, "CRISTALLO ACQUA",  CAT_EQUIPAGGIAMENTO, 0, "Equipaggiato: mosse Acqua x1,1." },
    { 32, "SEME VITALE",      CAT_EQUIPAGGIAMENTO, 0, "Equipaggiato: cura l'1% degli HP a ogni turno." },
    { 33, "PIETRA FERROSA",   CAT_EQUIPAGGIAMENTO, 0, "Equipaggiato: DEF x1,1." },
    { 34, "ANELLO LUMEN",     CAT_EQUIPAGGIAMENTO, 0, "Equipaggiato: mosse Astrale x1,1." },
    { 35, "OMBRA FRAMMENTATA", CAT_EQUIPAGGIAMENTO, 0, "Equipaggiato: mosse Ombra x1,1." },
};

const ItemData *GetItem(int id) {
    for (size_t i = 0; i < sizeof(ITEMS) / sizeof(ITEMS[0]); i++)
        if (ITEMS[i].id == id) return &ITEMS[i];
    return NULL;
}
