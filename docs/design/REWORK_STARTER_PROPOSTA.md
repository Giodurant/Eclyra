# ECLYRA — Rework degli starter: analisi e proposta (Fasi A–B)

**Stato**: PROPOSTA, in attesa di approvazione. Nessun file di gioco e nessuno sprite è stato modificato.
Concept di direzione ricevuto: `art_src/concept/starter_rework/riferimento_direzione_2026-10-19.png`.

---

## 1. Fase A — Cosa c'è oggi nel repository

| Aspetto | Stato attuale | Fonte |
|---|---|---|
| Dimensione | 64×64 per fotogramma (boss 128×128). Facoltativa una versione **80×80 disegnata** per il lato giocatore | ASSET_SPEC §3 |
| Vista | 3/4 di profilo, **muso verso sinistra**. Il lato giocatore è lo specchio | STYLE BIBLE §1 |
| Appoggio | pixel opaco più basso a y = 59–60 (base). La riga 63 contiene solo l'ombra al suolo | STYLE BIBLE §1 |
| Ingombro di uno stadio base | 40–49 × 37–45 px; 17–28% di pixel opachi | STYLE BIBLE §1, §7 |
| Luce e toni | luce dall'alto a sinistra; 4 toni per materiale; niente sfumature né dithering | STYLE BIBLE §2 |
| Palette | rampe di 5 toni; al massimo 5 materiali, più occhi e Lumen | STYLE BIBLE §3, `tools/design/palette.py` |
| Contorno | 1 px colorato e selettivo; linee interne dove le parti si sovrappongono | STYLE BIBLE §4 |
| Occhi | 2 visibili, sclera, iride a 3 toni, pupilla 2×2, 2 riflessi | STYLE BIBLE §5 |
| Nodo di Lumen | cristallo a rombo di 3–4 px nello stadio base, sulla fronte o sul petto | STYLE BIBLE §6 |
| Fotogrammi | riposo, respiro (+1 px alle parti `su`), battito, affondo, colpito | ASSET_SPEC §3 |
| Produzione attuale | **procedurale**: forme geometriche (ellissi, capsule, poligoni) e cel shading automatico | `tools/design/elyon_design.py`, `sprites.py` |
| Uso nella slice | atlante `slice/assets/elyon.png`, prodotto da `tools/design/slice_export.py`. Per ogni specie: 64 px (fronte), 70 px (nemico) e 80 px (mio). 70 e 80 si ottengono **ri-renderizzando le forme** a scala 1,1 e 1,25 | `slice_export.py:205` |
| Uso nel gioco principale | `assets/sprites/elyon_fronte.png` / `elyon_retro.png`: griglia 10×10 di celle 64×64, ancora la grafica provvisoria delle Fasi 1–4 | `src/graphics.c:19`, `provenienza.csv` |
| Controlli | `tools/design/valida_elyon.py` (taglia, appoggio, % opachi, alfa) e checklist umana. `tools/assets/valida.py` non esiste ancora | ASSET_SPEC §10 |
| Provenienza | `docs/assets/provenienza.csv`, obbligatoria. Prompt e concept vanno in `art_src/` | ASSET_SPEC §9.3 |

### Perché gli sprite attuali sembrano un prototipo

Il problema non sono le regole, ma il metodo di produzione: corpi fatti di ellissi e capsule.
- La silhouette è "a blocchi", con zampe a cilindro.
- La testa è sproporzionata e gli occhi sono troppo tondi e sporgenti.
- Le fiamme e le pinne sono coni ripetuti.
- La vista del muso dà un'espressione stralunata.

Le regole della Style Bible (luce, toni, contorni, occhi, Lumen) restano valide. Va cambiato il **disegno**.

## 2. Tre punti da decidere prima di disegnare

### 2.1 I 9 sprite di riferimento sono congelati

`DECISIONI.md` (2026-10-02, punto 3) e la Style Bible dichiarano **approvati e congelati** i 9 sprite attuali come riferimento di tutti gli Elyon. Questo rework è quindi un cambio di una decisione congelata, e va registrato come tale.

**Proposta**:
- nuova voce in `DECISIONI.md`: "i 9 sprite di riferimento vengono ridisegnati a mano";
- **le regole della Style Bible §1–§12 restano** (misure, luce, contorni, occhi, Lumen);
- cambiano solo le immagini di riferimento e il metodo di produzione (§11 della Style Bible).

### 2.2 Il concept allegato e l'originalità (punto critico)

Il concept è ottimo per **palette, atmosfera, qualità della luce, espressività degli occhi e tenerezza**. Però le tre creature hanno **la stessa anatomia**: lo stesso cucciolo volpe-gatto, cambiano solo colori e accessori.

Ci sono due problemi:
1. Contraddice il §6 della tua richiesta: silhouette diverse, niente "stessa anatomia ricolorata".
2. Il cucciolo-volpe che diventa variante di fuoco, d'acqua o di foglia è un modello di design molto riconoscibile di un franchise esistente:
   - pinne e collare dell'acquatico;
   - orecchie a foglia del vegetale.

   La checklist di originalità (Style Bible §12.9, Design System §16) lo segnalerebbe.

**Proposta**: dal concept prendo colori, luce, occhi e personalità. Anatomia e silhouette restano le **specie canoniche** del Design System §14 e del Bestiario:
- PYRIN = **lince di brace**;
- AQUIL = **lontra-pinna**;
- MOSSEN = **cinghialino di muschio**.

Queste tre specie hanno già silhouette diverse e descrizioni nel Bestiario che le richiamano: "pinne a goccia", "una zolla di muschio con due occhi vispi".

### 2.3 Dettaglio e risoluzione

Nel concept le creature sono larghe circa 300 px. Nel gioco lo stadio base occupa **circa 45×42 px** su un canvas di 320×180. A quella scala il dettaglio del concept non si può riprodurre pixel per pixel: ogni ciuffo di pelo diventerebbe rumore.

La qualità commerciale a 64×64 si ottiene così:
- forme principali chiare;
- pochi dettagli scelti con cura (occhi, tratto distintivo, Lumen);
- contorno pulito.

**Proposta**: resto a **64×64**, come da specifica congelata. La versione **80×80 disegnata** per il lato giocatore (già prevista in ASSET_SPEC §3) la valuto solo dopo la validazione.

## 3. Metodo di produzione proposto

Questo ambiente non ha un generatore di immagini. Anche se ci fosse, un'immagine generata e poi ridotta a 64 px andrebbe comunque ridisegnata (ASSET_SPEC §9.2). Propongo quindi la **pixel art piazzata a mano, in un sorgente testuale riproducibile**:

1. **Sorgente**: `art_src/elyon/<nnn>_<nome>.txt`.
   - Griglia 64×64 in cui ogni carattere è un indice di colore: `.` è trasparente, mentre `a`–`e` sono i 5 toni del materiale 1, e così via.
   - Più **livelli**: corpo, parti `su` per il respiro, occhi chiusi per il battito, fiamma o elemento animabile.
   - Si modifica a mano, carattere per carattere. Si può importare ed esportare con LibreSprite, Pixelorama o Aseprite.
2. **Script** `tools/design/pixel_elyon.py`. Legge il sorgente, compone i 5 fotogrammi standard ed esporta i PNG con nomi conformi (`elyon_001_pyrin.png`, 320×64; `_icona` 64×32).
3. **Validazione**: `valida_elyon.py`, esteso dove serve con:
   - colori solo dalle rampe dichiarate;
   - nessun pixel isolato;
   - contorno presente;
   - alfa solo 0/255, tranne l'ombra al suolo e l'alone del Lumen.
4. **Tavole di verifica**: a 1×, 4×, 6× (telefono), in sagoma nera, a 32×32 e sul cerchio runico della lotta, affiancate agli sprite attuali.
5. **Provenienza**:
   - origine "manuale" per gli sprite, perché ogni pixel è piazzato nel sorgente;
   - il concept allegato va registrato come "generativo, solo ispirazione" (mi serve sapere con quale strumento è stato fatto: §5).

Gli sprite procedurali attuali **non vengono cancellati**: restano le schede in `elyon_design.py`, la cronologia git e una copia dei PNG in `art_src/archivio/`.

## 4. Fase B — Schede visive

Regole comuni ai tre:
- stessa luce e stesso contorno;
- occhi di raggio 3 e 2, con espressione felice o decisa;
- un nodo di Lumen di 3–4 px;
- 4 materiali, di cui uno neutro chiaro;
- taglia dello stadio base (84%).

Le silhouette si distinguono per **direzione dominante**: PYRIN **verticale**, AQUIL **orizzontale**, MOSSEN **a cupola**.

### PYRIN — lince di brace (FIAMMA)

| Voce | Scelta |
|---|---|
| Silhouette | Compatta, seduta in avanti, petto alto. **Forma a V**: orecchie a punta con ciuffi da lince da un lato, coda alzata dall'altro. Ingombro circa 44×42 px |
| Tratto distintivo | **Coda corta e folta che finisce in una fiamma a goccia**, grande circa quanto la testa. Nucleo giallo, corpo arancio, bordo rosso, senza contorno (è emissiva). Si anima nei fotogrammi di riposo e respiro |
| Testa | Muso corto, guance con collare di pelo crema a tre punte, interno delle orecchie bruno scuro |
| Occhi | Iride ambra (rampa brace), palpebra superiore spessa: sguardo **deciso e vivace** |
| Materiali | pelo arancio-rosso (rampa `M_PYR.pelo`), crema (petto, muso, collare), fuliggine (punte delle orecchie, zampe "bruciacchiate"), fiamma (emissiva) |
| Lumen | gemma color brace di 3 px sulla fronte, tra le orecchie: è il punto "che brilla quando si fida" |
| Dal concept | coda fiammeggiante, collare crema, toni ambra e rosso, braci di 1 px attorno alla coda (solo nell'animazione) |
| Da evitare | fiamme sparse su testa e corpo, criniera di fuoco (è il tratto di PYRION), contorno nero uniforme |
| Riposo | la fiamma ondeggia (3 forme in ciclo), il petto respira di 1 px |

### AQUIL — lontra-pinna (ACQUA)

| Voce | Scelta |
|---|---|
| Silhouette | **Bassa e allungata**, in orizzontale: corpo di lontra disteso, testa alta e attenta, coda piatta a timone. Ingombro circa 49×37 px. In sagoma nera non si confonde con PYRIN |
| Tratto distintivo | **Pinne a goccia** ai lati della testa, al posto delle orecchie: membrana traslucida (rampa `pinna`) con la punta a goccia di cristallo. In più, una pinna dorsale bassa |
| Coda | larga e piatta, con una pinna a mezzaluna in punta. Ha la stessa forma a goccia delle pinne: è la "firma" della famiglia |
| Testa | muso arrotondato con vibrisse (2 pixel chiari), ventre e gola bianco-azzurri, zampe palmate |
| Occhi | iride ciano-marea, grandi e lucidi: **curioso** |
| Materiali | pelo blu (`M_AQU.pelo`), ventre bianco-azzurro, pinna ciano, Lumen azzurro |
| Lumen | goccia-cristallo di 3–4 px sul petto (si distingue dalla gemma frontale di PYRIN) |
| Dal concept | palette blu, ciano e bianco; pinne cristalline; freschezza, contrasto bianco e blu |
| Da evitare | collare a ventaglio e orecchie a pinna "da volpe" del concept (troppo vicini a un design esistente), bolle dentro lo sprite (sono effetti a parte) |
| Riposo | le pinne a goccia si alzano e abbassano di 1 px, la coda batte |

### MOSSEN — cinghialino di muschio (NATURA)

| Voce | Scelta |
|---|---|
| Silhouette | **A cupola**: corpo tondo e basso, zampe corte e robuste, grugno in avanti. Sul dorso una **zolla di muschio** rialzata con un germoglio a due foglie. Ingombro circa 44×40 px |
| Tratto distintivo | **Germoglio a due foglie** che spunta dalla zolla di muschio: cresce lungo la linea (felci in MOSSKA, alberelli in VERDANTOR) |
| Testa | grugno largo color crema, due piccole zanne a germoglio (2 px), orecchie a foglia corta e pendente (forma di foglia, ma inserite come orecchie suine e non dritte) |
| Corpo | pelo bruno-terra con striature chiare da cucciolo (un tono più scure, seguono la luce). Il muschio copre spalle e dorso, con bordo irregolare ma pulito |
| Occhi | iride verde-lime, palpebra morbida: **felice, vispo** |
| Materiali | pelo bruno (`M_MOS.pelo`), muschio, foglia, crema (grugno, zanne, zoccoli) |
| Lumen | seme di Lumen verde di 3 px alla base del germoglio |
| Dal concept | palette verde, verde chiaro, crema e marrone; foglie nette a due toni; venature di 1 px |
| Da evitare | foglie sparse su tutto il corpo, orecchie a foglia dritte "da volpe", coda a ricciolo vegetale del concept |
| Riposo | il germoglio oscilla, il corpo respira |

### Tabella di coerenza

| | PYRIN | AQUIL | MOSSEN |
|---|---|---|---|
| Massa dominante | verticale (V) | orizzontale | cupola |
| Testa / corpo | 1 : 1 | 0,8 : 1 | 0,9 : 1 |
| Tratto distintivo | coda-fiamma | pinne a goccia e coda a timone | germoglio sulla zolla |
| Lumen | fronte | petto | base del germoglio |
| Carattere | deciso, vivace | curioso | felice, tranquillo |
| Materiali | 3 + emissivo | 4 | 4 |

## 5. Direzione delle evoluzioni (solo indicativa, Fase G)

- **PYRION**: lince adolescente slanciata; la fiamma della coda si allunga e compare una criniera corta di brace. **PYROVAR**: grande felino da fucina, con placche d'acciaio e ottone (secondo tipo METALLO) e la stessa coda-fiamma ingrandita.
- **AQUILON**: lontra lunga e flessuosa, pinne più ampie, prime stelle sulla coda. **AQUARYS**: creatura di marea con pinne stellate e cresta astrale (secondo tipo ASTRALE).
- **MOSSKA**: cinghiale giovane con felci sul dorso e spalle di pietra. **VERDANTOR**: colosso con dorso di roccia e alberelli, zanne di pietra (secondo tipo ROCCIA).

Le evoluzioni seguono il Design System §14 e partono solo dopo l'approvazione dei tre base.

## 6. Piano delle fasi successive

| Fase | Contenuto | Ti fermo per approvare? |
|---|---|---|
| C | Sorgenti `.txt` e script; PYRIN, AQUIL, MOSSEN: fotogramma di riposo e animazione di riposo | sì, insieme alla D |
| D | Tavola di verifica: 1×, 4×, sagome, 32×32, cerchio runico, confronto con gli attuali; screenshot della slice con gli sprite nuovi in una build locale (non pubblicata) | **sì** |
| E | Correzioni dalle tue note | sì, se ci sono cambi di forma |
| F | Integrazione: `slice_export.py` usa i PNG disegnati per i 3 starter; il gioco principale aggiorna le celle 0/3/6 dei fogli; copia degli attuali in `art_src/archivio/`; provenienza; `ctest` e validatore | resoconto |
| G | Le 6 evoluzioni, con la stessa procedura | sì, prima di PYROVAR, AQUARYS e VERDANTOR |
| H | Style Bible §11 (nuovo metodo), nuove tavole di riferimento, voce in DECISIONI | resoconto |

Nota tecnica per la Fase F: oggi la slice usa versioni a 70 e 80 px ottenute ri-renderizzando le forme procedurali. Con la pixel art questo non si può fare: una scala non intera rovina i pixel. Per la prova userò il **64 px su entrambi i lati**. È una modifica di soli valori di posizionamento in `slice_export.py` e `vista_lotta.c`, senza toccare la logica di lotta. La versione 80×80 per il lato giocatore resta una scelta da fare dopo.

Gameplay, statistiche, mosse, tipi, evoluzioni, Kael e architettura multipiattaforma **non vengono toccati**.
