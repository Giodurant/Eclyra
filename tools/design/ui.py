"""
ECLYRA - Componenti UI (Design System v1): cornici, pergamena, cursore cristallo, barre,
badge dei tipi, pulsanti, targhe HUD. Tutto in pixel art a 1x (risoluzione logica 320x180).
"""
import math
import random

from PIL import Image, ImageDraw

import font as F
import palette as P

# ---------------------------------------------------------------- primitive


def px(im, x, y, c):
    if 0 <= x < im.width and 0 <= y < im.height and c[3] > 0:
        if c[3] == 255:
            im.putpixel((x, y), c)
        else:
            base = im.getpixel((x, y))
            a = c[3] / 255
            im.putpixel((x, y), tuple(int(base[i] * (1 - a) + c[i] * a) for i in range(3)) + (max(base[3], c[3]),))


def rect(im, x, y, w, h, c):
    for j in range(y, y + h):
        for i in range(x, x + w):
            px(im, i, j, c)


def hline(im, x, y, w, c):
    rect(im, x, y, w, 1, c)


def vline(im, x, y, h, c):
    rect(im, x, y, 1, h, c)


def gradiente_v(im, x, y, w, h, c1, c2, dither=True):
    for j in range(h):
        t = j / max(1, h - 1)
        for i in range(w):
            tt = t
            if dither:   # dithering ordinato 2x2: transizioni morbide senza nuovi colori
                tt += ((i + j) % 2 - 0.5) * (1.0 / max(4, h))
            px(im, x + i, y + j, P.mix(c1, c2, min(1, max(0, tt))))


def bagliore(im, cx, cy, r, colore, forza=120):
    """Alone morbido (additivo) attorno a un punto: per cristalli, effetti Lumen, luci."""
    for y in range(int(cy - r), int(cy + r) + 1):
        for x in range(int(cx - r), int(cx + r) + 1):
            d = math.hypot(x - cx, y - cy) / r
            if d < 1 and 0 <= x < im.width and 0 <= y < im.height:
                a = (1 - d) ** 2 * forza / 255
                b = im.getpixel((x, y))
                im.putpixel((x, y), tuple(min(255, int(b[i] + colore[i] * a)) for i in range(3)) + (max(b[3], int(255 * a)) if b[3] < 255 else 255,))


# ---------------------------------------------------------------- cristallo Lumen

def cristallo(im, x, y, h=7, pal=None, bagliore_r=0):
    """Cristallo di Lumen a rombo allungato (firma visiva di ECLYRA). (x, y) = punta in alto."""
    pal = pal or P.LUMEN
    w = max(3, h // 2 + (1 if h % 2 == 0 else 0))
    meta = h // 2
    if bagliore_r:
        bagliore(im, x, y + meta, bagliore_r, pal[3], 90)
    for j in range(h):
        larg = (j if j <= meta else h - 1 - j) * w // max(1, meta) // 2
        for i in range(-larg, larg + 1):
            if i < 0:
                c = pal[3] if j < meta else pal[2]
            elif i == 0:
                c = pal[4] if j < meta else pal[3]
            else:
                c = pal[2] if j < meta else pal[1]
            px(im, x + i, y + j, c)
    px(im, x, y, pal[4])


def gemma(im, cx, cy, pal=None):
    """Gemma 5x5 a rombo per gli angoli delle cornici"""
    pal = pal or P.LUMEN
    mappa = ["..a..", ".bac.", "bbacc", ".dcd.", "..d.."]
    col = {"a": pal[4], "b": pal[3], "c": pal[2], "d": pal[1]}
    for j, r in enumerate(mappa):
        for i, ch in enumerate(r):
            if ch in col:
                px(im, cx - 2 + i, cy - 2 + j, col[ch])


# ---------------------------------------------------------------- cornici

def _bordo_oro(im, x, y, w, h):
    hline(im, x, y, w, P.NOTTE[0]); hline(im, x, y + h - 1, w, P.NOTTE[0])
    vline(im, x, y, h, P.NOTTE[0]); vline(im, x + w - 1, y, h, P.NOTTE[0])
    hline(im, x + 1, y + 1, w - 2, P.ORO[3]); vline(im, x + 1, y + 1, h - 2, P.ORO[3])
    hline(im, x + 1, y + h - 2, w - 2, P.ORO[1]); vline(im, x + w - 2, y + 1, h - 2, P.ORO[1])
    px(im, x + 1, y + 1, P.ORO[4])
    hline(im, x + 2, y + 2, w - 4, P.NOTTE[0]); hline(im, x + 2, y + h - 3, w - 4, P.NOTTE[0])
    vline(im, x + 2, y + 2, h - 4, P.NOTTE[0]); vline(im, x + w - 3, y + 2, h - 4, P.NOTTE[0])


def _angolo(im, x, y, sx, sy, gemme=True):
    """Angolo decorato: castone d'oro a rombo con gemma di Lumen e due rebbi lungo i bordi"""
    for k in range(3, 9):
        px(im, x + k * sx, y + 3 * sy, P.ORO[3] if k < 7 else P.ORO[2])
        px(im, x + 3 * sx, y + k * sy, P.ORO[3] if k < 7 else P.ORO[2])
    if gemme:
        cx, cy = x + 2 * sx, y + 2 * sy
        for dy in range(-4, 5):
            for dx in range(-4, 5):
                d = abs(dx) + abs(dy)
                if d == 4:
                    px(im, cx + dx, cy + dy, P.ORO[4] if dx + dy < 0 else P.ORO[1])
                elif d < 4:
                    c = P.LUMEN[4] if dx + dy <= -2 else P.LUMEN[3] if dx + dy <= 0 else P.LUMEN[2] if d < 3 else P.LUMEN[1]
                    px(im, cx + dx, cy + dy, c)


def cornice(im, x, y, w, h, stile="notte", gemme=True, titolo=None):
    """Pannello standard. stile: 'notte' (menu, HUD) o 'pergamena' (dialoghi, liste chiare)."""
    if stile == "pergamena":
        gradiente_v(im, x + 3, y + 3, w - 6, h - 6, P.PERGAMENA[4], P.PERGAMENA[3])
        rnd = random.Random(w * 31 + h)
        for _ in range(w * h // 40):
            px(im, x + 3 + rnd.randrange(max(1, w - 6)), y + 3 + rnd.randrange(max(1, h - 6)), P.con_alfa(P.PERGAMENA[1], 60))
        hline(im, x + 3, y + 3, w - 6, P.PERGAMENA[4])
    else:
        gradiente_v(im, x + 3, y + 3, w - 6, h - 6, P.NOTTE[2], P.NOTTE[1])
        hline(im, x + 3, y + 3, w - 6, P.NOTTE[3])
    _bordo_oro(im, x, y, w, h)
    if gemme and w >= 24 and h >= 18:
        _angolo(im, x + 1, y + 1, 1, 1)
        _angolo(im, x + w - 2, y + 1, -1, 1)
        _angolo(im, x + 1, y + h - 2, 1, -1)
        _angolo(im, x + w - 2, y + h - 2, -1, -1)
    if titolo:
        intestazione(im, x + 8, y - 5, titolo)


def intestazione(im, x, y, titolo, larg=None):
    """Targa del titolo appoggiata sul bordo superiore di un pannello"""
    tw = F.larghezza_grassetto(titolo)
    larg = larg or tw + 20
    rect(im, x, y, larg, 13, P.NOTTE[0])
    gradiente_v(im, x + 1, y + 1, larg - 2, 11, P.NOTTE[3], P.NOTTE[1])
    hline(im, x + 1, y + 1, larg - 2, P.ORO[3]); hline(im, x + 1, y + 11, larg - 2, P.ORO[1])
    vline(im, x + 1, y + 1, 11, P.ORO[3]); vline(im, x + larg - 2, y + 1, 11, P.ORO[1])
    cristallo(im, x + 6, y + 3, 7)
    titolo_oro(im, titolo, x + 12, y + 2)
    return larg


def oro_verticale(riga):
    return [P.ORO[5], P.ORO[4], P.ORO[4], P.ORO[3], P.ORO[3], P.ORO[2], P.ORO[2], P.ORO[2], P.ORO[1]][min(8, riga)]


def titolo_oro(im, s, x, y, centrato=False):
    if centrato:
        x -= F.larghezza_grassetto(s) // 2
    F.testo(im, s, x, y, oro_verticale, ombra=P.ORO[0], grassetto=True)


def separatore(im, x, y, w):
    hline(im, x, y, w, P.ORO[1])
    hline(im, x, y + 1, w, P.NOTTE[0])
    cristallo(im, x + w // 2, y - 3, 7)


# ---------------------------------------------------------------- selezione

def cursore(im, x, y, frame=0):
    """Cursore: piccolo cristallo inclinato che pulsa (x, y = centro sinistro della riga)"""
    forma = ["#....", "##...", "###..", "####.", "###..", "##...", "#...."]
    for j, r in enumerate(forma):
        for i, ch in enumerate(r):
            if ch == "#":
                c = P.LUMEN[4] if j < 3 else P.LUMEN[3] if j < 5 else P.LUMEN[2]
                px(im, x + i + (frame % 2), y - 3 + j, c)
    px(im, x + (frame % 2), y - 3, P.ORO[4])


def riga_selezionata(im, x, y, w, h=11):
    for i in range(w):
        a = int(150 * (1 - i / w) + 30)
        vline(im, x + i, y, h, P.con_alfa(P.AZZURRO[1], a))
    hline(im, x, y, w, P.con_alfa(P.ORO[3], 110))
    hline(im, x, y + h - 1, w, P.con_alfa(P.ORO[2], 80))


# ---------------------------------------------------------------- barre

def barra(im, x, y, w, frazione, rampa, etichetta=None, h=4):
    """Barra HP/XP: cornice scura, traccia, riempimento a 3 toni con tacche ogni 10%"""
    lx = x
    if etichetta:
        ew = F.larghezza_micro(etichetta) + 4
        rect(im, x, y - 1, ew, h + 2, P.NOTTE[0])
        F.micro(im, etichetta, x + 2, y - 1 + (h + 2 - 5) // 2, P.ORO[3])
        lx = x + ew
    rect(im, lx, y - 1, w + 2, h + 2, P.NOTTE[0])
    rect(im, lx + 1, y, w, h, P.NOTTE[1])
    hline(im, lx + 1, y + h - 1, w, P.NOTTE[2])
    pieno = max(0, min(w, int(round(w * frazione))))
    if frazione > 0 and pieno == 0:
        pieno = 1
    if pieno:
        rect(im, lx + 1, y, pieno, h, rampa[1])
        hline(im, lx + 1, y, pieno, rampa[2])
        hline(im, lx + 1, y + h - 1, pieno, rampa[0])
        px(im, lx + pieno, y, P.con_alfa((255, 255, 255, 255), 200))
    for t in range(1, 10):
        tx = lx + 1 + w * t // 10
        if tx < lx + 1 + pieno:
            px(im, tx, y + h - 1, rampa[0])
    return lx + w + 2


def rampa_hp(f):
    return P.HP_ALTO if f > 0.5 else P.HP_MEDIO if f > 0.2 else P.HP_BASSO


# ---------------------------------------------------------------- tipi

GLIFI_TIPO = {
    "FIAMMA": "...#... ..##... ..###.. .#####. .##.##. .#...#. ..###..",
    "ACQUA": "...#... ...#... ..###.. .#####. .##.##. .#####. ..###..",
    "NATURA": "....### ..##### .####.# .###.#. .##.#.. .#.#... #......",
    "IMPULSO": "....##. ...##.. ..##... .#####. ...##.. ..##... .##....",
    "VENTO": "#####.. .....#. ####.#. ....#.. ######. ......# .#####.",
    "ROCCIA": "...##.. ..####. .#####. ######. ####.## ####### .......",
    "GELO": "...#... .#.#.#. ..###.. ####### ..###.. .#.#.#. ...#...",
    "OMBRA": "..###.. .##.... ##..... ##..... ##..... .##.... ..###..",
    "ASTRALE": "...#... ...#... ####### .#####. ..###.. .##.##. .#...#.",
    "TOSSICO": "....##. ...#..# .#..##. #.#.... .#.###. ..#...# ...###.",
    "PSICHE": "....... ..###.. .#...#. #..#..# .#...#. ..###.. .......",
    "METALLO": "..###.. .#####. ##...## ##...## ##...## .#####. ..###..",
    "NEUTRO": "..###.. .#...#. #.....# #..#..# #.....# .#...#. ..###..",
}


def badge_tipo(im, tipo, x, y):
    """Badge 11x11 del tipo: gemma colorata con glifo chiaro"""
    sc, base, chiaro = P.TIPI[tipo]
    forma = ["..#######..", ".#########.", "###########", "###########", "###########", "###########",
             "###########", "###########", "###########", ".#########.", "..#######.."]
    for j, r in enumerate(forma):
        for i, ch in enumerate(r):
            if ch == "#":
                c = base
                if j <= 1 or i <= 1:
                    c = P.mix(base, chiaro, 0.45)
                if j >= 9 or i >= 9:
                    c = sc
                px(im, x + i, y + j, c)
    for j, r in enumerate(GLIFI_TIPO[tipo].split()):
        for i, ch in enumerate(r):
            if ch == "#":
                px(im, x + 2 + i, y + 2 + j, P.mix(chiaro, (255, 255, 255, 255), 0.6))


def pillola_tipo(im, tipo, x, y, larg=52):
    """Etichetta del tipo: badge + nome su fascia colorata"""
    sc, base, chiaro = P.TIPI[tipo]
    rect(im, x + 5, y + 1, larg - 5, 9, P.NOTTE[0])
    rect(im, x + 6, y + 2, larg - 7, 7, base)
    hline(im, x + 6, y + 2, larg - 7, P.mix(base, chiaro, 0.5))
    hline(im, x + 6, y + 8, larg - 7, sc)
    lum = 0.3 * base[0] + 0.59 * base[1] + 0.11 * base[2]
    if lum > 150:
        F.micro(im, tipo, x + 13, y + 3, sc)
    else:
        F.micro(im, tipo, x + 13, y + 3, P.mix(chiaro, (255, 255, 255, 255), 0.7), ombra=sc)
    badge_tipo(im, tipo, x, y)


def pillola_stato(im, sigla, x, y):
    c = P.STATI[sigla]
    w = F.larghezza_micro(sigla) + 6
    rect(im, x, y, w, 9, P.NOTTE[0])
    rect(im, x + 1, y + 1, w - 2, 7, c)
    hline(im, x + 1, y + 1, w - 2, P.mix(c, (255, 255, 255, 255), 0.35))
    F.micro(im, sigla, x + 3, y + 2, (255, 255, 255, 255))
    return w


def pulsante(im, x, y, w, h, etichetta, icona=None, attivo=False, frame=0):
    """Pulsante dei comandi (lotta e menu touch)"""
    rect(im, x, y, w, h, P.NOTTE[0])
    if attivo:
        gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.AZZURRO[1], P.NOTTE[3])
        hline(im, x + 1, y + 1, w - 2, P.ORO[4]); hline(im, x + 1, y + h - 2, w - 2, P.ORO[2])
        vline(im, x + 1, y + 1, h - 2, P.ORO[3]); vline(im, x + w - 2, y + 1, h - 2, P.ORO[2])
    else:
        gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.NOTTE[3], P.NOTTE[2])
        hline(im, x + 1, y + 1, w - 2, P.NOTTE[4]); hline(im, x + 1, y + h - 2, w - 2, P.NOTTE[1])
    tx = x + 6
    if icona:
        icona(im, x + 4, y + (h - 11) // 2)
        tx = x + 18
    col = P.ORO[4] if attivo else P.CREMA
    F.testo(im, etichetta, tx, y + (h - 7) // 2, col, ombra=P.OMBRA_TESTO)
    if attivo:
        cursore(im, x - 6, y + h // 2, frame)


# ---------------------------------------------------------------- icone 11x11 dei comandi

def _icona(im, x, y, forma, col):
    for j, r in enumerate(forma):
        for i, ch in enumerate(r):
            if ch in col:
                px(im, x + i, y + j, col[ch])


def icona_spada(im, x, y):
    _icona(im, x, y, ["........aa.", ".......aab.", "......aab..", ".....aab...", "....aab....",
                      ".c.aab.....", "..cab......", "..dc.......", ".d..c......", "dd.........", "d.........."],
           {"a": P.PIETRA[5], "b": P.PIETRA[3], "c": P.ORO[3], "d": P.LEGNO[3]})


def icona_elyon(im, x, y):
    _icona(im, x, y, ["...aaaaa...", "..abbbbba..", ".abbccbbba.", ".abcddcbba.", "aabcddcbbaa",
                      "aabbccbbbaa", ".abbbbbbba.", ".aabbbbbaa.", "..aaaaaaa..", "...........", "..........."],
           {"a": P.ORO[2], "b": P.NOTTE[3], "c": P.LUMEN[2], "d": P.LUMEN[4]})


def icona_zaino(im, x, y):
    _icona(im, x, y, ["...aaaaa...", "..a.....a..", ".bbbbbbbbb.", "bcccccccccb", "bcccdddcccb",
                      "bcccdedcccb", "bcccdddcccb", "bcccccccccb", "bcccccccccb", ".bbbbbbbbb.", "..........."],
           {"a": P.LEGNO[1], "b": P.LEGNO[1], "c": P.LEGNO[3], "d": P.ORO[2], "e": P.LUMEN[3]})


def icona_fuga(im, x, y):
    _icona(im, x, y, ["....aa.....", "...aaaa....", "....aa.....", "..aaaaab...", ".a.aaa..b..",
                      "...aaa.....", "..aa.aa....", ".aa...aa...", ".a.....a...", "cccc.cccc..", "..........."],
           {"a": P.CREMA, "b": P.CREMA, "c": P.con_alfa(P.AZZURRO[2], 160)})


def icona_core(im, x, y):
    """Core di Lumen: sfera di cristallo incastonata in un anello d'oro (NON una sfera bicolore)"""
    _icona(im, x, y, ["....aaa....", "..aabbbaa..", ".abcccccba.", ".bcdddddcb.", "abcdeeedcba",
                      "abcdeeedcba", "abcdddddcba", ".bccccccb..", ".abbcccbba.", "..aabbbaa..", "....aaa...."],
           {"a": P.ORO[1], "b": P.ORO[3], "c": P.LUMEN[1], "d": P.LUMEN[2], "e": P.LUMEN[4]})
