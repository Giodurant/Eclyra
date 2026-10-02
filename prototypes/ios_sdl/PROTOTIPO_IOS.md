# ECLYRA — Prototipo tecnico iOS (raylib + SDL2)

**Scopo**: validare il principale rischio tecnico del progetto prima di continuare.
Se il prototipo supera la prova su un dispositivo reale, l'architettura multipiattaforma viene congelata
e si procede con l'implementazione.

## 1. Cosa contiene

È una piccola app separata dal gioco; il codice del gioco non viene toccato. Usa la stessa configurazione prevista per ECLYRA su iOS:
- raylib 5.5 con il backend SDL2 (`PLATFORM_DESKTOP_SDL`) e OpenGL ES 2;
- canvas a pixel interi da `MULTIPLATFORM_ARCHITECTURE.md` §5;
- input astratto a azioni (§3);
- platform layer minimo (§4).

| File | Ruolo |
|---|---|
| `src/main.c` | scena di prova, diagnostica, prova automatica |
| `src/platform/platform_sdl.c` | ciclo di vita, tempo, cartelle, salvataggio atomico, framebuffer |
| `src/platform/platform_ios.m` | area sicura (notch e barra home) e vibrazione: solo iOS |
| `src/render/canvas.c` | calcolo del canvas: 320–400 × 180–240, scala intera |
| `src/input/input.c` | tastiera, controller e touch producono le stesse `InputAction` |
| `src/proto_audio.c` | musica sintetizzata in streaming ed effetto sonoro, senza file |
| `ios/Info.plist.in` | orizzontale, schermo intero, controller, cartella visibile nell'app File |
| `ios/LaunchScreen.storyboard` | schermo di avvio, senza il quale iOS mostra l'app ridotta |
| `build_ios.sh` | genera il progetto Xcode e compila per il Simulatore o per il dispositivo |

**Nella scena**:
- PYRIN di riferimento, mosso con la croce;
- una scacchiera di 1 px che diventa grigia se lo scaling non è nitido;
- il contorno del canvas (oro), la zona sicura 320×180 (azzurro) e l'area sicura del dispositivo (arancio);
- i controlli touch: croce, A, B, MENU.

**Comandi**:
- croce: muove PYRIN;
- A: bip;
- B: cambia la misura dei controlli (×0,75, ×1, ×1,25);
- MENU: mostra o nasconde la diagnostica;
- **A + B tenuti 2 secondi**: prova automatica sul dispositivo, con i risultati sullo schermo.

## 2. Già verificato (Linux, stessa strada SDL)

Eseguito in questa sessione: `./eclyra_proto --selftest` sotto Xvfb, con SDL di sistema e con SDL 2.30.9 compilata dai sorgenti (la stessa usata su iOS).

| Prova | Esito |
|---|---|
| Asset caricato dalla cartella dell'app | OK |
| Canvas 1280×720 → 320×180 ×4 | OK |
| Canvas 1280×800 → 320×200 ×4 (Steam Deck) | OK |
| Canvas 1560×720 → 390×180 ×4 (telefono 19,5:9) | OK |
| Canvas 1024×768 → 341×240 ×3 (iPad 4:3) | OK |
| Scacchiera di 1 px nitida a ogni scala | OK |
| `INPUT_RIGHT` e `INPUT_CONFIRM` (azioni astratte) | OK |
| Controlli touch dentro il canvas e separati | OK |
| Musica in streaming che avanza, e che si ferma in pausa | OK (dispositivo audio nullo) |
| Background → logica ferma, salvataggio di sospensione con CRC | OK |
| Ripresa: il tempo in background non conta, il rendering riparte | OK |
| File con CRC sbagliato riconosciuto e scartato | OK |
| Ramo "mobile" del codice (compilato con `PROTO_FORCE_MOBILE`) | 14/14 OK |

**Non verificabile qui**:
- il touch reale: SDL registra le dita solo dal sistema operativo;
- il framebuffer di iOS;
- la sessione audio di iOS;
- la firma;
- l'installazione sul dispositivo.

Queste sono le prove della §4, da fare su un iPhone o iPad.

## 3. Come compilare (Mac)

Requisiti:
- un Mac con **Xcode** (App Store) e **CMake** (`brew install cmake`);
- un **Apple ID**: va bene anche gratuito, ma l'app resta installata 7 giorni;
- un iPhone o iPad con cavo.

1. In Xcode → Settings → Accounts aggiungi il tuo Apple ID e copia il **Team ID**.
2. Sul dispositivo abilita la modalità sviluppatore: Impostazioni → Privacy e sicurezza → Modalità sviluppatore.
3. Esegui:
   ```bash
   cd prototypes/ios_sdl
   ./build_ios.sh                 # 1) prima prova: compila per il Simulatore
   ./build_ios.sh TEAMID --open   # 2) apre Xcode: scegli il tuo iPhone e premi Run
   ```
4. Al primo avvio iOS può chiedere di autorizzare lo sviluppatore: Impostazioni → Generali → VPN e gestione dispositivi.

Se `cmake` si lamenta della versione minima di una dipendenza, lo script passa già `CMAKE_POLICY_VERSION_MINIMUM=3.5`, necessario con CMake 4.

## 4. Protocollo di prova sul dispositivo

Segna OK o NO per ogni riga. Usa almeno un **iPhone**; se possibile aggiungi un **iPad**.

| # | Area | Cosa fare | Superata se |
|---|---|---|---|
| 1 | Avvio | Tocca l'icona | parte in orizzontale e a schermo intero, senza bande nere da "app vecchia"; nessun crash in 3 avvii; il contatore "avvii" sale |
| 2 | Rendering 320×180 | Guarda la diagnostica | `canvas` ha una scala intera e il canvas sta fra 320–400 × 180–240 |
| 3 | Scaling | Guarda la scacchiera in alto a sinistra con la lente o con uno screenshot ingrandito | quadretti netti bianchi e neri, nessuna sfumatura grigia; PYRIN nitido |
| 4 | Area sicura | Guarda il riquadro arancio | sui modelli con notch o Dynamic Island evita notch e angoli; i controlli touch ci stanno dentro |
| 5 | Touch | Croce, A, B, MENU; poi croce + A insieme | PYRIN si muove anche in diagonale; A suona; vibrazione leggera; `dita` arriva a 2 o più; nessun "clic fantasma" |
| 6 | Controlli | Premi B tre volte | i controlli cambiano misura e restano usabili |
| 7 | Controller | Collega in Bluetooth un controller Xbox o DualSense | `pad` mostra il nome; croce/levetta muovono, A = bip; i controlli touch spariscono, e un tocco li fa ricomparire |
| 8 | Tastiera (iPad) | Tastiera Bluetooth o Magic Keyboard | WASD/frecce muovono, Z = bip |
| 9 | Audio | Ascolta | arpeggio continuo senza scatti; il bip si sovrappone; il volume risponde ai tasti |
| 10 | Pausa / ripresa | Torna alla Home, aspetta 10 s, rientra | musica ferma in background; al ritorno `background` e `primo piano` salgono di 1, `pausa` segna i secondi, il tempo di gioco no; la scena riparte |
| 11 | Interruzioni | Blocca lo schermo; ricevi una chiamata o avvia un timer; Centro di Controllo | dopo ciascuna l'app riprende con audio e immagine; nessuno schermo nero |
| 12 | Salvataggio | Chiudi l'app dal multitasking e riaprila | `avvii` +1, `salvataggio OK`; nell'app File → Sul mio iPhone → ECLYRA Proto c'è `proto_stato.txt` |
| 13 | Prova automatica | Tieni A + B per 2 s | tutte le righe OK (rapporto anche in File: `selftest_rapporto.txt`) |
| 14 | Prestazioni | 5 minuti di uso | 60 FPS stabili (30 minimo); il telefono non scalda in modo anomalo |

**Da annotare**: modello, versione di iOS, i valori della diagnostica (schermo, punti, canvas, area sicura, fbo) e una foto dello schermo con la diagnostica aperta.

### Esito

Il prototipo è **stabile** se le prove 1–5, 9, 10, 12 e 13 sono OK su almeno un iPhone, e le altre non hanno blocchi.
In quel caso si congela l'architettura (`MULTIPLATFORM_ARCHITECTURE.md`) e si parte con il piano di migrazione.

## 5. Rischi tecnici e cosa fa già il prototipo

| Rischio | Contromisura nel codice |
|---|---|
| Su iOS lo schermo **non è il framebuffer 0**: raylib ci torna dopo ogni render texture e si vede tutto nero | `Platform_BindScreenFramebuffer()` ribinda il framebuffer di SDL (`SDL_GetWindowWMInfo` → `uikit.framebuffer`) dopo ogni `EndTextureMode()` |
| raylib con SDL **non gestisce l'HiDPI**: disegnerebbe in "punti", cioè sfocato o in un angolo | viewport e proiezione impostati sulla dimensione reale del drawable (`SDL_GL_GetDrawableSize`) |
| **miniaudio** su iOS richiede AVAudioSession | `raudio.c` compilato come Objective-C, con i framework AVFoundation, AudioToolbox e CoreAudio |
| Disegnare con GL **in background** fa chiudere l'app da iOS | in pausa niente GL e niente logica; eventi `SDL_APP_*` gestiti subito con `SDL_AddEventWatch` |
| Senza schermo di avvio l'app gira **ridotta** | `LaunchScreen.storyboard` + `UILaunchStoryboardName` |
| **Doppio input**: un dito genera anche un clic | `SDL_HINT_TOUCH_MOUSE_EVENTS=0` |
| Uscita accidentale con lo swipe dal basso | `SDL_HINT_IOS_HIDE_HOME_INDICATOR=2` |
| Salvataggio troncato se iOS chiude l'app | scrittura atomica (file temporaneo + rename) e CRC; il file corrotto viene scartato senza crash |
| **OpenGL ES deprecato** da Apple dal 2018 (iOS 12) | per ora funziona; la prova 1 conferma che c'è ancora sull'iOS attuale. Strada di riserva, senza cambiare il codice del gioco: raylib sopra **ANGLE** (OpenGL ES tradotto in Metal) |

## 6. Cosa non copre

Non fanno parte di questa prova:
- App Store e TestFlight;
- iCloud;
- Game Center;
- acquisti in-app;
- localizzazione;
- prestazioni con il gioco completo.

Questi punti arrivano dopo il congelamento dell'architettura.
