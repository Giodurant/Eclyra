# ECLYRA — Specifica degli asset

Versione 1 (proposta in revisione). Complementare a `ECLYRA_DESIGN_SYSTEM.md`, che definisce l'aspetto;
qui si definisce **come ogni asset nasce, si chiama, si controlla e si importa**.

Contesto: non c'è budget per un pixel artist esterno. Gli asset si producono internamente con
**strumenti generativi + rifinitura manuale + strumenti procedurali** del progetto.
La pipeline deve rendere ogni asset sostituibile in qualsiasi momento senza toccare il codice di gioco.

---

## 1. Regole generali

| Regola | Valore |
|---|---|
| Formato | PNG a 32 bit RGBA, senza profilo colore né metadati (si rimuovono all'export) |
| Trasparenza | alfa solo 0 o 255 per sprite, tile e UI; alfa intermedia ammessa solo per effetti, aloni, ombre e livello touch |
| Colori | solo quelli di `tools/design/palette.py` (tolleranza 0); gli effetti emissivi possono usare le rampe LUMEN e del tipo con alfa |
| Scala | 1× logico: un pixel dell'asset è un pixel del canvas 320×180. Mai esportare ingrandito |
| Nomi | minuscolo, ASCII, `snake_case`, senza spazi né accenti; numeri a 3 cifre con zeri iniziali (`001`) |
| Origine | per sprite e personaggi il punto d'appoggio è al centro in basso (piedi); per tile e UI l'angolo in alto a sinistra |
| Fogli di animazione | fotogrammi in fila orizzontale, tutti delle stesse dimensioni, senza spaziatura |
| Sorgenti | ogni PNG finale ha un sorgente modificabile (`.aseprite`, `.pxo` o lo script che lo genera) in `art_src/` |

## 2. Struttura delle cartelle

```
assets/
  manifest.txt                  elenco degli asset caricati dal gioco (§8)
  palette/eclyra.hex            palette ufficiale (una riga per colore), generata da palette.py
  fonts/eclyra_corpo.png        atlante del font (glifi separati da colonne magenta)
  fonts/eclyra_micro.png
  ui/
    cornici/  pannello_notte.png, pannello_pergamena.png, targa.png, nastro.png   (9-slice, §5)
    icone/    icona_<nome>.png 11x11; tipo_<tipo>.png 11x11; stato_<sigla>.png
    tasti/    tasto_<dispositivo>_<tasto>.png 9x9  (tastiera, xbox, ps, generico)
    touch/    dpad.png, tasto_a.png, tasto_b.png, menu.png
    cursore.png, cristallo_7.png, cristallo_13.png, gemma.png
  sprites/
    elyon/    elyon_<nnn>_<nome>.png          lotta 64x64, fogli di animazione (§3)
              elyon_<nnn>_<nome>_icona.png    32x32, 2 fotogrammi
              elyon_100_aurion_boss.png       128x128
    personaggi/  pers_<nome>.png              16x24, 4 direzioni x 3 fotogrammi (§4)
    oggetti/     oggetto_<nnn>_<nome>.png     11x11 (zaino) e 16x16 (a terra)
    effetti/     fx_<nome>.png                fogli di effetti (§7)
  tilesets/
    <bioma>.png                               tile 16x16, 16 colonne (§6)
    <bioma>.tiles                             metadati: collisione, autotile, animazione
    edifici/  edificio_<nome>.png             multipli di 16
  sfondi/
    lotta_<bioma>.png                         400x180 (copre anche i telefoni larghi)
    titolo.png, mappa_asterra.png             400x240 (ritagliati sul canvas reale)
  maps/      *.map                            mappe (formato in src/map.h)
  text/      dialoghi.txt
  audio/     musica_<nome>.ogg, sfx_<nome>.wav
art_src/                                      sorgenti modificabili, concept e prompt (non distribuiti)
  concept/<asset>/                            immagini di concept approvate
  prompt/<asset>.txt                          prompt e parametri dello strumento generativo
docs/assets/provenienza.csv                   registro di provenienza (§9)
```

## 3. Elyon

| Asset | Dimensione | Fotogrammi | Note |
|---|---|---|---|
| Lotta | 64×64 per fotogramma | 5: riposo, respiro, battito, affondo, colpito | 3/4 di profilo, muso a sinistra; piedi a y = 58; il lato giocatore è lo specchio |
| Lotta, versione grande (facoltativa) | 80×80 | 5 | se presente sostituisce lo specchio ingrandito sul lato giocatore; disegnata, non scalata |
| Icona | 32×32 | 2: riposo, saltello | semplificata a mano: silhouette e colori principali, al massimo 1 riflesso |
| Boss | 128×128 | 5 | AURION, VESPERA e forme speciali |
| Variante Lumen | come la base | come la base | file `_lumen`: stessa forma, palette alternativa registrata |

**Specifiche visive obbligatorie** (dalle 9 schede di riferimento):
- luce dall'alto a sinistra, 4 toni per materiale;
- contorno di 1 px colorato;
- occhi con sclera, iride e riflesso;
- nodo di Lumen;
- taglia per stadio 84%, 93% o 100%;
- ombra al suolo semitrasparente (unico pixel con alfa intermedia ammesso nello sprite).

**Tempi**:
- riposo e respiro: 500 ms ciascuno;
- battito: 1 fotogramma ogni 3–5 s;
- affondo: 14 passi;
- colpito: alternato al riposo.

Esempio: `elyon_001_pyrin.png` (320×64, 5 fotogrammi), `elyon_001_pyrin_icona.png` (64×32).

## 4. Personaggi del mondo

| Asset | Dimensione | Struttura |
|---|---|---|
| Personaggio | 16×24 per fotogramma | 3 righe (giù, su, sinistra) × 3 fotogrammi (fermo, passo A, passo B); destra = specchio di sinistra, a meno che il personaggio sia asimmetrico: in quel caso 4ª riga obbligatoria |
| Corsa | facoltativa | 3 fotogrammi aggiuntivi per direzione |
| Ritratto | non previsto | i dialoghi usano la targa del nome (niente ritratti a scala diversa) |

- **Collisione**: 16×16 ai piedi (la tile occupata).
- **Ordine di profondità**: per riga di tile, dalla y del piede.
- **Personaggi principali**: Esploratore (asimmetrico, 4 righe), Kael, Elian, i 7 Guardiani, Vael, reclute e comandante Nexus, 12 abitanti di base ricolorabili per regione.

## 5. Interfaccia

**Pannelli in 9-slice**:
- angoli 4×4, bordi ripetibili, centro ripetibile;
- il file contiene tutto il pannello a 12×12;
- il codice sa che i margini sono 4 px.

| File | Uso |
|---|---|
| `pannello_notte.png` | menu, HUD |
| `pannello_pergamena.png` | dialoghi |
| `targa.png` | targa del titolo (3-slice orizzontale) |
| `nastro.png` | nastro dei messaggi (3-slice orizzontale) |

**Icone**:
- 11×11 su sfondo trasparente, contorno di 1 px;
- per i tipi: gemma colorata col glifo;
- per gli stati: pillola 9 px d'altezza (generata dal codice, solo la sigla è testo).

**Prompt dei tasti**:
- 9×9, uno per tasto e per famiglia di dispositivo;
- disegnati da noi, senza loghi né grafica ufficiale dei produttori.

**Touch**:
- croce direzionale 45×45, tasti tondi 25×25 e 21×21, pillola MENU;
- alfa 30–55%, ridimensionabili dal codice solo con scale intere: si preparano anche le versioni grandi (`_l`, 1,5×) disegnate.

## 6. Tile e mondo

| Asset | Dimensione | Note |
|---|---|---|
| Tile | 16×16 | foglio di 16 colonne; righe per famiglia (terreni, transizioni, acqua, decorazioni) |
| Autotile | 16 varianti (maschera dei 4 vicini: alto 1, destra 2, basso 4, sinistra 8) | per sentieri, acqua, sabbia, neve; la variante n è alla colonna n della riga della famiglia |
| Animazione | 2–4 fotogrammi nella stessa riga | acqua (4 × 250 ms), erba alta (3 × 200 ms), fiori (2 × 600 ms), cristalli (4 × 150 ms) |
| Alberi e oggetti alti | 32×40, 16×32 | disegnati sopra i personaggi della stessa riga |
| Edifici | multipli di 16 | porta al centro in basso; metadati delle tile solide e della tile-porta |

**Metadati** (`<bioma>.tiles`, testo):

```
# indice  nome         collisione  autotile   animazione
0         erba         libero      -          -
1         erba_alta    erba        -          3x200
16        sentiero     libero      sentiero   -
32        acqua        acqua       acqua      4x250
```

**Tipi di collisione**: libero, solido, acqua (Sigillo Maree), erba (incontri), porta, sporgenza (salto verso il basso), interazione.

## 7. Effetti

| Effetto | Dimensione | Fotogrammi | Alfa |
|---|---|---|---|
| Impatto per tipo | 32×32 | 6 × 2 passi | sì |
| Lampo Lumen | 32×32 | 3 | sì |
| Anelli di cattura | 48×16 | 8 | sì |
| Raggi di evoluzione | procedurali nel codice | - | - |
| Pulviscolo, scintille | 3×3 | 2 | sì |
| Meteo (pioggia, neve, nebbia) | procedurali nel codice | - | - |

Gli effetti emissivi non hanno contorno; nucleo chiaro, bordo nel colore del tipo, alone a bassa opacità.

## 8. Importazione nel gioco

1. L'asset finito va nella cartella giusta col nome giusto.
2. Si aggiunge o aggiorna una riga in `assets/manifest.txt`:
   ```
   # id                  file                                   fotogrammi  ms   pivot
   elyon.001             sprites/elyon/elyon_001_pyrin.png      5           500  32,58
   elyon.001.icona       sprites/elyon/elyon_001_pyrin_icona.png 2          300  16,30
   ui.pannello.notte     ui/cornici/pannello_notte.png          1           -    -
   ```
3. Si esegue il validatore (`tools/assets/valida.py`, da realizzare dopo l'approvazione; controlli al §10).
4. CMake copia `assets/` accanto all'eseguibile, e dentro il pacchetto su Android e iOS.
5. Il gioco carica gli asset solo per **id**, mai per percorso scritto nel codice. Se un file manca usa un segnaposto magenta riconoscibile e lo scrive nel log, senza bloccarsi.

Così uno sprite si sostituisce cambiando il PNG: nessuna modifica al codice.

## 9. Pipeline di produzione

```
CONCEPT -> REVISIONE -> PIXEL ART -> RIFINITURA -> PNG -> VALIDAZIONE -> IMPORT
```

| Fase | Chi / con cosa | Risultato | Criterio per passare oltre |
|---|---|---|---|
| 1. Concept | strumento generativo di immagini, con il modello di prompt (§9.1) | 4–8 proposte in `art_src/concept/<asset>/` | almeno una rispetta silhouette e famiglia |
| 2. Revisione | checklist di originalità e coerenza (§6 del Design System, §11 qui) | una proposta approvata, annotata | nessuna somiglianza con creature, personaggi o marchi esistenti |
| 3. Pixel art | `tools/assets/pixelizza.py` (da realizzare) oppure ridisegno su griglia; per gli Elyon anche la scheda procedurale (`tools/design/elyon_design.py`) come base | bozza alla dimensione finale con la palette ECLYRA | dimensioni, palette e luce corrette |
| 4. Rifinitura | editor gratuito: LibreSprite o Pixelorama (open source), o Aseprite | contorni puliti, occhi, riflessi, animazioni | confronto affiancato con le 9 schede di riferimento |
| 5. PNG | export a 1× senza metadati | file finale in `assets/` | nome secondo la convenzione |
| 6. Validazione | `tools/assets/valida.py` | rapporto senza errori | §10 |
| 7. Import | riga nel manifest | l'asset compare nel gioco e nello screenshot automatico | revisione visiva nel gioco |

### 9.1 Modello di prompt per i concept

Si descrive solo ciò che è nostro, senza nominare giochi, franchise, autori o personaggi esistenti:

```
Concept art di una creatura originale per un RPG fantasy a 16 bit.
[descrizione della scheda: corpo, materiali, tratti di famiglia, nodo di Lumen]
Vista di 3/4 di profilo rivolta a sinistra, intera, su sfondo neutro uniforme.
Palette: [colori del tipo dal Design System], contorni scuri colorati, luce dall'alto a sinistra.
Silhouette leggibile, occhi grandi ed espressivi, proporzioni [base | intermedio | finale].
```

### 9.2 Conversione in pixel (strumento da realizzare dopo l'approvazione)

`tools/assets/pixelizza.py`:
1. Ritaglio e allineamento al punto d'appoggio.
2. Riduzione alla dimensione finale col metodo della mediana per blocco.
3. Quantizzazione sulla palette ECLYRA: colore più vicino nello spazio OKLab, con rampa limitata al tipo.
4. Pulizia dei pixel isolati e degli scalini doppi.
5. Contorno selettivo di 1 px.
6. Export e confronto affiancato con la scheda di riferimento.

Il risultato è sempre una **bozza**: la rifinitura manuale è obbligatoria per occhi, contorni e animazioni.

### 9.3 Provenienza e Steam

- Per ogni asset si registra in `docs/assets/provenienza.csv`:
  - file;
  - origine: procedurale, generativo + rifinitura, oppure manuale;
  - strumento e versione;
  - prompt (file in `art_src/prompt/`);
  - data;
  - autore della rifinitura.
- **Steam** chiede di dichiarare nel questionario sui contenuti i contenuti generati con l'IA prima del rilascio (grafica, testi, audio usati nel gioco o nella pagina del negozio); le regole del gennaio 2026 escludono gli strumenti di sviluppo come gli assistenti al codice. Il registro rende la dichiarazione facile e precisa.
- Si usano solo strumenti generativi le cui condizioni permettono l'uso commerciale del risultato; le condizioni vanno ricontrollate a ogni cambio di strumento.

## 10. Validatore (controlli automatici)

| Controllo | Errore se |
|---|---|
| Nome | non rispetta la convenzione o non è nel manifest |
| Dimensioni | non corrispondono alla tabella per il tipo di asset; larghezza del foglio non multipla del fotogramma |
| Palette | un pixel opaco usa un colore fuori palette |
| Alfa | valori intermedi in sprite, tile o UI (eccetto ombra al suolo ed effetti) |
| Punto d'appoggio | Elyon: nessun pixel opaco sotto y = 60; personaggi: piedi non sull'ultima riga |
| Contorno | pixel opachi a contatto col trasparente più chiari del 60% della rampa (contorno mancante) |
| Fotogrammi | numero diverso da quello del manifest |
| Animazioni | fotogrammi identici dove è richiesto movimento |
| Leggibilità | Elyon: silhouette che occupa meno del 25% o più del 95% del riquadro |

## 11. Checklist di revisione (umana)

1. Rispetta le 9 schede di riferimento (taglia, luce, contorno, occhi)?
2. La silhouette riempita di nero si distingue dalle altre?
3. La famiglia evolutiva è riconoscibile?
4. Il nodo di Lumen è presente e colorato secondo la famiglia?
5. Nessuna somiglianza con creature, personaggi o marchi esistenti?
6. Funziona sul cerchio runico, nel medaglione della squadra e come icona 32×32?
7. È leggibile su telefono (canvas ×6) e Steam Deck (×4)?

## 12. Priorità di produzione

1. Le 9 schede starter (riferimento, già impostate in `tools/design/elyon_design.py`).
2. UI: pannelli, icone, prompt, touch, font (già impostati in `tools/design/ui.py` e `font.py`).
3. Tileset di Verdan (Lunaria, Route 1, Bosco Verdalia, Verdan) e personaggi principali.
4. Elyon della prima parte (habitat di Route 1 e del Bosco, squadre di Kael, Nexus e Lira).
5. Fondali di lotta: prato, bosco, arena.
6. Le altre regioni in ordine di storia.
