# ECLYRA — Vertical slice visiva

**Stato**: pronta per la revisione (fase B della sequenza del 2026-10-02).

**Obiettivo**: dimostrare che lo stile approvato funziona come prodotto completo, prima di produrre gli asset definitivi. Non è l'implementazione completa e non aggiunge contenuti.

## 1. Cosa contiene

È un eseguibile separato, `eclyra_slice`. Il gioco principale (`eclyra`) non cambia.

| Richiesta | Nella slice |
|---|---|
| Lunaria | Mappa vera del gioco con:<br>• terra battuta;<br>• piazza lastricata con la rosa del Lumen e aiuole;<br>• case, laboratorio, alberi, cartelli;<br>• NPC che si muovono e parlano;<br>• banner del luogo e suggerimento "Parla". |
| Route 1 | Mappa vera:<br>• sentiero, erba alta (le gambe ci affondano), acqua;<br>• NPC e oggetto raccoglibile;<br>• incontri selvatici. |
| Battaglia | Motore di lotta e IA veri. Layout approvato:<br>• Elyon di profilo;<br>• rosa dei comandi;<br>• carte delle mosse con dettaglio;<br>• squadra e zaino;<br>• cattura;<br>• prima sfida di Kael con `AI_INTRO`. |
| Menu principale | Ordine ufficiale: ELYON, ZAINO, BESTIARIO, MAPPA, SCHEDA, SALVA, OPZIONI. Nella slice SALVA e OPZIONI mostrano un avviso. |
| Bestiario | Codice dei Custodi:<br>• stati IGNOTO, VISTO, CATTURATO;<br>• filtri;<br>• voce a destra;<br>• voce a schermo intero. |
| Mappa | Asterra con le 7 regioni:<br>• nebbia sulle regioni non esplorate;<br>• luoghi;<br>• cursore che salta al luogo più vicino;<br>• posizione del giocatore;<br>• pannello della regione;<br>• barra dei Sigilli. |
| Dialogo | Pergamena con targa del nome. Testi veri dal database dei dialoghi, impaginati con il font Eclyra. |
| Scheda Elyon | Tre pagine:<br>• INFO;<br>• STATISTICHE, con le frecce della natura;<br>• MOSSE.<br>A sinistra Elyon sul cerchio, natura, abilità, oggetto, amicizia. |
| Evoluzione | Sequenza completa:<br>• annuncio;<br>• sagoma che alterna le forme;<br>• lampo;<br>• nuova forma.<br>È la stessa vista usata in lotta. |
| Titolo e scelta dello starter | Titolo approvato. La scelta fra PYRIN, AQUIL e MOSSEN serve a provare tutti e tre gli starter. |

**Solo i 9 Elyon di riferimento** compaiono, come richiesto. Per questo nella slice:
- gli incontri di Route 1 usano MOSSEN, AQUIL e PYRIN allo stesso livello delle specie previste;
- gli allenatori di Route 1 con altre specie le sostituiscono allo stesso modo.

Queste sostituzioni valgono solo nella slice (`main.c`, `SpecieSlice`) e non toccano i dati del gioco.

Gli interni (case, laboratorio, negozio) non fanno parte della slice: la porta mostra un avviso. Una sconfitta riporta davanti a casa.

## 2. Come si prova

```bash
cmake -S . -B build && cmake --build build -j
./build/slice/eclyra_slice            # gioca
./build/slice/eclyra_slice --touch    # controlli touch sempre visibili; il mouse fa da dito
./build/slice/eclyra_slice --tour docs/design/vertical_slice   # foto automatiche di tutte le schermate
```

Sul Mac basta CMake (`brew install cmake`); raylib si scarica da sola. Su Steam Deck si compila in modalità desktop (Linux).

**Comandi**:

| Azione | Tastiera | Controller | Touch / mouse |
|---|---|---|---|
| Muovi / scegli | WASD, frecce | croce, levetta | croce virtuale, tocco diretto sulle voci |
| Conferma | Z, Invio, Spazio | A (in basso) | A, tocco |
| Annulla | X, Esc | B (a destra) | B |
| Menu | TAB, M | Start | MENU |
| Corsa | Shift | X tenuto (anche RB) | — |
| Lotta: ZAINO / ELYON diretti | C / V | X / Y | tocco sul pulsante |

**Tasti di prova (desktop)**:
- F1: 16:9;
- F2: Steam Deck 16:10;
- F3: telefono 19,5:9;
- F4: iPad 4:3;
- F5: lotta selvatica;
- F6: lotta con Kael;
- F7: evoluzione;
- F8: cura;
- F9: controlli touch;
- F12: foto.

## 3. Come è costruita (e perché conta per l'architettura)

- **Logica del gioco invariata**:
  - la slice compila `eclyra_core` più `world.c`, `player.c`, `npc.c`, `battle_ui.c`, `ui_common.c` e `input.c` del gioco, senza modificarli;
  - sostituisce solo il loro livello di disegno (`slice/src/legacy_shim.c`).
  - È la prova pratica della separazione logica/rendering di `MULTIPLATFORM_ARCHITECTURE.md` §1.
- **Input astratto** (`azioni.c`):
  - tastiera, controller, mouse e touch producono le stesse azioni;
  - i controlli touch cambiano col contesto: esplorazione, dialogo, menu, lotta.
- **Canvas variabile** (`canvas.c`): 320–400 × 180–240 a scala intera, come nel prototipo iOS.
- **Asset**:
  - tutti generati da `tools/design/slice_export.py` con gli stessi generatori dei mockup approvati e la palette ufficiale;
  - nessuno stile nuovo;
  - provenienza registrata in `docs/assets/provenienza.csv`;
  - ogni asset si sostituisce senza toccare la logica: atlanti e coordinate in `slice/src/gen/`.

## 4. Verifiche già fatte

### B. Vertical slice
Il tour automatico gioca la slice da solo e salva **31 schermate per ciascuno dei 3 formati** (93 foto):
- `docs/design/vertical_slice/16x9_pc/`;
- `docs/design/vertical_slice/19x9_telefono/` (con i controlli touch);
- `docs/design/vertical_slice/16x10_steamdeck/`.

### C. Coerenza dei 9 Elyon
`python3 tools/design/valida_elyon.py` controlla le regole misurabili della STYLE BIBLE. Esito: **tutti i controlli superati**. Rapporto in `docs/design/verifica_elyon.md`, tavola in `docs/design/verifica_elyon.png`.

Regole controllate:
- riquadro e linea d'appoggio;
- ingombro per stadio e crescita lungo la linea;
- contorno;
- occhi;
- nodo di Lumen.

Da fare: le icone 32×32 sono ancora ridotte automaticamente; la STYLE BIBLE le vuole semplificate a mano.

### D. Interfaccia desktop, mobile e Steam Deck
Stesse schermate nei 3 formati:
- canvas 320×180 ×4 (16:9);
- canvas 390×180 ×4 (telefono);
- canvas 320×200 ×4 (Steam Deck).

Le HUD restano ai bordi e il contenuto essenziale nella zona sicura 320×180. Il testo più piccolo (font micro, 5 px) diventa 20 px fisici a 1280×800, sopra il minimo di 9 px richiesto da Steam Deck Verified.

Da verificare a mano:
- leggibilità reale sul telefono;
- comodità dei controlli touch con le dita.

### E. Prima battaglia con i tre starter
- **Simulazioni** (ctest, 4000 lotte per riga): PYRIN 86%, AQUIL 91%, MOSSEN 73% per il giocatore esperto.
- **Nella slice**, con il giocatore automatico che usa sempre la prima mossa: tre vittorie.
  - PYRIN in 25 turni;
  - AQUIL in 23;
  - MOSSEN in 13.
- **Da osservare**: a livello 5 le lotte sono lunghe, perché con la formula degli HP del documento gli Elyon hanno circa 70 HP e i colpi fanno 4–11 danni. Vale anche oltre AQUIL; va giudicato nei test reali (`docs/test/TEST_PRIMA_LOTTA.md`).

## 5. Da approvare insieme alla slice

1. **Dati del Bestiario mancanti nel database** (Design System §9: "Dati da aggiungere"). La slice mostra una **proposta** per i 9 Elyon, in `slice/src/bestiario.c`:

   | Elyon | Categoria | Altezza | Peso |
   |---|---|---|---|
   | PYRIN | Cucciolo di brace | 0,4 m | 5,2 kg |
   | PYRION | Corridore di brace | 0,9 m | 18,5 kg |
   | PYROVAR | Fucina vivente | 1,6 m | 96,0 kg |
   | AQUIL | Lontra-pinna (dal mockup approvato) | 0,5 m | 6,8 kg |
   | AQUILON | Danzatore d'onde | 1,0 m | 22,4 kg |
   | AQUARYS | Guida stellare | 1,8 m | 84,0 kg |
   | MOSSEN | Zolla di muschio | 0,4 m | 9,5 kg |
   | MOSSKA | Custode dei germogli | 0,9 m | 41,0 kg |
   | VERDANTOR | Collina che cammina | 2,0 m | 310,0 kg |

   La Nota dei Custodi (stato COMPLETO) non è mostrata: è lore e va scritta da te.
2. **Personaggi in 4 direzioni**: fronte, retro e lato sono derivati dal personaggio dei mockup. Le direzioni retro e lato sono nuove e vanno approvate.
3. **Piazza di Lunaria**:
   - la mappa vera ha una piazza ampia con aiuola e cartello: la slice la lastrica e ci intarsia la rosa del Lumen del mockup;
   - fontana e lampioni del mockup non ci sono, perché aggiungerli cambierebbe la mappa (ostacoli).
4. **Scelta dello starter con tre carte**: è una schermata di prova, fatta solo con componenti approvati. Nel gioco la scelta resta nel laboratorio, nella Fase 5.

## 6. Limiti noti della slice

- Niente audio: non ci sono ancora musiche e suoni definitivi.
- Niente salvataggi.
- Niente interni, PC, negozio.
- Lotte su un solo fondale (prato). Il cerchio dell'avversario è sempre turchese.
- La scheda allenatore usa lo sprite del personaggio ingrandito ×2, in attesa di un ritratto dedicato.
