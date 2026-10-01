#!/usr/bin/env python3
"""
Genera la pixel art di partenza del gioco in assets/:
  tileset.png  256x64  caselle 16x16 (erba, erba alta, fiori, cartello,
                       sentiero e acqua con bordi automatici)
  albero.png   16x32   albero in vista 3/4 (la chioma sporge sulla riga sopra)
  casa.png     80x64   casetta 5x4 caselle
  giocatore.png 48x72  3 fotogrammi x 3 direzioni (giu', su, sinistra), 16x24

Sono disegni ORIGINALI in stile GBA. Puoi ritoccarli con Aseprite:
dopo ogni modifica lancia  python3 tools/incorpora_grafica.py
"""
import math
import os
from PIL import Image

T = 16
OUT = os.path.join(os.path.dirname(__file__), "..", "assets")
os.makedirs(OUT, exist_ok=True)

# ------------------------------------------------------------------
# Palette (ispirata ai colori brillanti del GBA)
# ------------------------------------------------------------------
TR = (0, 0, 0, 0)
G_BASE = (120, 200, 104, 255)
G_DARK = (88, 168, 80, 255)
G_LIGHT = (160, 224, 128, 255)
G_SHADOW = (64, 136, 72, 255)

TG_OUT = (32, 96, 48, 255)
TG_DARK = (56, 136, 64, 255)
TG_MID = (80, 168, 72, 255)
TG_LIGHT = (136, 208, 104, 255)

P_BASE = (232, 208, 152, 255)
P_DARK = (208, 176, 120, 255)
P_LIGHT = (248, 232, 184, 255)
P_EDGE = (168, 136, 88, 255)

W_BASE = (80, 144, 232, 255)
W_LIGHT = (128, 184, 248, 255)
W_SHINE = (200, 232, 255, 255)
W_DEEP = (56, 112, 208, 255)
W_EDGE = (40, 80, 160, 255)
W_FOAM = (232, 248, 255, 255)

L_OUT = (24, 72, 48, 255)
L_DARK = (40, 112, 64, 255)
L_MID = (64, 152, 72, 255)
L_LIGHT = (112, 192, 88, 255)
L_SHINE = (168, 224, 120, 255)
TRUNK = (136, 88, 48, 255)
TRUNK_D = (96, 56, 32, 255)


def img(w, h, fill=TR):
    return Image.new("RGBA", (w, h), fill)


def put(im, x, y, c):
    if 0 <= x < im.width and 0 <= y < im.height:
        im.putpixel((x, y), c)


def contorna(im, colore):
    """Aggiunge un contorno di 1 pixel attorno ai pixel non trasparenti."""
    src = im.copy()
    for y in range(im.height):
        for x in range(im.width):
            if src.getpixel((x, y))[3] == 0:
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < im.width and 0 <= ny < im.height and src.getpixel((nx, ny))[3] > 0:
                        im.putpixel((x, y), colore)
                        break
    return im


def da_ascii(righe, palette, larghezza):
    h = len(righe)
    im = img(larghezza, h)
    for y, r in enumerate(righe):
        assert len(r) == larghezza, f"riga {y} lunga {len(r)}: {r!r}"
        for x, ch in enumerate(r):
            if ch != ".":
                im.putpixel((x, y), palette[ch])
    return im


def rng(seed):
    # generatore pseudo-casuale deterministico (stesse immagini a ogni esecuzione)
    state = [seed * 2654435761 % 2**32 or 1]

    def r():
        state[0] = (1103515245 * state[0] + 12345) % 2**31
        return state[0]
    return r


# ------------------------------------------------------------------
# Erba
# ------------------------------------------------------------------
def erba(seed):
    t = img(T, T, G_BASE)
    r = rng(seed)
    # ciuffetti a "v" come nei giochi GBA
    for _ in range(3):
        x, y = 1 + r() % 13, 2 + r() % 12
        put(t, x, y, G_DARK); put(t, x + 2, y, G_DARK); put(t, x + 1, y + 1, G_DARK)
        put(t, x, y - 1, G_LIGHT)
    for _ in range(4):
        put(t, r() % 16, r() % 16, G_LIGHT)
    return t


# ------------------------------------------------------------------
# Erba alta: fili a punta con contorno scuro
# ------------------------------------------------------------------
def fili_erba_alta():
    b = img(T, T)
    ciuffi = [(0, 1), (8, 1), (4, 8), (12, 8), (-4, 8)]
    for cx, cy in ciuffi:
        for i, (ox, h) in enumerate(((0, 5), (2, 7), (4, 5))):
            base = cy + 7
            x0 = cx + ox
            for yy in range(h):
                y = base - yy
                larg = 1 if yy >= h - 2 else 2
                for xx in range(larg):
                    c = TG_LIGHT if (xx == 0 and i != 2) else TG_MID
                    if yy < 2:
                        c = TG_DARK
                    put(b, x0 + xx, y, c)
    return contorna(b, TG_OUT)


def erba_alta():
    t = img(T, T, TG_DARK)
    f = fili_erba_alta()
    t.alpha_composite(f)
    return t


def erba_alta_sopra():
    """Solo la meta' bassa dei fili: si disegna sopra al giocatore."""
    f = fili_erba_alta()
    o = img(T, T)
    for y in range(9, T):
        for x in range(T):
            o.putpixel((x, y), f.getpixel((x, y)))
    return o


# ------------------------------------------------------------------
# Fiori (2 fotogrammi, oscillano)
# ------------------------------------------------------------------
def fiori(frame):
    t = erba(7)
    pos = [(4, 4, (248, 96, 96, 255)), (11, 10, (248, 216, 72, 255)), (3, 12, (248, 248, 248, 255))]
    for x, y, col in pos:
        x += frame if y < 8 else -frame
        put(t, x, y + 2, G_SHADOW)
        for dx, dy in ((0, -1), (-1, 0), (1, 0), (0, 1)):
            put(t, x + dx, y + dy, col)
        put(t, x, y, (248, 232, 160, 255) if col[1] < 200 else (232, 128, 48, 255))
    return t


# ------------------------------------------------------------------
# Cartello
# ------------------------------------------------------------------
def cartello():
    pal = {"k": (72, 40, 24, 255), "a": (216, 168, 104, 255), "A": (168, 112, 64, 255),
           "t": (136, 88, 48, 255), "s": (0, 0, 0, 60)}
    r = [
        "................",
        "................",
        ".kkkkkkkkkkkkkk.",
        ".kaaaaaaaaaaaak.",
        ".kattattattatak.",
        ".kaaaaaaaaaaaak.",
        ".katattatatatak.",
        ".kAAAAAAAAAAAAk.",
        ".kkkkkkkkkkkkkk.",
        "......kAak......",
        "......kAak......",
        "......kAak......",
        ".....kkAakk.....",
        ".....ssssss.....",
        "................",
        "................",
    ]
    t = erba(3)
    t.alpha_composite(da_ascii(r, pal, 16))
    return t


# ------------------------------------------------------------------
# Autotile: il bordo si disegna solo dove il vicino e' diverso.
# maschera: bit0 = sopra uguale, bit1 = destra, bit2 = sotto, bit3 = sinistra
# ------------------------------------------------------------------
def lati_mancanti(m):
    return {"su": not m & 1, "dx": not m & 2, "giu": not m & 4, "sx": not m & 8}


def sentiero(m):
    t = img(T, T, P_BASE)
    r = rng(99)
    for _ in range(5):
        x, y = r() % 15, r() % 15
        put(t, x, y, P_DARK); put(t, x + 1, y, P_LIGHT)
    lati = lati_mancanti(m)
    for i in range(T):
        if lati["su"]:
            put(t, i, 0, G_BASE); put(t, i, 1, P_EDGE)
        if lati["giu"]:
            put(t, i, 15, G_BASE); put(t, i, 14, P_EDGE)
        if lati["sx"]:
            put(t, 0, i, G_BASE); put(t, 1, i, P_EDGE)
        if lati["dx"]:
            put(t, 15, i, G_BASE); put(t, 14, i, P_EDGE)
    # angoli arrotondati
    for (a, b, cx, cy, sx, sy) in (("su", "sx", 0, 0, 1, 1), ("su", "dx", 15, 0, -1, 1),
                                   ("giu", "sx", 0, 15, 1, -1), ("giu", "dx", 15, 15, -1, -1)):
        if lati[a] and lati[b]:
            put(t, cx + sx, cy, G_BASE); put(t, cx, cy + sy, G_BASE); put(t, cx + sx, cy + sy, G_BASE)
            put(t, cx + 2 * sx, cy + sy, P_EDGE); put(t, cx + sx, cy + 2 * sy, P_EDGE)
    return t


def acqua(m, frame):
    t = img(T, T, W_BASE)
    # onde che scorrono
    for y in range(T):
        for x in range(T):
            v = math.sin((x + frame * 4) * 0.8 + y * 1.7) + math.sin(y * 0.9 - frame * 2)
            if v > 1.55:
                put(t, x, y, W_LIGHT)
            elif v < -1.6:
                put(t, x, y, W_DEEP)
    for (x, y) in ((3 + frame * 2, 4), (10 - frame * 2, 11)):
        put(t, x, y, W_SHINE); put(t, x + 1, y, W_SHINE)
    lati = lati_mancanti(m)
    for i in range(T):
        if lati["su"]:
            put(t, i, 0, G_SHADOW); put(t, i, 1, W_EDGE); put(t, i, 2, W_FOAM if (i + frame) % 4 else W_LIGHT)
        if lati["giu"]:
            put(t, i, 15, G_BASE); put(t, i, 14, W_EDGE)
        if lati["sx"]:
            put(t, 0, i, G_SHADOW); put(t, 1, i, W_EDGE); put(t, 2, i, W_LIGHT if lati["su"] and i < 3 else t.getpixel((2, i)))
        if lati["dx"]:
            put(t, 15, i, G_SHADOW); put(t, 14, i, W_EDGE)
    for (a, b, cx, cy, sx, sy) in (("su", "sx", 0, 0, 1, 1), ("su", "dx", 15, 0, -1, 1),
                                   ("giu", "sx", 0, 15, 1, -1), ("giu", "dx", 15, 15, -1, -1)):
        if lati[a] and lati[b]:
            put(t, cx + sx, cy, G_SHADOW); put(t, cx, cy + sy, G_SHADOW); put(t, cx + sx, cy + sy, G_SHADOW)
            put(t, cx + 2 * sx, cy + sy, W_EDGE); put(t, cx + sx, cy + 2 * sy, W_EDGE)
    return t


# ------------------------------------------------------------------
# Albero 16x32: chioma a grappoli con luce da in alto a sinistra
# ------------------------------------------------------------------
def albero():
    a = img(16, 32)
    # tronco
    for y in range(24, 31):
        for x in range(6, 10):
            put(a, x, y, TRUNK_D if x in (6, 9) else TRUNK)
    put(a, 5, 30, TRUNK_D); put(a, 10, 30, TRUNK_D)
    # chioma: grappoli circolari
    grappoli = [(8, 11, 6.2), (4.5, 16, 4.4), (11.5, 16, 4.4), (8, 19.5, 5.0), (8, 7, 4.0)]
    for y in range(32):
        for x in range(16):
            best = None
            for (cx, cy, rr) in grappoli:
                d = math.hypot(x + 0.5 - cx, y + 0.5 - cy)
                if d <= rr:
                    rel = d / rr
                    score = (cy, -rel)
                    if best is None or score > best[0]:
                        best = (score, cx, cy, rr, d)
            if best:
                _, cx, cy, rr, d = best
                dx, dy = (x + 0.5 - cx) / rr, (y + 0.5 - cy) / rr
                luce = -dx * 0.5 - dy * 0.9
                if luce > 0.55 and d < rr * 0.8:
                    c = L_SHINE if luce > 0.8 else L_LIGHT
                elif luce > -0.1:
                    c = L_MID
                elif luce > -0.6:
                    c = L_DARK
                else:
                    c = L_OUT if d > rr - 1.2 else L_DARK
                put(a, x, y, c)
    contorna(a, L_OUT)
    # ombra ai piedi
    for x in range(3, 13):
        if a.getpixel((x, 31))[3] == 0:
            put(a, x, 31, (0, 0, 0, 50))
    return a


# ------------------------------------------------------------------
# Casa 80x64 (5x4 caselle)
# ------------------------------------------------------------------
def casa():
    h = img(80, 64)
    ROOF = (208, 72, 64, 255); ROOF_L = (240, 112, 88, 255); ROOF_D = (152, 48, 48, 255)
    WALL = (248, 232, 200, 255); WALL_D = (216, 192, 152, 255)
    TRIM = (136, 88, 56, 255); OUT = (56, 32, 32, 255)
    WIN = (112, 176, 240, 255); WIN_L = (208, 240, 255, 255)
    DOOR = (160, 96, 56, 255); DOOR_D = (112, 64, 40, 255)
    # muro (parte bassa)
    for y in range(34, 62):
        for x in range(4, 76):
            c = WALL
            if (y - 34) % 7 == 6:
                c = WALL_D
            put(h, x, y, c)
    for x in range(4, 76):
        put(h, x, 34, TRIM); put(h, x, 61, TRIM)
    # tetto: righe di tegole, piu' largo del muro
    for y in range(2, 36):
        inset = max(0, 6 - y)
        for x in range(0 + inset, 80 - inset):
            c = ROOF
            if (y - 2) % 6 == 5:
                c = ROOF_D
            elif (y - 2) % 6 == 0:
                c = ROOF_L
            if (x + ((y - 2) // 6) * 5) % 10 == 0 and (y - 2) % 6 != 5:
                c = ROOF_D
            put(h, x, y, c)
    # colmo
    for x in range(6, 74):
        put(h, x, 2, ROOF_L); put(h, x, 3, ROOF_L)
    # finestre
    for wx in (10, 56):
        for y in range(40, 52):
            for x in range(wx, wx + 14):
                c = WIN
                if x in (wx, wx + 13) or y in (40, 51):
                    c = TRIM
                elif x == wx + 7 or y == 46:
                    c = TRIM
                elif (x - wx) + (y - 40) < 6:
                    c = WIN_L
                put(h, x, y, c)
        for x in range(wx - 1, wx + 15):
            put(h, x, 52, WALL_D)
    # porta al centro della riga in basso (colonna 2 della casa)
    for y in range(44, 62):
        for x in range(33, 47):
            c = DOOR
            if x in (33, 46) or y == 44:
                c = DOOR_D
            elif x == 40:
                c = DOOR_D
            put(h, x, y, c)
    put(h, 43, 53, (248, 216, 72, 255))
    for x in range(31, 49):
        put(h, x, 62, (200, 184, 152, 255)); put(h, x, 63, (176, 160, 128, 255))
    contorna(h, OUT)
    # ombra sul muro sotto il tetto
    for x in range(4, 76):
        if h.getpixel((x, 36))[:3] == WALL[:3]:
            put(h, x, 36, WALL_D)
    return h


# ------------------------------------------------------------------
# Giocatore: personaggio originale (fascia gialla, giacca arancione,
# zainetto verde). 16x24 per fotogramma.
# ------------------------------------------------------------------
PAL_G = {
    "k": (40, 40, 56, 255), "h": (88, 56, 40, 255), "H": (136, 88, 56, 255),
    "b": (248, 208, 64, 255), "s": (248, 200, 160, 255), "S": (216, 152, 112, 255),
    "e": (32, 32, 48, 255), "j": (240, 120, 48, 255), "J": (192, 80, 32, 255),
    "w": (248, 248, 248, 255), "p": (64, 88, 168, 255), "P": (40, 56, 120, 255),
    "f": (200, 56, 56, 255), "F": (136, 32, 40, 255), "g": (72, 160, 96, 255),
    "G": (40, 112, 64, 255),
}

TESTA_GIU = [
    "................",
    ".....kkkkkk.....",
    "....khhhhhhk....",
    "...khhHHhhhhk...",
    "..khhhhhhhhhhk..",
    "..kbbbbbbbbbbk..",
    "..khsssssssshk..",
    "..kssessssessk..",
    "..kssessssessk..",
    "..kSssssssssSk..",
    "...kSssSSssSk...",
    "....kksssskk....",
]
CORPO_GIU = [
    "...kjjwwwwjjk...",
    "..kjjjjwwjjjjk..",
    "..kjJjjwwjjJjk..",
    "..ksjjjjjjjjsk..",
    "..kSjjjjjjjjSk..",
    "...kJJJJJJJJk...",
    "...kppppppppk...",
]
TESTA_SU = [
    "................",
    ".....kkkkkk.....",
    "....khhhhhhk....",
    "...khhHHhhhhk...",
    "..khhhhhhhhhhk..",
    "..kbbbbbbbbbbk..",
    "..khhhhhhhhhhk..",
    "..khhhhhHhhhhk..",
    "..khhhhhhhhhhk..",
    "..kShhhhhhhhSk..",
    "...kShhhhhhSk...",
    "....kksssskk....",
]
CORPO_SU = [
    "...kjggggggjk...",
    "..kjgGggggGgjk..",
    "..kjggggggggjk..",
    "..ksgGGGGGGgsk..",
    "..kSjggggggjSk..",
    "...kJJJJJJJJk...",
    "...kppppppppk...",
]
GAMBE_FRONTE = [
    [  # fermo
        "...kpppkkpppk...",
        "...kPppk.kppPk..",
        "...kfffk.kfffk..",
        "...kFFFk.kFFFk..",
        "....kkk...kkk...",
    ],
    [  # passo sinistro
        "...kpppkkpppk...",
        "...kPppk.kppPk..",
        "...kfffk.kFFFk..",
        "...kFFFk..kkk...",
        "....kkk.........",
    ],
    [  # passo destro
        "...kpppkkpppk...",
        "...kPppk.kppPk..",
        "...kFFFk.kfffk..",
        "....kkk..kFFFk..",
        "..........kkk...",
    ],
]
TESTA_SX = [
    "................",
    "......kkkkk.....",
    ".....khhhhhk....",
    "....khHHhhhhk...",
    "...khhhhhhhhhk..",
    "...kbbbbbbbbbbk.",
    "...kssshhhhhhbk.",
    "..kssesshhhhhk..",
    "..kssesshhhhhk..",
    "..kSsssShhhhk...",
    "...kSSssSShk....",
    "....kkssskk.....",
]
CORPO_SX = [
    "....kjjjjggk....",
    "...kjjjjjgGgk...",
    "...kjjsjjgGgk...",
    "...kjjsjjgggk...",
    "...kJjSjjGGGk...",
    "....kJJJJJJk....",
    "....kppppppk....",
]
GAMBE_SX = [
    [
        "....kppppppk....",
        "....kPppppPk....",
        "...kffffffk.....",
        "...kFFFFFFk.....",
        "....kkkkkk......",
    ],
    [
        "...kpppkppppk...",
        "..kPppk.kpppk...",
        "..kffk...kffk...",
        "..kFFFk..kFFFk..",
        "...kkk....kkk...",
    ],
    [
        "....kppppppk....",
        "...kPpppppPk....",
        "...kfffkffk.....",
        "..kFFFkkFFFk....",
        "...kkk..kkk.....",
    ],
]


def giocatore():
    sheet = img(48, 72)
    direzioni = [(TESTA_GIU, CORPO_GIU, GAMBE_FRONTE), (TESTA_SU, CORPO_SU, GAMBE_FRONTE),
                 (TESTA_SX, CORPO_SX, GAMBE_SX)]
    for d, (testa, corpo, gambe) in enumerate(direzioni):
        for f in range(3):
            righe = testa + corpo + gambe[f]
            assert len(righe) == 24, (d, f, len(righe))
            sheet.alpha_composite(da_ascii(righe, PAL_G, 16), (f * 16, d * 24))
    return sheet


def sottobosco():
    """Terreno scuro sotto gli alberi: tra una fila e l'altra sembra ombra."""
    t = img(T, T, L_DARK)
    r = rng(11)
    for _ in range(10):
        put(t, r() % 16, r() % 16, L_OUT)
    for _ in range(4):
        put(t, r() % 16, r() % 16, L_MID)
    return t


def main():
    ts = img(256, 64)
    fila0 = [erba(1), erba(2), erba(5), erba_alta(), erba_alta_sopra(), fiori(0), fiori(1), cartello(), sottobosco()]
    for i, t in enumerate(fila0):
        ts.alpha_composite(t, (i * T, 0))
    for m in range(16):
        ts.alpha_composite(sentiero(m), (m * T, 16))
        ts.alpha_composite(acqua(m, 0), (m * T, 32))
        ts.alpha_composite(acqua(m, 1), (m * T, 48))
    ts.save(os.path.join(OUT, "tileset.png"))
    albero().save(os.path.join(OUT, "albero.png"))
    casa().save(os.path.join(OUT, "casa.png"))
    giocatore().save(os.path.join(OUT, "giocatore.png"))
    print("Grafica generata in", os.path.abspath(OUT))


if __name__ == "__main__":
    main()
