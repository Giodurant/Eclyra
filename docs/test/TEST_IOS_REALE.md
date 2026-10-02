# Test reale iPhone / iPad — registro (fase A)

Protocollo completo: `prototypes/ios_sdl/PROTOTIPO_IOS.md` §4.

Il supporto iOS **non è validato** finché questo registro non è compilato con un dispositivo reale.

## Dispositivo

| Campo | Valore |
|---|---|
| Modello (iPhone / iPad) | |
| Versione di iOS / iPadOS | |
| Data del test | |
| Versione di Xcode | |
| Apple ID (gratuito o a pagamento) | |

**Valori della diagnostica** (MENU nel prototipo):

| Campo | Valore |
|---|---|
| schermo (pixel) | |
| punti | |
| canvas e scala | |
| bande | |
| area sicura | |
| fbo | |

## Esito

| # | Area | OK / NO | Note |
|---|---|---|---|
| 1 | Avvio (orizzontale, schermo intero, 3 avvii senza crash) | | |
| 2 | Rendering 320×180 (canvas a scala intera, 320–400 × 180–240) | | |
| 3 | Scaling (scacchiera di 1 px nitida, PYRIN nitido) | | |
| 4 | Area sicura (notch e angoli evitati, controlli dentro) | | |
| 5 | Touch (croce, A, B, MENU, diagonali, 2 dita, vibrazione) | | |
| 6 | Misura dei controlli (B cambia la scala) | | |
| 7 | Controller (nome riconosciuto, comandi, touch nascosto) | | |
| 8 | Tastiera (iPad: WASD, frecce, Z) | | |
| 9 | Audio (musica continua, bip, volume) | | |
| 10 | Pausa / ripresa (Home, 10 s, ritorno) | | |
| 11 | Interruzioni (blocco, chiamata o timer, Centro di Controllo) | | |
| 12 | Salvataggio (avvii +1, file visibile in File) | | |
| 13 | Test automatico (A+B per 2 s: tutte le righe OK) | | |
| 14 | Prestazioni (5 minuti a 60 FPS, temperatura normale) | | |

**Il prototipo è stabile se** le prove 1–5, 9, 10, 12 e 13 sono OK e le altre non hanno blocchi.

Allegare una foto della diagnostica e il file `selftest_rapporto.txt` (app File → ECLYRA Proto).
