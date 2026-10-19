# Rework degli starter — Fasi C–F

**Stato** (2026-10-19): PYRIN, AQUIL e MOSSEN disegnati a mano, con animazione di riposo e icona 32×32 disegnata.
**Integrati** nella slice (`slice/assets/elyon.png`, `titolo.png`) e nelle celle 001/004/007 dei fogli del gioco
principale. Versione precedente archiviata in `art_src/archivio/2026-10-19_starter_procedurali/`.
Fase G, prima parte: anche gli stadi intermedi PYRION, AQUILON e MOSSKA sono disegnati a mano e integrati
(celle 002/005/008). Restano procedurali i 3 stadi finali PYROVAR, AQUARYS e VERDANTOR, in attesa di approvazione.

Fase E (correzioni): gorgiera di PYRIN più morbida (non più a "dentini"), icone 32×32 disegnate a mano.

## File

| File | Cosa contiene |
|---|---|
| `art_src/elyon/001_pyrin.txt` … `008_mosska.txt` (6 sorgenti) | sorgenti disegnati a mano: mappa delle parti, ritocchi (occhi, naso, Lumen), occhi chiusi, fiamme |
| `tools/design/pixel_elyon.py` | resa dei sorgenti: luce dall'alto a sinistra in 4 toni, linee interne, contorno colorato, ombra al suolo, aloni, 5 fotogrammi |
| `tools/design/verifica_starter.py` | controlli automatici e tavola; esporta i fogli in `art_src/elyon/export/` (320×64, 5 fotogrammi) |
| `verifica.md`, `verifica.png` | esito dei controlli; tavola con prima/ora, 4×, sagoma, prova a 32×32, i 5 fotogrammi |
| `riposo_*.gif` | animazione di riposo (respiro, fiamma, battito di ciglia), ingrandita 4× |
| `confronto_slice_pc.png` | slice prima (sinistra) e con i nuovi sprite (destra): scelta dello starter, lotta, carte delle mosse, squadra |
| `telefono_*.png` | le stesse schermate nel formato telefono 19,5:9 |

Gli screenshot vengono da una build **locale e temporanea** della slice (non pubblicata), con `--tour`: 93 foto, 0 errori.

## Note per la revisione

- Gli sprite sono a 64×64 su entrambi i lati della lotta, senza ingrandimenti a scala non intera. Per questo il lato giocatore
  appare più piccolo di prima (prima: 80 px ottenuti ridisegnando le forme procedurali). Rimedio previsto: la versione
  80×80 disegnata (ASSET_SPEC §3), da decidere.
- La sovrapposizione delle sagome è riportata come numero, non come esito: la STYLE BIBLE chiede sagome riconoscibili
  ma non fissa una soglia.
- MOSSKA è alto 45 px: la STYLE BIBLE misura 47–55 px per gli intermedi di riferimento (dato descrittivo, non un
  controllo). Il corpo è più massiccio e basso per natura; se serve più slancio si alzano le felci.
- Nel dettaglio del Bestiario della slice il medaglione copre la prima riga della descrizione: succedeva già con
  gli sprite precedenti (le foto differiscono solo nell'area dello sprite), è un dettaglio di impaginazione.
