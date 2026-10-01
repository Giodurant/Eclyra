# Changelog

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
