# Changelog

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
