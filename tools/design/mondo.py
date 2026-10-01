"""
ECLYRA - Tile, edifici, personaggi e scenari (Design System v1).

Regole: tile 16x16, luce dall'alto a sinistra, ombre portate verso il basso a destra
(1 tile di lunghezza massima), contorni scuri colorati (mai nero puro), 3-5 toni per materiale.
Le transizioni (sentiero/erba, riva/acqua) sono autotile a 4 vicini: ogni bordo e' irregolare
ma deterministico (dipende dalla posizione), cosi' il motore C potra' ricostruirle uguali.
"""
import math
import random

from PIL import Image

import palette as P
import ui

T = 16
px, rect, hline, vline = ui.px, ui.rect, ui.hline, ui.vline


def rnd(*seme):
    """RNG deterministico (stesso risultato a ogni esecuzione, a differenza di hash())"""
    return random.Random("|".join(str(v) for v in seme))


# ================================================================ terreni

def erba(im, x, y, seme=0):
    r = rnd("erba", x // T, y // T, seme)
    rect(im, x, y, T, T, P.ERBA[3])
    for _ in range(10):
        bx, by = x + r.randrange(T), y + r.randrange(T)
        px(im, bx, by, P.ERBA[2])
        px(im, bx, by - 1, P.ERBA[4]) if r.random() < 0.5 else None
    for _ in range(3):   # ciuffi
        bx, by = x + r.randrange(1, T - 2), y + r.randrange(2, T)
        px(im, bx, by, P.ERBA[2]); px(im, bx + 1, by - 1, P.ERBA[4]); px(im, bx + 2, by, P.ERBA[2])


def erba_alta(im, x, y, frame=0):
    rect(im, x, y, T, T, P.ERBA[2])
    for col in range(0, T, 4):
        for riga in (0, 8):
            bx, by = x + col + (riga // 8) * 2, y + riga + 7
            sway = (frame + col) % 3 - 1 if frame else 0
            for k, (dx, h) in enumerate(((0, 6), (2, 8), (4, 6))):
                for j in range(h):
                    c = P.ERBA[1] if j < 2 else P.ERBA[3] if j < h - 2 else P.ERBA[4]
                    px(im, bx + dx + (sway if j > h - 3 else 0), by - j, c)
            px(im, bx + 2, by - 8, P.ERBA[5])
            hline(im, bx - 1, by + 1, 6, P.ERBA[0])


def fiori(im, x, y, seme=0):
    erba(im, x, y, seme)
    r = rnd("fiori", x // T, y // T, seme)
    for _ in range(4):
        fx, fy = x + r.randrange(2, T - 3), y + r.randrange(3, T - 2)
        c = P.FIORI[r.randrange(len(P.FIORI))]
        px(im, fx, fy + 1, P.ERBA[1])
        for dx, dy in ((0, -1), (-1, 0), (1, 0), (0, 1)):
            px(im, fx + dx, fy + dy, c)
        px(im, fx, fy, P.ORO[4])


def sentiero(im, x, y, vicini):
    """vicini: (su, giu, sinistra, destra) = True se anche li' c'e' sentiero"""
    r = rnd("sent", x // T, y // T)
    rect(im, x, y, T, T, P.TERRA[3])
    for _ in range(9):
        sx, sy = x + r.randrange(T), y + r.randrange(T)
        px(im, sx, sy, P.TERRA[2] if r.random() < 0.6 else P.TERRA[4])
    for _ in range(2):   # sassolini
        sx, sy = x + r.randrange(1, T - 2), y + r.randrange(1, T - 2)
        px(im, sx, sy, P.PIETRA[3]); px(im, sx + 1, sy, P.PIETRA[4]); px(im, sx, sy + 1, P.PIETRA[2])
    su, giu, sx_, dx_ = vicini
    for i in range(T):
        o = rnd("bordo", x // T, y // T, i)
        if not su:
            d = 1 + o.randrange(2)
            for k in range(d):
                px(im, x + i, y + k, P.ERBA[3])
            px(im, x + i, y + d, P.ERBA[1]) if i % 3 else None
        if not giu:
            d = 1 + o.randrange(2)
            for k in range(d):
                px(im, x + i, y + T - 1 - k, P.ERBA[3])
            px(im, x + i, y + T - 1 - d, P.TERRA[2])
        if not sx_:
            d = 1 + o.randrange(2)
            for k in range(d):
                px(im, x + k, y + i, P.ERBA[3])
            px(im, x + d, y + i, P.TERRA[2])
        if not dx_:
            d = 1 + o.randrange(2)
            for k in range(d):
                px(im, x + T - 1 - k, y + i, P.ERBA[3])
            px(im, x + T - 1 - d, y + i, P.TERRA[2])


def lastricato(im, x, y):
    """Piazza di Lunaria: pietre chiare a corsi sfalsati"""
    rect(im, x, y, T, T, P.PIETRA[3])
    for riga in range(0, T, 4):
        off = 4 if (y // 4 + riga // 4) % 2 else 0
        hline(im, x, y + riga, T, P.PIETRA[2])
        for c in range(-off, T, 8):
            vline(im, x + c + off, y + riga, 4, P.PIETRA[2])
            if 0 <= c + off + 1 < T:
                hline(im, x + max(0, c + off + 1), y + riga + 1, 6, P.PIETRA[4])


def acqua(im, x, y, vicini, frame=0):
    su, giu, sx_, dx_ = vicini
    rect(im, x, y, T, T, P.ACQUA[2])
    r = rnd("acqua", x // T, y // T)
    for _ in range(3):
        wx, wy = x + r.randrange(T - 4), y + r.randrange(T)
        sh = (frame + wy) % 4
        hline(im, wx + sh % 2, wy, 3, P.ACQUA[3])
        px(im, wx + 1 + sh % 2, wy - 1, P.ACQUA[4]) if r.random() < 0.3 else None
    for i in range(T):
        if not su:
            px(im, x + i, y, P.ACQUA[5]); px(im, x + i, y + 1, P.ACQUA[4] if i % 4 else P.ACQUA[5])
            px(im, x + i, y + 2, P.ACQUA[1])
        if not sx_:
            px(im, x, y + i, P.ACQUA[4]); px(im, x + 1, y + i, P.ACQUA[3])
        if not dx_:
            px(im, x + T - 1, y + i, P.ACQUA[1])
        if not giu:
            px(im, x + i, y + T - 1, P.ACQUA[3])


def riva(im, x, y, lato):
    """bordo di terra sopra l'acqua (piccolo dislivello)"""
    for i in range(T):
        vline(im, x + i, y + T - 3, 3, P.TERRA[1])
        px(im, x + i, y + T - 3, P.TERRA[2])


def ponte(im, x, y, orizzontale=False):
    """Ponte di legno: assi con chiodi d'ottone e corrimano"""
    if orizzontale:
        for i in range(0, T, 4):
            rect(im, x + i, y + 2, 3, 12, P.LEGNO[3])
            vline(im, x + i, y + 2, 12, P.LEGNO[4])
            vline(im, x + i + 3, y + 2, 12, P.LEGNO[1])
            px(im, x + i + 1, y + 4, P.ORO[3]); px(im, x + i + 1, y + 11, P.ORO[3])
        hline(im, x, y + 1, T, P.LEGNO[1]); hline(im, x, y + 14, T, P.LEGNO[0])
        hline(im, x, y, T, P.LEGNO[3])
    else:
        for j in range(0, T, 4):
            rect(im, x + 2, y + j, 12, 3, P.LEGNO[3])
            hline(im, x + 2, y + j, 12, P.LEGNO[4])
            hline(im, x + 2, y + j + 3, 12, P.LEGNO[1])
            px(im, x + 4, y + j + 1, P.ORO[3]); px(im, x + 11, y + j + 1, P.ORO[3])
        vline(im, x + 1, y, T, P.LEGNO[1]); vline(im, x + 14, y, T, P.LEGNO[0])
        vline(im, x, y, T, P.LEGNO[2]); vline(im, x + 15, y, T, P.LEGNO[0])


# ================================================================ oggetti

def ombra_ellisse(im, cx, cy, rx, ry, a=80):
    for yy in range(int(cy - ry), int(cy + ry) + 1):
        for xx in range(int(cx - rx), int(cx + rx) + 1):
            if ((xx - cx) / rx) ** 2 + ((yy - cy) / ry) ** 2 <= 1:
                px(im, xx, yy, (8, 14, 30, a))


def albero(im, x, y, seme=0, specie="quercia"):
    """Albero 32x40 (x, y = angolo in alto a sinistra; base del tronco a y+38)"""
    r = rnd("albero", x, y, seme)
    ombra_ellisse(im, x + 18, y + 38, 13, 4, 90)
    rect(im, x + 13, y + 26, 6, 12, P.LEGNO[2])
    vline(im, x + 13, y + 26, 12, P.LEGNO[3]); vline(im, x + 18, y + 26, 12, P.LEGNO[1])
    px(im, x + 12, y + 37, P.LEGNO[1]); px(im, x + 19, y + 37, P.LEGNO[1])
    verdi = P.ERBA if specie == "quercia" else [P.H("0E2E2A"), P.H("164A3E"), P.H("1E6A52"), P.H("2F8A62"), P.H("5AB27A"), P.H("A2DE9A")]
    # chioma: gruppi di sfere con ombreggiatura propria (luce dall'alto a sinistra)
    gruppi = [(16, 15, 13, 11), (8, 19, 8, 7), (24, 19, 8, 7), (12, 9, 8, 7), (21, 9, 8, 7), (16, 22, 10, 6)]
    canopy = Image.new("RGBA", (34, 34), (0, 0, 0, 0))
    for (cx, cy, rx, ry) in gruppi:
        for yy in range(-ry, ry + 1):
            for xx in range(-rx, rx + 1):
                d = (xx / rx) ** 2 + (yy / ry) ** 2
                if d <= 1:
                    l = -(xx / rx) * 0.55 - (yy / ry) * 0.7 + math.sqrt(max(0, 1 - d)) * 0.6
                    idx = 4 if l > 0.85 else 3 if l > 0.35 else 2 if l > -0.2 else 1
                    canopy.putpixel((cx + xx, cy + yy), verdi[idx])
    # foglie a ciuffi sul bordo e riflessi
    for _ in range(26):
        fx, fy = r.randrange(2, 31), r.randrange(1, 30)
        if canopy.getpixel((fx, fy))[3]:
            c = canopy.getpixel((fx, fy))
            canopy.putpixel((fx, fy), verdi[min(5, verdi.index(c) + 1)] if c in verdi else c)
    # contorno
    a = canopy.load()
    out = canopy.copy()
    for yy in range(34):
        for xx in range(34):
            if a[xx, yy][3] == 0:
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = xx + dx, yy + dy
                    if 0 <= nx < 34 and 0 <= ny < 34 and a[nx, ny][3]:
                        out.putpixel((xx, yy), verdi[0])
                        break
    im.alpha_composite(out, (x - 1, y - 1))
    if specie == "lumen":   # albero sacro: frutti di Lumen che brillano
        for (fx, fy) in ((9, 17), (22, 12), (17, 22)):
            ui.bagliore(im, x + fx, y + fy, 5, P.LUMEN[3], 70)
            px(im, x + fx, y + fy, P.LUMEN[4])


def roccia(im, x, y, grande=False):
    w, h = (14, 10) if grande else (9, 6)
    ombra_ellisse(im, x + w / 2 + 1, y + h, w / 2 + 1, 2, 70)
    for yy in range(h):
        for xx in range(w):
            d = ((xx - w / 2 + 0.5) / (w / 2)) ** 2 + ((yy - h + 0.5) / h) ** 2
            if d <= 1:
                l = -(xx / w) - (yy / h) * 0.8
                c = P.PIETRA[4] if l > -0.5 else P.PIETRA[3] if l > -1.0 else P.PIETRA[2]
                px(im, x + xx, y + yy, c)
    px(im, x + w // 3, y + 1, P.PIETRA[5])


def recinto(im, x, y, w):
    for i in range(0, w, 8):
        rect(im, x + i, y, 3, 11, P.LEGNO[3]); vline(im, x + i, y, 11, P.LEGNO[4]); vline(im, x + i + 2, y, 11, P.LEGNO[1])
        px(im, x + i + 1, y - 1, P.LEGNO[3])
    hline(im, x, y + 3, w, P.LEGNO[3]); hline(im, x, y + 4, w, P.LEGNO[1])
    hline(im, x, y + 7, w, P.LEGNO[3]); hline(im, x, y + 8, w, P.LEGNO[1])


def lampione(im, x, y, frame=0):
    """Lampione di Lunaria: palo di ferro scuro, gabbia d'oro, cristallo di Lumen acceso"""
    ui.bagliore(im, x + 3, y + 3, 14, P.LUMEN[3], 55 + 10 * (frame % 2))
    vline(im, x + 3, y + 8, 18, P.NOTTE[1]); vline(im, x + 2, y + 8, 18, P.NOTTE[2])
    rect(im, x + 1, y + 25, 5, 2, P.NOTTE[1])
    rect(im, x, y + 1, 7, 1, P.ORO[2]); rect(im, x, y + 7, 7, 1, P.ORO[1])
    vline(im, x, y + 1, 7, P.ORO[2]); vline(im, x + 6, y + 1, 7, P.ORO[1])
    ui.cristallo(im, x + 3, y + 1, 7)
    px(im, x + 3, y, P.ORO[4])


def cartello(im, x, y):
    ombra_ellisse(im, x + 8, y + 15, 6, 2)
    vline(im, x + 7, y + 8, 7, P.LEGNO[1]); vline(im, x + 8, y + 8, 7, P.LEGNO[2])
    rect(im, x + 1, y + 1, 14, 8, P.LEGNO[3])
    hline(im, x + 1, y + 1, 14, P.LEGNO[4]); hline(im, x + 1, y + 8, 14, P.LEGNO[1])
    vline(im, x + 1, y + 1, 8, P.LEGNO[4]); vline(im, x + 14, y + 1, 8, P.LEGNO[1])
    for k in range(3):
        hline(im, x + 4, y + 3 + k * 2, 8 - k * 2, P.LEGNO[1])


def cespuglio(im, x, y):
    ombra_ellisse(im, x + 8, y + 13, 7, 2)
    for (cx, cy, r) in ((5, 9, 5), (11, 9, 5), (8, 6, 5)):
        for yy in range(-r, r + 1):
            for xx in range(-r, r + 1):
                if xx * xx + yy * yy <= r * r:
                    l = -xx - yy
                    px(im, x + cx + xx, y + cy + yy, P.ERBA[4] if l > 3 else P.ERBA[3] if l > -2 else P.ERBA[2])
    px(im, x + 6, y + 4, P.ERBA[5])


# ================================================================ edifici

def casa(im, x, y, w=64, tetto=None, insegna=None, finestre_accese=True):
    """Casa di Lunaria: intonaco chiaro, tetto in ardesia blu con colmo d'oro e pinnacolo di cristallo.
    (x, y) = angolo in alto a sinistra del tetto; altezza totale 60."""
    tetto = tetto or P.TETTO_BLU
    ombra_ellisse(im, x + w / 2 + 4, y + 60, w / 2 + 2, 4, 80)
    # muri
    rect(im, x + 4, y + 26, w - 8, 32, P.INTONACO[2])
    for j in range(26, 58, 6):
        hline(im, x + 4, y + j, w - 8, P.INTONACO[1]) if j > 26 else None
    vline(im, x + 4, y + 26, 32, P.INTONACO[3]); vline(im, x + w - 5, y + 26, 32, P.INTONACO[0])
    hline(im, x + 4, y + 57, w - 8, P.INTONACO[0])
    # travi di legno
    for tx in (x + 4, x + w - 7):
        rect(im, tx, y + 26, 3, 32, P.LEGNO[2]); vline(im, tx, y + 26, 32, P.LEGNO[3])
    hline(im, x + 4, y + 40, w - 8, P.LEGNO[1]); hline(im, x + 4, y + 41, w - 8, P.LEGNO[2])
    # tetto spiovente a righe di tegole
    for j in range(28):
        larg = w // 2 + 6 - (28 - j) * (w // 2 + 6) // 30
        cx = x + w // 2
        sx = cx - larg - 2
        for i in range(2 * larg + 4):
            fila = (j // 3) % 2
            c = tetto[3] if (i + fila * 2) % 4 == 0 and j % 3 == 0 else tetto[2]
            if j % 3 == 2:
                c = tetto[1]
            if i < 3:
                c = tetto[3]
            px(im, sx + i, y + j, c)
        px(im, sx - 1, y + j, P.mix(tetto[0], (0, 0, 0, 255), 0.4))
        px(im, sx + 2 * larg + 4, y + j, P.mix(tetto[0], (0, 0, 0, 255), 0.4))
    hline(im, x - 4, y + 28, w + 8, tetto[0])
    hline(im, x - 3, y + 27, w + 6, tetto[1])
    # colmo d'oro e pinnacolo
    cx = x + w // 2
    for j in range(0, 4):
        hline(im, cx - 3 + j, y + j - 1, 7 - 2 * j, P.ORO[3] if j < 2 else P.ORO[2])
    vline(im, cx, y - 8, 7, P.ORO[2])
    ui.cristallo(im, cx, y - 14, 7, bagliore_r=8)
    # porta
    dx = x + w // 2 - 6
    rect(im, dx, y + 42, 12, 16, P.LEGNO[1])
    rect(im, dx + 1, y + 43, 10, 15, P.LEGNO[2])
    vline(im, dx + 6, y + 43, 15, P.LEGNO[1])
    hline(im, dx + 1, y + 43, 10, P.LEGNO[3])
    px(im, dx + 8, y + 51, P.ORO[4])
    rect(im, dx - 1, y + 40, 14, 2, P.PIETRA[3])
    # finestre con luce calda e fioriere
    for fx in (x + 10, x + w - 22):
        rect(im, fx, y + 30, 12, 9, P.LEGNO[1])
        luce = (P.H("FFD98A") if finestre_accese else P.ACQUA[1])
        rect(im, fx + 1, y + 31, 10, 7, luce)
        hline(im, fx + 1, y + 31, 10, P.H("FFF2C8") if finestre_accese else P.ACQUA[3])
        vline(im, fx + 6, y + 31, 7, P.LEGNO[1]); hline(im, fx + 1, y + 34, 10, P.LEGNO[1])
        rect(im, fx - 1, y + 39, 14, 3, P.LEGNO[2])
        for k in range(0, 12, 3):
            px(im, fx + k, y + 38, P.FIORI[k % 3]); px(im, fx + k + 1, y + 37, P.ERBA[4])
        if finestre_accese:
            ui.bagliore(im, fx + 6, y + 34, 9, P.H("FFC860"), 35)
    if insegna:
        rect(im, x + w // 2 - 10, y + 22, 20, 7, P.NOTTE[1])
        hline(im, x + w // 2 - 10, y + 22, 20, P.ORO[3])
        ui.cristallo(im, x + w // 2, y + 22, 7)


def laboratorio(im, x, y, w=96):
    """Laboratorio di Elian: pietra chiara, cupola di vetro con un grande cristallo di Lumen, antenna"""
    ombra_ellisse(im, x + w / 2 + 5, y + 74, w / 2 + 3, 5, 90)
    rect(im, x + 2, y + 32, w - 4, 40, P.INTONACO[3])
    for j in range(32, 72, 5):
        off = 0 if (j // 5) % 2 else 6
        hline(im, x + 2, y + j, w - 4, P.INTONACO[1])
        for i in range(off, w - 4, 12):
            vline(im, x + 2 + i, y + j, 5, P.INTONACO[1])
    vline(im, x + 2, y + 32, 40, P.INTONACO[3]); vline(im, x + w - 3, y + 32, 40, P.INTONACO[0])
    rect(im, x - 1, y + 28, w + 2, 5, P.TETTO_BLU[1]); hline(im, x - 1, y + 28, w + 2, P.TETTO_BLU[3])
    hline(im, x - 1, y + 32, w + 2, P.TETTO_BLU[0])
    hline(im, x - 1, y + 27, w + 2, P.ORO[3])
    # cupola
    cx, cy, R = x + w // 2, y + 28, 22
    for yy in range(-R, 1):
        for xx in range(-R, R + 1):
            if xx * xx + yy * yy <= R * R:
                l = -xx / R * 0.6 - yy / R * 0.2
                c = P.AZZURRO[3] if l > 0.35 else P.AZZURRO[2] if l > -0.1 else P.AZZURRO[1]
                if (xx + 40) % 8 == 0 or yy % 7 == 0:
                    c = P.ORO[2]
                px(im, cx + xx, cy + yy, c)
    for k in range(-R, R + 1):
        yy = -int(math.sqrt(max(0, R * R - k * k)))
        px(im, cx + k, cy + yy - 1, P.ORO[1])
    ui.bagliore(im, cx, cy - 10, 18, P.LUMEN[3], 80)
    ui.cristallo(im, cx, cy - 20, 15)
    vline(im, cx + 14, y - 6, 18, P.PIETRA[2]); px(im, cx + 14, y - 7, P.LUMEN[4])
    ui.bagliore(im, cx + 14, y - 7, 5, P.LUMEN[3], 90)
    # porta doppia e vetrate
    dx = x + w // 2 - 10
    rect(im, dx, y + 50, 20, 22, P.NOTTE[1]); rect(im, dx + 1, y + 51, 18, 21, P.AZZURRO[1])
    vline(im, dx + 10, y + 51, 21, P.ORO[2]); hline(im, dx + 1, y + 51, 18, P.AZZURRO[3])
    rect(im, dx - 2, y + 47, 24, 3, P.ORO[2]); hline(im, dx - 2, y + 47, 24, P.ORO[4])
    ui.cristallo(im, dx + 10, y + 41, 7)
    for fx in (x + 10, x + 30, x + w - 42, x + w - 22):
        rect(im, fx, y + 38, 12, 14, P.NOTTE[1]); rect(im, fx + 1, y + 39, 10, 12, P.H("9FE8F2"))
        hline(im, fx + 1, y + 39, 10, P.H("E2FFFF")); hline(im, fx + 1, y + 45, 10, P.NOTTE[2])
        ui.bagliore(im, fx + 6, y + 45, 9, P.LUMEN[3], 30)
    rect(im, x - 2, y + 72, w + 4, 3, P.PIETRA[3]); hline(im, x - 2, y + 72, w + 4, P.PIETRA[4])


# ================================================================ personaggi 16x24

_PERS_META = [
    "......oo", "....oohh", "...ohhHH", "..ohhHHh", "..ohhhhh", "..ohgggg", "..ohgLLg", "..ohssss",
    "..ohsess", "..ohsess", "...ossrs", "....otTT", "...otTTt", "..ojjjjg", ".ojjjjjg", ".osjjjjg",
    "..ojjjjg", "..okkkkG", "..oppppp", "..opppo.", "..opppo.", "..obbbo.", ".obbbbo.", ".oooooo.",
]


def personaggio(im, x, y, abito, frame=0):
    """Personaggio 16x24 visto di fronte. abito: dict con capelli, pelle, giacca, sciarpa, pantaloni, stivali."""
    col = {
        "o": P.H("140F26"), "h": abito["capelli"][1], "H": abito["capelli"][2],
        "s": P.PELLE[2], "e": P.H("140F26"), "r": P.H("E8907A"),
        "g": abito.get("fascia", P.ORO)[2], "L": P.LUMEN[3], "G": P.ORO[4],
        "t": abito["sciarpa"][1], "T": abito["sciarpa"][2],
        "j": abito["giacca"][1], "k": P.LEGNO[1], "p": abito["pantaloni"], "b": abito["stivali"],
    }
    ombra_ellisse(im, x + 8, y + 23, 6, 2, 90)
    for j, meta in enumerate(_PERS_META):
        riga = meta + meta[::-1]
        if frame and j >= 19:   # passo: una gamba avanti
            riga = riga[1:] + "." if j % 2 else "." + riga[:-1]
        for i, ch in enumerate(riga):
            if ch in col:
                c = col[ch]
                if ch == "j" and i in (2, 3, 12, 13):
                    c = abito["giacca"][0]
                if ch == "j" and i in (5, 6):
                    c = abito["giacca"][2]
                if ch == "s" and j < 11 and i >= 11:
                    c = P.PELLE[1]
                px(im, x + i, y + j, c)
    # dettagli asimmetrici: coda della sciarpa e borsa
    for j in range(11, 17):
        px(im, x + 12 + (j - 11) // 3, y + j, abito["sciarpa"][1])
    rect(im, x + 1, y + 15, 3, 3, P.LEGNO[2]); px(im, x + 1, y + 15, P.LEGNO[3])
    if "extra" in abito:
        abito["extra"](im, x, y)


ABITO_GIOCATORE = {"capelli": P.TERRA, "giacca": P.NOTTE[2:5], "sciarpa": P.LUMEN[1:4],
                   "pantaloni": P.H("2A2E48"), "stivali": P.LEGNO[2]}
ABITO_KAEL = {"capelli": [P.H("1A1420"), P.H("2E2438"), P.H("4A3A58")], "giacca": [P.H("6A2A18"), P.H("A8462A"), P.H("D2703E")],
              "sciarpa": P.ORO[1:4], "pantaloni": P.H("3A2E2A"), "stivali": P.LEGNO[1], "fascia": P.NOTTE}
ABITO_ELIAN = {"capelli": [P.H("8A8A9A"), P.H("B8BAC8"), P.H("E2E4EE")], "giacca": [P.H("A8AEC0"), P.H("DCE0EA"), P.H("F6F8FC")],
               "sciarpa": P.LUMEN[1:4], "pantaloni": P.H("3A3E58"), "stivali": P.LEGNO[1], "fascia": P.NOTTE}
ABITO_ABITANTE = {"capelli": [P.H("5A2A10"), P.H("A0522A"), P.H("D2864A")], "giacca": [P.H("2A6A3A"), P.H("3E9A50"), P.H("7ACB78")],
                  "sciarpa": P.ORO[1:4], "pantaloni": P.H("4A3A2A"), "stivali": P.LEGNO[1], "fascia": P.LEGNO}
ABITO_LIRA = {"capelli": [P.H("1E5A2A"), P.H("2E8A3E"), P.H("6AC85A")], "giacca": [P.H("2A4A2A"), P.H("4A7A3A"), P.H("8AB85A")],
              "sciarpa": P.ORO[2:5], "pantaloni": P.H("2E3A2A"), "stivali": P.LEGNO[2], "fascia": P.ORO}
ABITO_VAEL = {"capelli": [P.H("D8D8E8"), P.H("EAEAF6"), P.H("FFFFFF")], "giacca": [P.H("0E0A18"), P.H("1E1630"), P.H("3A2A58")],
              "sciarpa": [P.H("4A0E1A"), P.H("8A1E2E"), P.H("C8304A")], "pantaloni": P.H("0E0A18"), "stivali": P.H("1E1630"), "fascia": P.NOTTE}
