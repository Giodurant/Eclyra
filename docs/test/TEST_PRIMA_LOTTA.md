# Test della prima battaglia con i tre starter (fase E)

Kael resta al Lv5 con l'IA introduttiva (`AI_INTRO`), senza modifiche artificiali alle statistiche; la sconfitta non ha penalità.

**AQUIL resta sotto osservazione**: se la sua prima lotta risulta troppo lunga o poco interessante, si interviene sulle sue mosse iniziali, non sulla difficoltà di Kael.

## Come provare

1. `./build/slice/eclyra_slice` → NUOVA PARTITA → scegli lo starter.
2. A Lunaria parla con Kael, il ragazzo con la giacca rossa vicino al laboratorio. In alternativa usa F6.
3. Gioca normalmente, senza tasti di prova. Ripeti per PYRIN, AQUIL e MOSSEN, almeno 2 volte ciascuno.

## Risultati automatici (riferimento)

| Prova | PYRIN | AQUIL | MOSSEN |
|---|---|---|---|
| Simulazione, giocatore esperto (4000 lotte) | 86% vittorie | 91% | 73% |
| Simulazione, giocatore principiante | 78% | 74% | 56% |
| Slice, giocatore automatico (prima mossa sempre) | vittoria, 25 turni | vittoria, 23 turni | vittoria, 13 turni |

## Registro dei test reali

| Data | Starter | Esito | Turni (circa) | Durata percepita (giusta / lunga / corta) | Interessante? | Note |
|---|---|---|---|---|---|---|
| | PYRIN | | | | | |
| | PYRIN | | | | | |
| | AQUIL | | | | | |
| | AQUIL | | | | | |
| | MOSSEN | | | | | |
| | MOSSEN | | | | | |

**Domande per il giudizio**:
- Kael esita troppo spesso? (soprattutto con AQUIL)
- La lotta si capisce senza spiegazioni: rosa dei comandi, carte delle mosse, efficacia?
- La durata a Lv5 è accettabile per una prima lotta?
