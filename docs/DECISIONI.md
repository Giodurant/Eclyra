# ECLYRA — Registro delle decisioni

Le decisioni di progetto prese dal designer (Giorgio). Ciò che è qui è **congelato**: si cambia solo con una nuova decisione esplicita, registrata con la data.

---

## 2026-10-02 — Approvazione del Design System v2

| # | Tema | Decisione | Dove è applicata |
|---|---|---|---|
| 1 | Le 12 schermate | **Approvate.** Lo stile generale va congelato. Non si rifà il design da zero; sono ammesse solo rifiniture di leggibilità, spaziature e dettagli UI. | `ECLYRA_DESIGN_SYSTEM.md` (stato: approvato), `docs/design/mockup_v2/` |
| 2 | Battaglia | **Direzione definitiva**, uno degli elementi distintivi di ECLYRA (dettagli sotto). | Design System §11 |
| 3 | Sprite di riferimento | **Approvati**: i 9 sprite delle tre linee starter sono la STYLE BIBLE di tutti gli Elyon. Gli altri 91 devono derivare dalle regole, non imitare a caso lo stile. | `ELYON_STYLE_BIBLE.md`, `docs/design/tavola_starter.png` |
| 4 | iOS / multipiattaforma | Prima di sviluppare oltre: **prototipo tecnico iOS con SDL**. iOS non è impossibile: è il principale rischio tecnico, da validare subito. Se il prototipo è stabile, si congela l'architettura multipiattaforma e si continua. | `prototypes/ios_sdl/`, `PROTOTIPO_IOS.md` |
| 5 | Asset AI | **Approvati**, con le condizioni sotto. | `ECLYRA_ASSET_SPEC.md` §9.3, §10 |
| 6 | Kain | **Eccezione canonica confermata**: THORVAN Lv26 (evoluzione normale Lv32). Il livello non si modifica. | dati dell'allenatore `GUARDIANO_KAIN`, test in `tests/test_database.c` |
| 7 | Comandante Nexus | **Eccezione canonica confermata**: VENOROK Lv26 (evoluzione normale Lv29). Il livello non si modifica. | dati dell'allenatore `NEXUS_COMMANDER`, test in `tests/test_database.c` |
| 8 | Prima lotta con Kael | Kael **resta al Lv5**. Si rende più blanda la prima IA, come lotta introduttiva/tutorial. La sconfitta resta senza penalità. Simulazioni da rifare con tutti e tre gli starter. | `AI_INTRO` in `src/battle.c`, `tests/test_bilanciamento.c` |

**Battaglia (punto 2)**:
- Elyon di profilo, uno di fronte all'altro: giocatore a sinistra, avversario a destra.
- Rosa dei comandi ispirata alla disposizione A/B/X/Y:
  - basso = LOTTA (A);
  - destra = FUGA (B);
  - sinistra = ZAINO (X);
  - alto = ELYON (Y).
- Le quattro mosse sono carte, selezionabili con controller o tastiera e direttamente con il touch.

**Asset AI (punto 5)**. Si usano perché non c'è budget per un pixel artist esterno. Ogni asset destinato alla build finale deve:
- essere controllato e rifinito;
- rispettare la palette e la pixel art di ECLYRA;
- avere la provenienza registrata;
- conservare, quando possibile, la sorgente o il prompt.

Su Steam l'uso di contenuti generativi AI distribuiti nel gioco va dichiarato nel Content Survey.

**Obiettivo dichiarato**: non rifare il progetto, ma congelare ciò che funziona e validare subito i due rischi principali, iOS e bilanciamento della prima battaglia.

### Esiti

**Bilanciamento della prima lotta con Kael.** Simulazioni con semi fissi, 4000 lotte per riga, Lv5 contro Lv5, nessun oggetto.

| Starter del giocatore | Prima (IA casuale), esperto | Dopo (`AI_INTRO`), esperto | Dopo, principiante |
|---|---|---|---|
| PYRIN (contro AQUIL) | 12,1% | **85,8%** | 78,1% |
| AQUIL (contro MOSSEN) | 0,2% | **90,5%** | 73,6% |
| MOSSEN (contro PYRIN) | 3,2% | **73,4%** | 55,9% |

- "Esperto" usa la mossa con più danno atteso; "principiante" sceglie a caso fra le mosse d'attacco.
- Obiettivi, verificati da `ctest`:
  - esperto fra il 70% e il 95%;
  - principiante almeno 40%;
  - al massimo 20 punti di differenza fra gli starter.
- Da tenere d'occhio (non bloccante): con AQUIL la lotta dura in media 24–29 turni e l'Elyon di Kael esita in circa 6 turni su 10. La causa sono i dati: a Lv5 AQUIL ha solo mosse deboli o resistite contro MOSSEN, che in più si cura con RADICE_VIVA. L'IA lo compensa, ma la lotta resta lunga. Se in gioco risultasse noioso, la soluzione è nei dati (mosse di AQUIL a Lv5), non nel livello di Kael.

**Prototipo iOS.**
- Prova automatica superata su Linux con la stessa strada SDL: 20/20 prove, e 14/14 nel ramo mobile.
- La prova su iPhone/iPad reale va fatta su un Mac con Xcode, seguendo il protocollo di `PROTOTIPO_IOS.md` §4.
- **Esito sul dispositivo: in attesa.**
