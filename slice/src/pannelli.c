/* pannelli.c - Menu di gioco nell'ordine ufficiale (ELYON, ZAINO, BESTIARIO, MAPPA, SCHEDA, SALVA,
 * OPZIONI) e le sue schermate: squadra, Scheda Elyon, zaino, scheda allenatore.
 * Mockup di riferimento: 03_menu, 07_scheda_elyon, 08_zaino. */
#include "slice.h"
#include "gfx.h"
#include "vista_dialogo.h"
#include "data/abilities_data.h"
#include "data/items_data.h"
#include "gen/atlante_ui.h"
#include "ui_common.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct { const char *nome; Rectangle icona; } VoceMenu;
static const char *SOTTO[7] = { "La tua squadra", "Oggetti e Core", NULL, "Asterra", "Il tuo profilo", "Non disponibile nella slice", "Non disponibile nella slice" };

static VoceMenu Voce(int i) {
    switch (i) {
    case 0: return (VoceMenu){ "ELYON", UI_ICONA_ELYON };
    case 1: return (VoceMenu){ "ZAINO", UI_ICONA_ZAINO };
    case 2: return (VoceMenu){ "BESTIARIO", UI_ICONA_LIBRO };
    case 3: return (VoceMenu){ "MAPPA", UI_ICONA_MAPPA };
    case 4: return (VoceMenu){ "SCHEDA", UI_ICONA_SCHEDA };
    case 5: return (VoceMenu){ "SALVA", UI_ICONA_SALVA };
    default: return (VoceMenu){ "OPZIONI", UI_ICONA_OPZIONI };
    }
}

static int Ox(const Slice *s) { return (s->canvas.w - 320) / 2; }
static int Oy(const Slice *s) { return (s->canvas.h - 180) / 2; }

void Slice_Avviso(Slice *s, const char *testo) {
    snprintf(s->avviso, sizeof(s->avviso), "%s", testo);
    s->pannelloDopoAvviso = s->pannello;
    s->pannello = PAN_AVVISO;
}

void Pannelli_Apri(Slice *s, Pannello p) {
    s->pannelloPrima = s->pannello;
    s->pannello = p;
    if (p == PAN_MENU) { s->cursoreMenu = s->cursoreMenu % 7; }
    if (p == PAN_SQUADRA || p == PAN_ZAINO) s->cursoreLista = 0;
    if (p == PAN_SCHEDA_ELYON) s->paginaScheda = 1;
}

/* ------------------------------------------------------------------ geometria */

static Rectangle RigaMenu(const Slice *s, int i) {
    int x = 6 + s->canvas.safeL, y = Oy(s) + 6 + 32 + i * 19;
    return (Rectangle){ (float)(x + 3), (float)(y - 1), 122, 18 };
}

static Rectangle RigaSquadra(const Slice *s, int i) {
    return (Rectangle){ (float)(Ox(s) + 10), (float)(Oy(s) + 28 + i * 23), 170, 22 };
}

static Rectangle RigaZaino(const Slice *s, int i) {
    return (Rectangle){ (float)(Ox(s) + 10), (float)(Oy(s) + 28 + i * 15), 178, 14 };
}

static int ListaZaino(const Slice *s, int *lista) {
    int n = 0;
    for (int id = 1; id < ITEM_ID_MAX; id++)
        if (GetItem(id) && Inventario_Quantita(&s->partita.inventario, id) > 0) lista[n++] = id;
    return n;
}

/* ------------------------------------------------------------------ aggiornamento */

static int Scoperti(const Slice *s) {
    int n = 0;
    for (int i = 1; i <= 100; i++) n += s->visto[i];
    return n;
}

bool Pannelli_Aggiorna(Slice *s) {
    Input *in = &s->in;
    switch (s->pannello) {
    case PAN_MENU:
        if (in->suP) s->cursoreMenu = (s->cursoreMenu + 6) % 7;
        if (in->giuP) s->cursoreMenu = (s->cursoreMenu + 1) % 7;
        for (int i = 0; i < 7; i++) if (Azioni_Tap(&s->az, RigaMenu(s, i))) { s->cursoreMenu = i; in->conferma = true; }
        if (in->indietro || in->menu) { s->pannello = PAN_NESSUNO; break; }
        if (in->conferma) {
            switch (s->cursoreMenu) {
            case 0: Pannelli_Apri(s, PAN_SQUADRA); break;
            case 1: Pannelli_Apri(s, PAN_ZAINO); break;
            case 2: s->cursoreBest = 0; Pannelli_Apri(s, PAN_BESTIARIO); break;
            case 3: Pannelli_Apri(s, PAN_MAPPA); break;
            case 4: Pannelli_Apri(s, PAN_ALLENATORE); break;
            default: Slice_Avviso(s, "Salvataggi e opzioni arriveranno con l'implementazione completa: la slice prova solo lo stile."); break;
            }
        }
        break;
    case PAN_SQUADRA: {
        int n = Collezione_NumSquadra(&s->partita.collezione);
        if (n > 0) {
            if (in->suP) s->cursoreLista = (s->cursoreLista + n - 1) % n;
            if (in->giuP) s->cursoreLista = (s->cursoreLista + 1) % n;
        }
        for (int i = 0; i < n; i++)
            if (Azioni_Tap(&s->az, RigaSquadra(s, i))) { if (s->cursoreLista == i) in->conferma = true; s->cursoreLista = i; }
        if (in->indietro) s->pannello = PAN_MENU;
        else if (in->conferma && n > 0) { s->cursoreScheda = s->cursoreLista; Pannelli_Apri(s, PAN_SCHEDA_ELYON); }
        break;
    }
    case PAN_SCHEDA_ELYON: {
        int n = Collezione_NumSquadra(&s->partita.collezione);
        if (in->sinistraP) s->paginaScheda = (s->paginaScheda + 2) % 3;
        if (in->destraP) s->paginaScheda = (s->paginaScheda + 1) % 3;
        if (n > 0 && in->suP) s->cursoreScheda = (s->cursoreScheda + n - 1) % n;
        if (n > 0 && in->giuP) s->cursoreScheda = (s->cursoreScheda + 1) % n;
        for (int i = 0; i < 3; i++)
            if (Azioni_Tap(&s->az, (Rectangle){ (float)(Ox(s) + 110 + i * 68), (float)(Oy(s) + 5), 64, 13 })) s->paginaScheda = i;
        if (in->indietro) s->pannello = PAN_SQUADRA;
        break;
    }
    case PAN_ZAINO: {
        int lista[ITEM_ID_MAX];
        int n = ListaZaino(s, lista);
        if (n > 0) {
            if (in->suP) s->cursoreLista = (s->cursoreLista + n - 1) % n;
            if (in->giuP) s->cursoreLista = (s->cursoreLista + 1) % n;
        }
        for (int i = 0; i < n && i < 8; i++) if (Azioni_Tap(&s->az, RigaZaino(s, i))) s->cursoreLista = i;
        if (in->indietro) s->pannello = PAN_MENU;
        else if (in->conferma && n > 0) Slice_Avviso(s, "Nella slice gli oggetti si usano in lotta: dallo zaino si consultano.");
        break;
    }
    case PAN_ALLENATORE:
        if (in->indietro || in->conferma || s->az.tap) s->pannello = PAN_MENU;
        break;
    case PAN_BESTIARIO: Bestiario_Aggiorna(s); break;
    case PAN_MAPPA: Mappa_Aggiorna(s); break;
    case PAN_AVVISO:
        if (in->conferma || in->indietro) s->pannello = s->pannelloDopoAvviso;
        break;
    default: break;
    }
    return s->pannello != PAN_NESSUNO;
}

/* ------------------------------------------------------------------ disegno */

static void Prompt(const Slice *s, int x, int y, AzioneIn az, const char *testo) {
    if (s->az.ultimo == DISP_TOUCH) return;
    const char *p = Azioni_Prompt(&s->az, az);
    G_GlifoTasto(x, y, p, false);
    int w = T_LarghezzaMicro(p) <= 3 ? 9 : T_LarghezzaMicro(p) + 6;
    T_Micro(testo, x + w + 3, y + 2, PAL_ORO[3]);
}

static void DisegnaMenu(const Slice *s) {
    const Canvas *c = &s->canvas;
    G_Oscura(0, 0, c->w, c->h, 110);
    int x = 6 + c->safeL, y = Oy(s) + 6, w = 128;
    G_Cornice(x, y, w, 168, CORNICE_NOTTE, true);
    T_TestoOmbra(s->partita.nomeGiocatore, x + 8, y + 7, PAL_ORO[4]);
    char riga[48];
    unsigned t = s->partita.secondiGioco;
    snprintf(riga, sizeof(riga), "%s  %02u:%02u", s->world.mappa.nome, t / 3600, (t / 60) % 60);
    T_Micro(riga, x + 8, y + 18, PAL_GRIGIO_UI);
    G_Icona(UI_ICONA_CORE, x + 82, y + 6);
    snprintf(riga, sizeof(riga), "%d", s->partita.inventario.lumen);
    T_Micro(riga, x + 96, y + 9, PAL_LUMEN[3]);
    G_LineaH(x + 6, y + 27, w - 12, PAL_ORO[1]);
    for (int i = 0; i < 7; i++) {
        int yy = y + 32 + i * 19;
        bool sel = s->cursoreMenu == i;
        bool spento = i >= 5;
        if (sel) { G_RigaSelezionata(x + 3, yy - 1, w - 6, 18); G_Cursore(x + 6, yy + 8, s->frame); }
        VoceMenu v = Voce(i);
        G_Icona(v.icona, x + 14, yy + 3);
        T_TestoOmbra(v.nome, x + 30, yy + 2, sel ? PAL_ORO[4] : spento ? PAL_GRIGIO_UI : PAL_CREMA);
        char sott[40];
        if (i == 2) snprintf(sott, sizeof(sott), "%d scoperti", Scoperti(s));
        else snprintf(sott, sizeof(sott), "%s", SOTTO[i]);
        for (char *p = sott; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
        sott[22] = 0;
        T_Micro(sott, x + 30, yy + 11, sel ? PAL_LUMEN[3] : PAL_GRIGIO_UI);
    }
    /* anteprima della squadra */
    int n = Collezione_NumSquadra(&s->partita.collezione);
    for (int k = 0; k < n && k < 6; k++)
        G_Elyon(s->partita.collezione.squadra[k].specie, VISTA_ICONA, (s->frame / 20 + k) % 2, x + w + 16 + k * 34, Oy(s) + 128, WHITE);
    if (s->az.ultimo != DISP_TOUCH) {
        int bx = x + w + 6, by = c->h - c->safeB - 15;
        G_Rett(bx, by, c->w - c->safeR - bx - 4, 12, (Color){ PAL_NOTTE[0].r, PAL_NOTTE[0].g, PAL_NOTTE[0].b, 200 });
        G_LineaH(bx, by, c->w - c->safeR - bx - 4, PAL_ORO[1]);
        Prompt(s, bx + 6, by + 2, AZ_CONFERMA, "APRI");
        Prompt(s, bx + 46, by + 2, AZ_ANNULLA, "CHIUDI");
        Prompt(s, bx + 98, by + 2, AZ_MENU, "MENU RAPIDO");
    }
}

static void Freccia(int x, int y, int verso, Color c) {
    for (int j = 0; j < 4; j++) {
        int w = verso > 0 ? j : 3 - j;
        G_LineaH(x + 3 - w, y + j, 2 * w + 1, c);
    }
}

static void Cerchietto(int cx, int cy, int rx, int ry) {
    DrawEllipse(cx, cy, (float)rx, (float)ry, PAL_TERRA[2]);
    DrawEllipseLines(cx, cy, (float)rx, (float)ry, PAL_ORO[2]);
    for (int k = 0; k < 24; k += 3) {
        float a = (float)k / 24.0f * 6.2831853f;
        DrawPixel(cx + (int)(cosf(a) * rx * 0.8f), cy + (int)(sinf(a) * ry * 0.8f), PAL_ORO[4]);
    }
}

static void DisegnaSquadra(const Slice *s) {
    const Canvas *c = &s->canvas;
    G_Gradiente(0, 0, c->w, c->h, PAL_NOTTE[2], PAL_NOTTE[0], true);
    int ox = Ox(s), oy = Oy(s);
    G_Intestazione(ox + 6, oy + 4, "SQUADRA");
    G_Cornice(ox + 6, oy + 22, 178, 154, CORNICE_NOTTE, true);
    const Collezione *col = &s->partita.collezione;
    int n = Collezione_NumSquadra(col);
    for (int i = 0; i < MAX_SQUADRA; i++) {
        Rectangle r = RigaSquadra(s, i);
        int x = (int)r.x, y = (int)r.y;
        if (i >= n) {   /* posto libero: cristallo spento */
            G_Cristallo(x + 18, y + 7, 7, false);
            T_Micro("POSTO LIBERO", x + 32, y + 8, PAL_NOTTE[4]);
            continue;
        }
        const Elyon *e = &col->squadra[i];
        bool sel = s->cursoreLista == i;
        if (sel) { G_RigaSelezionata(x, y, (int)r.width, 22); G_Cursore(x + 3, y + 11, s->frame); }
        BeginScissorMode(x + 8, y, 26, 22);
        G_Elyon(e->specie, VISTA_ICONA, sel ? (s->frame / 15) % 2 : 0, x + 5, y - 8, WHITE);
        EndScissorMode();
        T_TestoOmbra(Elyon_Nome(e), x + 36, y + 3, sel ? PAL_ORO[4] : PAL_CREMA);
        char lv[12];
        snprintf(lv, sizeof(lv), "LV %d", e->livello);
        T_Micro(lv, x + (int)r.width - 6 - T_LarghezzaMicro(lv), y + 4, PAL_ORO[3]);
        float f = (float)e->hp / (float)e->stat[STAT_HP];
        G_Barra(x + 36, y + 15, 84, f, G_RampaHp(f), "HP", 3);
        char hp[16];
        snprintf(hp, sizeof(hp), "%d/%d", e->hp, e->stat[STAT_HP]);
        T_Micro(hp, x + 140, y + 14, PAL_CREMA);
    }
    /* anteprima a destra: Elyon selezionato sul cerchio runico */
    G_Cornice(ox + 188, oy + 22, 126, 154, CORNICE_NOTTE, true);
    if (n > 0) {
        const Elyon *e = &col->squadra[s->cursoreLista];
        const SpeciesData *sp = GetSpecies(e->specie);
        Cerchietto(ox + 251, oy + 92, 38, 9);
        G_Elyon(e->specie, VISTA_FRONTE, (s->frame / 30) % 2, ox + 219, oy + 32, WHITE);
        T_TitoloOro(Elyon_Nome(e), ox + 251 - T_LarghezzaGrassetto(Elyon_Nome(e)) / 2, oy + 106);
        G_PillolaTipo(sp->tipo1, ox + 196, oy + 122, 56);
        if (sp->tipo2 != TYPE_NONE) G_PillolaTipo(sp->tipo2, ox + 254, oy + 122, 56);
        Prompt(s, ox + 196, oy + 158, AZ_CONFERMA, "SCHEDA");
        Prompt(s, ox + 248, oy + 158, AZ_ANNULLA, "INDIETRO");
    }
}

static void DisegnaScheda(const Slice *s) {
    const Canvas *c = &s->canvas;
    G_Gradiente(0, 0, c->w, c->h, PAL_NOTTE[2], PAL_NOTTE[0], true);
    int ox = Ox(s), oy = Oy(s);
    const Elyon *e = &s->partita.collezione.squadra[s->cursoreScheda];
    const SpeciesData *sp = GetSpecies(e->specie);
    G_Intestazione(ox + 6, oy + 4, Elyon_Nome(e));
    static const char *TAB[3] = { "INFO", "STATISTICHE", "MOSSE" };
    for (int i = 0; i < 3; i++) {
        int x = ox + 110 + i * 68;
        bool att = s->paginaScheda == i;
        G_Rett(x, oy + 5, 64, 13, PAL_NOTTE[0]);
        G_Gradiente(x + 1, oy + 6, 62, 11, att ? PAL_AZZURRO[1] : PAL_NOTTE[3], PAL_NOTTE[2], true);
        if (att) G_LineaH(x + 1, oy + 6, 62, PAL_ORO[4]);
        T_Testo(TAB[i], x + 32 - T_Larghezza(TAB[i]) / 2, oy + 8, att ? PAL_ORO[4] : PAL_CREMA);
    }
    /* colonna sinistra: Elyon, livello, tipo, natura, abilita', oggetto, amicizia */
    G_Cornice(ox + 6, oy + 22, 128, 154, CORNICE_NOTTE, true);
    Cerchietto(ox + 70, oy + 92, 40, 9);
    G_Elyon(e->specie, VISTA_FRONTE, (s->frame / 30) % 2 + ((s->frame % 200) < 8 ? 2 - (s->frame / 30) % 2 : 0), ox + 38, oy + 32, WHITE);
    T_Micro("LV", ox + 14, oy + 108, PAL_ORO[3]);
    char t[48];
    snprintf(t, sizeof(t), "%d", e->livello);
    T_TestoOmbra(t, ox + 24, oy + 106, PAL_CREMA);
    G_PillolaTipo(sp->tipo1, ox + 50, oy + 106, 60);
    const AbilityData *ab = GetAbilityResolved(sp->abilita);
    const ItemData *og = e->oggetto ? GetItem(e->oggetto) : NULL;
    char amic[8];
    snprintf(amic, sizeof(amic), "%d", e->amicizia);
    const char *k[4] = { "NATURA", "ABILITÀ", "OGGETTO", "AMICIZIA" };
    const char *v[4] = { Natura_Nome(e->natura), ab ? ab->nome : "-", og ? og->nome : "Nessuno", amic };
    for (int i = 0; i < 4; i++) {
        T_Micro(k[i], ox + 14, oy + 122 + i * 12, PAL_ORO[3]);
        char val[40];
        snprintf(val, sizeof(val), "%s", v[i]);
        /* nomi del database in maiuscolo: come nei mockup, iniziale maiuscola e resto minuscolo */
        for (char *p = val; *p; p++) {
            if (*p == '_') *p = ' ';
            else if (p != val && *p >= 'A' && *p <= 'Z') *p += 32;
        }
        T_Testo(val, ox + 54, oy + 120 + i * 12, PAL_CREMA);
    }
    /* colonna destra */
    G_Cornice(ox + 136, oy + 22, 178, 154, CORNICE_NOTTE, true);
    int x0 = ox + 146;
    if (s->paginaScheda == 1) {
        static const char *NS[6] = { "HP", "ATTACCO", "DIFESA", "ATT. SPEC.", "DIF. SPEC.", "VELOCITÀ" };
        for (int i = 0; i < 6; i++) {
            int y = oy + 32 + i * 17;
            int nat = i == 0 ? 0 : (Natura_Percento(e->natura, (StatId)i) > 100 ? 1 : Natura_Percento(e->natura, (StatId)i) < 100 ? -1 : 0);
            Color col = nat > 0 ? PAL_TIPI[TYPE_FIAMMA][2] : nat < 0 ? PAL_AZZURRO[3] : PAL_CREMA;
            T_TestoOmbra(NS[i], x0, y, col);
            if (nat) Freccia(x0 + 52, y + 1, nat, col);
            char val[16];
            if (i == 0) snprintf(val, sizeof(val), "%d/%d", e->hp, e->stat[0]);
            else snprintf(val, sizeof(val), "%d", e->stat[i]);
            T_TestoOmbra(val, x0 + 80 + (i == 0 ? 6 : 0) - T_Larghezza(val), y, PAL_CREMA);
            float f = i == 0 ? (float)e->hp / (float)e->stat[0] : (float)e->stat[i] / 120.0f;
            G_Barra(x0 + 90, y + 2, 64, f, i == 0 ? G_RampaHp(f) : PAL_XP, NULL, 4);
        }
        G_Separatore(x0, oy + 136, 158);
        int32_t xp = e->xp, prossimo = Elyon_XpPerLivello(sp, e->livello + 1) - e->xp;
        T_Micro("XP", x0, oy + 146, PAL_LUMEN[3]);
        snprintf(t, sizeof(t), "%d", (int)xp);
        T_Testo(t, x0 + 14, oy + 144, PAL_CREMA);
        T_Micro("AL PROSSIMO", x0 + 58, oy + 146, PAL_GRIGIO_UI);
        snprintf(t, sizeof(t), "%d", (int)(prossimo > 0 ? prossimo : 0));
        T_Testo(t, x0 + 108, oy + 144, PAL_CREMA);
        float fx = UI_FrazioneXp(e->specie, e->livello, e->xp);
        G_Barra(x0, oy + 158, 156, fx, PAL_XP, NULL, 3);
    } else if (s->paginaScheda == 2) {
        for (int i = 0; i < MAX_MOSSE_ELYON; i++) {
            const MoveData *m = GetMove(e->mosse[i]);
            int y = oy + 30 + i * 30;
            if (!m) { G_Cristallo(x0 + 6, y + 6, 7, false); T_Micro("- - -", x0 + 16, y + 8, PAL_NOTTE[4]); continue; }
            G_PillolaTipo(m->tipo, x0, y, 58);
            T_TestoOmbra(m->nome, x0 + 64, y + 1, PAL_CREMA);
            char info[48];
            snprintf(info, sizeof(info), "%s  POT %s  PRE %s", Categoria_Nome(m->categoria), m->potenza ? TextFormat("%d", m->potenza) : "-",
                     (m->flag & MOSSA_PRECISIONE_AUTOMATICA) ? "-" : TextFormat("%d", m->precisione));
            T_Micro(info, x0, y + 13, PAL_GRIGIO_UI);
            snprintf(info, sizeof(info), "PP %d/%d", e->pp[i], m->pp);
            T_Micro(info, x0 + 156 - T_LarghezzaMicro(info), y + 13, PAL_CREMA);
            G_Barra(x0, y + 21, 154, m->pp ? (float)e->pp[i] / (float)m->pp : 0, PAL_XP, NULL, 1);
        }
    } else {
        T_Micro(TextFormat("N. %03d", sp->id), x0, oy + 30, PAL_GRIGIO_UI);
        T_TitoloOro(sp->nome, x0, oy + 38);
        G_PillolaTipo(sp->tipo1, x0, oy + 52, 60);
        if (sp->tipo2 != TYPE_NONE) G_PillolaTipo(sp->tipo2, x0 + 64, oy + 52, 60);
        T_ACapo(sp->descrizione, x0, oy + 68, 156, 11, 5, PAL_CREMA, true);
        G_Separatore(x0, oy + 130, 158);
        if (ab) {
            T_Micro("ABILITÀ", x0, oy + 138, PAL_ORO[3]);
            T_ACapo(ab->descrizione, x0, oy + 146, 156, 10, 2, PAL_GRIGIO_UI, false);
        }
    }
    if (s->az.ultimo != DISP_TOUCH && c->h - c->safeB >= oy + 186) {
        T_MicroOmbra(TextFormat("%s %s PAGINA   %s %s ELYON", Azioni_Prompt(&s->az, AZ_SINISTRA), Azioni_Prompt(&s->az, AZ_DESTRA),
                                Azioni_Prompt(&s->az, AZ_SU), Azioni_Prompt(&s->az, AZ_GIU)),
                     ox + 6, oy + 179, PAL_ORO[3], PAL_NOTTE[0]);
    }
}

static void DisegnaZaino(const Slice *s) {
    const Canvas *c = &s->canvas;
    G_Gradiente(0, 0, c->w, c->h, PAL_NOTTE[2], PAL_NOTTE[0], true);
    int ox = Ox(s), oy = Oy(s);
    G_Intestazione(ox + 6, oy + 4, "ZAINO");
    G_Cornice(ox + 6, oy + 22, 186, 132, CORNICE_PERGAMENA, true);
    int lista[ITEM_ID_MAX];
    int n = ListaZaino(s, lista);
    for (int i = 0; i < n && i < 8; i++) {
        Rectangle r = RigaZaino(s, i);
        int x = (int)r.x, y = (int)r.y + 2;
        if (s->cursoreLista == i) {
            G_Rett(x, y - 2, 178, 14, (Color){ PAL_ORO[3].r, PAL_ORO[3].g, PAL_ORO[3].b, 90 });
            G_LineaH(x, y - 2, 178, PAL_ORO[2]); G_LineaH(x, y + 11, 178, PAL_ORO[2]);
            G_Cursore(x + 2, y + 4, s->frame);
        }
        G_IconaOggetto(lista[i], x + 10, y);
        T_Testo(GetItem(lista[i])->nome, x + 26, y + 2, PAL_INCHIOSTRO);
        char q[12];
        snprintf(q, sizeof(q), "×%d", Inventario_Quantita(&s->partita.inventario, lista[i]));
        T_Testo(q, x + 172 - T_Larghezza(q), y + 2, PAL_INCHIOSTRO);
    }
    G_Cornice(ox + 196, oy + 22, 118, 132, CORNICE_NOTTE, true);
    if (n > 0) {
        int id = lista[s->cursoreLista < n ? s->cursoreLista : 0];
        const ItemData *it = GetItem(id);
        int cx = ox + 255, cy = oy + 62;
        G_Bagliore(cx, cy, 22, PAL_LUMEN[3], 50);
        DrawCircle(cx, cy, 20, PAL_NOTTE[1]);
        DrawCircleLines(cx, cy, 20, PAL_NOTTE[3]);
        G_IconaOggetto(id, cx - 5, cy - 5);
        T_TestoOmbra(it->nome, cx - T_Larghezza(it->nome) / 2, oy + 88, PAL_ORO[4]);
        T_ACapo(it->descrizione, ox + 204, oy + 102, 104, 11, 4, PAL_CREMA, false);
    }
    G_Cornice(ox + 6, oy + 158, 308, 19, CORNICE_NOTTE, false);
    if (s->az.ultimo != DISP_TOUCH) {
        Prompt(s, ox + 14, oy + 163, AZ_ANNULLA, "INDIETRO");
        if (n > 0) T_Micro(TextFormat("PREZZO %d", GetItem(lista[s->cursoreLista < n ? s->cursoreLista : 0])->prezzo), ox + 250, oy + 165, PAL_GRIGIO_UI);
    }
}

static void DisegnaAllenatore(const Slice *s) {
    const Canvas *c = &s->canvas;
    G_Oscura(0, 0, c->w, c->h, 140);
    int ox = Ox(s), oy = Oy(s);
    G_Cornice(ox + 30, oy + 24, 260, 132, CORNICE_NOTTE, true);
    G_Intestazione(ox + 38, oy + 18, "SCHEDA");
    /* ritratto: personaggio del giocatore (fronte) ingrandito x2 */
    DrawTexturePro(Gfx_Tex(TX_PERSONAGGI), (Rectangle){ 0, 8 * 24, 16, 24 }, (Rectangle){ (float)(ox + 46), (float)(oy + 44), 32, 48 }, (Vector2){ 0, 0 }, 0, WHITE);
    T_TitoloOro(s->partita.nomeGiocatore, ox + 92, oy + 44);
    const char *k[4] = { "TEMPO DI GIOCO", "LUMEN", "SCOPERTI", "SIGILLI" };
    unsigned t = s->partita.secondiGioco;
    char v[4][24];
    snprintf(v[0], sizeof(v[0]), "%u:%02u", t / 3600, (t / 60) % 60);
    snprintf(v[1], sizeof(v[1]), "%d", s->partita.inventario.lumen);
    snprintf(v[2], sizeof(v[2]), "%d", Scoperti(s));
    snprintf(v[3], sizeof(v[3]), "0 / 7");
    for (int i = 0; i < 4; i++) {
        T_Micro(k[i], ox + 92, oy + 62 + i * 13, PAL_ORO[3]);
        T_Testo(v[i], ox + 190, oy + 60 + i * 13, PAL_CREMA);
    }
    static const ElementType sig[7] = { TYPE_NATURA, TYPE_IMPULSO, TYPE_ACQUA, TYPE_FIAMMA, TYPE_GELO, TYPE_ROCCIA, TYPE_ASTRALE };
    for (int i = 0; i < 7; i++) G_Cristallo(ox + 60 + i * 30, oy + 122, 11, false), (void)sig[i];
    T_Micro("I SETTE SIGILLI DI ASTERRA", ox + 160 - T_LarghezzaMicro("I SETTE SIGILLI DI ASTERRA") / 2, oy + 138, PAL_GRIGIO_UI);
}

void Pannelli_Disegna(const Slice *s) {
    switch (s->pannello) {
    case PAN_MENU: DisegnaMenu(s); break;
    case PAN_SQUADRA: DisegnaSquadra(s); break;
    case PAN_SCHEDA_ELYON: DisegnaScheda(s); break;
    case PAN_ZAINO: DisegnaZaino(s); break;
    case PAN_ALLENATORE: DisegnaAllenatore(s); break;
    case PAN_BESTIARIO: Bestiario_Disegna(s); break;
    case PAN_MAPPA: Mappa_Disegna(s); break;
    case PAN_AVVISO:
        if (s->pannelloDopoAvviso != PAN_NESSUNO) {
            Slice *t = (Slice *)s;
            Pannello p = t->pannello;
            t->pannello = t->pannelloDopoAvviso;
            Pannelli_Disegna(t);
            t->pannello = p;
        }
        VistaDialogo_Testo("", s->avviso, &s->canvas, s->frame, true);
        break;
    default: break;
    }
}
