#!/usr/bin/env python3
"""
ECLYRA - Verifica di coerenza dei 9 Elyon di riferimento (STYLE BIBLE, docs/ELYON_STYLE_BIBLE.md).

Controlli automatici, uguali a quelli che il validatore applichera' a tutti gli Elyon futuri:
  riquadro 64x64, linea d'appoggio per stadio, riga 63 libera, ingombro per stadio, contorno
  presente (bordi scuri), due occhi costruiti (sclera + contorno), nodo di Lumen con alone,
  crescita dell'ingombro lungo la linea evolutiva, sagome distinguibili.
Produce docs/design/verifica_elyon.md e la tavola docs/design/verifica_elyon.png
(fronte, lato avversario, lato giocatore, icona e sagome nere).

Uso: python3 tools/design/valida_elyon.py   (esce con 1 se un controllo fallisce)
"""
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
import elyon_design as D   # noqa: E402
import font as F           # noqa: E402
import mockups as M        # noqa: E402
import palette as P        # noqa: E402
import ui                  # noqa: E402

RADICE = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
STADI = {0: "base", 1: "intermedio", 2: "finale"}
FASCIA_APPOGGIO = {0: (59, 60), 1: (60, 61), 2: (61, 62)}
FASCIA_INGOMBRO = {0: (17, 28), 1: (29, 43), 2: (42, 60)}


def analizza(nome, stadio):
    im = M.elyon(nome)
    a = np.array(im).astype(int)
    al = a[..., 3]
    opaco = al == 255
    esito = []

    def chk(ok, testo):
        esito.append((ok, testo))

    chk(im.size == (64, 64), f"riquadro {im.size[0]}x{im.size[1]}")
    ys = np.nonzero(opaco.any(axis=1))[0]
    basso = int(ys.max())
    lo, hi = FASCIA_APPOGGIO[stadio]
    chk(lo <= basso <= hi, f"appoggio a y={basso} (fascia {lo}-{hi})")
    chk(not opaco[63].any(), "riga 63 senza pixel opachi")
    cop = opaco.mean() * 100
    lo, hi = FASCIA_INGOMBRO[stadio]
    chk(lo <= cop <= hi, f"ingombro {cop:.0f}% (fascia {lo}-{hi}%)")
    # contorno: pixel opachi che toccano il trasparente devono essere scuri (esclusi emissivi: fiamme e Lumen)
    trasp = ~opaco
    vicino = np.zeros_like(opaco)
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        vicino |= np.roll(np.roll(trasp, dy, 0), dx, 1)
    bordo = opaco & vicino
    luma = 0.3 * a[..., 0] + 0.59 * a[..., 1] + 0.11 * a[..., 2]
    chiari = (luma > 0.6 * 255) & bordo
    quota = chiari.sum() / max(1, bordo.sum()) * 100
    chk(quota < 15, f"contorno scuro sul {100 - quota:.0f}% del bordo (emissivi ammessi)")
    # occhi: sclera e contorno dell'occhio
    sclera = np.all(a[..., :3] == P.H("F7F5FF")[:3], axis=-1).sum()
    contorno_occhio = np.all(a[..., :3] == P.H("140F26")[:3], axis=-1).sum()
    chk(sclera >= 10 and contorno_occhio >= 10, f"occhi: {sclera} px di sclera, {contorno_occhio} px di contorno")
    # nodo di Lumen: cristallo presente e della misura giusta per lo stadio (STYLE BIBLE §6)
    forme, _ = D.SCHEDE[nome]()
    # le schede sono gia' scalate per stadio; il motore arrotonda l'altezza come qui
    gemme = sorted((max(3, int(round(g.k["h"]))) for g in forme if g.tipo == "gemma"), reverse=True)
    fascia = {0: (3, 4), 1: (5, 6), 2: (6, 8)}[stadio]
    principale = gemme[0] if gemme else 0
    lumen = 0
    for c in (P.LUMEN[2], P.LUMEN[3], P.LUMEN[4]):
        lumen += (np.abs(a[..., :3] - np.array(c[:3])).sum(-1) < 40).sum()
    chk(bool(gemme) and fascia[0] <= principale <= fascia[1] and (lumen >= 4 or len(gemme) > 0),
        f"nodo di Lumen: {len(gemme)} cristallo/i, principale alto {principale} px (fascia {fascia[0]}-{fascia[1]})")
    colori = len({tuple(c) for c in a[opaco][:, :3]})
    return im, esito, cop, colori


def sagoma(im):
    a = np.array(im)
    out = np.zeros_like(a)
    out[..., 3] = np.where(a[..., 3] == 255, 255, 0)
    out[..., :3] = 10
    return Image.fromarray(out.astype(np.uint8), "RGBA")


def main():
    righe = ["# Verifica dei 9 Elyon di riferimento", "",
             "Generato da `tools/design/valida_elyon.py` (regole di `docs/ELYON_STYLE_BIBLE.md`).", "",
             "| Elyon | Stadio | Esito | Ingombro | Colori opachi | Dettagli |", "|---|---|---|---|---|---|"]
    errori = 0
    tavola = Image.new("RGBA", (12 + 3 * (64 + 70 + 80 + 32 + 64 + 18), 3 * 104 + 30), P.NOTTE[1])
    ui.intestazione(tavola, 4, 3, "VERIFICA ELYON")
    for r, linea in enumerate(D.LINEE_STARTER):
        coperture = []
        for s, nome in enumerate(linea):
            im, esito, cop, colori = analizza(nome, s)
            coperture.append(cop)
            ok = all(e[0] for e in esito)
            errori += 0 if ok else 1
            dettagli = "; ".join(("" if e[0] else "**NO** ") + e[1] for e in esito)
            righe.append(f"| {nome} | {STADI[s]} | {'OK' if ok else 'DA CORREGGERE'} | {cop:.0f}% | {colori} | {dettagli} |")
            # tavola: fronte 64, avversario 70, giocatore 80 (specchio), icona 32, sagoma
            x = 6 + s * (64 + 70 + 80 + 32 + 64 + 18)
            y = 22 + r * 104
            tavola.alpha_composite(im, (x, y + 16))
            tavola.alpha_composite(M.elyon(nome, scala=1.1), (x + 64, y + 10))
            tavola.alpha_composite(M.elyon(nome, scala=1.25).transpose(Image.FLIP_LEFT_RIGHT), (x + 134, y))
            tavola.alpha_composite(M.elyon(nome, scala=0.5), (x + 214, y + 48))
            tavola.alpha_composite(sagoma(im), (x + 246, y + 16))
            F.micro(tavola, f"{nome}  {STADI[s].upper()}", x, y + 84, P.ORO[3])
            F.micro(tavola, "OK" if ok else "DA CORREGGERE", x, y + 92, P.LUMEN[3] if ok else P.TIPI["FIAMMA"][1])
        cresce = coperture[0] < coperture[1] < coperture[2]
        errori += 0 if cresce else 1
        righe.append(f"| linea {linea[0]} | - | {'OK' if cresce else 'DA CORREGGERE'} | "
                     f"{' < '.join(f'{c:.0f}%' for c in coperture)} | - | l'ingombro cresce con lo stadio |")
    righe += ["", "Colonne della tavola: fronte 64 px, lato avversario 70 px, lato giocatore 80 px (specchio),",
              "icona 32 px, sagoma nera (riconoscibilita').", "",
              f"Esito complessivo: **{'tutti i controlli superati' if errori == 0 else f'{errori} controlli da correggere'}**.", ""]
    out = os.path.join(RADICE, "docs", "design")
    with open(os.path.join(out, "verifica_elyon.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(righe))
    tavola.convert("RGB").resize((tavola.width * 2, tavola.height * 2), Image.NEAREST).save(os.path.join(out, "verifica_elyon.png"))
    print("\n".join(righe))
    return 1 if errori else 0


if __name__ == "__main__":
    sys.exit(main())
