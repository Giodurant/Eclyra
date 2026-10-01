/* ui_common.c */
#include "ui_common.h"
#include <stdio.h>
#include <string.h>

Color UI_ColoreTipo(ElementType t) {
    switch (t) {
    case TYPE_FIAMMA:  return (Color){ 232, 96, 48, 255 };
    case TYPE_ACQUA:   return (Color){ 64, 136, 224, 255 };
    case TYPE_NATURA:  return (Color){ 72, 168, 72, 255 };
    case TYPE_IMPULSO: return (Color){ 216, 176, 32, 255 };
    case TYPE_VENTO:   return (Color){ 112, 176, 192, 255 };
    case TYPE_ROCCIA:  return (Color){ 160, 124, 84, 255 };
    case TYPE_GELO:    return (Color){ 96, 184, 224, 255 };
    case TYPE_OMBRA:   return (Color){ 88, 64, 128, 255 };
    case TYPE_ASTRALE: return (Color){ 200, 168, 72, 255 };
    case TYPE_TOSSICO: return (Color){ 152, 80, 176, 255 };
    case TYPE_PSICHE:  return (Color){ 224, 104, 168, 255 };
    case TYPE_METALLO: return (Color){ 136, 144, 160, 255 };
    default:           return (Color){ 152, 152, 144, 255 };
    }
}

void UI_EtichettaTipo(ElementType t, int x, int y) {
    Color c = UI_ColoreTipo(t);
    DrawRectangle(x, y, 44, 11, c);
    DrawRectangleLines(x, y, 44, 11, (Color){ c.r / 2, c.g / 2, c.b / 2, 255 });
    const char *n = t == TYPE_NONE ? "NEUTRO" : Type_Nome(t);
    int w = MeasureText(n, 10);
    DrawText(n, x + 22 - w / 2, y + 1, 10, WHITE);
}

void UI_BarraHp(int x, int y, int w, float hp, int max) {
    float f = max > 0 ? hp / (float)max : 0.0f;
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    Color c = f > 0.5f ? (Color){ 88, 208, 96, 255 } : f > 0.2f ? (Color){ 240, 192, 48, 255 } : (Color){ 232, 72, 56, 255 };
    DrawRectangle(x - 1, y - 1, w + 2, 6, (Color){ 64, 64, 72, 255 });
    DrawRectangle(x, y, w, 4, (Color){ 72, 80, 80, 255 });
    int pieno = (int)(f * (float)w + 0.5f);
    if (hp > 0 && pieno < 1) pieno = 1;
    DrawRectangle(x, y, pieno, 4, c);
    DrawRectangle(x, y, pieno, 1, (Color){ (unsigned char)(c.r / 2 + 127), (unsigned char)(c.g / 2 + 127), (unsigned char)(c.b / 2 + 127), 255 });
}

void UI_BarraXp(int x, int y, int w, float f) {
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    DrawRectangle(x - 1, y - 1, w + 2, 4, (Color){ 64, 64, 72, 255 });
    DrawRectangle(x, y, (int)(f * (float)w), 2, (Color){ 72, 160, 240, 255 });
}

float UI_FrazioneXp(int specie, int livello, int32_t xp) {
    const SpeciesData *s = GetSpecies(specie);
    if (!s || livello >= LIVELLO_MAX) return 0.0f;
    int32_t a = Elyon_XpPerLivello(s, livello), b = Elyon_XpPerLivello(s, livello + 1);
    if (b <= a) return 0.0f;
    return (float)(xp - a) / (float)(b - a);
}

const char *UI_SiglaStatus(StatusId s) {
    switch (s) {
    case STATUS_VELENO:       return "VEL";
    case STATUS_VELENO_GRAVE: return "VEL+";
    case STATUS_BRUCIATURA:   return "BRU";
    case STATUS_PARALISI:     return "PAR";
    case STATUS_SONNO:        return "SON";
    case STATUS_GELO:         return "GEL";
    default:                  return "";
    }
}

void UI_Status(StatusId s, int x, int y) {
    if (s == STATUS_NESSUNO || s == STATUS_CONFUSIONE) return;
    static const Color col[] = { { 0 }, { 168, 72, 176, 255 }, { 120, 40, 140, 255 }, { 232, 104, 48, 255 },
                                 { 216, 176, 32, 255 }, { 128, 128, 136, 255 }, { 96, 176, 232, 255 } };
    DrawRectangle(x, y, 22, 9, col[s]);
    DrawText(UI_SiglaStatus(s), x + 2, y, 10, WHITE);
}

void UI_SpriteElyon(int specie, int x, int y, bool retro, float scala, Color tinta) {
    if (specie < 1 || specie > 100) return;
    Texture2D t = Gfx_Texture(retro ? TEX_ELYON_RETRO : TEX_ELYON_FRONTE);
    Rectangle src = { (float)((specie - 1) % 10 * 64), (float)((specie - 1) / 10 * 64), 64, 64 };
    Rectangle dst = { (float)x, (float)y, 64 * scala, 64 * scala };
    DrawTexturePro(t, src, dst, (Vector2){ 0, 0 }, 0, tinta);
}

void UI_RigaSquadra(const Elyon *e, int x, int y, int w, bool selezionata, bool evidenziata) {
    Color sfondo = selezionata ? (Color){ 248, 232, 160, 255 } : (Color){ 232, 240, 248, 255 };
    if (evidenziata) sfondo = (Color){ 200, 232, 200, 255 };
    DrawRectangle(x, y, w, 22, sfondo);
    DrawRectangleLines(x, y, w, 22, selezionata ? (Color){ 224, 72, 64, 255 } : (Color){ 160, 176, 200, 255 });
    if (!e || Elyon_Vuoto(e)) {
        DrawText("-", x + 8, y + 6, 10, (Color){ 160, 160, 168, 255 });
        return;
    }
    UI_SpriteElyon(e->specie, x + 1, y - 5, false, 0.4f, Elyon_Esausto(e) ? GRAY : WHITE);
    Gfx_Testo(Elyon_Nome(e), x + 28, y + 2, COL_TESTO);
    Gfx_Testo(TextFormat("Lv%d", e->livello), x + 28, y + 12, COL_TESTO);
    int bx = x + w - 74;
    UI_BarraHp(bx, y + 4, 64, (float)e->hp, e->stat[STAT_HP]);
    DrawText(TextFormat("%d/%d", e->hp, e->stat[STAT_HP]), bx + 18, y + 11, 10, COL_TESTO);
    if (Elyon_Esausto(e)) DrawText("KO", bx - 12, y + 11, 10, (Color){ 200, 60, 60, 255 });
    else UI_Status(e->status, bx - 24, y + 12);
}

int UI_TestoACapo(const char *testo, int x, int y, int larghezza, int maxRighe, Color colore) {
    char riga[256] = "", parola[96];
    int righe = 0;
    const char *p = testo;
    while (*p && righe < maxRighe) {
        int n = 0;
        while (*p == ' ') p++;
        if (*p == '\n') { p++; Gfx_Testo(riga, x, y + righe * 12, colore); riga[0] = '\0'; righe++; continue; }
        while (*p && *p != ' ' && *p != '\n' && n < (int)sizeof(parola) - 1) parola[n++] = *p++;
        parola[n] = '\0';
        if (n == 0) break;
        char prova[400];
        snprintf(prova, sizeof(prova), "%s%s%s", riga, riga[0] ? " " : "", parola);
        if (Gfx_LarghezzaTesto(prova, 10) > larghezza && riga[0]) {
            Gfx_Testo(riga, x, y + righe * 12, colore);
            righe++;
            snprintf(riga, sizeof(riga), "%s", parola);
        } else {
            Utils_StrCopy(riga, sizeof(riga), prova);
        }
    }
    if (riga[0] && righe < maxRighe) { Gfx_Testo(riga, x, y + righe * 12, colore); righe++; }
    return righe;
}
