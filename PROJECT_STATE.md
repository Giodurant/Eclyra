# PROJECT_STATE — ECLYRA

Ultimo aggiornamento: Fase 1 completata (versione 0.1.0).

## Come riprendere il lavoro
1. Leggere `ECLYRA_MASTER_PER_CLAUDE` (specifica) e questo file.
2. `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j && ctest --test-dir build`.
3. `./build/eclyra --screenshot-test` e controllare i PNG generati in `build/`.
4. Proseguire dalla prima voce non spuntata del piano.

## Piano di consegna
- [x] **Fase 1** — scheletro C99/CMake/raylib, title, game loop, camera, player.
- [ ] **Fase 2** — mappe da file, collisioni, porte e interni, transizioni, NPC, dialoghi
      (testo progressivo con skip, velocità da opzioni), mappe Lunaria 40×30 e Route 1 60×25.
- [ ] **Fase 3** — database 100 specie / 100 mosse / abilità 001–046 / oggetti, controlli di consistenza
      (12 tipi, matrice 12×12 testata, evoluzioni, riferimenti validi).
- [ ] **Fase 4** — battle engine, IA, cattura, XP, level-up, evoluzione, inventario, box 5×20.
- [ ] **Fase 5** — vertical slice: Lunaria → Route 1 → Bosco Verdalia → Verdan → Lira → Sigillo Verde.
- [ ] **Fase 6** — 5 regioni, Guardiani, Nexus, Lega, boss, due finali.
- [ ] **Fase 7** — postgame, pixel art definitiva, audio, rifinitura, packaging.

## File (Fase 1)
| File | Ruolo |
|---|---|
| `src/main.c` | avvio, test con screenshot (Debug) |
| `src/game.c/h` | scene (titolo, opzioni, mondo), menu di gioco, passo fisso |
| `src/graphics.c/h` | finestra, render target 320×180, scala intera, texture, testo, finestre UI |
| `src/input.c/h` | tastiera con eventi accumulati |
| `src/menu.c/h` | menu verticale con voci disattivabili |
| `src/map.c/h` | tile, collisioni, autotile sentiero/acqua, disegno in profondità, Lunaria provvisoria |
| `src/player.c/h` | movimento a griglia e animazione |
| `src/world.c/h` | mappa corrente, giocatore, camera, banner nome zona, messaggi dei cartelli |
| `src/utils.c/h` | RNG xorshift32, clamp, divisione sicura, copia stringhe |
| `tests/test_utils.c` | test unitari |

## Decisioni prese
- Tutto il disegno avviene su una render texture 320×180, scalata di un fattore intero.
- Logica a 60 passi/s indipendente dal refresh; gli eventi "premuto" si accumulano fino al passo.
- `eclyra_core` è una libreria senza raylib: lì andranno dati e regole di lotta, così i test
  non richiedono una finestra.
- Esc non chiude la finestra (serve come "indietro"): si esce da ESCI o chiudendo la finestra.

## Problemi noti / da fare
- Lunaria è provvisoria e generata da codice; nessuna porta funzionante (Fase 2).
- Gli sprite usano la pixel art segnaposto originale: un solo modello di edificio (5×4).
- Sprite del giocatore 16×24 (la specifica cita 16×16/24×24: da rivedere con l'arte definitiva).
- Tasti debug F1–F8 non ancora presenti: arrivano con i sistemi a cui si riferiscono.
- Audio assente.
- Non verificata la compilazione su Windows/macOS: verificata su Linux (gcc, CMake 3.28).
