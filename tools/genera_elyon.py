#!/usr/bin/env python3
"""
ECLYRA - sprite segnaposto ORIGINALI e procedurali dei 100 Elyon (Fase 4).

Ogni specie riceve, in modo deterministico dal suo ID:
  - una forma del corpo (quadrupede, bipede, serpente, volatile, sfera, pesce, spettro)
    scelta con preferenze per tipo;
  - una taglia che cresce con lo stadio evolutivo;
  - la palette del tipo primario e i tratti del tipo (fiamme, pinne, foglie, ali,
    cristalli, placche, stelle, macchie, gemme...); il secondo tipo aggiunge i suoi tratti.
Sono asset sostituibili: la UI legge solo due fogli 640x640 (griglia 10x10 di 64x64).

Genera:
  assets/sprites/elyon_fronte.png   vista frontale di 3/4 (nemico, rivolto a sinistra)
  assets/sprites/elyon_retro.png    vista da dietro (Elyon del giocatore, rivolto a destra)

Uso: python3 tools/genera_elyon.py [--anteprima file.png]   (richiede Pillow)
"""
import math
import os
import random
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(__file__))
import genera_database as db   # noqa: E402  (tabelle canoniche: specie ed evoluzioni)

RADICE = os.path.join(os.path.dirname(__file__), "..", "assets", "sprites")
S = 64
TR = (0, 0, 0, 0)

# palette per tipo: base, scuro, chiaro, accento
PAL = {
    "FIAMMA":  ((232, 96, 48), (150, 48, 32), (255, 170, 90), (255, 214, 72)),
    "ACQUA":   ((64, 136, 224), (32, 72, 150), (130, 196, 250), (210, 240, 255)),
    "NATURA":  ((84, 176, 76), (40, 104, 48), (150, 220, 120), (240, 210, 90)),
    "IMPULSO": ((240, 204, 56), (170, 120, 30), (255, 238, 140), (90, 200, 255)),
    "VENTO":   ((150, 200, 210), (80, 120, 150), (220, 240, 245), (255, 255, 255)),
    "ROCCIA":  ((160, 128, 92), (96, 72, 52), (204, 178, 140), (120, 110, 100)),
    "GELO":    ((150, 210, 240), (70, 130, 180), (225, 245, 255), (255, 255, 255)),
    "OMBRA":   ((84, 64, 120), (40, 28, 64), (130, 108, 170), (220, 70, 110)),
    "ASTRALE": ((236, 220, 150), (170, 140, 70), (255, 248, 210), (140, 120, 255)),
    "TOSSICO": ((150, 84, 180), (88, 40, 110), (200, 140, 220), (150, 240, 90)),
    "PSICHE":  ((230, 120, 180), (150, 60, 110), (255, 180, 220), (120, 230, 240)),
    "METALLO": ((160, 168, 184), (88, 96, 112), (214, 220, 232), (90, 200, 220)),
}

FORME = ["quadrupede", "bipede", "serpente", "volatile", "sfera", "pesce", "spettro"]
PREFERENZE = {
    "FIAMMA": ["quadrupede", "bipede"], "ACQUA": ["pesce", "sfera", "serpente"],
    "NATURA": ["bipede", "quadrupede", "sfera"], "IMPULSO": ["quadrupede", "sfera"],
    "VENTO": ["volatile"], "ROCCIA": ["quadrupede", "bipede"], "GELO": ["quadrupede", "volatile", "sfera"],
    "OMBRA": ["spettro", "quadrupede"], "ASTRALE": ["volatile", "spettro", "sfera"],
    "TOSSICO": ["serpente", "sfera"], "PSICHE": ["spettro", "bipede"], "METALLO": ["bipede", "quadrupede"],
}


def scurisci(c, f=0.7):
    return tuple(max(0, int(v * f)) for v in c[:3]) + (255,)


def ellisse(d, cx, cy, rx, ry, colore):
    d.ellipse([cx - rx, cy - ry, cx + rx, cy + ry], fill=colore)


def poligono(d, punti, colore):
    d.polygon([(int(x), int(y)) for x, y in punti], fill=colore)


class Elyon:
    """Parametri deterministici di una specie."""

    def __init__(self, s, stadio, finale, radice):
        self.s = s
        # la forma dipende dalla radice della famiglia: le evoluzioni restano riconoscibili
        self.r = random.Random(radice * 7919 + 13)
        r = self.r
        self.t1, self.t2 = s["t1"], s["t2"]
        pref = PREFERENZE[self.t1]
        self.forma = pref[r.randrange(len(pref))] if r.random() < 0.8 else FORME[r.randrange(len(FORME))]
        leggendario = s["id"] >= 74 and s["id"] not in range(81, 99)
        self.scala = 1.0 if leggendario or s["id"] == 100 else (0.62, 0.8, 0.96)[stadio]
        if finale and stadio == 0:
            self.scala = 0.78       # specie senza evoluzioni: taglia media
        self.pal = PAL[self.t1]
        self.pal2 = PAL[self.t2] if self.t2 else None
        self.tozzo = 0.85 + r.random() * 0.4      # larghezza relativa
        self.testa = 0.8 + r.random() * 0.4       # dimensione della testa
        self.orecchie = r.randrange(4)            # 0 nessuna, 1 punte, 2 tonde, 3 corna
        self.coda = r.randrange(3)
        self.macchie = r.random() < 0.35
        # ogni stadio aggiunge qualcosa: corna/orecchie piu' marcate e coda piu' vistosa
        if stadio >= 1 and self.orecchie == 0:
            self.orecchie = 1 + r.randrange(3)
        if stadio >= 2:
            self.coda = 2 if self.coda == 0 else self.coda
            self.tozzo += 0.1


def disegna_corpo(e, d, vista):
    """Disegna la sagoma (colore base) e ritorna le ancore: testa, coda, schiena."""
    r = random.Random(e.s["id"])
    k = e.scala
    base = e.pal[0]
    cx, suolo = 32, 60
    # in vista frontale il muso guarda a sinistra, da dietro guarda a destra (specchio dopo)
    a = {}
    if e.forma == "quadrupede":
        bw, bh = 15 * k * e.tozzo, 9 * k
        by = suolo - 9 * k - bh
        ellisse(d, cx + 2, by, bw, bh, base)
        for dx in (-bw * 0.6, -bw * 0.15, bw * 0.3, bw * 0.75):
            d.rectangle([cx + dx - 2.5 * k, by, cx + dx + 2.5 * k, suolo - 1], fill=base)
        hr = 8 * k * e.testa
        hx, hy = cx - bw * 0.85, by - bh * 0.9
        ellisse(d, hx, hy, hr, hr * 0.9, base)
        ellisse(d, hx - hr * 0.7, hy + hr * 0.3, hr * 0.55, hr * 0.4, base)   # muso
        a.update(testa=(hx, hy, hr), coda=(cx + bw + 1, by - 2), schiena=(cx + 2, by - bh))
    elif e.forma == "bipede":
        bw, bh = 10 * k * e.tozzo, 13 * k
        by = suolo - 7 * k - bh
        ellisse(d, cx, by, bw, bh, base)
        for dx in (-bw * 0.5, bw * 0.5):
            ellisse(d, cx + dx, suolo - 5 * k, 4 * k, 5 * k, base)
        for dx in (-bw * 1.0, bw * 0.95):
            ellisse(d, cx + dx, by - bh * 0.1, 3 * k, 6 * k, base)                # braccia
        hr = 9 * k * e.testa
        hx, hy = cx - 2 * k, by - bh - hr * 0.5
        ellisse(d, hx, hy, hr, hr * 0.92, base)
        a.update(testa=(hx, hy, hr), coda=(cx + bw, by + bh * 0.5), schiena=(cx, by - bh * 0.6))
    elif e.forma == "serpente":
        punti = []
        for i in range(9):
            t = i / 8
            punti.append((cx + 18 * k - t * 30 * k, suolo - 6 * k - math.sin(t * math.pi * 1.6) * 8 * k - t * 18 * k))
        for i, (x, y) in enumerate(punti):
            rr = (4 + i * 0.9) * k
            ellisse(d, x, y, rr, rr * 0.85, base)
        hx, hy = punti[-1]
        hr = 9 * k * e.testa
        ellisse(d, hx - 2 * k, hy - 4 * k, hr, hr * 0.8, base)
        a.update(testa=(hx - 2 * k, hy - 4 * k, hr), coda=punti[0], schiena=punti[4])
    elif e.forma == "volatile":
        bw, bh = 10 * k * e.tozzo, 11 * k
        by = suolo - 14 * k - bh
        ellisse(d, cx + 2, by, bw, bh, base)
        hr = 8 * k * e.testa
        hx, hy = cx - bw * 0.5, by - bh - hr * 0.3
        ellisse(d, hx, hy, hr, hr * 0.9, base)
        poligono(d, [(hx - hr * 0.9, hy), (hx - hr * 1.8, hy + hr * 0.25), (hx - hr * 0.8, hy + hr * 0.45)], e.pal[3])
        for dx in (-3 * k, 4 * k):
            d.rectangle([cx + dx, by + bh - 1, cx + dx + 2 * k, suolo - 1], fill=e.pal[1])
        a.update(testa=(hx, hy, hr), coda=(cx + bw + 2, by + bh * 0.6), schiena=(cx + 2, by - bh * 0.4), ali=True)
    elif e.forma == "sfera":
        rr = 15 * k * e.tozzo
        by = suolo - rr
        ellisse(d, cx, by, rr, rr * 0.95, base)
        for dx in (-rr * 0.5, rr * 0.5):
            ellisse(d, cx + dx, suolo - 2 * k, 4 * k, 2.5 * k, e.pal[1])
        hr = rr * 0.9
        a.update(testa=(cx - rr * 0.15, by - rr * 0.1, hr), coda=(cx + rr, by + rr * 0.3), schiena=(cx, by - rr), interna=True)
    elif e.forma == "pesce":
        bw, bh = 17 * k * e.tozzo, 10 * k
        by = suolo - 14 * k
        ellisse(d, cx, by, bw, bh, base)
        poligono(d, [(cx + bw - 2, by), (cx + bw + 10 * k, by - 9 * k), (cx + bw + 10 * k, by + 9 * k)], e.pal[1])
        hr = bh
        a.update(testa=(cx - bw * 0.55, by - 1, hr * 0.8), coda=(cx + bw + 6 * k, by), schiena=(cx, by - bh), interna=True)
    else:  # spettro
        bw, bh = 12 * k * e.tozzo, 15 * k
        by = suolo - 12 * k - bh * 0.5
        ellisse(d, cx, by, bw, bh, base)
        poligono(d, [(cx - bw, by), (cx + bw, by), (cx + bw * 0.6, suolo - 2), (cx + bw * 0.1, suolo - 9 * k),
                     (cx - bw * 0.3, suolo - 1), (cx - bw * 0.8, suolo - 7 * k)], base)
        hr = bw
        a.update(testa=(cx - 1, by - bh * 0.35, hr), coda=(cx + bw * 0.6, suolo - 4), schiena=(cx, by - bh), interna=True)
    _ = r
    return a


def tratti_tipo(e, d, a, tipo, pal, secondario):
    """Tratti caratteristici del tipo, sopra o dietro la sagoma."""
    k = e.scala
    hx, hy, hr = a["testa"]
    tx, ty = a["coda"]
    sx, sy = a["schiena"]
    c1, c2, c3 = pal[3], pal[2], pal[1]
    if tipo == "FIAMMA":
        for i, dx in enumerate((-0.4, 0.1, 0.5)):
            h = (7 + 3 * (i == 1)) * k
            poligono(d, [(hx + dx * hr - 3 * k, hy - hr * 0.6), (hx + dx * hr, hy - hr * 0.6 - h), (hx + dx * hr + 3 * k, hy - hr * 0.6)], c1)
        ellisse(d, tx + 3 * k, ty - 3 * k, 4 * k, 5 * k, (255, 140, 40, 255))
        ellisse(d, tx + 3 * k, ty - 2 * k, 2 * k, 3 * k, c1)
    elif tipo == "ACQUA":
        poligono(d, [(sx - 6 * k, sy + 1), (sx + 1 * k, sy - 9 * k), (sx + 6 * k, sy + 1)], c3 if not secondario else c2)
        ellisse(d, hx + hr * 0.75, hy + 1, 3 * k, 5 * k, c2)
    elif tipo == "NATURA":
        poligono(d, [(hx, hy - hr * 0.7), (hx - 7 * k, hy - hr - 8 * k), (hx + 2 * k, hy - hr - 2 * k)], (90, 200, 80, 255))
        poligono(d, [(hx + 1, hy - hr * 0.7), (hx + 8 * k, hy - hr - 6 * k), (hx + 3 * k, hy - hr * 0.6)], (60, 160, 60, 255))
        ellisse(d, sx, sy, 4 * k, 3 * k, c1)
    elif tipo == "IMPULSO":
        x, y = tx, ty
        pts = [(x, y), (x + 5 * k, y - 4 * k), (x + 2 * k, y - 5 * k), (x + 8 * k, y - 11 * k), (x + 3 * k, y - 6 * k), (x + 5 * k, y - 5 * k)]
        poligono(d, pts, c1 if not secondario else pal[0])
        d.line([hx - 2, hy - hr * 0.8, hx - 5 * k, hy - hr - 7 * k], fill=c3, width=max(1, int(k * 1.5)))
        ellisse(d, hx - 5 * k, hy - hr - 7 * k, 2 * k, 2 * k, c1)
    elif tipo == "VENTO" or a.get("ali"):
        col = pal[2] if tipo == "VENTO" else PAL["VENTO"][2]
        poligono(d, [(sx, sy + 3 * k), (sx + 20 * k, sy - 12 * k), (sx + 16 * k, sy + 2 * k), (sx + 22 * k, sy - 2 * k), (sx + 6 * k, sy + 8 * k)], col)
        if tipo == "VENTO":
            for i in range(3):
                d.arc([hx - hr - 6 * k + i * 2, hy - hr - 4 * k - i * 3, hx + 2 * k + i * 2, hy - hr + 4 * k - i * 3], 200, 320, fill=pal[3])
    elif tipo == "ROCCIA":
        for i in range(3):
            x = sx - 8 * k + i * 7 * k
            poligono(d, [(x - 4 * k, sy + 3 * k), (x, sy - 6 * k - i % 2 * 3 * k), (x + 4 * k, sy + 3 * k)], c3)
        d.rectangle([hx - hr * 0.5, hy - hr * 0.9, hx + hr * 0.2, hy - hr * 0.6], fill=pal[2])
    elif tipo == "GELO":
        for i, dx in enumerate((-6, 0, 6)):
            x = sx + dx * k
            poligono(d, [(x - 2.5 * k, sy + 2), (x, sy - (9 + 3 * (i == 1)) * k), (x + 2.5 * k, sy + 2)], (235, 250, 255, 255))
            d.line([x, sy - (8 + 3 * (i == 1)) * k, x, sy], fill=pal[1])
    elif tipo == "OMBRA":
        poligono(d, [(tx, ty), (tx + 12 * k, ty - 6 * k), (tx + 7 * k, ty + 1), (tx + 13 * k, ty + 6 * k)], c3)
        for dx in (-0.6, 0.3):
            poligono(d, [(hx + dx * hr - 2 * k, hy - hr * 0.6), (hx + dx * hr + 1 * k, hy - hr - 6 * k), (hx + dx * hr + 3 * k, hy - hr * 0.5)], c3)
    elif tipo == "ASTRALE":
        x, y, rr = hx + 2 * k, hy - hr - 6 * k, 5 * k
        pts = []
        for i in range(10):
            ang = -math.pi / 2 + i * math.pi / 5
            rad = rr if i % 2 == 0 else rr * 0.45
            pts.append((x + math.cos(ang) * rad, y + math.sin(ang) * rad))
        poligono(d, pts, (255, 236, 120, 255))
        d.arc([hx - hr - 2, hy - hr * 1.3, hx + hr + 2, hy - hr * 0.4], 190, 350, fill=pal[3], width=max(1, int(k)))
    elif tipo == "TOSSICO":
        for i in range(3):
            poligono(d, [(sx - 7 * k + i * 6 * k, sy + 2), (sx - 5 * k + i * 6 * k, sy - 6 * k), (sx - 3 * k + i * 6 * k, sy + 2)], c1)
    elif tipo == "PSICHE":
        ellisse(d, hx - hr * 0.1, hy - hr * 0.55, 2.5 * k, 2.5 * k, pal[3])
        ellisse(d, hx - hr * 0.1, hy - hr * 0.55, 1.2 * k, 1.2 * k, (255, 255, 255, 255))
        d.arc([hx - hr - 4 * k, hy - hr - 4 * k, hx + hr + 4 * k, hy + hr], 200, 340, fill=pal[2], width=max(1, int(k)))
    elif tipo == "METALLO":
        poligono(d, [(hx - hr * 0.3, hy - hr * 0.7), (hx - hr * 0.9, hy - hr - 8 * k), (hx + hr * 0.1, hy - hr * 0.8)], pal[2])
        d.rectangle([sx - 6 * k, sy, sx + 6 * k, sy + 3 * k], fill=pal[1])


def tratti_generici(e, d, a):
    k = e.scala
    hx, hy, hr = a["testa"]
    tx, ty = a["coda"]
    if a.get("interna"):
        return
    if e.orecchie == 1:
        for dx in (-0.5, 0.35):
            poligono(d, [(hx + dx * hr - 3 * k, hy - hr * 0.5), (hx + dx * hr, hy - hr - 4 * k), (hx + dx * hr + 3 * k, hy - hr * 0.55)], e.pal[0])
    elif e.orecchie == 2:
        for dx in (-0.55, 0.45):
            ellisse(d, hx + dx * hr, hy - hr * 0.75, 3 * k, 3.5 * k, e.pal[0])
    elif e.orecchie == 3:
        for dx in (-0.4, 0.3):
            poligono(d, [(hx + dx * hr - 1.5 * k, hy - hr * 0.7), (hx + dx * hr + 3 * k, hy - hr - 6 * k), (hx + dx * hr + 1.5 * k, hy - hr * 0.6)], e.pal[2])
    if e.coda == 1:
        d.line([tx, ty, tx + 8 * k, ty - 7 * k], fill=e.pal[0], width=max(2, int(3 * k)))
    elif e.coda == 2:
        ellisse(d, tx + 4 * k, ty - 3 * k, 5 * k, 4 * k, e.pal[0])


def ombreggia(im, pal):
    """Luce dall'alto a sinistra: bordo scuro in basso/destra, riflesso in alto/sinistra, contorno."""
    w, h = im.size
    px = im.load()
    pieno = [[px[x, y][3] > 0 for y in range(h)] for x in range(w)]
    out = im.copy()
    po = out.load()
    for x in range(w):
        for y in range(h):
            if not pieno[x][y]:
                continue
            c = px[x, y]
            sotto = y + 2 < h and not pieno[x][y + 2]
            destra = x + 2 < w and not pieno[x + 2][y]
            sopra = y - 2 >= 0 and not pieno[x][y - 2]
            sinistra = x - 2 >= 0 and not pieno[x - 2][y]
            if sotto or destra:
                po[x, y] = scurisci(c, 0.72)
            elif (sopra or sinistra) and (x + y) % 2 == 0:
                po[x, y] = tuple(min(255, int(v * 1.18 + 20)) for v in c[:3]) + (255,)
    contorno = scurisci(pal[1], 0.55)
    for x in range(w):
        for y in range(h):
            if pieno[x][y]:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < w and 0 <= ny < h and pieno[nx][ny]:
                    po[x, y] = contorno
                    break
    return out


def occhi(e, d, a):
    k = e.scala
    hx, hy, hr = a["testa"]
    nero = (24, 20, 32, 255)
    bianco = (255, 255, 255, 255)
    iride = e.pal[3] if e.t1 in ("OMBRA", "PSICHE", "ASTRALE") else nero
    r = max(1.5, 2.2 * k)
    for i, dx in enumerate((-0.55, 0.05)):
        x, y = hx + dx * hr, hy - hr * 0.05
        rr = r * (0.8 if i == 0 else 1.0)     # occhio lontano piu' piccolo (3/4)
        ellisse(d, x, y, rr, rr * 1.15, bianco)
        ellisse(d, x - rr * 0.25, y + rr * 0.1, rr * 0.6, rr * 0.8, iride)
        d.point((int(x - rr * 0.4), int(y - rr * 0.4)), fill=bianco)
    if not a.get("interna") or e.forma == "sfera":
        d.line([hx - hr * 0.55, hy + hr * 0.45, hx - hr * 0.2, hy + hr * 0.5], fill=scurisci(e.pal[1], 0.6))


def macchie(e, d, a):
    if not e.macchie:
        return
    r = random.Random(e.s["id"] * 31)
    sx, sy = a["schiena"]
    col = (e.pal2 or e.pal)[2 if e.pal2 else 1]
    for _ in range(4):
        ellisse(d, sx + r.uniform(-10, 10) * e.scala, sy + r.uniform(3, 12) * e.scala, 1.8 * e.scala, 1.4 * e.scala, col)


def sprite(e, vista):
    im = Image.new("RGBA", (S, S), TR)
    d = ImageDraw.Draw(im)
    dietro = Image.new("RGBA", (S, S), TR)
    dd = ImageDraw.Draw(dietro)
    a = disegna_corpo(e, ImageDraw.Draw(Image.new("RGBA", (S, S))), vista)   # solo per le ancore
    # tratti che stanno dietro al corpo (ali, code speciali)
    if a.get("ali") or e.t1 == "VENTO" or e.t2 == "VENTO":
        tratti_tipo(e, dd, a, "VENTO", PAL["VENTO"], e.t1 != "VENTO")
    a = disegna_corpo(e, d, vista)
    tratti_generici(e, d, a)
    macchie(e, d, a)
    for tipo, pal, sec in ((e.t1, e.pal, False), (e.t2, e.pal2, True)):
        if tipo and tipo != "VENTO":
            tratti_tipo(e, d, a, tipo, pal, sec)
    dietro.alpha_composite(im)
    im = ombreggia(dietro, e.pal)
    d = ImageDraw.Draw(im)
    if vista == "fronte":
        occhi(e, d, a)
    else:
        # da dietro: niente viso, una fascia scura sulla nuca e una striscia sulla schiena
        hx, hy, hr = a["testa"]
        sx, sy = a["schiena"]
        d.arc([hx - hr * 0.8, hy - hr * 0.6, hx + hr * 0.8, hy + hr * 0.9], 200, 340, fill=scurisci(e.pal[1], 0.8), width=max(1, int(e.scala * 1.5)))
        d.line([sx - 5 * e.scala, sy + 3, sx + 5 * e.scala, sy + 3], fill=scurisci(e.pal[0], 0.8))
        im = im.transpose(Image.FLIP_LEFT_RIGHT)
    return im


def main():
    specie = db.carica_specie()
    evo = db.carica_evoluzioni(specie)
    pre = {a: da for da, a, _ in evo}
    evolve = {da for da, _, _ in evo}
    os.makedirs(RADICE, exist_ok=True)
    fronte = Image.new("RGBA", (S * 10, S * 10), TR)
    retro = Image.new("RGBA", (S * 10, S * 10), TR)
    for sid in range(1, 101):
        stadio, x = 0, sid
        while x in pre:
            stadio += 1
            x = pre[x]
        finale = sid not in evolve and stadio == 0
        e = Elyon(specie[sid], min(stadio, 2), finale, x)
        gx, gy = (sid - 1) % 10 * S, (sid - 1) // 10 * S
        fronte.paste(sprite(e, "fronte"), (gx, gy))
        retro.paste(sprite(e, "retro"), (gx, gy))
    fronte.save(os.path.join(RADICE, "elyon_fronte.png"))
    retro.save(os.path.join(RADICE, "elyon_retro.png"))
    print("scritti elyon_fronte.png e elyon_retro.png (640x640, griglia 10x10 di 64x64)")
    if "--anteprima" in sys.argv:
        out = sys.argv[sys.argv.index("--anteprima") + 1]
        prev = Image.new("RGBA", (S * 20, S * 10), (40, 44, 60, 255))
        prev.alpha_composite(fronte, (0, 0))
        prev.alpha_composite(retro, (S * 10, 0))
        prev.resize((prev.width * 2, prev.height * 2), Image.NEAREST).save(out)
        print("anteprima:", out)


if __name__ == "__main__":
    main()
