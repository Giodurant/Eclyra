/* items.c */
#include "items.h"
#include <string.h>

void Inventario_Azzera(Inventario *inv) { memset(inv, 0, sizeof(*inv)); }

int Inventario_Aggiungi(Inventario *inv, int id, int quantita) {
    if (!GetItem(id) || quantita <= 0 || id < 0 || id >= ITEM_ID_MAX) return 0;
    int spazio = QUANTITA_MAX - inv->quantita[id];
    int add = quantita < spazio ? quantita : spazio;
    inv->quantita[id] += add;
    return add;
}

bool Inventario_Rimuovi(Inventario *inv, int id, int quantita) {
    if (id < 0 || id >= ITEM_ID_MAX || quantita <= 0 || inv->quantita[id] < quantita) return false;
    inv->quantita[id] -= quantita;
    return true;
}

int Inventario_Quantita(const Inventario *inv, int id) {
    if (id < 0 || id >= ITEM_ID_MAX) return 0;
    return inv->quantita[id];
}

void Inventario_AggiungiLumen(Inventario *inv, int lumen) {
    long long v = (long long)inv->lumen + lumen;
    if (v < 0) v = 0;
    if (v > LUMEN_MAX) v = LUMEN_MAX;
    inv->lumen = (int)v;
}
