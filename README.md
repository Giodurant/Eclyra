# ECLYRA — Le Cronache dell'Equilibrio

RPG 2D in pixel art con cattura di creature a turni (gli **Elyon**), ambientato nel
continente di **Asterra**. Scritto in **C99** con **raylib** e **CMake**.
Tutti i contenuti (storia, creature, mosse, grafica) sono originali.

Stato attuale: **Fase 4 di 7** (motore di lotta) — vedi `PROJECT_STATE.md` e `CHANGELOG.md`.

## Requisiti

- Compilatore C99 (clang, gcc o MSVC)
- CMake ≥ 3.16
- git (CMake scarica raylib 5.5 da GitHub se non è installata nel sistema)

Su macOS: `xcode-select --install` e `brew install cmake` (oppure l'installer da cmake.org).

## Compilare ed eseguire

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
./build/eclyra
```

Su Windows con Visual Studio l'eseguibile è in `build\Debug\eclyra.exe`.
Per una versione senza strumenti di debug: `-DCMAKE_BUILD_TYPE=Release`.

Se raylib è già scaricata in locale: `-DFETCHCONTENT_SOURCE_DIR_RAYLIB=/percorso/raylib`.

## Test

```sh
ctest --test-dir build --output-on-failure
./build/eclyra --screenshot-test     # solo Debug: percorso automatico con screenshot PNG
```

## Controlli

| Tasto | Azione |
|---|---|
| Frecce / WASD | Muoversi (un tocco breve gira il personaggio) |
| Shift (tenuto) | Correre |
| Z / Invio | Conferma, interagisci |
| X / Esc | Indietro, apre il menu di gioco |
| M | Menu di gioco (ELYON, ZAINO, SCHEDA, SALVA, OPZIONI) |

In lotta: frecce per scegliere, Z per confermare, X per tornare indietro
(X interrompe anche un'evoluzione).

Tasti di debug (solo build Debug):

| Tasto | Azione |
|---|---|
| F1 | +1000 Lumen |
| F2 | Cura la squadra |
| F3 | +1 livello al primo Elyon (evolve se può) |
| F4 | Incontro selvatico immediato (habitat della mappa, altrimenti Route 1) |
| F6 | Imposta i flag dei sette Sigilli |
| F7 | Teletrasporto ciclico tra tutte le mappe |
| F8 | Lotta contro la Guardiana Lira |

## Come provare la Fase 4

Nuova partita → esci di casa → laboratorio (a nord-est della piazza) → parla con le capsule sul
tavolo e scegli PYRIN, AQUIL o MOSSEN. Poi: Kael davanti al laboratorio ti sfida, nell'erba alta
della Route 1 compaiono Elyon selvatici (catturabili con i CORE), Teo e Nina lottano, la mamma
cura la squadra, il PC del laboratorio apre il box.

## Modificare mappe e testi

- Le mappe sono file di testo in `assets/maps/*.map` (formato descritto in `src/map.h`).
  `tools/genera_mappe.py` le rigenera da zero (sovrascrive le modifiche a mano).
- Dialoghi e cartelli sono in `assets/text/dialoghi.txt` (formato descritto in testa al file).
- Dopo ogni modifica: `ctest --test-dir build` controlla raggiungibilità, porte e testi mancanti.

## Database di gioco

Specie, mosse, abilità, evoluzioni, allenatori e habitat sono generati in `src/data/`
da `tools/genera_database.py`, che contiene le tabelle canoniche del documento di design.
Per modificarli: cambia lo script, poi `python3 tools/genera_database.py` e ricompila.
Una versione leggibile di tutti i dati è in `docs/BESTIARIO.md`.

## Struttura

```
src/            codice del gioco (un modulo per file .c/.h)
src/data/       database generato: tipi, specie, mosse, abilità, evoluzioni, allenatori, habitat, oggetti
docs/           BESTIARIO.md (dati in forma leggibile)
assets/         sprite, tileset, mappe, interfaccia, audio, font
tests/          test automatici (CTest)
tools/          script per generare pixel art segnaposto (anche i 100 Elyon), mappe e database
saves/          salvataggi (dalla fase del sistema di salvataggio)
```

Grafica: risoluzione logica 320×180, scala intera pixel-perfect, filtro nearest.
