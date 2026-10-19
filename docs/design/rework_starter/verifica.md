# Rework degli starter: verifica automatica

Generato da `tools/design/verifica_starter.py` (regole di `docs/ELYON_STYLE_BIBLE.md`,
checklist di `docs/design/REWORK_STARTER_PROPOSTA.md`). Sorgenti in `art_src/elyon/`.

| Elyon | Esito | Controlli |
|---|---|---|
| PYRIN (`elyon_001_pyrin.png`) | OK | PNG RGBA 64x64, foglio 320x64<br>appoggio a y=59 (fascia 59-60)<br>riga 63 senza pixel opachi<br>ingombro 28% (fascia 17-28%)<br>riquadro occupato 46x45 px<br>alfa 0/255 sul corpo; 154 px semitrasparenti (ombra e aloni)<br>palette dichiarata (35 colori ammessi, 0 fuori)<br>3 materiali (massimo 5) + occhi, bocca, Lumen<br>0 pixel isolati<br>contorno scuro sul 100% del bordo (esclusi gli emissivi)<br>due occhi: 10 px di sclera<br>battito: occhi chiusi<br>nodo di Lumen: 1, con alone<br>5 fotogrammi: riposo, respiro, battito, affondo, colpito (tutti diversi) |
| AQUIL (`elyon_004_aquil.png`) | OK | PNG RGBA 64x64, foglio 320x64<br>appoggio a y=59 (fascia 59-60)<br>riga 63 senza pixel opachi<br>ingombro 27% (fascia 17-28%)<br>riquadro occupato 50x41 px<br>alfa 0/255 sul corpo; 100 px semitrasparenti (ombra e aloni)<br>palette dichiarata (34 colori ammessi, 0 fuori)<br>4 materiali (massimo 5) + occhi, bocca, Lumen<br>0 pixel isolati<br>contorno scuro sul 100% del bordo (esclusi gli emissivi)<br>due occhi: 9 px di sclera<br>battito: occhi chiusi<br>nodo di Lumen: 1, con alone<br>5 fotogrammi: riposo, respiro, battito, affondo, colpito (tutti diversi) |
| MOSSEN (`elyon_007_mossen.png`) | OK | PNG RGBA 64x64, foglio 320x64<br>appoggio a y=59 (fascia 59-60)<br>riga 63 senza pixel opachi<br>ingombro 25% (fascia 17-28%)<br>riquadro occupato 42x40 px<br>alfa 0/255 sul corpo; 88 px semitrasparenti (ombra e aloni)<br>palette dichiarata (43 colori ammessi, 0 fuori)<br>5 materiali (massimo 5) + occhi, bocca, Lumen<br>0 pixel isolati<br>contorno scuro sul 100% del bordo (esclusi gli emissivi)<br>due occhi: 9 px di sclera<br>battito: occhi chiusi<br>nodo di Lumen: 1, con alone<br>5 fotogrammi: riposo, respiro, battito, affondo, colpito (tutti diversi) |
| PYRIN / AQUIL | da valutare | sovrapposizione delle sagome 61% (stesso riquadro e appoggio; giudizio a occhio sulla tavola) |
| PYRIN / MOSSEN | da valutare | sovrapposizione delle sagome 47% (stesso riquadro e appoggio; giudizio a occhio sulla tavola) |
| AQUIL / MOSSEN | da valutare | sovrapposizione delle sagome 53% (stesso riquadro e appoggio; giudizio a occhio sulla tavola) |

Esito complessivo: **tutti i controlli superati**.

Restano alla revisione umana: originalità, espressione, leggibilità su telefono e Deck,
icona 32×32 disegnata a mano (la colonna 32×32 della tavola è solo una prova ridotta).
