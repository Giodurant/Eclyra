# PROJECT_STATE — ECLYRA

Ultimo aggiornamento: Fase 4 completata (versione 0.4.0). Proposta di nuova direzione artistica in revisione: `docs/DESIGN_SYSTEM.md`.

## Come riprendere il lavoro
1. Leggere `ECLYRA_MASTER_PER_CLAUDE` (specifica) e questo file.
2. `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j && ctest --test-dir build`.
3. `./build/eclyra --screenshot-test` (dalla cartella in cui salvare i PNG) e controllare le immagini.
4. Proseguire dalla prima voce non spuntata del piano.

## Revisione artistica (in corso)
- [x] Design System v1 proposta: `docs/DESIGN_SYSTEM.md`, mockup delle 18 schermate in `docs/design/mockup/`,
      tavole in `docs/design/`, generatori in `tools/design/` (palette, font originale, UI, sprite, mondo).
- [ ] Approvazione dell'utente (direzione, voci BESTIARIO e MAPPA nel menu, strategia per gli sprite finali).
- [ ] Implementazione in C: `src/palette.h`, `ui_kit.c/h`, nuovo tileset con autotile, sprite in tutte le misure,
      schermate aggiornate senza cambiare la logica.

## Piano di consegna
- [x] **Fase 1** — scheletro C99/CMake/raylib, title, game loop, camera, player.
- [x] **Fase 2** — mappe da file, collisioni, porte e interni, transizioni, NPC, dialoghi
      (testo progressivo con skip, velocità da opzioni), mappe Lunaria 40×30 e Route 1 60×25.
- [x] **Fase 3** — database 100 specie / 100 mosse / abilità 001–046 / oggetti, controlli di consistenza
      (12 tipi, matrice 12×12 testata, evoluzioni, riferimenti validi).
- [x] **Fase 4** — battle engine, IA, cattura, XP, level-up, evoluzione, inventario, box 5×20.
- [ ] **Fase 5** — vertical slice: Lunaria → Route 1 → Bosco Verdalia → Verdan → Lira → Sigillo Verde.
- [ ] **Fase 6** — 5 regioni, Guardiani, Nexus, Lega, boss, due finali.
- [ ] **Fase 7** — postgame, pixel art definitiva, audio, rifinitura, packaging.

## File
| File | Ruolo |
|---|---|
| `src/main.c` | avvio, percorso automatico con screenshot (Debug) |
| `src/game.c/h` | scene (titolo, opzioni, mondo, lotta), menu di gioco, richieste del mondo, debug F1–F8 |
| `src/elyon.c/h` | **core**: istanza di Elyon (stat, nature, IV, XP, mosse, evoluzione), squadra e box |
| `src/battle.c/h` | **core**: motore di lotta a eventi, IA, cattura, fuga, XP, generatore di incontri |
| `src/item_use.c/h` | **core**: effetti degli oggetti curativi (lotta e zaino) |
| `src/battle_ui.c/h` | schermata di lotta: riproduzione eventi, menu, animazioni, nuova mossa, evoluzione |
| `src/schermate.c/h` | menu ELYON (riepilogo, riordino), ZAINO con uso, SCHEDA, PC del box |
| `src/ui_common.c/h` | barre HP/XP, colori ed etichette dei tipi, sprite degli Elyon, righe squadra |
| `tools/genera_elyon.py` | sprite procedurali fronte/retro dei 100 Elyon |
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
| `src/world.c/h` | mappa corrente, NPC, porte e transizioni, interazioni, camera, starter, incontri, richieste di lotta |
| `src/utils.c/h` | **core**: RNG, clamp, divisione sicura, stringhe, lettura file |
| `src/data/types.c/h` | **core**: 12 tipi e matrice di efficacia dalle regole del documento |
| `src/data/species_data.*` | **core** (generato): 100 specie, learnset, descrizioni, forme |
| `src/data/moves_data.*` | **core** (generato): 100 mosse con effetti strutturati |
| `src/data/abilities_data.*` | **core** (generato): abilità 001–046 con alias |
| `src/data/evolution_data.*` | **core** (generato): 33 evoluzioni |
| `src/data/trainers_data.*` | **core** (generato): allenatori, Guardiani, Lega, boss |
| `src/data/encounters_data.*` | **core** (generato): 8 habitat |
| `tools/genera_database.py` | fonte unica del database, con validazioni |
| `docs/BESTIARIO.md` | dati in forma leggibile per la revisione di design |
| `assets/maps/*.map` | 6 mappe (generate da `tools/genera_mappe.py`, modificabili) |
| `assets/text/dialoghi.txt` | 45 voci di dialoghi e cartelli (varianti comprese) |
| `assets/sprites/elyon_*.png` | fogli 640×640 (10×10 sprite 64×64) fronte e retro |
| `tools/genera_grafica_fase2.py` | pixel art di edifici, interni, NPC |
| `tests/test_*.c` | test CTest (utils, mappe, dialoghi, database, elyon, battle) |

## Decisioni prese
- Tutto il disegno avviene su una render texture 320×180, scalata di un fattore intero.
- Logica a 60 passi/s indipendente dal refresh; gli eventi "premuto" si accumulano fino al passo.
- `eclyra_core` è una libreria senza raylib: lì andranno dati e regole di lotta, così i test
  non richiedono una finestra.
- Esc non chiude la finestra (serve come "indietro"): si esce da ESCI o chiudendo la finestra.
- Porte = warp "su urto" (la tile della porta e' bloccata); uscite = warp "all'arrivo". Le
  destinazioni cadono una tile dentro la mappa di arrivo, mai su un'altra uscita (testato).
- I testi stanno in un file unico; le varianti per i progressi della storia si scrivono `@ID?FLAG`.
- Database generato da uno script con le tabelle canoniche: si modifica lo script, non il C.
- Regole decise in Fase 3 (da confermare dal designer):
  - Rarità: #100 primordiale; #074–080 e #099 leggendari; starter e specie con totale ≥ 480 rari;
    le altre comuni (catchRate 0.20 / 0.08 / 0.015 / 0.0).
  - Crescita decisa dalla forma base della famiglia (XP coerente all'evoluzione): starter 1.0,
    totale < 300 → 0.8, leggendari o totale ≥ 560 → 1.2, altrimenti 1.0.
  - Resa XP: baseYield = max(4, round(totale / 50)) — da bilanciare nei test della Fase 4.
  - Learnset generati con la regola del documento (Lv 1, 5, 9, 13, 18, 24, 30, 38, 46, 55) e tetti di
    potenza per livello (≤50 al Lv 1, ≤70 al 9, ≤85 al 18, ≤100 al 30); espliciti per PYRIN/PYRION/
    PYROVAR (Pyrovar aggiunge IMPATTO FERROSO Lv 42 e CANNONE FERROSO Lv 50) e per AURION.
  - Forme di AURION: statistiche di base (il documento non ne specifica altre).
  - Squadre canoniche con Elyon sotto il livello di evoluzione registrate come eccezioni esplicite:
    Lira (LEAFORA Lv 18, decisione canonica), Kain (THORVAN Lv 26, evolve al 32) e Comandante Nexus
    (VENOROK Lv 26, evolve al 29). **Kain e Comandante sono da confermare**: alternativa alzare i livelli.
- Regole decise in Fase 4 (da confermare dal designer):
  - Fattore di stato nella cattura: sonno/gelo ×1.5, altri stati ×1.25, nessuno ×1.
  - Fuga: sempre riuscita se SPE ≥ SPE nemica, altrimenti clamp(0.25 + 0.5·rapporto + 0.2·(tentativi−1),
    0.25, 1); mai dagli allenatori.
  - Cattura animata con tre scosse indipendenti di probabilità p^(1/3): la probabilità totale resta p.
  - NUCLEO CONDIVISO: chi non lotta riceve metà dell'XP di un partecipante.
  - Senza PP l'Elyon "si dimena": COLPO senza consumare PP e contraccolpo 1/4 degli HP massimi.
  - Cambi, oggetti e fuga avvengono prima delle mosse; chi entra dopo un KO non agisce nel turno.
  - Se cadono entrambi gli ultimi Elyon nello stesso turno vince l'avversario (sconfitta senza penalità).
  - Vael intermezzo: la lotta si interrompe quando l'ultimo Elyon scende a metà HP o meno (anche con
    un colpo da KO); nessun premio e nessuna XP per quel colpo.
  - Cure dei boss: 1–3 (metà della squadra, limitata), ciascuna ridà il 50% degli HP; scattano sotto il 30%.
  - Sconfitta: ritorno a casa con squadra curata, nessuna perdita di Lumen (il documento non ne prevede).
  - Incontri: 12% a passo nell'erba alta; servono un Elyon in grado di lottare e il flag HA_ELYON.
  - Condizione "notte" delle abilità sempre falsa finché non esiste il ciclo giorno/notte.
- Nessuna coppia della tabella dei tipi è sia super efficace sia resistita: la regola ×1 è implementata
  e testata con la matrice, ma con i dati attuali non si attiva.

## Problemi noti / da fare
- **Bilanciamento della prima sfida di Kael** (Lv 5 contro lo starter in vantaggio, come da documento):
  su 2000 lotte simulate con la mossa migliore il giocatore vince il 13,5% con PYRIN, il 3% con MOSSEN e
  lo 0,1% con AQUIL; al Lv 9 sale al 78% / 54% / 47%. La sfida si attiva solo parlando con Kael, quindi
  ci si può allenare prima, e perdere non ha penalità. Da decidere in Fase 5 (es. prima sfida senza
  conseguenze come scena, IA di Kael più blanda, o livelli diversi).
- Scelta dello starter provvisoria (capsule + SÌ/NO): la scena con Elian, Kael e la creazione del
  personaggio arriva nella Fase 5. Anche i 5 CORE e le 2 ERBA VITALE consegnati sono provvisori.
- Gli allenatori sfidano solo quando ci si parla (niente "sguardo" a distanza per ora).
- SALVA non è implementato (Fase 5): CONTINUA nel titolo resta disattivato.
- F5 non ha funzione; F6 imposta solo i flag dei Sigilli.
- Il varco nord della Route 1 verso il Bosco Verdalia è presidiato: la mappa del Bosco arriva con la Fase 5.
- Il commesso dell'emporio non vende ancora (negozio: Fase 5).
- La creazione del personaggio (nome) è prevista nella Fase 5: per ora il nome è "ARI".
- Sprite degli Elyon procedurali e segnaposto (sagome semplici, nessun boss 96/128 px); audio assente.
- La schermata di lotta non mostra lo sprite dell'allenatore e non mostra i boost attivi.
- Le mosse "solo evento" (RITORNO DEL LUMEN, SIGILLO DI AURION) non hanno ancora il loro evento.
- NPC "vaga": nel disegno l'ordine di profondità usa la tile di arrivo (piccoli artefatti possibili).
- Il test automatico non copre i tasti di debug F1–F8 (solo le funzioni richiamate via codice).
- Verificata la compilazione solo su Linux (gcc 13, CMake 3.28); non su Windows/macOS.
