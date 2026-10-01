/* schermate.c */
#include "schermate.h"
#include "graphics.h"
#include "item_use.h"
#include "ui_common.h"
#include "data/abilities_data.h"
#include <stdio.h>
#include <string.h>

#define COL_TITOLO (Color){ 40, 88, 152, 255 }
#define COL_GRIGIO (Color){ 140, 140, 150, 255 }

void Schermate_Apri(Schermate *s, TipoSchermata tipo, Partita *p) {
    memset(s, 0, sizeof(*s));
    s->tipo = tipo;
    s->partita = p;
}

static void Messaggio(Schermate *s, const char *testo) {
    snprintf(s->messaggio, sizeof(s->messaggio), "%s", testo);
    s->fasePrimaMessaggio = s->fase;
    s->fase = FASE_MESSAGGIO;
}

static int CursoreCiclico(int c, int n, const Input *in) {
    if (n <= 0) return 0;
    if (in->suP) c = (c + n - 1) % n;
    if (in->giuP) c = (c + 1) % n;
    return c;
}

/* ------------------------------------------------------------------ squadra */

static bool AggiornaSquadra(Schermate *s, const Input *in) {
    Collezione *c = &s->partita->collezione;
    int n = Collezione_NumSquadra(c);
    switch (s->fase) {
    case FASE_LISTA:
        if (in->indietro || in->menu) return false;
        s->cursore = CursoreCiclico(s->cursore, n, in);
        if (in->conferma && n > 0) { s->fase = FASE_AZIONI; s->cursoreAzioni = 0; }
        break;
    case FASE_AZIONI:
        if (in->indietro) { s->fase = FASE_LISTA; break; }
        s->cursoreAzioni = CursoreCiclico(s->cursoreAzioni, 3, in);
        if (in->conferma) {
            if (s->cursoreAzioni == 0) s->fase = FASE_RIEPILOGO;
            else if (s->cursoreAzioni == 1) { s->fase = FASE_SPOSTA; s->spostaDa = s->cursore; }
            else s->fase = FASE_LISTA;
        }
        break;
    case FASE_SPOSTA:
        if (in->indietro) { s->fase = FASE_LISTA; break; }
        s->cursore = CursoreCiclico(s->cursore, n, in);
        if (in->conferma) {
            Elyon tmp = c->squadra[s->spostaDa];
            c->squadra[s->spostaDa] = c->squadra[s->cursore];
            c->squadra[s->cursore] = tmp;
            s->fase = FASE_LISTA;
        }
        break;
    case FASE_RIEPILOGO:
        if (in->indietro || in->conferma) s->fase = FASE_LISTA;
        if (in->sinistraP) s->cursore = (s->cursore + n - 1) % n;
        if (in->destraP) s->cursore = (s->cursore + 1) % n;
        break;
    case FASE_MESSAGGIO:
        if (in->conferma || in->indietro) s->fase = s->fasePrimaMessaggio;
        break;
    default:
        s->fase = FASE_LISTA;
        break;
    }
    return true;
}

static void DisegnaRiepilogo(const Elyon *e) {
    const SpeciesData *sp = GetSpecies(e->specie);
    Gfx_Finestra(0, 0, LOGICAL_W, LOGICAL_H);
    DrawRectangle(8, 8, 92, 92, (Color){ 216, 232, 248, 255 });
    UI_SpriteElyon(e->specie, 22, 20, false, 1.0f, WHITE);
    if (e->varianteLumen) DrawText("LUMEN", 12, 10, 10, (Color){ 200, 160, 40, 255 });
    Gfx_Testo(TextFormat("#%03d %s", sp->id, Elyon_Nome(e)), 108, 8, COL_TITOLO);
    Gfx_Testo(TextFormat("Lv %d", e->livello), 270, 8, COL_TESTO);
    UI_EtichettaTipo(sp->tipo1, 108, 22);
    if (sp->tipo2 != TYPE_NONE) UI_EtichettaTipo(sp->tipo2, 156, 22);
    UI_Status(e->status, 206, 23);
    const AbilityData *ab = GetAbilityResolved(sp->abilita);
    Gfx_Testo(TextFormat("Natura: %s", Natura_Nome(e->natura)), 108, 38, COL_TESTO);
    Gfx_Testo(TextFormat("Abilità: %s", ab ? ab->nome : "-"), 108, 50, COL_TESTO);
    int32_t prossimo = e->livello < LIVELLO_MAX ? Elyon_XpPerLivello(sp, e->livello + 1) - e->xp : 0;
    Gfx_Testo(TextFormat("XP %d  (al prossimo: %d)", (int)e->xp, (int)prossimo), 108, 62, COL_TESTO);
    UI_BarraXp(110, 76, 120, UI_FrazioneXp(e->specie, e->livello, e->xp));
    Gfx_Testo(TextFormat("Amicizia %d", e->amicizia), 108, 82, COL_TESTO);

    static const char *nomi[NUM_STAT] = { "HP", "ATK", "DEF", "SPA", "SPD", "SPE" };
    for (int k = 0; k < NUM_STAT; k++) {
        int y = 104 + k * 11;
        DrawText(nomi[k], 12, y, 10, COL_TESTO);
        int pct = Natura_Percento(e->natura, (StatId)k);
        Color c = pct > 100 ? (Color){ 200, 64, 48, 255 } : pct < 100 ? (Color){ 64, 96, 200, 255 } : COL_TESTO;
        if (k == STAT_HP) DrawText(TextFormat("%d/%d", e->hp, e->stat[STAT_HP]), 40, y, 10, c);
        else DrawText(TextFormat("%d", e->stat[k]), 40, y, 10, c);
    }
    for (int i = 0; i < MAX_MOSSE_ELYON; i++) {
        const MoveData *m = GetMove(e->mosse[i]);
        int y = 104 + i * 15;
        if (!m) { Gfx_Testo("-", 120, y, COL_GRIGIO); continue; }
        UI_EtichettaTipo(m->tipo, 104, y);
        Gfx_Testo(m->nome, 152, y, COL_TESTO);
        DrawText(TextFormat("PP %d/%d", e->pp[i], m->pp), 262, y + 1, 10, COL_TESTO);
    }
    DrawText("<- -> altro Elyon   X: indietro", 104, 165, 10, COL_GRIGIO);
}

static void DisegnaSquadra(const Schermate *s) {
    const Collezione *c = &s->partita->collezione;
    if (s->fase == FASE_RIEPILOGO) { DisegnaRiepilogo(&c->squadra[s->cursore]); return; }
    Gfx_Finestra(0, 0, LOGICAL_W, LOGICAL_H);
    Gfx_Testo(s->fase == FASE_SPOSTA ? "Con chi scambiarlo?" : "SQUADRA", 10, 6, COL_TITOLO);
    int n = Collezione_NumSquadra(c);
    if (n == 0) Gfx_Testo("Non hai ancora nessun Elyon.", 16, 30, COL_GRIGIO);
    for (int i = 0; i < n; i++)
        UI_RigaSquadra(&c->squadra[i], 8, 20 + i * 25, LOGICAL_W - 16, i == s->cursore, s->fase == FASE_SPOSTA && i == s->spostaDa);
    if (s->fase == FASE_AZIONI) {
        static const char *voci[3] = { "RIEPILOGO", "SPOSTA", "INDIETRO" };
        Gfx_Finestra(LOGICAL_W - 96, LOGICAL_H - 56, 92, 52);
        for (int i = 0; i < 3; i++) {
            Gfx_Testo(voci[i], LOGICAL_W - 80, LOGICAL_H - 50 + i * 14, COL_TESTO);
            if (i == s->cursoreAzioni) Gfx_Cursore(LOGICAL_W - 88, LOGICAL_H - 49 + i * 14);
        }
    }
}

/* ------------------------------------------------------------------ zaino */

static int OggettiPosseduti(const Inventario *inv, int *lista) {
    int n = 0;
    for (int id = 1; id < ITEM_ID_MAX; id++) if (GetItem(id) && inv->quantita[id] > 0) lista[n++] = id;
    return n;
}

static void UsaSuBersaglio(Schermate *s, int slotMossa) {
    Elyon *e = &s->partita->collezione.squadra[s->cursoreBersaglio];
    char msg[96];
    if (Oggetto_Usa(e, s->oggetto, slotMossa, msg, sizeof(msg))) {
        Inventario_Rimuovi(&s->partita->inventario, s->oggetto, 1);
        s->fase = FASE_LISTA;
        Messaggio(s, msg);
    } else {
        s->fase = FASE_BERSAGLIO;
        Messaggio(s, "Non avrebbe alcun effetto.");
    }
}

static bool AggiornaZaino(Schermate *s, const Input *in) {
    int lista[ITEM_ID_MAX];
    int n = OggettiPosseduti(&s->partita->inventario, lista);
    Collezione *c = &s->partita->collezione;
    switch (s->fase) {
    case FASE_LISTA:
        if (in->indietro || in->menu) return false;
        if (s->cursore >= n) s->cursore = n > 0 ? n - 1 : 0;
        s->cursore = CursoreCiclico(s->cursore, n, in);
        if (in->conferma && n > 0) {
            s->oggetto = lista[s->cursore];
            if (!Oggetto_SuElyon(s->oggetto)) Messaggio(s, Oggetto_FattoreCore(s->oggetto) > 0 ? "Si usa durante le lotte contro gli Elyon selvatici." : "Non puoi usarlo ora.");
            else if (Collezione_NumSquadra(c) == 0) Messaggio(s, "Non hai Elyon su cui usarlo.");
            else { s->fase = FASE_BERSAGLIO; s->cursoreBersaglio = 0; }
        }
        break;
    case FASE_BERSAGLIO:
        if (in->indietro) { s->fase = FASE_LISTA; break; }
        s->cursoreBersaglio = CursoreCiclico(s->cursoreBersaglio, Collezione_NumSquadra(c), in);
        if (in->conferma) {
            if (Oggetto_RichiedeMossa(s->oggetto)) { s->fase = FASE_MOSSA; s->cursoreMossa = 0; }
            else UsaSuBersaglio(s, 0);
        }
        break;
    case FASE_MOSSA: {
        if (in->indietro) { s->fase = FASE_BERSAGLIO; break; }
        s->cursoreMossa = CursoreCiclico(s->cursoreMossa, Elyon_NumMosse(&c->squadra[s->cursoreBersaglio]), in);
        if (in->conferma) UsaSuBersaglio(s, s->cursoreMossa);
        break;
    }
    case FASE_MESSAGGIO:
        if (in->conferma || in->indietro) s->fase = s->fasePrimaMessaggio == FASE_MESSAGGIO ? FASE_LISTA : s->fasePrimaMessaggio;
        break;
    default:
        s->fase = FASE_LISTA;
        break;
    }
    return true;
}

static void DisegnaZaino(const Schermate *s) {
    const Inventario *inv = &s->partita->inventario;
    const Collezione *c = &s->partita->collezione;
    int lista[ITEM_ID_MAX];
    int n = OggettiPosseduti(inv, lista);
    if (s->fase == FASE_BERSAGLIO || s->fase == FASE_MOSSA || (s->fase == FASE_MESSAGGIO && s->fasePrimaMessaggio == FASE_BERSAGLIO)) {
        Gfx_Finestra(0, 0, LOGICAL_W, LOGICAL_H);
        Gfx_Testo(TextFormat("Su chi usare %s?", GetItem(s->oggetto)->nome), 10, 6, COL_TITOLO);
        for (int i = 0; i < Collezione_NumSquadra(c); i++)
            UI_RigaSquadra(&c->squadra[i], 8, 20 + i * 25, LOGICAL_W - 16, i == s->cursoreBersaglio, false);
        if (s->fase == FASE_MOSSA) {
            const Elyon *e = &c->squadra[s->cursoreBersaglio];
            Gfx_Finestra(150, 40, 166, 76);
            for (int i = 0; i < Elyon_NumMosse(e); i++) {
                const MoveData *m = GetMove(e->mosse[i]);
                Gfx_Testo(m->nome, 168, 50 + i * 14, COL_TESTO);
                Gfx_Testo(TextFormat("%d/%d", e->pp[i], m->pp), 276, 50 + i * 14, COL_TESTO);
                if (i == s->cursoreMossa) Gfx_Cursore(160, 51 + i * 14);
            }
        }
    } else {
        Gfx_Finestra(0, 0, LOGICAL_W, LOGICAL_H);
        Gfx_Testo("ZAINO", 10, 6, COL_TITOLO);
        Gfx_Testo(TextFormat("LUMEN: %d", inv->lumen), LOGICAL_W - 110, 6, COL_TESTO);
        if (n == 0) Gfx_Testo("Lo zaino è vuoto.", 20, 26, COL_GRIGIO);
        int inizio = s->cursore > 7 ? s->cursore - 7 : 0;
        for (int i = inizio; i < n && i < inizio + 8; i++) {
            int y = 22 + (i - inizio) * 13;
            Gfx_Testo(GetItem(lista[i])->nome, 22, y, COL_TESTO);
            Gfx_Testo(TextFormat("x%d", inv->quantita[lista[i]]), 150, y, COL_TESTO);
            if (i == s->cursore) Gfx_Cursore(12, y + 1);
        }
        DrawRectangle(176, 22, 136, 100, (Color){ 232, 240, 248, 255 });
        if (n > 0) UI_TestoACapo(GetItem(lista[s->cursore < n ? s->cursore : 0])->descrizione, 180, 26, 128, 7, COL_TESTO);
    }
    if (s->fase == FASE_MESSAGGIO) {
        Gfx_Finestra(4, LOGICAL_H - 44, LOGICAL_W - 8, 40);
        UI_TestoACapo(s->messaggio, 14, LOGICAL_H - 36, LOGICAL_W - 30, 2, COL_TESTO);
    }
}

/* ------------------------------------------------------------------ scheda */

static void DisegnaScheda(const Schermate *s) {
    const Partita *p = s->partita;
    Gfx_Finestra(16, 12, LOGICAL_W - 32, LOGICAL_H - 24);
    DrawRectangle(19, 15, LOGICAL_W - 38, 18, (Color){ 72, 120, 184, 255 });
    Gfx_Testo("SCHEDA ESPLORATORE DEL LUMEN", 28, 19, COL_BIANCO_UI);
    Gfx_Testo(TextFormat("NOME:  %s", p->nomeGiocatore), 30, 44, COL_TESTO);
    Gfx_Testo(TextFormat("LUMEN:  %d", p->inventario.lumen), 30, 58, COL_TESTO);
    uint32_t t = p->secondiGioco;
    Gfx_Testo(TextFormat("TEMPO DI GIOCO:  %u:%02u", t / 3600, (t / 60) % 60), 30, 72, COL_TESTO);
    Gfx_Testo(TextFormat("ELYON:  %d in squadra, %d nel box", Collezione_NumSquadra(&p->collezione),
                         Collezione_NumBox(&p->collezione)), 30, 86, COL_TESTO);
    int sigilli = 0;
    for (int i = 1; i <= 7; i++) sigilli += Flags_Attivo(&p->flags, TextFormat("SIGILLO_%d", i));
    Gfx_Testo(TextFormat("SIGILLI:  %d / 7", sigilli), 30, 100, COL_TESTO);
    for (int i = 0; i < 7; i++) {
        bool ok = i < sigilli;
        DrawCircle(44 + i * 34, 130, 9, ok ? (Color){ 120, 232, 208, 255 } : (Color){ 200, 208, 216, 255 });
        DrawCircleLines(44 + i * 34, 130, 9, COL_CORNICE);
    }
    DrawText("Z / X: chiudi", LOGICAL_W - 100, LOGICAL_H - 26, 10, COL_GRIGIO);
}

/* ------------------------------------------------------------------ PC */

static bool AggiornaPc(Schermate *s, const Input *in) {
    Collezione *c = &s->partita->collezione;
    if (s->fase == FASE_MESSAGGIO) {
        if (in->conferma || in->indietro) s->fase = FASE_LISTA;
        return true;
    }
    if (in->indietro || in->menu) return false;
    if (s->pcPannello == 0) {
        int n = Collezione_NumSquadra(c);
        s->cursore = CursoreCiclico(s->cursore, n, in);
        if (in->destraP) s->pcPannello = 1;
        if (in->conferma) {
            if (n <= 1) Messaggio(s, "Non puoi depositare il tuo ultimo Elyon!");
            else if (Collezione_NumBox(c) >= NUM_BOX * POSTI_BOX) Messaggio(s, "Il box è pieno.");
            else {
                char nome[24];
                snprintf(nome, sizeof(nome), "%s", Elyon_Nome(&c->squadra[s->cursore]));
                if (Collezione_Deposita(c, s->cursore)) Messaggio(s, TextFormat("%s è stato depositato nel box.", nome));
                if (s->cursore >= Collezione_NumSquadra(c)) s->cursore = Collezione_NumSquadra(c) - 1;
            }
        }
    } else {
        int x = s->pcPosto % 5, y = s->pcPosto / 5;
        if (in->sinistraP) { if (x == 0) s->pcPannello = 0; else x--; }
        if (in->destraP) { if (x == 4) s->pcBox = (s->pcBox + 1) % NUM_BOX; else x++; }
        if (in->suP) y = (y + 3) % 4;
        if (in->giuP) y = (y + 1) % 4;
        s->pcPosto = y * 5 + x;
        if (in->conferma) {
            Elyon *e = &c->box[s->pcBox][s->pcPosto];
            if (Elyon_Vuoto(e)) Messaggio(s, "Questo posto è vuoto.");
            else if (Collezione_NumSquadra(c) >= MAX_SQUADRA) Messaggio(s, "La squadra è al completo (6 Elyon).");
            else {
                char nome[24];
                snprintf(nome, sizeof(nome), "%s", Elyon_Nome(e));
                if (Collezione_Ritira(c, s->pcBox, s->pcPosto)) Messaggio(s, TextFormat("%s si unisce alla squadra.", nome));
            }
        }
    }
    return true;
}

static void DisegnaPc(const Schermate *s) {
    const Collezione *c = &s->partita->collezione;
    DrawRectangle(0, 0, LOGICAL_W, LOGICAL_H, (Color){ 72, 96, 136, 255 });
    Gfx_Finestra(2, 2, 116, LOGICAL_H - 4);
    Gfx_Testo("SQUADRA", 12, 8, COL_TITOLO);
    for (int i = 0; i < Collezione_NumSquadra(c); i++) {
        const Elyon *e = &c->squadra[i];
        int y = 22 + i * 24;
        bool sel = s->pcPannello == 0 && s->cursore == i;
        if (sel) DrawRectangle(8, y - 2, 104, 22, (Color){ 248, 232, 160, 255 });
        UI_SpriteElyon(e->specie, 8, y - 6, false, 0.4f, WHITE);
        Gfx_Testo(Elyon_Nome(e), 36, y, COL_TESTO);
        DrawText(TextFormat("Lv%d", e->livello), 36, y + 10, 10, COL_TESTO);
    }
    Gfx_Finestra(120, 2, 198, LOGICAL_H - 4);
    Gfx_Testo(TextFormat("BOX %d / %d", s->pcBox + 1, NUM_BOX), 132, 8, COL_TITOLO);
    DrawText(TextFormat("%d Elyon", Collezione_NumBox(c)), 250, 9, 10, COL_GRIGIO);
    for (int p = 0; p < POSTI_BOX; p++) {
        int x = 130 + (p % 5) * 36, y = 24 + (p / 5) * 34;
        bool sel = s->pcPannello == 1 && s->pcPosto == p;
        DrawRectangle(x, y, 32, 30, sel ? (Color){ 248, 232, 160, 255 } : (Color){ 224, 232, 244, 255 });
        DrawRectangleLines(x, y, 32, 30, sel ? (Color){ 224, 72, 64, 255 } : (Color){ 176, 188, 208, 255 });
        const Elyon *e = &c->box[s->pcBox][p];
        if (!Elyon_Vuoto(e)) UI_SpriteElyon(e->specie, x, y - 2, false, 0.5f, WHITE);
    }
    const Elyon *info = s->pcPannello == 0 ? &c->squadra[s->cursore] : &c->box[s->pcBox][s->pcPosto];
    if (!Elyon_Vuoto(info))
        DrawText(TextFormat("%s  Lv%d  HP %d/%d", Elyon_Nome(info), info->livello, info->hp, info->stat[STAT_HP]), 130, 162, 10, COL_TESTO);
    else
        DrawText("Z: deposita / ritira   frecce: box", 130, 162, 10, COL_GRIGIO);
    if (s->fase == FASE_MESSAGGIO) {
        Gfx_Finestra(4, LOGICAL_H - 44, LOGICAL_W - 8, 40);
        UI_TestoACapo(s->messaggio, 14, LOGICAL_H - 36, LOGICAL_W - 30, 2, COL_TESTO);
    }
}

/* ------------------------------------------------------------------ */

bool Schermate_Aggiorna(Schermate *s, const Input *in) {
    s->frame++;
    switch (s->tipo) {
    case SCH_SQUADRA: return AggiornaSquadra(s, in);
    case SCH_ZAINO:   return AggiornaZaino(s, in);
    case SCH_SCHEDA:  return !(in->conferma || in->indietro || in->menu);
    case SCH_PC:      return AggiornaPc(s, in);
    default:          return false;
    }
}

void Schermate_Disegna(const Schermate *s) {
    switch (s->tipo) {
    case SCH_SQUADRA: DisegnaSquadra(s); break;
    case SCH_ZAINO:   DisegnaZaino(s); break;
    case SCH_SCHEDA:  DisegnaScheda(s); break;
    case SCH_PC:      DisegnaPc(s); break;
    default: break;
    }
}
