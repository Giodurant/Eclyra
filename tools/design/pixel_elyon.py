#!/usr/bin/env python3
"""
ECLYRA - Elyon in pixel art disegnata a mano (DECISIONI 2026-10-19).

Il sorgente e' un file di testo in art_src/elyon/<nnn>_<nome>.txt, modificabile carattere per carattere:
  - "== forma ==": la mappa piatta delle PARTI (un carattere = un pixel, '.' = trasparente);
  - "== dettagli ==": ritocchi a mano sopra la luce automatica (cifra 0-4 = tono del materiale
    sotto, lettera = colore della tabella "colore"); '.' o spazio = nessun ritocco;
  - "== chiuso ==": ritocchi del fotogramma "battito" (occhi chiusi);
  - "== fiamma N ==": pixel emissivi (senza contorno) del fotogramma N, con alone;
  - "== icona forma ==" e "== icona dettagli ==": l'icona 32x32 semplificata a mano (ancora: icona_ancora).
La resa applica le regole della STYLE BIBLE per costruzione:
  luce dall'alto a sinistra in 4 toni per gruppo di forma, linee interne fra parti sovrapposte,
  contorno colorato esterno (piu' scuro in basso a destra), ombra al suolo, alone del nodo di Lumen.
I ritocchi a mano hanno sempre l'ultima parola.

Uso:
  python3 tools/design/pixel_elyon.py art_src/elyon/001_pyrin.txt [...]   esporta in art_src/elyon/export/
"""
import copy
import math
import os
import sys

import numpy as np
from PIL import Image
from scipy import ndimage

sys.path.insert(0, os.path.dirname(__file__))
import palette as P  # noqa: E402

RADICE = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
EXPORT = os.path.join(RADICE, "art_src", "elyon", "export")
S = 64
LUCE = np.array([-0.55, -0.65, 0.52])
LUCE = LUCE / np.linalg.norm(LUCE)
VERSO_IL_BUIO = (10, 8, 24, 255)
FOTOGRAMMI = ("riposo", "respiro", "battito", "affondo", "colpito")


class Parte:
    def __init__(self, ch, mat, z=0, su=False, gruppo=None, linea=True):
        self.ch, self.mat, self.z, self.su, self.gruppo, self.linea = ch, mat, z, su, gruppo or ch, linea


class Sorgente:
    def __init__(self, percorso):
        self.percorso = percorso
        self.meta, self.mat, self.parti, self.colori = {}, {}, {}, {}
        self.strati, self.gemme = {}, []
        corrente = None
        with open(percorso, encoding="utf-8") as f:
            for riga in f:
                riga = riga.rstrip("\n")
                if riga.startswith("== ") and riga.endswith(" =="):
                    corrente = riga[3:-3].strip()
                    self.strati[corrente] = []
                    continue
                if corrente is not None:
                    if riga.strip() == "" and not self.strati[corrente]:
                        continue
                    self.strati[corrente].append(riga)
                    continue
                testo = riga.split("#", 1)[0].strip()
                if not testo:
                    continue
                v = testo.split()
                if v[0] == "mat":
                    self.mat[v[1]] = [P.H(h) for h in v[2:7]]
                elif v[0] == "parte":
                    opz = dict(o.split("=", 1) for o in v[3:] if "=" in o)
                    self.parti[v[1]] = Parte(v[1], v[2], int(opz.get("z", 0)), "su" in v[3:], opz.get("gruppo"),
                                             opz.get("linea", "si") != "no")
                elif v[0] == "colore":
                    self.colori[v[1]] = P.H(v[2])
                elif v[0] == "gemma":
                    self.gemme.append((int(v[1]), int(v[2]), int(v[3])))
                elif v[0].endswith(":"):
                    self.meta[v[0][:-1]] = " ".join(v[1:])
        for nome in self.strati:
            while self.strati[nome] and not self.strati[nome][-1].strip():
                self.strati[nome].pop()
        self.ax, self.ay = (int(n) for n in self.meta["ancora"].split())
        self.n = S

    def icona(self):
        """Vista dell'icona 32x32: strati 'icona forma' e 'icona dettagli', stesse parti e materiali"""
        v = copy.copy(self)
        v.n = 32
        v.ax, v.ay = (int(n) for n in self.meta["icona_ancora"].split())
        v.strati = {"forma": self.strati["icona forma"], "dettagli": self.strati.get("icona dettagli", [])}
        v.gemme = []
        return v

    def griglia(self, strato):
        """strato -> matrice 64x64 di caratteri ('.' = vuoto), posata sull'ancora"""
        n = self.n
        g = np.full((n, n), ".", dtype="<U1")
        for y, riga in enumerate(self.strati.get(strato, [])):
            for x, ch in enumerate(riga):
                if ch not in ". " and 0 <= self.ay + y < n and 0 <= self.ax + x < n:
                    g[self.ay + y, self.ax + x] = ch
        return g


def _sposta_su(g, maschera):
    out = np.full_like(g, ".")
    out[:-1][maschera[1:]] = g[1:][maschera[1:]]
    return out


def mappa_fotogramma(src, respiro):
    """Mappa delle parti del fotogramma; nel respiro le parti 'su' salgono di 1 px"""
    forma = src.griglia("forma")
    if not respiro:
        return forma, np.zeros(forma.shape, bool)
    su = np.vectorize(lambda c: c in src.parti and src.parti[c].su)(forma)
    alto = _sposta_su(forma, su)
    fisso = np.where(su, ".", forma)
    z = np.vectorize(lambda c: src.parti[c].z if c in src.parti else -999)
    out = np.where(z(alto) >= z(fisso), np.where(alto != ".", alto, fisso), fisso)
    # dove una parte 'su' si e' alzata lasciando un buco, la parte fissa sotto (zampa) si allunga di 1 px
    buco = (out == ".") & (forma != ".")
    sotto = np.vstack([out[1:], np.full((1, forma.shape[1]), ".")])
    out = np.where(buco & (sotto != "."), sotto, out)
    return out, su


def luce_gruppo(maschera):
    """Toni 1-4 dalla luce dall'alto a sinistra, con il rilievo ricavato dalla forma disegnata"""
    if not maschera.any():
        return np.zeros(maschera.shape, int)
    d = ndimage.distance_transform_edt(maschera)
    h = np.sqrt(np.clip(d / max(1.0, d.max()), 0, 1))
    gy, gx = np.gradient(ndimage.gaussian_filter(h, 0.8))
    n = np.stack([-gx * 6, -gy * 6, np.ones_like(h)], -1)
    n /= np.linalg.norm(n, axis=-1, keepdims=True)
    i = (n * LUCE).sum(-1)
    return np.select([i > 0.88, i > 0.55, i > 0.12], [4, 3, 2], 1)


def rendi(src, fotogramma):
    respiro = fotogramma == "respiro"
    mappa, su = mappa_fotogramma(src, respiro)
    n = src.n
    col = np.zeros((n, n, 4), np.uint8)
    pieno = mappa != "."
    parte = {c: src.parti[c] for c in np.unique(mappa) if c != "."}
    for c in parte:
        if c not in src.parti:
            raise SystemExit(f"{src.percorso}: carattere '{c}' senza 'parte'")
    gruppo = np.vectorize(lambda c: src.parti[c].gruppo if c != "." else "")(mappa)
    zmap = np.vectorize(lambda c: src.parti[c].z if c != "." else -999)(mappa)
    tono = np.zeros((n, n), int)
    for g in np.unique(gruppo):
        if g:
            m = gruppo == g
            tono[m] = luce_gruppo(m)[m]
    # linee interne: il bordo della parte dietro si scurisce al tono 0
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        vg = np.roll(np.roll(gruppo, dy, 0), dx, 1)
        vz = np.roll(np.roll(zmap, dy, 0), dx, 1)
        linea = np.vectorize(lambda c: c != "." and src.parti[c].linea)(mappa)
        tono[pieno & linea & (vg != "") & (vg != gruppo) & (vz > zmap)] = 0
    # ritocchi a mano (seguono la parte a cui appartengono: salgono con lei nel respiro)
    strati = [src.griglia("dettagli")]
    if fotogramma == "battito":
        strati.append(src.griglia("chiuso"))
    esplicito = {}
    for st in strati:
        if respiro:
            alzati = _sposta_su(np.where(su, st, "."), np.ones((n, n), bool))
            st = np.where(alzati != ".", alzati, np.where(su, ".", st))
        for y, x in zip(*np.nonzero(st != ".")):
            ch = st[y, x]
            if ch.isdigit():
                tono[y, x] = int(ch)
                esplicito.pop((y, x), None)
            elif ch in src.colori:
                esplicito[(y, x)] = src.colori[ch]
            else:
                raise SystemExit(f"{src.percorso}: ritocco '{ch}' sconosciuto")
    for y, x in zip(*np.nonzero(pieno)):
        col[y, x] = src.mat[src.parti[mappa[y, x]].mat][tono[y, x]]
    for (y, x), c in esplicito.items():
        col[y, x] = c
        pieno[y, x] = True
    # contorno esterno colorato: tono 0 del materiale vicino, scurito sul lato in ombra
    esterno = ~pieno
    fatto = np.zeros((n, n), bool)
    for dy, dx in ((1, 0), (0, 1), (-1, 0), (0, -1)):
        v = np.roll(np.roll(mappa, dy, 0), dx, 1)
        nuovi = esterno & (v != ".") & ~fatto
        for y, x in zip(*np.nonzero(nuovi)):
            base = src.mat[src.parti[v[y, x]].mat][0]
            illuminato = dy == 1 or dx == 1   # il vicino pieno sta in basso o a destra: siamo sul lato della luce
            col[y, x] = base if illuminato else P.mix(base, VERSO_IL_BUIO, 0.35)
        fatto |= nuovi
    im = Image.fromarray(col, "RGBA").copy()
    # pixel emissivi (fiamme): nessun contorno, alone tenue sul trasparente
    nome_fiamma = {"riposo": "fiamma 0", "respiro": "fiamma 1", "battito": "fiamma 2"}.get(fotogramma, "fiamma 0")
    if nome_fiamma in src.strati:
        fg = src.griglia(nome_fiamma)
        if respiro:
            fg = _sposta_su(fg, np.ones((n, n), bool))
        _emissivi(im, fg, src)
    for gx, gy, h in src.gemme:
        _alone(im, src.ax + gx, src.ay + gy - (1 if respiro else 0), h, src.colori.get("alone", P.LUMEN[3]))
    _ombra(im)
    return im


def _emissivi(im, fg, src):
    a = im.load()
    pos = list(zip(*np.nonzero(fg != ".")))
    for y, x in pos:
        a[int(x), int(y)] = src.colori[fg[y, x]]
    tinta = src.colori.get("alone_fiamma", src.colori.get("alone", P.LUMEN[3]))
    vicino = np.zeros(fg.shape, int)
    m = fg != "."
    for r, alfa in ((1, 80), (2, 40)):
        d = ndimage.binary_dilation(m, iterations=r)
        vicino = np.where(d & (vicino == 0), alfa, vicino)
    for y, x in zip(*np.nonzero(vicino)):
        if a[int(x), int(y)][3] == 0:
            a[int(x), int(y)] = (tinta[0], tinta[1], tinta[2], int(vicino[y, x]))


def _alone(im, cx, cy, h, tinta):
    a = im.load()
    for y in range(cy - h - 1, cy + h + 2):
        for x in range(cx - h - 1, cx + h + 2):
            if 0 <= x < im.width and 0 <= y < im.height and a[x, y][3] == 0:
                d = math.hypot(x - cx, y - cy) / (h + 1)
                if d < 1:
                    a[x, y] = (tinta[0], tinta[1], tinta[2], int(110 * (1 - d)))


def _ombra(im):
    """Ombra al suolo semitrasparente sotto i piedi (righe fino alla 63)"""
    a = np.array(im)
    opachi = np.nonzero(a[..., 3] == 255)
    if not len(opachi[0]):
        return
    fondo = opachi[0].max()
    cx = (opachi[1].min() + opachi[1].max()) / 2
    rx = (opachi[1].max() - opachi[1].min()) / 2 * 0.8
    sh = Image.new("RGBA", im.size, (0, 0, 0, 0))
    for y in range(fondo - 2, min(im.height, fondo + 4)):
        for x in range(int(cx - rx), int(cx + rx) + 1):
            if 0 <= x < im.width and ((x - cx) / max(1, rx)) ** 2 + ((y - fondo - 0.5) / 2.5) ** 2 <= 1:
                sh.putpixel((x, y), (8, 10, 30, 90))
    sh.alpha_composite(im)
    im.paste(sh)


def fotogrammi(src):
    riposo = rendi(src, "riposo")
    affondo = Image.new("RGBA", (S, S))
    affondo.alpha_composite(riposo, (-4, 0))
    colpito = riposo.copy()
    a = colpito.load()
    for y in range(S):
        for x in range(S):
            r, g, b, al = a[x, y]
            if al == 255:
                a[x, y] = (min(255, r + 150), min(255, g + 150), min(255, b + 150), al)
    return [riposo, rendi(src, "respiro"), rendi(src, "battito"), affondo, colpito]


def fotogrammi_icona(src):
    """Icona 32x32 disegnata a mano: riposo e saltello (1 px piu' in alto)"""
    riposo = rendi(src.icona(), "riposo")
    saltello = Image.new("RGBA", riposo.size)
    saltello.alpha_composite(riposo, (0, -1))
    return [riposo, saltello]


def striscia(frames):
    w, h = frames[0].size
    out = Image.new("RGBA", (w * len(frames), h))
    for i, f in enumerate(frames):
        out.alpha_composite(f, (i * w, 0))
    return out


def esporta(percorso):
    src = Sorgente(percorso)
    frames = fotogrammi(src)
    os.makedirs(EXPORT, exist_ok=True)
    nome = f"elyon_{src.meta['numero']}_{src.meta['nome'].lower()}.png"
    striscia(frames).save(os.path.join(EXPORT, nome), optimize=True)
    if "icona forma" in src.strati:
        striscia(fotogrammi_icona(src)).save(os.path.join(EXPORT, nome.replace(".png", "_icona.png")), optimize=True)
    return src, frames, nome


if __name__ == "__main__":
    for p in sys.argv[1:]:
        _, _, n = esporta(p)
        print("esportato", n)
