# ECLYRA — Design System v1

Proposta di direzione artistica e di interfaccia. Sostituisce lo stile "prototipo" delle fasi 1–4.
Gameplay, storia, dati, motore di lotta e architettura del codice **non cambiano**.

Tutto ciò che è mostrato qui è generato da script nel repository (`tools/design/`), a 320×180 come
nel gioco: le stesse primitive diventeranno gli asset caricati dal codice C.

| Materiale | Dove |
|---|---|
| Mockup delle 18 schermate (1× e 3×) | `docs/design/mockup/` |
| Tavole: palette, tipografia, componenti, sprite | `docs/design/tavola_*.png` |
| Font originale (atlante per raylib) | `docs/design/font_eclyra_corpo.png` |
| Generatori | `tools/design/palette.py`, `font.py`, `ui.py`, `sprites.py`, `elyon_design.py`, `mondo.py`, `mockups.py`, `tavole.py` |

Rigenerare tutto: `python3 tools/design/tavole.py && python3 tools/design/mockups.py` (richiede Pillow, NumPy, SciPy).

---

## 1. Analisi dell'immagine di riferimento

**Cosa prendiamo**
- Pannelli **blu notte** con **cornici d'oro** e piccole gemme agli angoli: è il tratto più riconoscibile del riferimento.
- **Contrasto alto** tra UI scura e mondo luminoso e saturo; testo chiaro su scuro, testo scuro solo su superfici chiare.
- Overworld **caldo e dettagliato**: tetti, finestre accese, lampioni, fiori, acqua con riflessi, alberi a chioma piena.
- Il **cristallo azzurro** come simbolo ricorrente (logo, oggetti, mappa): diventa il nostro "Lumen".
- Mappa del mondo **colorata per regione**, con etichette in targhette e i sette Sigilli in basso.
- Gerarchia chiara nelle schermate: titolo in targa, contenuto in pannelli, suggerimenti in basso.

**Cosa NON prendiamo (troppo vicino a Pokémon / Nintendo)**
- Protagonista con **cappellino rosso** → il nostro Esploratore ha capelli scoperti, fascia con lente di Lumen e **sciarpa turchese**.
- Edificio a **tetto rosso** tipo "centro cure" → a Lunaria i tetti sono **ardesia blu con colmo d'oro e pinnacolo di cristallo**; il laboratorio ha una **cupola di vetro** con un cristallo.
- Icona a **sfera bicolore** nello zaino → il **Core** è una sfera di cristallo in un **anello d'oro**.
- Etichetta **"PS"** → usiamo "HP" (termine del documento), in micro-font oro.
- **Pedana ovale d'erba** sotto gli Elyon → **cerchio runico**: disco di terra battuta con anello di pietra e rune di Lumen accese nel colore della situazione.
- Il riferimento è un'illustrazione ad alta risoluzione: la traduciamo in **vera pixel art** a 320×180, senza sfumature morbide né pixel di dimensioni diverse.

## 2. Principi

1. **Il Lumen è la firma.** Ogni schermata contiene almeno un cristallo, un bagliore o una runa turchese.
2. **Notte e oro per l'interfaccia, colore pieno per il mondo.** La UI non compete con gli Elyon.
3. **Leggibilità prima di tutto.** Silhouette chiare, contorni scuri, testo mai sotto 7 px, colore mai unica informazione.
4. **Un solo pixel.** Tutto a 1× logico: niente scale non intere, rotazioni, sfocature o font vettoriali.
5. **Coerenza di famiglia.** Evoluzioni, regioni e schermate condividono forme, materiali e regole di luce.

## 3. Risoluzione, griglia, piattaforme

- Risoluzione logica **320×180**, scala intera: ×4 (1280×720), ×6 (1920×1080, telefoni 1080p), ×8 (2560×1440), ×12 (4K). Bande nere solo se la scala non è esatta.
- Tile **16×16**. Griglia dell'interfaccia: **2 px**; margini dei pannelli dal bordo: **4–6 px**.
- **Mobile**: aree toccabili ≥ **16×16 logici** (circa 7–8 mm su un telefono a ×6); i comandi di lotta sono già pulsanti 68×17. Margine di sicurezza di 8 px per notch e bordi curvi.
- **Steam**: tastiera e controller con la stessa navigazione a cursore.
- **Android**: supportato da raylib.
- **iOS**: raylib non ha un backend ufficiale; serve un lavoro di porting dedicato (da pianificare).

## 4. Palette

Definita in `tools/design/palette.py`; nella fase di implementazione verrà generato `src/palette.h`.
Rampe dal più scuro al più chiaro.

| Rampa | Esadecimali | Uso |
|---|---|---|
| NOTTE | `070A18 0D1330 141D45 1D2A5E 2A3C7A` | fondi dei pannelli, ombre UI |
| LUMEN | `0E4D5C 12808A 2BC4C0 7FF2E2 E6FFFA` | cristalli, cursori, XP, effetti |
| AZZURRO | `1B3F8F 2E6FD0 54A6F2 A6DBFF E2F4FF` | selezioni, cielo, vetro |
| ORO | `3D2709 6E4A16 A8772E E0B04A FFE596 FFF8DE` | cornici, titoli, valori importanti |
| PERGAMENA | `8C7651 C2AA7E E4D3AC F3E9CF FBF6E8` | dialoghi ed elenchi chiari |
| Testo | crema `F3EAD2`, secondario `8E98BE`, inchiostro `1A1F3D` | |
| Mondo | ERBA, TERRA, ACQUA, PIETRA, LEGNO, INTONACO, TETTO_BLU, TETTO_ROSSO, FIORI, PELLE | tile ed edifici |

**12 tipi + NEUTRO**: tre toni ciascuno (scuro, base, chiaro) **e un glifo** (fiamma, goccia, foglia,
fulmine, vortice, masso, fiocco, falce di luna, stella, bolle, occhio, dado esagonale, anello).
Il glifo rende i tipi riconoscibili anche a chi distingue male i colori.

**Barre**: HP verde > 50%, ambra > 20%, rosso sotto; XP turchese. Stati come pillole con sigla (VEL, BRU, PAR, SON, GEL, CNF).

Tavola: `docs/design/tavola_palette.png`.

## 5. Tipografia

Font **originali**, disegnati pixel per pixel in `tools/design/font.py`: nessun font esterno da licenziare.

| Livello | Font | Stile | Uso |
|---|---|---|---|
| Logo | lettere del corpo ingrandite ×4 | oro inciso, contorno notte, cristallo e linea dell'Equilibrio | titolo |
| H2 | Eclyra Corpo grassetto | gradiente oro verticale + ombra bruna | titoli di schermata, nomi in targa |
| H3 | Eclyra Corpo maiuscolo | oro `E0B04A` | etichette di sezione |
| Corpo | Eclyra Corpo | crema su notte, inchiostro su pergamena; maiuscole 7 px, minuscole con discendenti, interlinea 11 | dialoghi, menu, descrizioni |
| Micro | Eclyra Micro 3×5 | maiuscole e cifre | LV, HP, PP, statistiche, suggerimenti dei tasti |

- Accenti italiani completi: à è é ì ò ù À È É Ì Ò Ù, più « » — … ×.
- Mai più di due livelli per pannello; numeri importanti nel corpo, etichette in micro.

Tavola: `docs/design/tavola_tipografia.png`.

## 6. Interfaccia

**Componenti** (`tools/design/ui.py`, tavola `tavola_componenti.png`):
- **Pannello notte**: bordo scuro, filetto d'oro con luce in alto a sinistra, fondo a gradiente con dithering, castoni d'oro con gemma di Lumen agli angoli.
- **Pergamena**: stessa cornice, carta chiara con grana leggera; per dialoghi ed elenchi lunghi.
- **Targa del titolo**: si appoggia sul bordo superiore del pannello, con cristallo e titolo oro.
- **Selezione**: fascia azzurra che sfuma a destra con filetti d'oro; **cursore = cristallo inclinato** che pulsa di 1 px (al posto del triangolo generico).
- **Barre**: cornice, traccia scura, riempimento a 3 toni, tacche ogni 10%, riflesso in punta.
- **Badge e pillole dei tipi**, **pillole di stato**, **pulsanti** con icona 11×11 (stato normale e attivo con bordo d'oro).
- **Icone 11×11**: lotta (spada), Elyon (gemma), zaino, fuga, Core, bestiario, scheda, mappa, salva, opzioni.

**Regole**
- Lo sfondo delle schermate di menu non è mai piatto: gradiente notte con trama a losanghe molto tenue.
- Testi: crema su notte, inchiostro su pergamena. Mai bianco puro, mai nero puro.
- Ogni schermata ha in basso i suggerimenti dei tasti in micro oro (Z, X, frecce), che su mobile diventano pulsanti.
- Transizioni: dissolvenza in notte (14 passi); in lotta, lampo e bande che si aprono.

## 7. Sprite degli Elyon

Motore: `tools/design/sprites.py`. Schede: `tools/design/elyon_design.py`. Tavole: `tavola_sprite_*.png`.

**Formati**
| Uso | Dimensione |
|---|---|
| Lotta, fronte | 64×64 |
| Lotta, retro | 80×80, generato dalle stesse forme (mai ingrandito) |
| Icone di squadra e PC | 32×32 |
| Boss | 128×128 |

Tutte le dimensioni derivano dalle **stesse forme vettoriali**: niente ingrandimenti di pixel.

**Costruzione**
- Ogni Elyon è un insieme di forme (ellissi, capsule, poligoni, fiamme emissive) con un materiale a 5 toni e una profondità.
- **Luce unica dall'alto a sinistra**, 4 toni per materiale (cel shading, niente sfumature).
- **Contorno selettivo**: scuro e colorato in basso a destra, più tenue sul lato illuminato. **Linee interne** solo tra parti sovrapposte.
- **Occhi grandi ed espressivi**: sclera con ombra, iride a 2 toni, pupilla, 1–2 riflessi, palpebra che dà l'espressione (deciso, felice, fiero, neutro). Gli occhi crescono meno del corpo nelle forme grandi.
- **Nodo di Lumen**: ogni Elyon ha un cristallo o una gemma sul corpo, colorata secondo la famiglia (brace per Pyrin, azzurro per Aquil, verde per Mossen).
- **Proporzioni**: base = testa grande (circa 45% dell'altezza) e zampe corte; forme finali = corpo più massiccio e più dettagli, stessa testa riconoscibile.
- **Famiglie evolutive**: stessi materiali e segni, ogni stadio aggiunge un elemento.
  - Pyrin: ciuffi di brace → criniera di fiamme → corazza d'acciaio e ottone.
  - Aquil: pinne → pinne più ampie → pinne stellate e aloni.
  - Mossen: germoglio → manto di muschio → dorso di roccia con alberelli.
- **Vista da dietro**: si specchiano le forme, non l'immagine, così la luce resta coerente. Spariscono il viso e i dettagli frontali; compaiono i segni sulla nuca.
- **Animazione**: respiro (le parti alte salgono di 1 px), battito di ciglia, guizzo delle fiamme. In lotta: entrata, attacco, colpo, KO.
- **Test della silhouette**: riempito di nero, ogni Elyon deve restare distinguibile dagli altri della stessa forma corporea.

Disegnati finora per la proposta: PYRIN, PYRION, PYROVAR, AQUIL, AQUARYS, MOSSEN, VERDANTOR, LUMI, SHADEK, FROLI, LEAFORA, AURION. Le altre 88 specie seguiranno le stesse regole.

## 8. Overworld e mappe

Generatori: `tools/design/mondo.py`.

- **Tile ricchi**:
  - erba con ciuffi e variazioni;
  - erba alta animata a ciuffi;
  - fiori a 4 colori;
  - sentieri con bordi irregolari (autotile a 4 vicini, deterministico);
  - lastricato a corsi sfalsati;
  - acqua con riflessi, spuma e riva;
  - ponti di legno con chiodi d'ottone.
- **Oggetti**: alberi 32×40 (quercia, abete, albero sacro del Lumen), rocce, cespugli, recinti, cartelli, **lampioni con cristallo acceso**, fontana.
- **Edifici**: intonaco, travi di legno, tegole a file sfalsate, colmo d'oro, finestre con luce calda e fioriere; ombra portata verso il basso a destra.
- **Personaggi 16×24**:
  - Esploratore: sciarpa turchese, fascia con lente di Lumen, borsa;
  - Kael: giacca ruggine, sciarpa oro;
  - Elian: camice;
  - Lira, Vael, abitanti con palette proprie.
- **Luce cinematografica**: calda in alto a sinistra, vignetta, aloni dei cristalli e delle finestre sopra la scena.

**Identità delle regioni**
| Regione | Materiali e colori | Simbolo / landmark | Luce |
|---|---|---|---|
| Lunaria | intonaco chiaro, ardesia blu, oro | Fonte del Lumen nella piazza con la rosa a otto punte | mattino caldo |
| Verdan | legno, edera, tetti rossi, verde vivo | Arena del Sigillo Verde, albero sacro | sole filtrato tra le foglie |
| Arkas | arenaria, ocra, rovine dei Custodi | tavole incise, dune | sole alto, ombre corte |
| Nival | neve, ghiaccio, pino blu | grotte di cristallo di ghiaccio | luce fredda, neve |
| Talassa | sabbia, corallo, legno di relitto | fari, arcipelago | riflessi d'acqua |
| Vulkar | basalto, rame, brace | crateri, miniere | luce rossa dal basso |
| Elys | marmo, oro, vetro, isole sospese | Torre dell'Equilibrio con il cristallo | crepuscolo violetto |

**Mappa di Asterra**: regioni colorate, coste con spuma, landmark disegnati (montagne innevate, vulcani,
dune, foreste, isole, isole sospese), rotte tratteggiate d'oro, targhette con nome e tipo, rosa dei venti
e barra dei sette Sigilli.

## 9. Effetti Lumen

- **Cristallo**: rombo allungato a 3 toni con spigolo chiaro; è usato per cursore, angoli, titoli, oggetti, edifici, Elyon e leggendari.
- **Bagliore**: alone additivo circolare (cristalli, finestre, nuclei degli Elyon).
- **Pulviscolo**: scintille di 1 px con croce tenue che salgono lentamente.
- **Cerchio runico** in lotta: rune accese con il colore della situazione:
  - turchese per i selvatici;
  - oro per il giocatore;
  - colore del Sigillo per i Guardiani;
  - violetto per il Vuoto.
- **Cattura**: il Core si posa sul cerchio, tre anelli di Lumen si contraggono, tre gemme in basso si accendono una per scossa.
- **Evoluzione**: raggi turchesi e oro dal centro, anelli in espansione, sagoma bianca che alterna le due forme.
- **Leggendari e Aurion**: sette cristalli-Sigillo in orbita, nebulosa, barra del boss con le soglie di fase.
- **Luoghi sacri**: alberi con frutti di Lumen, lampioni, pinnacoli degli edifici.

## 10. Regole di coerenza (checklist per ogni nuovo asset)

1. Usa solo colori della palette (o rampe derivate e registrate in `palette.py`).
2. Luce dall'alto a sinistra; ombra portata in basso a destra.
3. Contorni scuri colorati, mai neri; un solo spessore di pixel.
4. Nessuna scala non intera, nessuna rotazione, nessuna sfocatura.
5. Testo nel font Eclyra; mai sotto 7 px (eccetto il micro per etichette).
6. Il colore non è mai l'unica informazione (glifi di tipo, sigle di stato, frecce di efficacia).
7. Almeno un elemento Lumen per schermata, ma un solo punto focale luminoso alla volta.
8. Nessun elemento riconducibile a Pokémon o Nintendo: niente sfere bicolori, cappellini rossi, centri a tetto rosso, pedane ovali d'erba, etichetta "PS".
9. Gli asset nascono dagli script di `tools/design/`: si modifica lo script, non il PNG.

## 11. Le 18 schermate proposte

| # | Schermata | Elementi chiave |
|---|---|---|
| 1 | Titolo | aurora di Lumen, Torre di Elys, promontorio con Esploratore e Pyrin, logo oro, menu in pannello |
| 2 | Overworld | Lunaria: piazza con Fonte e rosa del Lumen, case, laboratorio a cupola, ruscello e ponte, banner del luogo, suggerimento contestuale |
| 3 | Dialoghi | pergamena con targa del nome, indicatore a cristallo |
| 4 | Menu principale | luogo, tempo, Lumen; voci con icone ELYON, BESTIARIO, ZAINO, SCHEDA, MAPPA, SALVA, OPZIONI |
| 5 | Squadra | 6 schede con medaglione dello sprite, tipi, stato, HP; KO in grigio |
| 6 | Inventario | schede per categoria, elenco su pergamena, oggetto grande con descrizione |
| 7 | Bestiario | elenco con visti/catturati, scheda con sprite su cerchio runico, tipo, statistiche, descrizione |
| 8 | Battaglia | fondale illustrato, cerchi runici, targhe HUD oblique, messaggio + 4 comandi |
| 9 | Mosse | 4 carte con colore e glifo del tipo, PP; dettaglio con categoria, potenza, precisione, efficacia |
| 10 | Statistiche | schede INFO / STATISTICHE / MOSSE, frecce della natura, barra XP |
| 11 | Evoluzione | raggi, anelli, sagoma luminosa, X per interrompere |
| 12 | Cattura | Core sul cerchio, anelli di Lumen, scosse come gemme accese |
| 13 | Salvataggio | carta del salvataggio con Esploratore, luogo, tempo, squadra, Sigilli |
| 14 | Sconfitta | "Il Lumen si affievolisce": nessuna penalità, ritorno a casa (regola del documento) |
| 15 | Mappa | Asterra a sette regioni, rotte, landmark, Sigilli |
| 16 | Guardiano | arena con colonne e stendardi, cerchio del Sigillo, targa della prova, Elyon rimasti |
| 17 | Boss | Aurion 128 px, sette cristalli in orbita, barra a tutta larghezza con soglie di fase (50% e 20%) |
| 18 | Finale | alba su Asterra, Torre riaccesa, esito della scelta e riepilogo |

Nota: BESTIARIO e MAPPA nel menu sono voci nuove rispetto al documento (ELYON, ZAINO, SCHEDA, SALVA,
OPZIONI) e vanno confermate.

## 12. Implementazione nel codice (dopo l'approvazione)

L'architettura resta quella attuale (C99, raylib, CMake, `eclyra_core` senza grafica).

1. **Asset generati**:
   - `tools/design/` esporterà in `assets/ui/` cornici 9-slice, icone, badge, cursore e font;
   - gli sprite di lotta, retro, icone e boss andranno in `assets/sprites/elyon/`;
   - i tileset in `assets/tilesets/`.
2. **`src/palette.h`** generato dalla palette Python: un'unica fonte per i colori.
3. **`src/ui_kit.c/h`** (nuovo): font Eclyra caricato con `LoadFontFromImage`, pannelli 9-slice, targhe, barre, pulsanti, cursore, badge. Sostituisce gradualmente `Gfx_Finestra` e `ui_common.c`.
4. **Schermate**: `battle_ui.c`, `schermate.c`, `game.c` (titolo) e `world.c` adottano il kit senza cambiare logica.
5. **Mondo**: nuovo tileset, autotile per sentieri e acqua, livello di luce sopra la scena (texture con alfa) e aloni additivi.
6. **Test**: gli screenshot automatici esistenti vengono confrontati con i mockup.

**Limite dichiarato**: gli sprite procedurali raggiungono uno stile coerente e leggibile, ma per una
qualità pienamente commerciale su tutte le 100 specie serve rifinire ogni scheda una per una (o
affiancare un pixel artist). Il sistema è pensato per entrambe le strade: un artista può sostituire
qualsiasi PNG mantenendo le stesse dimensioni e la stessa palette.
