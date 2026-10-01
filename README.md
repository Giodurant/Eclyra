# ECLYRA — Le Cronache dell'Equilibrio

RPG 2D in pixel art con cattura di creature a turni (gli **Elyon**), ambientato nel
continente di **Asterra**. Scritto in **C99** con **raylib** e **CMake**.
Tutti i contenuti (storia, creature, mosse, grafica) sono originali.

Stato attuale: **Fase 2 di 7** — vedi `PROJECT_STATE.md` e `CHANGELOG.md`.

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
| M | Menu di gioco |
| F7 (solo Debug) | Teletrasporto ciclico tra tutte le mappe |

## Modificare mappe e testi

- Le mappe sono file di testo in `assets/maps/*.map` (formato descritto in `src/map.h`).
  `tools/genera_mappe.py` le rigenera da zero (sovrascrive le modifiche a mano).
- Dialoghi e cartelli sono in `assets/text/dialoghi.txt` (formato descritto in testa al file).
- Dopo ogni modifica: `ctest --test-dir build` controlla raggiungibilità, porte e testi mancanti.

## Struttura

```
src/            codice del gioco (un modulo per file .c/.h)
src/data/       database di specie, mosse, abilità, oggetti (dalla Fase 3)
assets/         sprite, tileset, mappe, interfaccia, audio, font
tests/          test automatici (CTest)
tools/          script per generare pixel art segnaposto e mappe
saves/          salvataggi (dalla fase del sistema di salvataggio)
```

Grafica: risoluzione logica 320×180, scala intera pixel-perfect, filtro nearest.
