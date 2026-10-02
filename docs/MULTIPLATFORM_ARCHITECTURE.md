# ECLYRA — Architettura multipiattaforma

Versione 1 (proposta in revisione). Obiettivo: **una sola logica di gioco** per Steam (Windows e Linux),
Steam Deck, Android, iOS e iPadOS, mantenendo C99 + raylib + CMake.

Il mobile non è un adattamento successivo: input, interfaccia, ciclo di vita e salvataggi sono pensati
da subito per il touch e per la sospensione dell'app.

---

## 1. Strati e regole di dipendenza

```
            +------------------------------------------------------+
            |                   game/  (scene, flussi)             |
            |  usa solo: core, input (azioni), ui, render, save    |
            +-----------+-----------+-----------+------------------+
                        |           |           |
     +------------------+   +-------+-----+  +--+------------------+
     |  core/  (logica) |   |  ui/        |  |  render/            |
     |  battaglia,      |   |  kit, scher-|  |  canvas, sprite,    |
     |  Elyon, dati,    |   |  mate, touch|  |  tile, effetti      |
     |  partita, mappe  |   |  overlay    |  |                     |
     +--------+---------+   +------+------+  +----------+----------+
              |                    |                    |
              |              +-----+------+             |
              |              |  input/    |  azioni astratte, mappature, prompt
              |              +-----+------+             |
     +--------+---------+          |                    |
     |  save/           |          |                    |
     |  formato, slot   |          |                    |
     +--------+---------+          |                    |
              |                    |                    |
     +--------+--------------------+--------------------+----------+
     |                     platform/                               |
     | platform_input  platform_storage  platform_audio            |
     | platform_time   platform_app (ciclo di vita, aree sicure)   |
     |   desktop/ (Windows, Linux, Steam Deck)  android/  ios/     |
     +-------------------------------------------------------------+
                         raylib (+ SDL dove serve, §8)
```

**Regole** (verificate in CMake e nei test):
1. **`core/` non include raylib né `platform/`.** È già così oggi (`eclyra_core`, testata con CTest).
2. **`game/` non conosce tasti, dita o pulsanti**: legge solo lo stato delle `InputAction` e i puntatori astratti (§3).
3. **`ui/` e `render/` usano raylib per disegnare**, ma non decidono le regole di gioco.
4. **Solo `platform/` contiene codice specifico di un sistema operativo** (`#ifdef` ammessi solo lì).
5. **`save/` serializza strutture di `core/` campo per campo**, senza puntatori, attraverso `platform_storage`.

## 2. Dalla struttura attuale a quella nuova

| Oggi (fase 4) | Dopo | Note |
|---|---|---|
| `src/*.c` di logica in `eclyra_core` | `src/core/` | spostamento senza cambiare codice; i test restano |
| `src/data/` | `src/core/data/` | generato da `tools/genera_database.py` |
| `game.c`, `world.c`, `player.c`, `npc.c` | `src/game/` | sostituire la lettura di `Input` con le `InputAction` |
| `input.c/h` (tastiera raylib) | `src/input/` + `src/platform/*/platform_input.c` | §3 |
| `graphics.c`, `map_render.c` | `src/render/` | aggiunta del canvas variabile (§5) |
| `menu.c`, `dialogue_ui.c`, `battle_ui.c`, `schermate.c`, `ui_common.c` | `src/ui/` | adottano `ui_kit` (Design System) |
| salvataggio non ancora presente | `src/save/` | §6 |
| (nessuno) | `src/platform/` | §4 |

La migrazione avviene per passi piccoli, con build e test verdi a ogni passo (§10).

## 3. Input astratto

### 3.1 Azioni

```c
/* src/input/input_action.h */
typedef enum {
    INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT,
    INPUT_CONFIRM, INPUT_CANCEL, INPUT_MENU, INPUT_RUN,
    INPUT_QUICK_MENU,      /* TAB, tasto View: voce del menu usata per ultima */
    INPUT_TAB_PREV, INPUT_TAB_NEXT,          /* LB/RB, Q/E: schede */
    INPUT_ACTION_COUNT
} InputAction;

typedef enum { DEVICE_KEYBOARD, DEVICE_GAMEPAD_XBOX, DEVICE_GAMEPAD_PS,
               DEVICE_GAMEPAD_GENERIC, DEVICE_TOUCH, DEVICE_MOUSE } InputDevice;

typedef struct {
    bool down[INPUT_ACTION_COUNT];       /* tenuto */
    bool pressed[INPUT_ACTION_COUNT];    /* premuto in questo passo logico (accumulato) */
    bool released[INPUT_ACTION_COUNT];
    int  repeat[INPUT_ACTION_COUNT];     /* ripetizione per i menu: 250 ms poi ogni 80 ms */
    InputDevice lastDevice;              /* per i prompt dei tasti */
    Pointer pointer;                     /* mouse o dito: posizione nel canvas logico, tap, trascinamento */
} InputState;

void Input_Update(InputState *s, const InputContext *ctx);   /* chiamata dal ciclo, una volta per frame */
void Input_EndStep(InputState *s);                          /* consuma i "pressed" dopo il passo logico */
const char *Input_PromptFor(InputAction a, InputDevice d);  /* "Z", "A", glifo croce, ... */
```

L'esempio della specifica diventa:

```
W / freccia / croce del controller / levetta / croce touch
        |
        v
   INPUT_RIGHT (down, pressed)
        |
        v
   Player_Update(): passo a destra   <- game/ non sa da dove arriva
```

### 3.2 Mappatura di base (modificabile da OPZIONI → Controlli)

| Azione | Tastiera | Controller (posizioni) | Touch |
|---|---|---|---|
| Su / giù / sinistra / destra | WASD, frecce | croce, levetta sinistra (zona morta 0,25) | croce virtuale |
| CONFIRM | Z, Invio, Spazio | tasto in basso (A / croce) | tasto A, tocco diretto su voci e pulsanti |
| CANCEL | X, Esc, Backspace | tasto a destra (B / cerchio) | tasto B |
| MENU | X/Esc fuori dai menu, M | Start | pillola MENU |
| QUICK_MENU | TAB | View / Select | (dal menu) |
| RUN | Shift tenuto | tasto a sinistra (X / quadrato) tenuto | doppio tocco sulla croce, o interruttore "corsa sempre" |
| TAB_PREV / TAB_NEXT | Q / E | LB / RB | scorrimento orizzontale |

Il mouse è un **puntatore**: passarci sopra seleziona, il clic sinistro conferma, il destro annulla.
La rotella scorre le liste.

### 3.3 Controller

- **API**: gamepad di raylib (`IsGamepadAvailable`, `GetGamepadButtonPressed`, assi). I nomi dei tasti sono per **posizione** (`GAMEPAD_BUTTON_RIGHT_FACE_DOWN` = tasto in basso), quindi Xbox, PlayStation e generici funzionano allo stesso modo.
- **Mappature dei controller**: si carica `gamecontrollerdb.txt` della comunità SDL con `SetGamepadMappings`, così i controller generici vengono riconosciuti.
- **Riconoscimento della famiglia** per i prompt: dal nome del dispositivo (`GetGamepadName`: "Xbox", "DualSense", "PS4"…), con fallback a generico.
- **Collegamento a caldo**: un controller collegato durante il gioco diventa attivo. Se il controller si scollega, la pausa è automatica.
- **Steam Input**: quando il gioco gira da Steam, Steam può rimappare. Il gioco si limita a leggere un controller con layout Xbox e mostra i prompt corrispondenti (requisito "glyph corretti" del Deck).

### 3.4 Touch

- `platform_input` fornisce i punti di contatto (fino a 4) in pixel fisici. `input/` li converte nel canvas logico (§5) e li assegna ai controlli del **layout touch del contesto**:

```c
typedef enum { TOUCH_CTX_EXPLORE, TOUCH_CTX_DIALOG, TOUCH_CTX_MENU,
               TOUCH_CTX_BATTLE, TOUCH_CTX_MAP } TouchContext;

typedef struct {               /* salvato nelle impostazioni */
    float scale;               /* 0,75 - 1,5 */
    float opacity;             /* 0,2 - 0,8 */
    Vector2 dpadPos, aPos, bPos, menuPos;   /* frazioni dell'area sicura, non pixel */
    bool analogDpad, autoHide, haptics;
} TouchLayout;
```

| Contesto | Controlli mostrati | Comportamento |
|---|---|---|
| EXPLORE | croce, A, B, MENU | la croce copre un'area d'aggancio più grande del disegno (raggio × 1,4) |
| DIALOG | nessuno | tocco ovunque = CONFIRM; scorrimento a destra = salta il testo |
| MENU | B | le voci si toccano direttamente (eventi puntatore della UI) |
| BATTLE | B | rosa dei comandi e carte delle mosse si toccano direttamente |
| MAP | B | tocco sui luoghi, trascinamento per spostarsi |

- Il contesto lo dichiara `game/` (`Input_SetContext`), che non sa nulla di dita: chiede solo quale tipo di interazione è attiva.
- La UI registra per ogni frame le proprie **aree toccabili** (rettangoli nel canvas logico, minimo 16×16) e il puntatore produce eventi tap, pressione lunga e trascinamento su quelle aree. Mouse e dito usano lo stesso codice.
- **Editor dei controlli**: schermata che trascina i controlli nell'area sicura, con anteprima sopra una scena di gioco e un pulsante per ripristinare.

## 4. Platform layer

```
src/platform/
  platform.h            versioni, nome della piattaforma, caratteristiche (touch, tastiera, rumble)
  platform_app.h        avvio, ciclo di vita (pausa, ripresa, chiusura), aree sicure, orientamento
  platform_input.h      tastiera, mouse, gamepad, touch grezzi -> InputState (tramite input/)
  platform_storage.h    cartella dei salvataggi, scrittura atomica, elenco file
  platform_audio.h      musica e suoni, focus audio sul mobile
  platform_time.h       orologio monotono (ciclo), orologio reale (data dei salvataggi)
  desktop/              Windows, Linux, Steam Deck (raylib con GLFW o SDL)
  android/              raylib PLATFORM_ANDROID (NDK + activity nativa)
  ios/                  backend SDL (§8)
```

| API | Funzioni principali |
|---|---|
| `platform_app` | `Platform_Init`, `Platform_ShouldQuit`, `Platform_OnPause(cb)`, `Platform_OnResume(cb)`, `Platform_SafeArea()` (inset in pixel fisici), `Platform_LockLandscape()` |
| `platform_storage` | `Storage_SaveDir()`, `Storage_WriteAtomic(path, data, n)`, `Storage_Read(path, buf, max)`, `Storage_Remove`, `Storage_Exists` |
| `platform_audio` | `Audio_PlayMusic(id)`, `Audio_PlaySfx(id)`, `Audio_SetVolumes`, `Audio_Suspend/Resume` |
| `platform_time` | `Time_Monotonic()` in secondi (double), `Time_WallClock()` (Unix), `Time_Sleep` |

**Cartelle dei salvataggi**
| Piattaforma | Percorso |
|---|---|
| Windows | `%APPDATA%\ECLYRA\` |
| Linux e Steam Deck | `$XDG_DATA_HOME/eclyra/` (di solito `~/.local/share/eclyra/`) |
| macOS | `~/Library/Application Support/ECLYRA/` |
| Android | cartella interna dell'app (`internalDataPath`) |
| iOS e iPadOS | `Library/Application Support/` del sandbox |

**Ciclo di vita mobile**: in pausa (app in background, chiamata, blocco schermo) il gioco ferma la logica,
abbassa l'audio e fa un **salvataggio automatico di sospensione**. Alla ripresa torna esattamente dove era.
Su desktop la perdita del focus mette in pausa solo se l'opzione è attiva.

## 5. Scaling e UI responsive

Regola: **pixel interi, nessuna deformazione, nessuna sfocatura.** Il lato corto del canvas è 180 px
(o fino a 240 sugli schermi più alti), quello lungo si allarga.

```c
/* render/canvas.c */
typedef struct { int w, h, scale, offX, offY; } Canvas;

Canvas Canvas_Compute(int screenW, int screenH) {
    int s = min(screenW / 320, screenH / 180);   /* scala intera massima che contiene 320x180 */
    if (s < 1) s = 1;
    Canvas c;
    c.scale = s;
    c.w = clamp(screenW / s, 320, 400);          /* 16:9 -> 320, 19.5:9 -> fino a 400 */
    c.h = clamp(screenH / s, 180, 240);          /* 16:10 -> 200, 4:3 -> fino a 240 */
    c.offX = (screenW - c.w * s) / 2;            /* il resto, pochi pixel, diventa una cornice scura */
    c.offY = (screenH - c.h * s) / 2;
    return c;
}
```

| Schermo | Scala | Canvas | Resto |
|---|---|---|---|
| 1920×1080 | 6 | 320×180 | 0 |
| 1280×800 (Steam Deck) | 4 | 320×200 | 0 |
| 2560×1440 | 8 | 320×180 | 0 |
| 3840×2160 | 12 | 320×180 | 0 |
| 2400×1080 (telefono) | 6 | 400×180 | 0 |
| 2532×1170 (iPhone) | 6 | 400×195 | piccole bande laterali di 66 px fisici |
| 2732×2048 (iPad Pro) | 8 | 341×240 | 16 px fisici in verticale |

- Il gioco disegna su una render texture `c.w × c.h` e la copia scalata di `c.scale` con filtro nearest. È lo stesso schema di oggi, con un canvas di dimensione variabile.
- **Ancore della UI**: ogni elemento dichiara un'ancora (alto-sinistra, alto-destra, basso-sinistra, basso-destra, centro). Le HUD stanno ai bordi del canvas reale.
- **Area sicura 320×180**, centrata, per tutto ciò che è essenziale. Gli inset di notch e angoli si convertono in pixel logici e si sommano ai margini.
- **Mondo**: la camera mostra più tile; le mappe piccole restano centrate.
- **Lotta**: il campo resta centrato, le HUD ai bordi (verificato nei mockup `04c_lotta_telefono` e `02c_overworld_steamdeck`).

## 6. Salvataggi

### 6.1 Contenuto

Il giocatore:
- nome;
- posizione (mappa, x, y, direzione);
- tempo di gioco;
- Lumen.

Il resto della partita:
- squadra e box: per ogni Elyon specie, forma, soprannome, livello, XP, IV, natura, HP, mosse, PP, stato, amicizia, oggetto, variante;
- inventario;
- flag della storia;
- missioni;
- stato del mondo: oggetti raccolti, allenatori sconfitti, luoghi visitati, Bestiario;
- seme del RNG;
- scelta finale.

Le **impostazioni** stanno in un file separato (`impostazioni.cfg`), perché valgono per tutte le partite:
testo, audio, schermo, mappatura dei tasti, layout touch.

### 6.2 Formato binario versionato

```
offset  dimensione  campo
0       4           magic "ECLY"
4       2           versione del formato (u16, little endian)
6       2           flag (u16)
8       4           lunghezza del contenuto (u32)
12      4           CRC32 del contenuto
16      8           data di salvataggio (u64, Unix)
24      4           tempo di gioco in secondi (u32)
28      ...         contenuto: sezioni TLV
```

- **Sezioni** `[id 4 byte][lunghezza u32][dati]`:

  | Id | Contenuto |
  |---|---|
  | `PLYR` | giocatore |
  | `PRTY` | squadra |
  | `BOXS` | box |
  | `INVT` | inventario |
  | `FLAG` | flag della storia |
  | `QUST` | missioni |
  | `WRLD` | stato del mondo |
  | `BEST` | Bestiario |
  | `RNG_` | seme del RNG |
  | `META` | dati vari, tra cui la scelta finale |

  Le sezioni sconosciute si saltano: un salvataggio nuovo resta leggibile da una versione vecchia, nei limiti del possibile.
- Ogni campo si scrive esplicitamente con funzioni `W_u8/u16/u32/i32/str`, in little endian. **Mai** `fwrite` di una struct, mai puntatori. I riferimenti (specie, mosse, oggetti) si salvano come ID interi stabili.
- **Migrazione**: `Save_Migrate(from, to)` converte i contenuti delle versioni precedenti; ogni cambio di formato ha un test.

### 6.3 Robustezza

- **Scrittura atomica**:
  1. scrittura in `slot1.sav.tmp`;
  2. flush su disco;
  3. il file precedente diventa `slot1.bak`;
  4. rinomina in `slot1.sav`.
- **Lettura**:
  1. verifica di magic, versione, lunghezza e CRC;
  2. se il file è corrotto, si prova il `.bak`;
  3. se anche il backup non va, il gioco lo segnala con un messaggio chiaro e non va in crash.
- **Slot**: 3 manuali + 1 automatico (cambio di zona, sospensione dell'app).
- **Test** (CTest, nella libreria `core`):
  - andata e ritorno esatti;
  - byte casuali corrotti;
  - file troncato;
  - versione futura;
  - versione vecchia migrata;
  - slot pieni e vuoti.

### 6.4 Salvataggi nel cloud (predisposizione)

| Servizio | Come | Codice necessario |
|---|---|---|
| Steam Cloud | Auto-Cloud configurata su Steamworks (cartella dei salvataggi) | nessuno |
| iCloud (iOS) | documenti iCloud Drive o NSUbiquitousKeyValueStore | piccolo modulo nativo in `platform/ios/` |
| Google Play (Android) | Saved Games di Play Games Services | modulo Java/JNI in `platform/android/` |

- Il formato è già adatto: è un file unico autosufficiente, con data e tempo di gioco nell'intestazione.
- In caso di conflitto si propongono entrambe le versioni, con data, luogo e tempo di gioco, e decide il giocatore.

## 7. Piattaforme

| Piattaforma | Build | Note |
|---|---|---|
| **Windows (Steam)** | CMake + MSVC o MinGW, raylib statica | Steamworks facoltativo (obiettivi più avanti); controller XInput e Steam Input |
| **Linux** | CMake + gcc o clang | build nativa anche per il Deck |
| **Steam Deck** | build Linux nel runtime di Steam (Steam Linux Runtime) oppure build Windows con Proton | requisiti Verified (§7.1) |
| **Android** | raylib `PLATFORM_ANDROID`, NDK, Gradle in `platform/android/` | orizzontale bloccato, schermo intero immersivo, pausa e ripresa, Android App Bundle per il Play Store |
| **iOS / iPadOS** | Xcode, backend SDL (§8) | richiede un Mac con Xcode e l'iscrizione all'Apple Developer Program |
| macOS | CMake + clang | utile per sviluppare e per la build iOS |

### 7.1 Steam Deck Verified

Criteri di Valve (documentazione Steamworks) e come li soddisfiamo:

| Criterio | Soluzione |
|---|---|
| Tutto il gioco si usa coi controlli fisici del Deck | navigazione a fuoco ovunque (Design System §17) |
| I prompt corrispondono ai tasti usati, senza glifi di tastiera | prompt per dispositivo, layout Xbox sul Deck |
| Risoluzione supportata (1280×800 preferita) | canvas 320×200 a ×4, senza bande |
| Testo mai sotto 9 px a 1280×800 (consigliati 12) | micro 5 px × 4 = 20 px; corpo 7 px × 4 = 28 px |
| 30 fps a 800p con le impostazioni predefinite | il gioco punta a 60 fps fissi; a 320×200 il carico è minimo |
| Nessun launcher non navigabile | il gioco parte direttamente dal titolo |
| Inserimento di testo da controller | nome del personaggio con tastiera virtuale interna navigabile; in alternativa la tastiera di Steam |

## 8. Backend: raylib, GLFW e SDL

- **Desktop e Deck**: raylib col backend predefinito (GLFW) funziona già.
- **Android**: raylib supporta `PLATFORM_ANDROID` ufficialmente.
- **iOS**: raylib **non ha un backend ufficiale**. Il tentativo di porting (`rcore_ios.c`) è stato chiuso senza essere incluso. Il problema principale è il ciclo di vita di iOS, che controlla il `main`.
- **Proposta**: usare il **backend SDL di raylib** (raylib 5.x può compilare il proprio strato di piattaforma su SDL invece che su GLFW) e SDL su iOS, che gestisce il ciclo di vita e il touch. La grafica resta OpenGL ES: su iOS è deprecata da Apple ma ancora funzionante.
- **Rischio principale del progetto**: va verificato con un **prototipo tecnico prima di investire**, cioè una scena 320×180 col canvas, il touch e un salvataggio, in esecuzione su iPhone e iPad. È il primo passo dell'implementazione multipiattaforma.
- **Opzione da valutare dopo il prototipo**: usare SDL anche su desktop. Darebbe un solo strato di input per tutto (miglior supporto dei controller, Steam Deck compreso).

## 9. Build con CMake

```
CMakePresets.json
  desktop-debug       ECLYRA_PLATFORM=DESKTOP  ECLYRA_DEBUG_TOOLS=ON
  desktop-release     ECLYRA_PLATFORM=DESKTOP
  linux-steamrt       build Linux da distribuire su Steam (runtime di Steam)
  android-arm64       toolchain NDK, ECLYRA_PLATFORM=ANDROID
  ios                 generatore Xcode, ECLYRA_PLATFORM=IOS, ECLYRA_BACKEND=SDL
```

| Bersaglio | Contenuto |
|---|---|
| `eclyra_core` | statica, senza raylib; come oggi |
| `eclyra_engine` | render, ui, input, save, audio; dipende da raylib |
| `eclyra_platform_<nome>` | uno per piattaforma |
| `eclyra` | eseguibile desktop; su Android libreria condivisa caricata dall'activity nativa; su iOS app Xcode |
| test | solo desktop (`ctest`) |

- **Asset**: copiati accanto all'eseguibile su desktop, in `assets/` dell'APK/AAB su Android, nel bundle dell'app su iOS. Il gioco li apre con un percorso relativo fornito da `platform_storage`.
- **Integrazione continua** (GitHub Actions):
  - build e test su Windows, Linux e macOS;
  - build Android;
  - build iOS senza firma, su un runner macOS;
  - screenshot automatici come artefatti.

## 10. Piano di migrazione (dopo l'approvazione)

Ogni passo lascia il gioco compilabile e i test verdi.

1. **Spostamento delle cartelle** (`core/`, `game/`, `render/`, `ui/`, `input/`): solo file e CMake, nessuna logica cambiata.
2. **InputAction**: nuovo modulo `input/`, mappatura di tastiera e controller; `game/` passa alle azioni. Test: tabelle di mappatura e ripetizione.
3. **Canvas variabile** e ancore della UI; screenshot automatici a 320×180, 320×200 e 400×180.
4. **Salvataggi** (formato, slot, backup, test di corruzione); menu SALVA e CONTINUA attivi.
5. **Puntatore e touch**: aree toccabili della UI, layout per contesto, editor dei controlli.
6. **UI kit** del Design System (font, pannelli 9-slice, prompt) e conversione delle schermate esistenti.
7. **Prototipo iOS** (backend SDL) e build Android; solo dopo, la grafica definitiva delle regioni.

## 11. Prestazioni e consumi

- 60 passi logici al secondo con accumulatore (come oggi), disegno a 60 fps.
- **Mobile**:
  - 60 fps in gioco;
  - 30 fps nei menu statici e nei dialoghi in attesa;
  - nessun disegno in pausa.
- **Memoria**: tutti gli asset di una regione stanno in pochi MB. Limite di progetto: 256 MB di RAM, 300 MB di pacchetto.
- **Batch**: sprite e tile dagli atlanti, al massimo poche centinaia di chiamate di disegno per frame a 320×180.

## 12. Accessibilità e localizzazione

- **Accessibilità**:
  - velocità del testo;
  - rimappatura completa;
  - corsa sempre attiva;
  - controlli touch ridimensionabili;
  - nessuna informazione affidata solo al colore;
  - opzione per ridurre i lampi (critici e transizioni).
- **Localizzazione**: oggi solo italiano. I testi sono già in un file a ID (`dialoghi.txt`); il font ha gli accenti e può crescere con altri alfabeti latini.
