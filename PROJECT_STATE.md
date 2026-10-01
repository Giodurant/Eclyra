# PROJECT_STATE — ECLYRA

Ultimo aggiornamento: Fase 2 completata (versione 0.2.0).

## Come riprendere il lavoro
1. Leggere `ECLYRA_MASTER_PER_CLAUDE` (specifica) e questo file.
2. `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j && ctest --test-dir build`.
3. `./build/eclyra --screenshot-test` e controllare i PNG generati in `build/`.
4. Proseguire dalla prima voce non spuntata del piano.

## Piano di consegna
- [x] **Fase 1** — scheletro C99/CMake/raylib, title, game loop, camera, player.
- [x] **Fase 2** — mappe da file, collisioni, porte e interni, transizioni, NPC, dialoghi
      (testo progressivo con skip, velocità da opzioni), mappe Lunaria 40×30 e Route 1 60×25.
- [ ] **Fase 3** — database 100 specie / 100 mosse / abilità 001–046 / oggetti, controlli di consistenza
      (12 tipi, matrice 12×12 testata, evoluzioni, riferimenti validi).
- [ ] **Fase 4** — battle engine, IA, cattura, XP, level-up, evoluzione, inventario, box 5×20.
- [ ] **Fase 5** — vertical slice: Lunaria → Route 1 → Bosco Verdalia → Verdan → Lira → Sigillo Verde.
- [ ] **Fase 6** — 5 regioni, Guardiani, Nexus, Lega, boss, due finali.
- [ ] **Fase 7** — postgame, pixel art definitiva, audio, rifinitura, packaging.

## File
| File | Ruolo |
|---|---|
| `src/main.c` | avvio, percorso automatico con screenshot (Debug) |
| `src/game.c/h` | scene (titolo, opzioni, mondo), menu di gioco, zaino, debug F7, passo fisso |
| `src/graphics.c/h` | finestra, render target 320×180, scala intera, texture, testo, finestre UI |
| `src/input.c/h` | tastiera con eventi accumulati |
| `src/menu.c/h` | menu verticale con voci disattivabili |
| `src/map.c/h` | **core**: formato .map, parser, tile, collisioni, warp, cartelli, oggetti |
| `src/map_render.c/h` | disegno esterni/interni, autotile, profondità di alberi ed edifici |
| `src/npc.c/h` | NPC: movimento (fermo/gira/vaga), voltarsi, disegno |
| `src/dialogue.c/h` | **core**: database testi, varianti con flag, segnaposto, testo progressivo |
| `src/dialogue_ui.c/h` | a capo automatico, impaginazione, riquadro con nome |
| `src/events.c/h` | **core**: flag di gioco con nome |
| `src/items.c/h` | **core**: inventario e valuta |
| `src/data/items_data.c/h` | **core**: tabella oggetti con ID stabili |
| `src/partita.c/h` | **core**: stato persistente (futuro salvataggio), senza puntatori |
| `src/player.c/h` | movimento a griglia, urti (per le porte), animazione |
| `src/world.c/h` | mappa corrente, NPC, porte e transizioni, interazioni, camera |
| `src/utils.c/h` | **core**: RNG, clamp, divisione sicura, stringhe, lettura file |
| `assets/maps/*.map` | 6 mappe (generate da `tools/genera_mappe.py`, modificabili) |
| `assets/text/dialoghi.txt` | 42 testi di dialoghi e cartelli |
| `tools/genera_grafica_fase2.py` | pixel art di edifici, interni, NPC |
| `tests/test_*.c` | test CTest (utils, mappe, dialoghi) |

## Decisioni prese
- Tutto il disegno avviene su una render texture 320×180, scalata di un fattore intero.
- Logica a 60 passi/s indipendente dal refresh; gli eventi "premuto" si accumulano fino al passo.
- `eclyra_core` è una libreria senza raylib: lì andranno dati e regole di lotta, così i test
  non richiedono una finestra.
- Esc non chiude la finestra (serve come "indietro"): si esce da ESCI o chiudendo la finestra.
- Porte = warp "su urto" (la tile della porta e' bloccata); uscite = warp "all'arrivo". Le
  destinazioni cadono una tile dentro la mappa di arrivo, mai su un'altra uscita (testato).
- I testi stanno in un file unico; le varianti per i progressi della storia si scrivono `@ID?FLAG`.

## Problemi noti / da fare
- Il varco nord della Route 1 verso il Bosco Verdalia è presidiato da un NPC: la mappa del Bosco
  arriva con la Fase 5 (non esiste ancora un warp verso di essa).
- Gli allenatori della Route 1 non lottano ancora (Fase 4); i loro testi lo spiegano in gioco.
- Il commesso dell'emporio non vende ancora (negozio: Fase 4/5).
- ELYON, SCHEDA e SALVA nel menu di gioco non sono implementati; ZAINO mostra solo il contenuto.
- La creazione del personaggio (nome) è prevista nella Fase 5: per ora il nome è "ARI".
- NPC "vaga": nel disegno l'ordine di profondità usa la tile di arrivo (piccoli artefatti possibili).
- Grafica e sprite sono segnaposto originali (sprite 16×24); audio assente.
- Il tasto F7 non è coperto dal test automatico (testato solo il teletrasporto via codice).
- Verificata la compilazione solo su Linux (gcc 13, CMake 3.28); non su Windows/macOS.
