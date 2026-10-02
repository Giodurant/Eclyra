# Changelog

## 0.4.2 — Vertical slice visiva

Aggiunto
- `slice/`: eseguibile `eclyra_slice`, separato dal gioco principale che non cambia. Usa la logica vera del gioco e la grafica del Design System v2.
  - Contenuti: Lunaria, Route 1, lotte (selvatica, cattura, Kael), menu ufficiale, squadra, Scheda Elyon, zaino, scheda allenatore, Bestiario, Mappa di Asterra, dialoghi, evoluzione, titolo, scelta dello starter.
  - Solo i 9 Elyon di riferimento.
  - Input astratto (tastiera, controller, mouse, touch contestuale), canvas variabile, tour automatico di foto (`--tour`).
- `tools/design/slice_export.py`: atlanti e font della slice, generati dagli stessi generatori dei mockup. Ci sono anche i personaggi in 4 direzioni.
- `tools/design/valida_elyon.py`: verifica automatica della STYLE BIBLE sui 9 Elyon, con tavola di confronto.
- `docs/VERTICAL_SLICE.md`, `docs/test/TEST_IOS_REALE.md`, `docs/test/TEST_PRIMA_LOTTA.md`, 93 foto di controllo in `docs/design/vertical_slice/`.

## 0.4.1 — Design v2 approvato, prima lotta di Kael, prototipo iOS

Aggiunto
- `AI_INTRO`, l'IA introduttiva per la prima sfida di Kael (Lv5 invariato). Regole:
  - niente stati sul giocatore;
  - mosse super efficaci al 20%;
  - esitazione calcolata a inizio lotta, in modo che ogni starter parta alla pari, più un elastico sugli HP;
  - una sola grazia sul colpo da KO.
- Nuovo messaggio di lotta: «X esita: non è ancora abituato ai comandi!».
- `tests/test_bilanciamento.c`: simulazioni dei 3 starter, giocatore esperto e principiante, con obiettivi verificati da ctest.
- Test delle eccezioni canoniche: Kain THORVAN Lv26 (evolve al 32), Nexus VENOROK Lv26 (evolve al 29).
- `prototypes/ios_sdl/`: prototipo tecnico iOS con raylib 5.5 e backend SDL2:
  - canvas a scala intera, azioni di input, touch, controller, audio in streaming;
  - pausa e ripresa, salvataggio atomico con CRC;
  - prova automatica (`--selftest`, o A+B tenuti sul dispositivo);
  - progetto Xcode via CMake e workflow di CI.
- `docs/DECISIONI.md`, `docs/ELYON_STYLE_BIBLE.md`, `docs/assets/provenienza.csv`.

Modificato
- Design System, Asset Spec e Architettura segnati come approvati.
- Asset Spec: regole per gli asset AI, controllo della provenienza nel validatore, fasce di appoggio e di ingombro allineate agli sprite di riferimento.

## 0.4.0 — Fase 4 (lotta, cattura, crescita, squadra e box)

Aggiunto
- `elyon.c/h` (core): istanza di Elyon con IV 0–15, 10 nature, formule delle statistiche del
  documento, curva XP floor(crescita·L³) senza overflow, level-up con HP in proporzione, amicizia,
  mosse (apprendimento e sostituzione), evoluzione mai prima del livello definito, variante Lumen 1/256.
  Squadra (max 6) e box 5×20 con aggiunta, deposito (mai squadra vuota), ritiro, compattazione, cura.
- `battle.c/h` (core): motore di lotta a eventi. Ordine per priorità, velocità e spareggio casuale;
  formula dei danni del documento con STAB 1.25, efficacia, critico 1.5 (6%, 12% per le mosse a critico
  alto), casuale 0.90–1.00, meteo e abilità, minimo 1; stati (veleno 1/8, grave n/16, bruciatura 1/16
  e ATK −20%, paralisi 25% e SPE ×0.5, sonno 1–3 turni, gelo 20% di scongelamento, confusione 1–3 turni
  e 40% di autocolpo); boost −6..+6; meteo di 5 turni; tutti gli effetti di mossa del database
  (stordimento, cura, radicamento, colpi multipli, contraccolpo, campo di tipo, riflessione, furia,
  blocco del cambio, ignora boost/abilità, equilibrio...); abilità (moltiplicatori di tipo e stat con
  condizioni, immunità di tipo e di stato, cure a fine turno, critico ridotto/alto, SCUDO LUMEN,
  veleno su attacco, effetto di campo di Aurion ogni 3 turni); cambio, cambio forzato al KO, oggetti,
  fuga, cattura solo dei selvatici (formula del documento, Core ×1/1.5/2.5/3.5, primordiale solo con
  evento), XP divisa tra i partecipanti (×1.5 contro allenatori, NUCLEO CONDIVISO 50%), premio in
  Lumen, interruzione narrativa di Vael a metà HP dell'ultimo Elyon, "dimenarsi" senza PP.
- IA a quattro livelli: SIMPLE (mossa casuale valida), STANDARD (danno atteso × precisione, valore delle
  mosse di stato, 15% di imprevedibilità), SMART (colpo da KO, cambio quando l'Elyon in campo cadrebbe),
  BOSS (come SMART più 1–3 cure d'emergenza limitate).
- `item_use.c/h` (core): effetto degli oggetti curativi, PP e rianimazione, condiviso da lotta e zaino.
- Schermata di lotta: transizione, pedane e sfondo con meteo, riquadri con nome, livello, barra HP
  animata, stato e XP; menu COMBATTI / ELYON / ZAINO / FUGA; mosse con PP, tipo, categoria ed efficacia;
  animazioni di entrata, attacco, colpo, KO, lancio del Core con scosse e stelle, boost, livello;
  richiesta di sostituzione della mossa; schermata di evoluzione (interrompibile con X).
- Sprite procedurali originali dei 100 Elyon, fronte e retro 64×64 (`tools/genera_elyon.py`): forma
  del corpo coerente in tutta la famiglia evolutiva, taglia per stadio, palette e tratti del tipo.
- Gioco: scelta provvisoria dello starter dalle capsule del laboratorio (Lv 5, con 5 CORE e 2 ERBA
  VITALE), incontri nell'erba alta della Route 1 (12% a passo, habitat ROUTE1), allenatori Teo e Nina
  e prima sfida di Kael (contro lo starter in vantaggio, come da documento), testi prima/dopo la lotta,
  la mamma cura la squadra, sconfitta = ritorno sicuro a casa con squadra curata e nessuna penalità.
- Menu di gioco: ELYON (lista, riepilogo con statistiche/natura/abilità/XP/mosse, riordino), ZAINO
  (uso degli oggetti con scelta dell'Elyon e della mossa), SCHEDA (nome, Lumen, tempo di gioco, Elyon,
  Sigilli), PC del box (deposito e ritiro, 5 box da 20).
- Formato mappe: direttive HABITAT, ALLENATORE (con segnaposto {STARTER}), CURATORE, PC, STARTER.
- Debug F1–F4, F6–F8 (vedi README).
- Test: `test_elyon` (3665 verifiche) e `test_battle` (20809 verifiche, tra cui 4000 lotte simulate
  con scelte casuali e controllo degli invarianti, determinismo con seme fisso); nuovi controlli in
  `test_mappe` e `test_dialoghi`; percorso automatico esteso con starter, lotta con Kael, incontro e
  cattura, menu, nuova mossa, evoluzione e sconfitta.

Modificato
- Partita contiene la collezione (squadra e box); il tempo di gioco ora avanza.
- Testo di Elian aggiornato per la scelta dalle capsule; varianti nuove per Kael, mamma, Elian,
  capsule, guardiano e allenatori della Route 1.
- I due PC del laboratorio aprono il box invece di mostrare un cartello.
- Allenatori ROUTE1_TEO e ROUTE1_NINA aggiunti al generatore (non canonici, segnati come Fase 4).

## 0.3.0 — Fase 3 (database)

Aggiunto
- 12 tipi elementali e matrice 12×12 costruita dalle regole del documento (super efficacia,
  resistenze, "super efficace e resistito = ×1", nessuna immunità); NEUTRO = TYPE_NONE.
- 100 specie (ID 001–100) con statistiche canoniche, tipi, abilità, rarità, probabilità di cattura,
  crescita, resa XP, learnset e una descrizione originale ciascuna; forme AURION VOID e ORIGINALE
  come forme di #100 (senza ID aggiuntivo).
- 100 mosse con effetti tradotti in campi strutturati (stato, stordimento, boost, cura, critico alto,
  colpi multipli, contraccolpo, campo, riflessione, furia, ...), priorità e flag (solo Aurion,
  solo evento, precisione automatica per le mosse su se stessi).
- Abilità 001–046 in un'unica tabella, alias 034/036/041/045 risolti verso l'originale; abilità di
  ogni specie secondo le eccezioni esplicite o il tipo primario.
- 33 evoluzioni canoniche con API GetEvolution / GetPreEvolution / livello minimo.
- 27 allenatori e boss (Kael iniziale e Campione nelle tre varianti, Nexus, 7 Guardiani con Sigilli,
  Vael intermezzo con interruzione narrativa, Lega, Vael finale, Aurion fasi 1–2, post-game).
- 8 habitat con le percentuali di incontro del documento.
- API: GetSpecies, GetSpeciesByName, GetMove, GetMoveByName, GetAbility, GetAbilityResolved,
  GetItem, GetTrainer, GetEvolution, GetHabitat, GetForm.
- `tools/genera_database.py` (fonte unica, con validazioni) e `docs/BESTIARIO.md` leggibile.
- Test `test_database`: 3782 verifiche su ID, nomi normalizzati, numeri autoritativi, matrice dei tipi
  (confronto con le tabelle ricopiate nel test), mosse, evoluzioni, abilità, learnset di Pyrin,
  allenatori, eccezioni di livello, habitat, oggetti.

## 0.2.0 — Fase 2 (mappe, porte, NPC, dialoghi)

Aggiunto
- Mappe caricate da file di testo (`assets/maps/*.map`) con parser robusto: messaggi di errore con
  riga e motivo, nessun crash su file malformati o mancanti.
- Porte degli edifici (si attivano urtando la porta) e uscite ai bordi (si attivano arrivando sulla
  tile), con dissolvenza in uscita/entrata e banner con il nome della zona.
- Mappe: Lunaria 40×30 (casa 8,18 · laboratorio 7×5 in 28,8 · negozio 6,7 · piazza 19,15 · uscita 19,0
  · casa dei vicini), Route 1 60×25 (entrata 30,24 · varco del Bosco 30,0 presidiato · 4 zone d'erba ·
  lago a est · CORE in 45,14) e 4 interni collegati (casa, laboratorio, emporio, casa dei vicini).
- 17 NPC (fermi, che si girano o che passeggiano entro un raggio), che si voltano verso il giocatore
  quando ci si parla; tra questi il Prof. Elian nel laboratorio e Kael davanti al laboratorio.
- Sistema di dialogo: testi in `assets/text/dialoghi.txt`, varianti in base ai flag (`@ID?FLAG`),
  nome di chi parla, segnaposto {GIOCATORE}/{RIVALE}, testo progressivo con salto (Z/X),
  a capo automatico e impaginazione su 3 righe, velocità dalle opzioni. 42 testi italiani.
- Cartelli e oggetti ispezionabili negli interni.
- Flag di gioco con nome, inventario e valuta Lumen; tabella oggetti completa (ID 1–15, 20–35).
- Oggetti a terra raccoglibili (CORE sulla Route 1) e menu ZAINO funzionante (solo consultazione).
- Pixel art segnaposto originale: casa blu, emporio, laboratorio, tileset interni (21 tile), 8 NPC.
- Debug F7: teletrasporto ciclico tra le mappe (solo build Debug).
- Test: `test_mappe` (caricamento, raggiungibilità di spawn/porte/NPC/cartelli/oggetti, destinazioni
  valide e senza rimbalzo, collegamento di tutte le mappe, coordinate di design, parser con input
  malformati) e `test_dialoghi` (varianti, pagine, segnaposto, salto, UTF-8, testi mancanti,
  riferimenti dalle mappe).

Modificato
- La nuova partita comincia in casa del giocatore; la Lunaria provvisoria generata da codice è stata rimossa.
- `eclyra_core` contiene ora mappe, flag, inventario, dialoghi e stato della partita (senza raylib).

## 0.1.0 — Fase 1 (scheletro)

Aggiunto
- Progetto C99 + CMake + raylib 5.5 (find_package oppure FetchContent).
- Finestra con risoluzione logica 320×180, scala intera centrata, filtro nearest, schermo intero.
- Game loop a passo fisso 60 Hz con input accumulato (stessa velocità su schermi 60/120 Hz).
- Schermata del titolo: NUOVA PARTITA, CONTINUA (disattivata senza salvataggio), OPZIONI, ESCI.
- Opzioni: velocità del testo (memorizzata, usata dai dialoghi dalla Fase 2), schermo intero.
- Lunaria provvisoria 40×30 con edifici alle coordinate di design (casa 8,18 · laboratorio 28,8 ·
  negozio 6,7 · piazza 19,15 · uscita 19,0), alberi, laghetto, cartello.
- Giocatore: movimento a griglia, giro sul posto, corsa con Shift, animazione a 3 fotogrammi.
- Camera che segue il giocatore e resta nei bordi della mappa; disegno in profondità (alberi ed
  edifici coprono chi sta dietro).
- Menu di gioco ELYON / ZAINO / SCHEDA / SALVA / OPZIONI (le prime quattro mostrano
  "funzione in una fase successiva": non sono ancora implementate).
- Asset mancanti sostituiti da una texture segnaposto invece di bloccare il gioco.
- Test: `test_utils` (RNG deterministico con valore di regressione, limiti, divisione per zero,
  stringhe). Test con screenshot `--screenshot-test` nella build Debug.
