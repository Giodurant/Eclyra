# Rework degli starter — Fasi C e D (prima versione, in attesa di approvazione)

**Stato**: prima versione di PYRIN, AQUIL e MOSSEN, con animazione di riposo. **Non ancora integrata nel gioco**:
`slice/assets/` e `assets/sprites/` non sono cambiati.

## File

| File | Cosa contiene |
|---|---|
| `art_src/elyon/001_pyrin.txt`, `004_aquil.txt`, `007_mossen.txt` | sorgenti disegnati a mano: mappa delle parti, ritocchi (occhi, naso, Lumen), occhi chiusi, fiamme |
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
- La colonna 32×32 della tavola è una riduzione di prova: l'icona vera va disegnata a mano (fase E o F).
- La sovrapposizione delle sagome è riportata come numero, non come esito: la STYLE BIBLE chiede sagome riconoscibili
  ma non fissa una soglia.
