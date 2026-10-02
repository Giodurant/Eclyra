# ECLYRA — Style Bible degli Elyon

**Stato**: APPROVATA e congelata. Riferimento ufficiale per tutti i 100 Elyon.

**Fonte di verità**: i 9 sprite di riferimento in `docs/design/tavola_starter.png`, generati da `tools/design/elyon_design.py` con il motore `tools/design/sprites.py`:
- PYRIN → PYRION → PYROVAR;
- AQUIL → AQUILON → AQUARYS;
- MOSSEN → MOSSKA → VERDANTOR.

Gli altri 91 Elyon **derivano da queste regole**: non imitano "a occhio" lo stile. Ogni regola qui sotto è misurata sui 9 sprite. Le regole in **grassetto** sono verificate dal validatore (ASSET_SPEC §10); le altre dalla checklist umana (§12).

---

## 1. Riquadro e proporzioni

| Misura | Base | Intermedio | Finale |
|---|---|---|---|
| Riquadro | **64×64** | **64×64** | **64×64** (boss: 128×128) |
| Taglia di riferimento | 84% | 93% | 100% |
| Ingombro misurato (L×A, px) | 40–49 × 37–45 | 51–59 × 47–55 | 62–64 × 55–62 |
| Rapporto testa/corpo | testa ≈ corpo, forme tonde | testa più piccola, corpo allungato | testa piccola, corpo massiccio o slanciato |

- **Linea d'appoggio** (misurata): il pixel opaco più basso cade a y = 59–60 (base), 60–61 (intermedio), 61–62 (finale). **La riga 63 non ha pixel opachi**: è dell'ombra al suolo, che occupa le righe 58–63.
- Le taglie si scalano attorno al punto d'appoggio (32, 58), quindi gli stadi piccoli "poggiano" un po' più in alto.
- **Vista**: 3/4 di profilo, **muso verso sinistra** (sprite frontale). Il lato giocatore in battaglia è lo specchio.
- La vista da dietro si ottiene specchiando le **forme**, non l'immagine, così la luce resta in alto a sinistra.
- **Il finale riempie il riquadro**; il base lascia almeno 8 px d'aria per lato.

## 2. Luce e ombreggiatura

- **Luce dall'alto a sinistra**, vettore (−0,55; −0,65; 0,52): identica per tutti.
- **Cel shading a 4 toni** per materiale (toni 1–4 della rampa; lo 0 è riservato a contorni e linee interne).
- Soglie: ≥ 0,88 luce piena, ≥ 0,55 chiaro, ≥ 0,12 medio, sotto ombra.
- Vietati: sfumature, dithering, anti-aliasing verso il trasparente, ombre nere pure.
- **Macchie e segni sul manto** seguono la luce del corpo sotto, un tono più scure: mai toppe piatte.

## 3. Palette

- **Ogni materiale è una rampa di 5 toni** (dal più scuro al più chiaro), presa o derivata da `tools/design/palette.py`.
- **Al massimo 5 materiali** per Elyon (i 9 di riferimento ne usano 4–5), più occhi, bocca e Lumen.
- Il materiale principale segue i **colori del tipo** (Design System §3):

| Tipo | Scuro | Base | Accento |
|---|---|---|---|
| FIAMMA | #8C2414 | #E8542F | #FFC24A |
| ACQUA | #123E8A | #2F86E0 | #7FE0F2 |
| NATURA | #1E5E2A | #4CAF4A | #A8E07A |
| IMPULSO | #8A6410 | #F2C230 | #FFF6A0 |
| VENTO | #3E7EA6 | #8CCDEA | #E2F6FF |
| ROCCIA | #5A3E20 | #A57A4A | #B8B2A4 |
| GELO | #1E9EBE | #B6ECFF | #FFFFFF |
| OMBRA | #0E0A18 | #3E2E7A | #8A6CD2 |
| ASTRALE | #1E2A78 | #6E5AD8 | #F0D27A |
| TOSSICO | #4E1A66 | #9A3FC8 | #A8E83A |
| PSICHE | #6A1E6A | #E0409A | #FFA6D2 |
| METALLO | #3A4458 | #8E9CB0 | #BCD6F2 |
| NEUTRO | #5C5F70 | #9EA1B0 | #E6E2D6 |

- Il secondo tipo entra come **materiale secondario**, quasi sempre allo stadio finale. Esempi: le placche di metallo di PYROVAR, la cresta astrale di AQUARYS, la roccia e il bosco di VERDANTOR.
- Un **materiale neutro chiaro** (crema o ventre) dà riposo all'occhio: muso, ventre, interno delle orecchie.

## 4. Contorni

- **Contorno di 1 px colorato**, mai nero uniforme:
  - lato in ombra (basso e destra): tono 0 del materiale, scurito del 35% verso #0A0818;
  - lato illuminato (alto e sinistra): tono 0 puro, più leggero.
- **Linee interne**: dove una parte copre un'altra, il bordo della parte dietro diventa tono 0. Si leggono così zampe, orecchie e coda sovrapposte.
- Nessun contorno doppio, nessuno scalino di 2 px sulle curve, nessun pixel isolato.

## 5. Occhi

Sono il tratto più importante dello stile. Valgono per tutti.

| Elemento | Regola |
|---|---|
| Numero | **sempre 2 visibili** nella vista di 3/4: l'occhio vicino ha raggio 3, quello lontano raggio 2 |
| Posizione | nella metà alta della testa, sguardo verso sinistra (verso l'avversario) |
| Sclera | #F7F5FF, con l'ultima riga in basso #C6CCE6 |
| Contorno | #140F26, ovale leggermente più alto che largo |
| Iride | 3 toni del colore della famiglia (scuro in alto, chiaro in basso) |
| Pupilla | 2×2 px (1×2 negli occhi piccoli) |
| Riflessi | 2 bianchi in alto a sinistra, più 1 tenue in basso a destra negli occhi grandi |
| Palpebra | linea superiore spessa che dà l'espressione |
| Battito | fotogramma 2: occhio chiuso = una linea |

**Espressione per stadio** (linguaggio delle evoluzioni):
- base: *felice* o *deciso*;
- intermedio: *deciso*;
- finale: *fiero*.

Gli Elyon OMBRA e PSICHE possono avere iridi luminose, ma la struttura dell'occhio resta questa.

## 6. Nodo di Lumen

Ogni Elyon ha **almeno un nodo di Lumen**: il segno del mondo di Asterra.

- **Forma**: cristallo a rombo con 4 facce (luce in alto a sinistra) e alone tenue intorno. Due pixel scuri alle punte.
- **Posizione**: fronte o petto, nel terzo anteriore e alto del corpo, sempre visibile nello sprite frontale.
- **Altezza per stadio**:
  - base 3–4 px;
  - intermedio 5–6 px;
  - finale 6–8 px.
- **Numero**:
  - base e intermedio: 1;
  - finale: 1 principale + 0–2 secondari più piccoli (4 px), su cresta, spalle o coda.
- **Colore**:
  - di norma la rampa LUMEN;
  - la famiglia può tingerlo con i suoi colori (es. AQUARYS);
  - le varianti Lumen cambiano solo questo e la palette, mai la forma.
- L'alone è l'unico punto, insieme all'ombra al suolo, dove si accetta un'alfa intermedia.

## 7. Silhouette

- **Riempita di nero, deve riconoscere la specie** fra le altre della stessa linea e del tipo.
- **3 masse principali** leggibili: testa, corpo e un tratto distintivo (coda, cresta, guscio).
- Il tratto distintivo della famiglia **resta in tutta la linea evolutiva** e cresce. Esempi: la fiamma della coda di PYRIN, le pinne a goccia di AQUIL, il muschio sul dorso di MOSSEN.
- **Deve funzionare a 32×32** (icona semplificata a mano) e sul cerchio runico della lotta.
- **Pixel opachi sul totale del riquadro** (misurati: base 20–22%, intermedio 33–38%, finale 46–53%): base 17–28%, intermedio 29–43%, finale 42–60%. Un boss segue la fascia del finale sul suo riquadro 128×128.

## 8. Linguaggio delle evoluzioni

| Aspetto | Base | Intermedio | Finale |
|---|---|---|---|
| Corpo | cucciolo tondo, zampe corte | giovane allungato, arti più lunghi | adulto, postura stabile e potente |
| Elemento | accenno (3 fiamme, 1 germoglio) | più presente (5 fiamme, felci) | dominante, con il secondo tipo se previsto |
| Materiali | principale + neutro | uguali, proporzioni diverse | si aggiunge il materiale del secondo tipo |
| Nodo di Lumen | 1, piccolo | 1, medio | 1 grande + secondari |
| Espressione | felice o deciso | deciso | fiero |
| Tratto distintivo | presente | più grande | trasformato ma riconoscibile |

Una linea a 2 stadi usa le colonne base e finale.

Un Elyon senza evoluzioni ha l'aspetto del suo ruolo:
- comune: base;
- raro: intermedio;
- leggendario o primordiale: finale o boss.

## 9. Motivi per tipo

Per i tipi già coperti dai 9 sprite (FIAMMA, ACQUA, NATURA, più METALLO, ASTRALE e ROCCIA come secondari) i motivi sono definiti. Per gli altri sono **proposte da confermare** con il primo Elyon prodotto di quel tipo.

| Tipo | Motivi ammessi | Stato |
|---|---|---|
| FIAMMA | fiamme emissive animate (nucleo chiaro, bordo rosso), brace, fuliggine sulle zampe | da riferimento |
| ACQUA | pinne a goccia, ventre chiaro, macchie a stella, superfici lisce | da riferimento |
| NATURA | muschio, felci, germogli, pietra di fondo | da riferimento |
| METALLO | placche con riflesso freddo, rivetti di 1 px | da riferimento (PYROVAR) |
| ASTRALE | creste con stelle, gemme secondarie | da riferimento (AQUARYS) |
| ROCCIA | blocchi a facce piatte, crepe, licheni | da riferimento (VERDANTOR) |
| IMPULSO | punte a zigzag, scintille emissive di 1–2 px | proposta |
| VENTO | piume o membrane sottili, linee di corrente | proposta |
| GELO | cristalli trasparenti (tono 4 + bianco), brina sui bordi | proposta |
| OMBRA | parti che sfumano nel contorno, occhi luminosi | proposta |
| TOSSICO | sacche e chiazze acide, gocce | proposta |
| PSICHE | anelli e simboli fluttuanti, gemma frontale più grande | proposta |
| NEUTRO | pelo e forme semplici, nodo di Lumen ben in vista | proposta |

## 10. Animazione

Fotogrammi obbligatori (ASSET_SPEC §3):
- **riposo**;
- **respiro**: le parti con tag `su` salgono di 1 px;
- **battito**: occhi chiusi;
- **affondo**: 4 px in avanti;
- **colpito**: lampo chiaro.

Fiamme, scintille e alone del nodo possono animarsi nei fotogrammi di riposo e respiro. Niente sotto-pixel e niente rotazioni.

## 11. Come si produce un nuovo Elyon

1. **Scheda procedurale** in `elyon_design.py`: forme, materiali, occhi, nodo e taglia per stadio. Garantisce luce, contorni, occhi e taglia per costruzione.
2. **Concept generativo** (facoltativo) per idee di forma, con il modello di prompt di ASSET_SPEC §9.1. Serve solo a ispirare la scheda o la rifinitura.
3. **Rifinitura manuale** in LibreSprite, Pixelorama o Aseprite: dettagli, tratto distintivo, icona 32×32.
4. **Confronto affiancato** con la tavola di riferimento: stessa taglia per stadio, stessa luce, stessi occhi.
5. **Validatore** + checklist (§12) + registro di provenienza. Poi import.

Ordine di produzione: le linee che servono prima nella storia (ASSET_SPEC §12), un tipo nuovo alla volta, così il primo Elyon di ogni tipo conferma i motivi della §9.

## 12. Checklist di accettazione

1. Riquadro, linea d'appoggio e taglia per stadio corretti (§1).
2. Luce in alto a sinistra, 4 toni, nessuna sfumatura (§2).
3. Palette: rampe valide, materiale principale del tipo (§3).
4. Contorno colorato e selettivo, linee interne presenti (§4).
5. Due occhi costruiti secondo la §5, espressione giusta per lo stadio.
6. Nodo di Lumen visibile, della misura giusta per lo stadio (§6).
7. Silhouette riconoscibile in nero e a 32×32 (§7).
8. La famiglia evolutiva si riconosce, e il tratto distintivo resta (§8).
9. Originalità: nessuna somiglianza con creature, personaggi o marchi esistenti (Design System §16).
10. Provenienza registrata, con sorgente o prompt quando c'è (ASSET_SPEC §9.3).
