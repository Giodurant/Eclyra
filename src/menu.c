/* menu.c */
#include "menu.h"
#include "graphics.h"

void Menu_Init(Menu *m, const char *const *voci, int numVoci) {
    if (numVoci > MENU_MAX_VOCI) numVoci = MENU_MAX_VOCI;
    m->numVoci = numVoci;
    m->cursore = 0;
    for (int i = 0; i < numVoci; i++) {
        m->voci[i] = voci[i];
        m->abilitata[i] = true;
    }
}

static void Sposta(Menu *m, int delta) {
    for (int tentativi = 0; tentativi < m->numVoci; tentativi++) {
        m->cursore = (m->cursore + delta + m->numVoci) % m->numVoci;
        if (m->abilitata[m->cursore]) return;
    }
}

int Menu_Aggiorna(Menu *m, const Input *in) {
    if (m->numVoci == 0) return -1;
    if (!m->abilitata[m->cursore]) Sposta(m, 1);
    if (in->suP) Sposta(m, -1);
    if (in->giuP) Sposta(m, 1);
    if (in->conferma && m->abilitata[m->cursore]) return m->cursore;
    if (in->indietro) return -2;
    return -1;
}

void Menu_Disegna(const Menu *m, int x, int y, int larghezza) {
    int altezza = m->numVoci * 14 + 10;
    Gfx_Finestra(x, y, larghezza, altezza);
    for (int i = 0; i < m->numVoci; i++) {
        int ty = y + 7 + i * 14;
        Color c = m->abilitata[i] ? COL_TESTO : (Color){ 176, 176, 176, 255 };
        Gfx_Testo(m->voci[i], x + 16, ty, c);
        if (i == m->cursore) Gfx_Cursore(x + 7, ty + 1);
    }
}
