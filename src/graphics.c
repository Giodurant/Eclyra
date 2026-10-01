/* graphics.c */
#include "graphics.h"
#include <stdio.h>

static RenderTexture2D schermo;
static Texture2D texture[TEX_COUNT];
static bool texValida[TEX_COUNT];

static const char *FILE_TEXTURE[TEX_COUNT] = {
    "assets/tilesets/tileset.png",
    "assets/tilesets/albero.png",
    "assets/tilesets/casa.png",
    "assets/sprites/giocatore.png",
};

/* Se un PNG manca il gioco non si blocca: usa una scacchiera magenta ben visibile */
static Texture2D TextureSegnaposto(void) {
    Image im = GenImageChecked(64, 64, 8, 8, MAGENTA, BLACK);
    Texture2D t = LoadTextureFromImage(im);
    UnloadImage(im);
    return t;
}

static Texture2D CaricaTexture(const char *relativo, bool *ok) {
    char percorso[1024];
    /* Gli asset vengono copiati accanto all'eseguibile da CMake */
    snprintf(percorso, sizeof(percorso), "%s%s", GetApplicationDirectory(), relativo);
    if (!FileExists(percorso)) snprintf(percorso, sizeof(percorso), "%s", relativo);
    *ok = FileExists(percorso);
    if (!*ok) {
        TraceLog(LOG_WARNING, "ECLYRA: asset mancante %s, uso un segnaposto", relativo);
        return TextureSegnaposto();
    }
    Texture2D t = LoadTexture(percorso);
    SetTextureFilter(t, TEXTURE_FILTER_POINT);
    return t;
}

bool Gfx_Init(const char *titolo) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(LOGICAL_W * 4, LOGICAL_H * 4, titolo);
    if (!IsWindowReady()) return false;
    SetWindowMinSize(LOGICAL_W, LOGICAL_H);
    SetExitKey(KEY_NULL);   /* Esc serve per "indietro", non per chiudere */

    schermo = LoadRenderTexture(LOGICAL_W, LOGICAL_H);
    SetTextureFilter(schermo.texture, TEXTURE_FILTER_POINT);
    for (int i = 0; i < TEX_COUNT; i++) texture[i] = CaricaTexture(FILE_TEXTURE[i], &texValida[i]);
    SetTextLineSpacing(4);
    return true;
}

void Gfx_Shutdown(void) {
    for (int i = 0; i < TEX_COUNT; i++) UnloadTexture(texture[i]);
    UnloadRenderTexture(schermo);
    CloseWindow();
}

int Gfx_Scala(void) {
    int sx = GetScreenWidth() / LOGICAL_W;
    int sy = GetScreenHeight() / LOGICAL_H;
    int s = sx < sy ? sx : sy;
    return s < 1 ? 1 : s;
}

void Gfx_InizioFrame(void) {
    BeginTextureMode(schermo);
    ClearBackground(BLACK);
}

void Gfx_FineFrame(void) {
    EndTextureMode();
    int s = Gfx_Scala();
    int w = LOGICAL_W * s, h = LOGICAL_H * s;
    int ox = (GetScreenWidth() - w) / 2, oy = (GetScreenHeight() - h) / 2;
    BeginDrawing();
    ClearBackground(BLACK);
    /* altezza negativa: le render texture sono capovolte in OpenGL */
    DrawTexturePro(schermo.texture, (Rectangle){ 0, 0, LOGICAL_W, -LOGICAL_H },
                   (Rectangle){ (float)ox, (float)oy, (float)w, (float)h }, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

void Gfx_SchermoIntero(bool attivo) {
    if (attivo != IsWindowFullscreen()) ToggleFullscreen();
}

bool Gfx_IsSchermoIntero(void) { return IsWindowFullscreen(); }

Texture2D Gfx_Texture(TexId id) { return texture[id]; }

void Gfx_Testo(const char *testo, int x, int y, Color colore) {
    Font f = GetFontDefault();
    DrawTextEx(f, testo, (Vector2){ (float)x + 1, (float)y + 1 }, 10, 1, COL_OMBRA_TESTO);
    DrawTextEx(f, testo, (Vector2){ (float)x, (float)y }, 10, 1, colore);
}

int Gfx_LarghezzaTesto(const char *testo, int dimensione) {
    return (int)MeasureTextEx(GetFontDefault(), testo, (float)dimensione, (float)dimensione / 10.0f).x;
}

void Gfx_TestoCentrato(const char *testo, int cx, int y, int dimensione, Color colore) {
    int w = Gfx_LarghezzaTesto(testo, dimensione);
    DrawTextEx(GetFontDefault(), testo, (Vector2){ (float)(cx - w / 2), (float)y }, (float)dimensione,
               (float)dimensione / 10.0f, colore);
}

void Gfx_Finestra(int x, int y, int w, int h) {
    DrawRectangle(x + 1, y, w - 2, h, COL_CORNICE);    /* angoli smussati */
    DrawRectangle(x, y + 1, w, h - 2, COL_CORNICE);
    DrawRectangle(x + 2, y + 2, w - 4, h - 4, COL_BORDO_UI);
    DrawRectangle(x + 3, y + 3, w - 6, h - 6, COL_BIANCO_UI);
}

void Gfx_Cursore(int x, int y) {
    /* triangolo che punta a destra (vertici in senso antiorario) */
    DrawTriangle((Vector2){ (float)x, (float)y }, (Vector2){ (float)x, (float)y + 7 },
                 (Vector2){ (float)x + 4, (float)y + 3.5f }, (Color){ 224, 72, 64, 255 });
}

void Gfx_Dissolvenza(float q) {
    if (q <= 0.0f) return;
    if (q > 1.0f) q = 1.0f;
    DrawRectangle(0, 0, LOGICAL_W, LOGICAL_H, (Color){ 0, 0, 0, (unsigned char)(q * 255) });
}

void Gfx_EsportaFrame(const char *file) {
    Image im = LoadImageFromTexture(schermo.texture);
    ImageFlipVertical(&im);
    ExportImage(im, file);
    UnloadImage(im);
}
