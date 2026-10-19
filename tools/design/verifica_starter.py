#!/usr/bin/env python3
"""
ECLYRA - Verifica dei nuovi starter disegnati a mano (DECISIONI 2026-10-19, rework degli starter).

Per ogni sorgente in art_src/elyon/ esporta il foglio (art_src/elyon/export/) e controlla le regole
della STYLE BIBLE e della checklist QC: dimensioni, alfa, appoggio, ingombro, contorno, pixel isolati,
palette dichiarata, numero di materiali, occhi, nodo di Lumen, fotogrammi diversi, sagome distinte.
Produce docs/design/rework_starter/verifica.md, la tavola verifica.png e le animazioni di riposo (GIF).

Uso: python3 tools/design/verifica_starter.py   (esce con 1 se un controllo fallisce)
"""
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
import font as F           # noqa: E402
import mockups as M        # noqa: E402
import palette as P        # noqa: E402
import pixel_elyon as X    # noqa: E402
import ui                  # noqa: E402

RADICE = X.RADICE
OUT = os.path.join(RADICE, "docs", "design", "rework_starter")
SORGENTI = ["001_pyrin", "004_aquil", "007_mossen"]
FASCIA_APPOGGIO, FASCIA_INGOMBRO = (59, 60), (17, 28)


def controlla(src, frames):
    esito = []

    def chk(ok, testo):
        esito.append((bool(ok), testo))

    im = frames[0]
    a = np.array(im).astype(int)
    al = a[..., 3]
    opaco = al == 255
    chk(im.size == (64, 64) and im.mode == "RGBA", f"PNG RGBA {im.size[0]}x{im.size[1]}, foglio {64 * len(frames)}x64")
    ys = np.nonzero(opaco.any(axis=1))[0]
    chk(FASCIA_APPOGGIO[0] <= ys.max() <= FASCIA_APPOGGIO[1], f"appoggio a y={ys.max()} (fascia 59-60)")
    chk(not opaco[63].any(), "riga 63 senza pixel opachi")
    cop = opaco.mean() * 100
    chk(FASCIA_INGOMBRO[0] <= cop <= FASCIA_INGOMBRO[1], f"ingombro {cop:.0f}% (fascia 17-28%)")
    xs = np.nonzero(opaco.any(axis=0))[0]
    chk(True, f"riquadro occupato {xs.max() - xs.min() + 1}x{ys.max() - ys.min() + 1} px")
    # alfa intermedia solo per ombra al suolo e aloni (fiamma, Lumen): mai su pixel del corpo
    mezza = (al > 0) & (al < 255)
    chk(not (mezza & opaco).any(), f"alfa 0/255 sul corpo; {mezza.sum()} px semitrasparenti (ombra e aloni)")
    # palette: ogni pixel opaco viene da una rampa o da un colore dichiarato nel sorgente
    ammessi = {tuple(c[:3]) for r in src.mat.values() for c in r} | {tuple(c[:3]) for c in src.colori.values()}
    ammessi |= {tuple(P.mix(r[0], X.VERSO_IL_BUIO, 0.35)[:3]) for r in src.mat.values()}
    fuori = {tuple(c) for c in a[opaco][:, :3]} - ammessi
    chk(not fuori, f"palette dichiarata ({len(ammessi)} colori ammessi, {len(fuori)} fuori)")
    chk(len(src.mat) <= 5, f"{len(src.mat)} materiali (massimo 5) + occhi, bocca, Lumen")
    # pixel isolati: pixel opachi senza vicini opachi
    vicini = np.zeros_like(opaco, dtype=int)
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        vicini += np.roll(np.roll(opaco, dy, 0), dx, 1)
    isolati = int((opaco & (vicini == 0)).sum())
    chk(isolati == 0, f"{isolati} pixel isolati")
    # contorno: i pixel di bordo sono scuri, tranne gli emissivi (fiamma) dichiarati come tali
    emissivi = X.Sorgente.griglia(src, "fiamma 0") != "."
    bordo = opaco & (vicini < 4) & ~emissivi
    luma = 0.3 * a[..., 0] + 0.59 * a[..., 1] + 0.11 * a[..., 2]
    quota = ((luma > 0.6 * 255) & bordo).sum() / max(1, bordo.sum()) * 100
    chk(quota < 10, f"contorno scuro sul {100 - quota:.0f}% del bordo (esclusi gli emissivi)")
    # occhi (STYLE BIBLE §5): sclera e contorno dell'occhio presenti, chiusi nel battito
    def conta(img, colore):
        return int(np.all(np.array(img)[..., :3] == np.array(colore[:3]), axis=-1).sum())
    sclera = conta(im, P.H("F7F5FF")) + conta(im, P.H("C6CCE6"))
    chk(sclera >= 8 and conta(im, P.H("140F26")) >= 20, f"due occhi: {sclera} px di sclera")
    chk(conta(frames[2], P.H("F7F5FF")) == 0, "battito: occhi chiusi")
    chk(src.gemme and all(2 <= h <= 3 for _, _, h in src.gemme), f"nodo di Lumen: {len(src.gemme)}, con alone")
    diversi = [not np.array_equal(np.array(frames[0]), np.array(f)) for f in frames[1:]]
    chk(all(diversi), "5 fotogrammi: riposo, respiro, battito, affondo, colpito (tutti diversi)")
    # icona 32x32 disegnata a mano (ASSET_SPEC §3): 2 fotogrammi, semplificata, al massimo 1 riflesso
    icone = X.fotogrammi_icona(src)
    ia = np.array(icone[0]).astype(int)
    iop = ia[..., 3] == 255
    fuori_i = {tuple(c) for c in ia[iop][:, :3]} - ammessi
    riflessi = conta(icone[0], P.H("FFFFFF"))
    chk(icone[0].size == (32, 32) and len(icone) == 2 and not fuori_i and riflessi <= 1
        and 28 <= np.nonzero(iop.any(axis=1))[0].max() <= 30 and not iop[31].any(),
        f"icona 32x32 disegnata, 2 fotogrammi, palette dichiarata, {riflessi} riflesso")
    return esito, cop, opaco


def sagoma(im, colore=(10, 10, 16, 255)):
    a = np.array(im)
    out = np.zeros_like(a)
    out[a[..., 3] == 255] = colore
    return Image.fromarray(out, "RGBA")


def tavola(righe_dati):
    cella = 64
    Z = 4
    larg = 8 + (cella + 6) * 3 + cella * Z + 10 + (cella + 6) * 2 + 40
    alt = 22 + len(righe_dati) * (cella * Z + 18)
    t = Image.new("RGBA", (larg, alt), P.NOTTE[1])
    ui.intestazione(t, 4, 3, "REWORK STARTER - VERIFICA")
    for r, (nome, vecchio, frames) in enumerate(righe_dati):
        y = 22 + r * (cella * Z + 18)
        x = 6
        F.micro(t, "PRIMA", x, y, P.GRIGIO_UI)
        t.alpha_composite(vecchio, (x, y + 10))
        F.micro(t, "ORA 1X", x + cella + 6, y, P.ORO[3])
        t.alpha_composite(frames[0], (x + cella + 6, y + 10))
        F.micro(t, "SAGOMA", x + 2 * (cella + 6), y, P.GRIGIO_UI)
        chiaro = Image.new("RGBA", (cella, cella), P.PERGAMENA[2])
        chiaro.alpha_composite(sagoma(frames[0]))
        t.alpha_composite(chiaro, (x + 2 * (cella + 6), y + 10))
        F.micro(t, "ICONA 32X32", x, y + 10 + cella + 8, P.GRIGIO_UI)
        ico = X.fotogrammi_icona(X.Sorgente(os.path.join(RADICE, "art_src", "elyon", SORGENTI[r] + ".txt")))[0]
        t.alpha_composite(ico, (x, y + 10 + cella + 16))
        t.alpha_composite(ico.resize((64, 64), Image.NEAREST), (x + 40, y + 10 + cella + 16))
        x4 = x + 3 * (cella + 6)
        F.micro(t, f"{nome} 4X", x4, y, P.ORO[3])
        t.alpha_composite(frames[0].resize((cella * Z, cella * Z), Image.NEAREST), (x4, y + 10))
        xf = x4 + cella * Z + 10
        for i, nomef in enumerate(X.FOTOGRAMMI):
            cx, cy = xf + (i % 2) * (cella + 6), y + 10 + (i // 2) * (cella + 10)
            F.micro(t, nomef.upper(), cx, cy - 8 if i < 2 else cy - 8, P.GRIGIO_UI)
            t.alpha_composite(frames[i], (cx, cy))
    return t


def main():
    os.makedirs(OUT, exist_ok=True)
    righe = ["# Rework degli starter: verifica automatica", "",
             "Generato da `tools/design/verifica_starter.py` (regole di `docs/ELYON_STYLE_BIBLE.md`,",
             "checklist di `docs/design/REWORK_STARTER_PROPOSTA.md`). Sorgenti in `art_src/elyon/`.", "",
             "| Elyon | Esito | Controlli |", "|---|---|---|"]
    errori, dati, sagome = 0, [], []
    for nome in SORGENTI:
        src, frames, file_png = X.esporta(os.path.join(RADICE, "art_src", "elyon", nome + ".txt"))
        esito, cop, opaco = controlla(src, frames)
        ok = all(e[0] for e in esito)
        errori += 0 if ok else 1
        righe.append(f"| {src.meta['nome']} (`{file_png}`) | {'OK' if ok else 'DA CORREGGERE'} | "
                     + "<br>".join(("" if e[0] else "**NO** ") + e[1] for e in esito) + " |")
        dati.append((src.meta["nome"], M.elyon(src.meta["nome"]), frames))
        sagome.append((src.meta["nome"], opaco))
        # animazione di riposo: riposo e respiro alternati (500 ms), battito ogni tanto
        seq = [frames[0], frames[1]] * 3 + [frames[2], frames[1]]
        big = []
        for f in seq:
            b = Image.new("RGBA", (64, 64), P.NOTTE[2])
            b.alpha_composite(f)
            big.append(b.convert("RGB").resize((256, 256), Image.NEAREST))
        big[0].save(os.path.join(OUT, f"riposo_{src.meta['nome'].lower()}.gif"), save_all=True,
                    append_images=big[1:], duration=[500] * 6 + [150, 500], loop=0)
    # sagome: sovrapposizione (IoU) fra coppie, allineate al punto d'appoggio
    for i in range(len(sagome)):
        for j in range(i + 1, len(sagome)):
            a, b = sagome[i][1], sagome[j][1]
            iou = (a & b).sum() / max(1, (a | b).sum())
            # informativo: la STYLE BIBLE chiede sagome riconoscibili, ma non fissa una soglia numerica
            righe.append(f"| {sagome[i][0]} / {sagome[j][0]} | da valutare | "
                         f"sovrapposizione delle sagome {iou * 100:.0f}% (stesso riquadro e appoggio; giudizio a occhio sulla tavola) |")
    righe += ["", f"Esito complessivo: **{'tutti i controlli superati' if errori == 0 else f'{errori} controlli da correggere'}**.",
              "", "Restano alla revisione umana: originalità, espressione, leggibilità su telefono e Deck,",
              "versione 80×80 per il lato giocatore (non ancora decisa).", ""]
    with open(os.path.join(OUT, "verifica.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(righe))
    tv = tavola(dati)
    tv.convert("RGB").resize((tv.width * 2, tv.height * 2), Image.NEAREST).save(os.path.join(OUT, "verifica.png"))
    print("\n".join(righe))
    return 1 if errori else 0


if __name__ == "__main__":
    sys.exit(main())
