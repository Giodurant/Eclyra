# ECLYRA — Design System

Versione 2 — **APPROVATA e congelata** (vedi `docs/DECISIONI.md`). Sono ammesse solo rifiniture di leggibilità,
spaziature e dettagli UI. Sostituisce la v1, archiviata in `docs/design/archivio_v1/`.
Storia, mondo, dati e regole di gioco restano quelli di `ECLYRA_MASTER_PER_CLAUDE.md`: questo documento
definisce **come appare e come si usa** il gioco su tutte le piattaforme.

Documenti collegati:
- `ECLYRA_ASSET_SPEC.md`: formati, nomi e pipeline degli asset.
- `MULTIPLATFORM_ARCHITECTURE.md`: input, scaling, salvataggi e build.
- `ELYON_STYLE_BIBLE.md`: regole per disegnare tutti gli Elyon a partire dai 9 di riferimento.

Ogni immagine citata è generata dagli script in `tools/design/` alla risoluzione reale del gioco:

| Materiale | Dove |
|---|---|
| Mockup v2 delle 12 schermate richieste, più le varianti per telefono e Steam Deck | `docs/design/mockup_v2/` |
| Tavole di riferimento | `docs/design/tavola_*.png` |

Rigenerazione:

```sh
python3 tools/design/tavole.py
python3 tools/design/mockups_v2.py
```

---

## 0. Identità in una frase

> Un RPG portatile a 16 bit dove la notte e l'oro incorniciano un mondo luminoso, e il **Lumen**
> (cristalli turchesi) lega creature, luoghi e interfaccia.

Priorità del progetto:
1. Identità visiva.
2. Coerenza.
3. Gameplay.
4. Usabilità.
5. Performance.
6. Multipiattaforma.

## 1. Principi

1. **Il Lumen è la firma.** Ogni schermata ha almeno un cristallo, una runa o un bagliore turchese, ma un solo punto luminoso dominante.
2. **L'interfaccia lascia spazio al gioco.** Cornici sottili, pannelli solo dove servono, campo di battaglia ampio.
3. **Leggibile ovunque.** Telefono, tablet, Steam Deck e monitor: testo mai sotto 7 px logici, contrasto alto, colore mai unica informazione.
4. **Un solo pixel.** Tutto a 1× logico; scala intera; niente sfocature, rotazioni o scale non intere.
5. **Ogni input è uguale.** Tastiera, mouse, controller e touch producono le stesse azioni; la UI mostra i prompt del dispositivo usato per ultimo.
6. **Originale.** Nessun elemento riconoscibile di altri giochi (vedi §16).

## 2. Risoluzione, griglia e aree sicure

- **Canvas logico base 320×180** (16:9), scalato di un fattore intero con filtro nearest.
- **Rapporti diversi**: il lato corto resta fisso e il canvas si allarga invece di deformarsi (algoritmo in `MULTIPLATFORM_ARCHITECTURE.md` §5).

| Dispositivo | Canvas logico | Scala |
|---|---|---|
| Monitor 1080p | 320×180 | ×6 |
| Steam Deck 1280×800 | 320×200 | ×4 |
| Telefono 2400×1080 | 400×180 | ×6 |
| iPad 2732×2048 | 341×240 | ×8 |

- **Area sicura**: il riquadro centrale 320×180 contiene sempre tutte le informazioni essenziali. Le HUD si ancorano ai bordi del canvas reale, il campo resta centrato, il mondo mostra semplicemente più tile.
- **Margini**: 4 px dal bordo; ai lati di notch e angoli arrotondati si aggiunge l'inset del sistema convertito in pixel logici.
- **Griglia UI**: 2 px. Tile del mondo: 16×16.

## 3. Palette

Fonte unica: `tools/design/palette.py`; nella fase di implementazione verrà generato `src/palette.h`.
Tavola: `docs/design/tavola_palette.png`.

**Colori principali**
| Nome | Rampa (scuro → chiaro) | Uso |
|---|---|---|
| NOTTE (navy) | `070A18 0D1330 141D45 1D2A5E 2A3C7A` | fondi dei pannelli, HUD, ombre UI |
| LUMEN (azzurro Lumen) | `0E4D5C 12808A 2BC4C0 7FF2E2 E6FFFA` | cristalli, cursore, XP, effetti, selezione |
| AZZURRO | `1B3F8F 2E6FD0 54A6F2 A6DBFF E2F4FF` | evidenziazioni, cielo, vetro |
| ORO | `3D2709 6E4A16 A8772E E0B04A FFE596 FFF8DE` | cornici, titoli, valori, prompt |
| BIANCO CALDO | crema `F3EAD2`, pergamena `F3E9CF FBF6E8` | testo su notte, superfici chiare |
| Testo secondario | `8E98BE` | etichette, voci disattivate |
| Inchiostro | `1A1F3D` | testo su pergamena |

**Mondo**: ERBA, TERRA, ACQUA, PIETRA, LEGNO, INTONACO, TETTO_BLU, TETTO_ROSSO, FIORI, PELLE (rampe da 4–6 toni). Ogni regione ne usa un sottoinsieme (§12).

**Tipi**: tre toni (scuro, base, accento) e un glifo 7×7.

| Tipo | Scuro | Base | Accento | Glifo |
|---|---|---|---|---|
| FIAMMA | `8C2414` rosso | `E8542F` arancio | `FFC24A` giallo | fiamma |
| ACQUA | `123E8A` blu | `2F86E0` azzurro | `7FE0F2` ciano | goccia |
| NATURA | `1E5E2A` verde | `4CAF4A` | `A8E07A` verde chiaro | foglia |
| IMPULSO | `8A6410` | `F2C230` giallo | `FFF6A0` elettrico | fulmine |
| VENTO | `3E7EA6` | `8CCDEA` azzurro chiaro | `E2F6FF` | vortice |
| ROCCIA | `5A3E20` marrone | `A57A4A` ocra | `B8B2A4` grigio | masso |
| GELO | `1E9EBE` ciano | `B6ECFF` azzurro | `FFFFFF` bianco | fiocco |
| OMBRA | `0E0A18` nero | `3E2E7A` indaco | `8A6CD2` viola | falce di luna |
| ASTRALE | `1E2A78` blu profondo | `6E5AD8` viola | `F0D27A` oro | stella |
| TOSSICO | `4E1A66` viola | `9A3FC8` | `A8E83A` verde acido | bolle |
| PSICHE | `6A1E6A` viola | `E0409A` magenta | `FFA6D2` rosa | occhio |
| METALLO | `3A4458` grigio | `8E9CB0` acciaio | `BCD6F2` blu freddo | dado esagonale |
| NEUTRO | `5C5F70` | `9EA1B0` | `E6E2D6` | anello |

**Semantici**:
- HP: verde > 50%, ambra > 20%, rosso sotto.
- XP: turchese.
- Stati come pillole con sigla: VEL, BRU, PAR, SON, GEL, CNF.
- Efficacia: freccia doppia ▲▲ superefficace, ▼ poco efficace, testo "NORMALE".

**Regola**: nessun colore fuori palette. Le rampe nuove si aggiungono a `palette.py` con un nome e un uso.

## 4. Tipografia

Font originali, disegnati pixel per pixel (`tools/design/font.py`): nessuna licenza esterna.
Tavola: `tavola_tipografia.png`. Atlante per raylib: `font_eclyra_corpo.png`.

| Livello | Font | Aspetto | Uso |
|---|---|---|---|
| Logo | lettere del corpo ×4 | oro inciso, contorno notte, cristallo e linea dell'Equilibrio | titolo |
| Titolo | Eclyra Corpo grassetto | gradiente oro, ombra bruna | targhe dei pannelli, nomi importanti |
| Etichetta | Eclyra Corpo maiuscolo | oro `E0B04A` | sezioni, pulsanti attivi |
| Corpo | Eclyra Corpo | crema su notte / inchiostro su pergamena; maiuscole 7 px, discendenti, interlinea 11 | dialoghi, menu, descrizioni |
| Micro | Eclyra Micro 3×5 | maiuscole e cifre | LV, HP, PP, statistiche, prompt dei tasti |

- Accenti italiani completi (à è é ì ò ù À È É Ì Ò Ù, « » — … ×).
- Al massimo due livelli per pannello. Cifre allineate per HP, PP e prezzi.

## 5. Cornici e pannelli

Approvate, nella versione **sottile** (tavola `tavola_componenti.png`):
- **Pannello notte**: 1 px blu notte esterno + 1 px d'oro (più chiaro in alto a sinistra), fondo notte a gradiente leggermente trasparente (240/255).
- **Gemme**: rombo di Lumen di 5 px su ogni angolo, solo nei pannelli alti almeno 18 px; nei pannelli bassi una sola gemma centrale o nessuna.
- **Pergamena**: stessa cornice, carta chiara con grana leggera. Si usa per dialoghi e pagine del Bestiario.
- **Targa del titolo**: barra di 13 px appoggiata sul bordo superiore, con cristallo e titolo oro e filetto d'oro sotto.
- **Nastro**: fascia scura con filetto d'oro in alto. Serve per messaggi di lotta e suggerimenti, senza angoli né gemme.
- **Divieti**: niente filigrane spesse, castoni grandi o cornici doppie. Una cornice non supera mai 2 px.

## 6. Componenti di interfaccia

| Componente | Specifica |
|---|---|
| Selezione | fascia azzurra che sfuma verso destra, filetti d'oro sopra e sotto |
| Cursore | cristallo inclinato 5×7 che pulsa di 1 px (500 ms) |
| Pulsante | 1 px scuro, gradiente notte; attivo con bordo d'oro, gradiente azzurro e testo oro; icona 11×11 a sinistra |
| Barra HP/XP | cornice scura, traccia, riempimento a 3 toni, tacche ogni 10%, riflesso in punta; HP 4 px, XP 1–2 px |
| Badge tipo | gemma 11×11 col glifo |
| Pillola tipo | badge + fascia col nome |
| Pillola stato | sigla bianca su colore dello stato |
| Prompt del tasto | cerchio 9×9 con anello d'oro e carattere del dispositivo (§15) |
| Chip di campo | etichetta micro con gemma: turno, meteo, campo di tipo |
| Icone | 11×11: lotta, Elyon, zaino, fuga, Core, bestiario, mappa, scheda, salva, opzioni; oggetti 11×11 |

## 7. Menu di gioco

Voci ufficiali, in quest'ordine:

| Voce | Sottotitolo | Contenuto |
|---|---|---|
| ELYON | La tua squadra | squadra, riepilogo, riordino |
| ZAINO | Oggetti e Core | categorie, uso, dettaglio |
| BESTIARIO | 28 scoperti | §9 |
| MAPPA | Asterra | §10 |
| SCHEDA | Esploratore del Lumen | nome, tempo, Lumen, Sigilli, record |
| SALVA | Ultimo: 2 min fa | §13 |
| OPZIONI | Audio e controlli | testo, audio, schermo, controlli touch, rimappatura |

- Layout: pannello verticale a sinistra (128 px) con giocatore, luogo, tempo e Lumen in testa; voci con icona, nome e sottotitolo micro.
- Il mondo resta visibile e oscurato; in basso, un nastro coi prompt.
- Mockup: `03_menu`.
- Si apre con MENU (X/Esc o TAB da tastiera, Start o tasto Menu da controller, pillola MENU su touch).
- Il menu rapido (TAB, tasto View) apre direttamente la voce usata per ultima.

## 8. Dialoghi

- Pergamena in basso (308×52) con **targa del nome** sul bordo superiore. Tre righe di corpo e un cristallo che pulsa quando il testo è completo.
- Testo progressivo alla velocità scelta nelle opzioni; CONFERMA completa la riga, poi avanza. Su touch avanza toccando qualsiasi punto.
- Le scelte (SÌ/NO e liste brevi) appaiono in un pannello notte a destra, sopra la pergamena.
- Mockup: `09_dialogo`.

## 9. Bestiario

Un **codice dei Custodi**: copertina blu notte e due pagine di pergamena. Non è un dispositivo né un elenco su schermo.

- **Pagina sinistra**: filtri (TUTTI e regioni) e lista con numero, icona di stato e nome.
- **Pagina destra**: la voce selezionata.
- Mockup: `05_bestiario`.

**Stati e sblocco progressivo**
| Stato | Icona in lista | Cosa si vede |
|---|---|---|
| IGNOTO | `?` grigio | numero e "- - -" |
| VISTO | occhio | nome, sagoma colorata, tipo, habitat in cui è stato visto; descrizione nascosta ("Cattura questo Elyon per completare la voce") |
| CATTURATO | Core | tutto: sprite, tipi, categoria, habitat, altezza, peso, descrizione |
| COMPLETO | Core con anello | catturato + intera linea evolutiva registrata: si aggiunge la **Nota dei Custodi** (una riga di lore) e il sigillo d'oro sulla pagina |

- **Campi della voce**: numero, nome, categoria ("Lontra-pinna"), tipi, sprite su medaglione, habitat, altezza, peso, descrizione, sigillo di stato.
- **Dati da aggiungere al database**:
  - categoria;
  - altezza (m, una cifra decimale);
  - peso (kg, una cifra decimale);
  - Nota dei Custodi;
  - habitat, ricavato da `encounters_data` e dagli eventi.
- **Navigazione**:
  - su e giù per la lista; sinistra e destra per i filtri;
  - CONFERMA apre la voce a schermo intero (sprite animato, verso);
  - su touch si tocca la riga o si scorre la lista.

## 10. Mappa di Asterra

- Illustrazione unica 320×180 con le sette regioni distinguibili per colore, materiali e landmark disegnati: montagne innevate, dune, vulcani, foreste, arcipelago, isole sospese.
- Elementi in sovrapposizione, guidati dai dati:
  - **Luoghi**: città con rombo d'oro, percorsi tratteggiati, luoghi importanti con icona. Tabella luoghi: id, nome, regione, x, y, tipo, flag di visita.
  - **Nebbia** sulle regioni non ancora esplorate: colore desaturato verso la notte, trama a 4 px, nome senza sottotitolo.
  - **Posizione del giocatore**: Esploratore con due anelli di Lumen che pulsano.
  - **Pannello della regione** (in alto a destra): nome, tipo, luoghi con stato visitato (gemma) o non visitato (punto grigio).
  - **Barra dei Sigilli** in basso: sette cristalli col colore della prova, accesi quando ottenuti.
- **Navigazione**:
  - il cursore salta al luogo più vicino nella direzione premuta;
  - CONFERMA mostra i dettagli;
  - su touch si tocca il luogo.
- Mockup: `06_mappa`.

## 11. Battaglia

Layout **ECLYRA**, pensato per essere riconoscibile e diverso dai giochi del genere (mockup `04_lotta`, `04b_lotta_mosse`, `04c_lotta_telefono`, `11_cattura`, `12_guardiano`).

**Campo**
- Fondale illustrato per bioma: cielo, montagne, bosco o arena, terreno con profondità, raggi di luce, pulviscolo di Lumen.
- **Gli Elyon si fronteggiano di profilo**: il giocatore a sinistra e più vicino, l'avversario a destra e più indietro. Entrambi mostrano il muso e l'espressione; non esiste la vista da dietro.
- Sprite grandi: lato giocatore a scala 1,25 (80 px), avversario a 1,1 (70 px), derivati dalle stesse forme e mai ingranditi a pixel.
- Ogni Elyon sta su un **cerchio runico**: disco di terra battuta, anello di pietra, 24 rune che si accendono.

  | Situazione | Colore delle rune |
  |---|---|
  | Giocatore | oro |
  | Selvatico | turchese |
  | Guardiano | colore del Sigillo |
  | Vuoto | violetto |

**HUD**
- **In alto, una targa per lato**, larga 148 px, ancorata al bordo.
  - Giocatore: tipi, nome, LV, barra HP con numeri, XP sottile, stato.
  - Avversario: tipi, nome, LV, barra HP senza numeri, stato.
- **Chip centrale**: turno, meteo con turni rimasti, campo di tipo attivo.
- **Guardiani e boss**: targa della prova sotto il chip; cristalli degli Elyon rimasti sotto la targa avversaria; per i boss, barra a tutta larghezza con le soglie di fase (50% e 20%).

**Comandi: la rosa del Lumen**
- Quattro pulsanti a rombo attorno a un cristallo, in basso a destra. Ogni posizione corrisponde al tasto frontale del controller nella stessa posizione:

  | Posizione | Comando | Tasto |
  |---|---|---|
  | Basso | LOTTA | A, conferma |
  | Destra | FUGA | B |
  | Sinistra | ZAINO | X |
  | Alto | ELYON | Y |

- Tastiera e croce direzionale spostano la selezione; CONFERMA la attiva. Su touch e mouse si tocca o si clicca il pulsante.
- **Nastro dei messaggi** a sinistra della rosa: due righe e cristallo di avanzamento.

**Selezione delle mosse**
- Quattro **carte in fila** lungo il bordo inferiore (75×30): fascia del tipo con colore, glifo e nome; nome della mossa; PP con barra.
- La carta selezionata si alza di 4 px e prende il bordo d'oro.
- Sopra la carta, un **riquadro di dettaglio**: categoria, potenza, precisione, efficacia contro il bersaglio (testo e frecce).
- ANNULLA torna alla rosa.

**Contesto e input**
| Fase | Tastiera / controller | Touch / mouse |
|---|---|---|
| Rosa | frecce + conferma, oppure tasto frontale diretto | tocco sul pulsante |
| Mosse | sinistra e destra tra le carte, conferma | tocco sulla carta (primo tocco seleziona, secondo conferma) |
| Messaggi | conferma per avanzare | tocco ovunque |
| Bersaglio, squadra, zaino | liste navigabili | tocco sulla riga |

Su telefono la croce direzionale **non compare** in lotta: i comandi sono già pulsanti grandi.

**Animazioni e tempi** (60 passi al secondo):

| Evento | Durata | Effetto |
|---|---|---|
| Transizione d'ingresso | 48 passi | lampi e bande |
| Entrata | 22 passi | scivolamento |
| Affondo d'attacco | 14 passi | |
| Lampo della mossa | 14 passi | colore e glifo del tipo |
| Colpito | 18 passi | lampeggio, 3 passi per fase |
| KO | 22 passi | caduta e dissolvenza |
| Barra HP | circa 48 passi per la barra intera | |
| Critico | — | lampo bianco di 2 passi e scossa dello schermo di 1 px |

Le sequenze di cattura ed evoluzione sono descritte nel §13 (Altre schermate).

## 12. Mondo: tile, edifici, regioni

**Tile** 16×16 con 3–5 toni per materiale; luce dall'alto a sinistra; ombre portate in basso a destra.
- Terreni: erba (ciuffi e variazioni), erba alta animata, fiori, sentiero e lastricato, acqua (riflessi, spuma, riva), sabbia, neve, roccia, legno.
- Transizioni **autotile a 4 vicini** (16 varianti), con bordi irregolari ma deterministici.
- Oggetti:
  - alberi 32×40 (quercia, abete, albero sacro del Lumen);
  - rocce, cespugli, recinti, cartelli, **lampioni con cristallo**, fontane, ponti con chiodi d'ottone;
  - elementi interattivi segnalati da un luccichio di Lumen ogni 2 secondi.
- **Zone segrete**: passaggi dietro alberi o rocce con una variazione di tono di 1 grado, mai invisibili del tutto, più un luccichio raro.

**Edifici**:
- intonaco o pietra, travi, tegole a file sfalsate, colmo e pinnacolo, finestre con luce calda;
- porta sempre al centro in basso;
- negozio con insegna appesa, laboratorio con cupola di vetro e cristallo, arena del Guardiano con stendardi e il cristallo del Sigillo.

**Personaggi** 16×24, 4 direzioni:
| Personaggio | Tratti |
|---|---|
| Esploratore | capelli scoperti, fascia con lente di Lumen, sciarpa turchese, borsa |
| Kael | giacca ruggine, sciarpa oro |
| Elian | camice |
| Guardiani | colori del proprio Sigillo |
| Ordine Nexus | nero e bordeaux, con un cristallo spento (grigio) come simbolo |
| Abitanti | palette proprie per regione |

**Identità delle regioni**
| Regione | Materiali e colori | Landmark | Luce | Segreto tipico |
|---|---|---|---|---|
| Lunaria | intonaco, ardesia blu, oro | Fonte del Lumen con la rosa a otto punte | mattino caldo | giardino dietro il laboratorio |
| Verdan | legno, edera, tetti rossi | Arena del Sigillo Verde, albero sacro | sole filtrato | radure tra i rovi (Sigillo Verde) |
| Arkas | arenaria, ocra, rovine | tavole dei Custodi, dune | sole alto, ombre corte | camere sepolte |
| Nival | neve, ghiaccio, pino blu | grotte di cristallo di ghiaccio | luce fredda | sentieri ghiacciati |
| Talassa | sabbia, corallo, relitti | fari, arcipelago | riflessi | grotte marine (Sigillo Maree) |
| Vulkar | basalto, rame, brace | crateri, miniere | luce rossa dal basso | passaggi vulcanici |
| Elys | marmo, oro, vetro | Torre dell'Equilibrio, isole sospese | crepuscolo viola | piani nascosti della Torre |

## 13. Altre schermate

| Schermata | Specifica | Mockup |
|---|---|---|
| Titolo | aurora di Lumen, Torre di Elys lontana, promontorio con Esploratore e Pyrin, logo, menu NUOVA PARTITA / CONTINUA / OPZIONI / ESCI in pannello sottile | `01_titolo` |
| Scheda Elyon | schede INFO, STATISTICHE, MOSSE; medaglione con lo sprite su cerchio runico; frecce della natura (▲ in rosso, ▼ in blu, mai solo il colore); barra XP | `07_scheda_elyon` |
| Zaino | schede per categoria (cure, Core, evoluzione, chiave, equipaggiamento), lista su pergamena, oggetto grande con descrizione, prezzo | `08_zaino` |
| Evoluzione | fondo notte, raggi turchesi e oro, anelli, sagoma bianca che alterna le due forme accelerando; ANNULLA interrompe | `10_evoluzione` |
| Cattura | Core sul cerchio dell'avversario, tre anelli di Lumen che si stringono, tre gemme che si accendono a ogni scossa | `11_cattura` |
| Guardiano | arena con colonne e stendardi, cerchio del Sigillo, targa della prova, Guardiano visibile dietro il suo Elyon | `12_guardiano` |
| Salvataggio | carta con Esploratore, luogo, tempo, squadra, Sigilli; conferma e avanzamento con cristallo che si riempie | v1 |
| Sconfitta | "Il Lumen si affievolisce", nessuna penalità, ritorno a casa | v1 |
| Boss | Aurion 128 px, barra a fasi | v1 |
| Finale | alba e riepilogo | v1 |

## 14. Sprite degli Elyon

Riferimento definitivo: **le 9 schede delle linee starter** (tavola `tavola_starter.png`, animazioni in `tavola_animazione_pyrin.png`).
Tutte le altre 91 specie devono rispettarle.

| Regola | Valore |
|---|---|
| Vista | 3/4 di profilo, muso a sinistra; il lato giocatore è lo stesso sprite specchiato |
| Suolo | piedi a y = 58 su 64; ombra ellittica semitrasparente |
| Taglia per stadio | base 84%, intermedio 93%, finale 100% del riquadro; leggendari 100%; boss 128×128 |
| Proporzioni | base: testa circa 45% dell'altezza, zampe corte, occhi grandi; finale: corpo massiccio, testa riconoscibile |
| Luce | dall'alto a sinistra, 4 toni per materiale, niente sfumature |
| Contorno | 1 px colorato (tono più scuro del materiale vicino), più chiaro sul lato illuminato; linee interne solo tra parti sovrapposte |
| Occhi | sclera con ombra, iride a 2 toni, pupilla 2×2 (1×2 negli occhi piccoli), 1–2 riflessi, palpebra che dà l'espressione |
| Espressioni | deciso, felice, fiero, neutro; per stadio: base felice o deciso, finali fieri |
| Nodo di Lumen | ogni Elyon ha un cristallo o una gemma emissiva, colorata secondo la famiglia |
| Elementi emissivi | fiamme, nebulose, cristalli: senza contorno, nucleo chiaro, alone tenue |
| Famiglia | stessi materiali e segni; ogni stadio aggiunge un elemento |
| Silhouette | riempito di nero, distinguibile dagli altri Elyon della stessa forma corporea |
| Icone 32×32 | disegnate o rifinite a mano, semplificate (niente rimpicciolimenti automatici nel gioco finale) |

**Evoluzione delle tre linee**
| Linea | Base | Intermedio | Finale |
|---|---|---|---|
| Pyrin | lince di brace, ciuffi di fiamma, stemma d'ottone | criniera di fiamme, bracciali | corazza d'acciaio e ottone, elmo con corna, nucleo sul petto |
| Aquil | lontra con pinne | pinne ampie, prime stelle sulla coda | pinne stellate, aloni astrali |
| Mossen | cinghialino con germoglio e muschio | manto di muschio, spalle di pietra | dorso di roccia con alberelli, zanne di pietra |

**Animazioni** (fotogrammi numerati come in `tavola_animazione_pyrin.png`)
| Animazione | Fotogrammi | Tempi |
|---|---|---|
| Riposo | 0 ↔ 1 (respiro: le parti alte salgono di 1 px) | 500 ms per fotogramma |
| Battito di ciglia | 2 | 1 fotogramma ogni 3–5 s (casuale) |
| Elementi emissivi | guizzo a 3 fasi | 120 ms |
| Affondo | 3 (4 px in avanti) | 14 passi |
| Colpito | 4 (lampo chiaro) | alternato |
| KO e ingresso | gestiti dal motore (spostamento e trasparenza), nessun fotogramma extra | |

## 15. Lumen: linguaggio visivo

| Elemento | Forma | Colori | Dove |
|---|---|---|---|
| Cristallo | rombo allungato, spigolo chiaro, 7/11/13/15 px | LUMEN 2–4 | logo, targhe, cursore, lampioni, oggetti, Elyon, leggendari |
| Gemma | rombo di 5 px | LUMEN + castone d'oro | angoli delle cornici, Sigilli, salvataggi |
| Bagliore | alone additivo circolare | LUMEN 3 a bassa opacità | cristalli, finestre, nuclei |
| Pulviscolo | punto di 1 px, a volte con croce tenue | LUMEN 4 | lotte, luoghi sacri, titolo |
| Runa | 24 segni su un'ellisse | colore della situazione | cerchi di lotta, luoghi dei Custodi |
| Anelli | ellissi concentriche che si stringono o allargano | LUMEN 4 | cattura, evoluzione, cure |
| Lumen spento | stesse forme in grigio pietra | PIETRA 1–4 | Ordine Nexus, Sigilli non ottenuti, sconfitta |

Le mosse di tipo usano il colore del tipo, ma l'impatto ha sempre un lampo di Lumen di 2 passi: è il marchio delle lotte di ECLYRA.

## 16. Originalità: cosa non usiamo mai

- Sfere bicolori per la cattura (il **Core** è una sfera di cristallo in un anello d'oro).
- Protagonista col cappellino rosso; edifici-centro a tetto rosso.
- Pedane ovali d'erba; vista da dietro dell'Elyon del giocatore con HUD a destra e riquadro comandi 2×2 (il layout classico del genere).
- L'etichetta "PS" e le icone di altri marchi; i glifi dei controller sono disegnati da noi (§17).
- Creature che ricordano specie esistenti: ogni nuova scheda passa la revisione dell'`ECLYRA_ASSET_SPEC.md` §6.

## 17. Interfaccia per controller

- **Navigazione a fuoco**: c'è sempre un elemento selezionato, con cursore a cristallo e fascia azzurra. Nessuna funzione richiede mouse o tastiera (requisito Steam Deck).
- **Prompt del tasto** (cerchio 9×9 con anello d'oro): il carattere cambia col dispositivo usato per ultimo.

  | Dispositivo | Prompt |
  |---|---|
  | Tastiera | Z, X, TAB, frecce |
  | Controller con layout Xbox (anche Steam Deck) | A, B, X, Y |
  | Controller PlayStation | croce, cerchio, quadrato, triangolo, disegnati come forme semplici |
  | Touch | nessun prompt |

- **Mappatura di base**:

  | Azione | Controller | Tastiera |
  |---|---|---|
  | Muoversi | croce o levetta | WASD o frecce |
  | Conferma | A | Z o Invio |
  | Annulla | B | X o Esc |
  | Correre | tenere B o X fuori dai menu | Shift |
  | Menu | Start | TAB per il menu rapido |
  | Cambio scheda | LB e RB | |
  | Scorrimento veloce | LT e RT | |

- **Leggibilità sul Deck**: canvas 320×200 a ×4. Testo del corpo 7 px → 28 px fisici; micro 5 px → 20 px; aree selezionabili ≥ 16 px logici.

## 18. Interfaccia mobile (touch)

Mockup: `02b_overworld_telefono`, `04c_lotta_telefono`.

- **Orientamento**: orizzontale (landscape), bloccato.
- **Livello touch** disegnato sopra il gioco, trasparente (fondo notte al 30%, bordi oro al 55%), mai sopra le HUD.
  - **Croce direzionale** a sinistra: raggio 22 px logici (circa 1,7 cm su un telefono a ×6), con zona morta centrale. Su richiesta può diventare analogica (trascinamento dal punto toccato).
  - **A / CONFERMA** (raggio 12) e **B / ANNULLA** (raggio 10) a destra, disposti come i tasti di un portatile; pillola **MENU** in alto a destra.
- **Personalizzazione** (OPZIONI → Controlli touch): dimensione dal 75% al 150%, posizione trascinabile in un editor con anteprima, opacità dal 20% all'80%, nascondi automaticamente, ripristina.
- **Adattamento al contesto**:

  | Contesto | Controlli |
  |---|---|
  | Esplorazione | croce + A + B + MENU |
  | Dialogo | tocco ovunque = avanza; B nascosto |
  | Menu e liste | tocco diretto sulle voci + B; croce nascosta |
  | Lotta | croce nascosta; comandi e carte toccabili; B per annullare |
  | Mappa e Bestiario | tocco e scorrimento; B |

- **Feedback**: il pulsante premuto si schiarisce. Vibrazione breve opzionale su conferma e colpo critico.

## 19. Regole di coerenza (checklist)

1. Solo colori della palette; rampe nuove registrate in `palette.py`.
2. Luce dall'alto a sinistra, ombra portata in basso a destra.
3. Contorni di 1 px colorati, mai neri; cornici UI di 2 px al massimo.
4. Scala intera, niente rotazioni, sfocature o scale non intere; nessun pixel di dimensioni diverse nella stessa scena.
5. Testo solo nei font Eclyra; mai sotto 7 px (eccetto il micro per etichette).
6. Il colore non è mai l'unica informazione: glifi di tipo, sigle di stato, frecce di efficacia.
7. Almeno un elemento Lumen per schermata, un solo punto luminoso dominante.
8. Ogni schermata funziona con tastiera, controller, mouse e touch, e mostra i prompt giusti.
9. Le informazioni essenziali restano nel riquadro sicuro 320×180.
10. Gli asset rispettano `ECLYRA_ASSET_SPEC.md` e passano il validatore prima dell'import.
