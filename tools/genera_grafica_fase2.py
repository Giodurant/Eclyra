#!/usr/bin/env python3
"""
ECLYRA - pixel art segnaposto ORIGINALE per la Fase 2.
Genera in assets/:
  tilesets/casa_blu.png     80x64   casa con tetto blu
  tilesets/negozio.png      80x64   emporio (tetto verde, insegna)
  tilesets/laboratorio.png  112x80  laboratorio del professor Elian (7x5 tile)
  tilesets/interni.png      256x32  tile degli interni (pavimenti, muri, mobili)
  sprites/npc.png           384x72  8 personaggi, 3 fotogrammi x 3 direzioni ciascuno

Uso: python3 tools/genera_grafica_fase2.py   (richiede Pillow)
"""
import os
import sys
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
import genera_grafica_base as base   # noqa: E402  (funzioni di disegno condivise)

RADICE = os.path.join(os.path.dirname(__file__), "..", "assets")
T = 16
TR = (0, 0, 0, 0)


def img(w, h, c=TR):
    return Image.new("RGBA", (w, h), c)


def put(im, x, y, c):
    if 0 <= x < im.width and 0 <= y < im.height:
        im.putpixel((x, y), c)


def rect(im, x, y, w, h, c):
    for j in range(y, y + h):
        for i in range(x, x + w):
            put(im, i, j, c)


def ricolora(im, mappa):
    out = im.copy()
    px = out.load()
    for y in range(out.height):
        for x in range(out.width):
            p = px[x, y]
            if p[:3] in mappa:
                px[x, y] = mappa[p[:3]] + (p[3],)
    return out


# ------------------------------------------------------------------
# Edifici
# ------------------------------------------------------------------
ROSSO = [(208, 72, 64), (240, 112, 88), (152, 48, 48)]


def casa_blu():
    blu = [(64, 112, 200), (104, 152, 232), (40, 72, 152)]
    return ricolora(base.casa(), {ROSSO[i]: blu[i] for i in range(3)})


def negozio():
    verde = [(72, 160, 96), (112, 200, 128), (40, 112, 64)]
    h = ricolora(base.casa(), {ROSSO[i]: verde[i] for i in range(3)})
    # insegna sopra la porta con un cristallo di Lumen (simbolo del negozio)
    OUT = (56, 32, 32, 255)
    rect(h, 28, 31, 24, 9, OUT)
    rect(h, 29, 32, 22, 7, (248, 232, 160, 255))
    cx, cy = 40, 35
    for dy in range(-2, 3):
        for dx in range(-2 + abs(dy), 3 - abs(dy)):
            put(h, cx + dx, cy + dy, (72, 184, 176, 255) if dy >= 0 else (136, 232, 216, 255))
    return h


def laboratorio():
    W, H = 112, 80
    h = img(W, H)
    TETTO = (88, 104, 136, 255); TETTO_L = (128, 144, 176, 255); TETTO_D = (56, 64, 96, 255)
    MURO = (240, 240, 232, 255); MURO_D = (208, 208, 200, 255)
    VETRO = (120, 192, 232, 255); VETRO_L = (208, 240, 255, 255); TELAIO = (72, 88, 112, 255)
    OUT = (40, 40, 56, 255)
    # muro
    rect(h, 4, 40, W - 8, 38, MURO)
    for y in range(40, 78, 8):
        rect(h, 4, y, W - 8, 1, MURO_D)
    rect(h, 4, 40, W - 8, 2, TELAIO)
    rect(h, 4, 76, W - 8, 2, TELAIO)
    # tetto piatto a lastre, piu' largo del muro
    rect(h, 0, 14, W, 28, TETTO)
    for y in range(14, 42, 7):
        rect(h, 0, y, W, 1, TETTO_D)
        rect(h, 0, y + 1, W, 1, TETTO_L)
    for x in range(0, W, 16):
        rect(h, x, 14, 1, 28, TETTO_D)
    # pannelli e antenna con cristallo di Lumen
    rect(h, 14, 18, 20, 8, (48, 72, 120, 255))
    rect(h, 15, 19, 18, 6, (72, 112, 176, 255))
    rect(h, 78, 4, 2, 14, TELAIO)
    for dy in range(-4, 5):
        for dx in range(-3 + abs(dy) * 3 // 4, 4 - abs(dy) * 3 // 4):
            put(h, 79 + dx, 4 + dy, (120, 232, 208, 255) if dy < 1 else (64, 168, 176, 255))
    put(h, 78, 2, (255, 255, 255, 255))
    # finestre grandi
    for wx in (10, 30, 70, 88):
        rect(h, wx, 48, 14, 14, TELAIO)
        rect(h, wx + 1, 49, 12, 12, VETRO)
        for k in range(5):
            put(h, wx + 2 + k, 50 + k, VETRO_L)
        rect(h, wx + 6, 49, 1, 12, TELAIO)
    # porta a vetri doppia al centro (colonna 3 di 7 -> x 48..63)
    rect(h, 47, 54, 18, 24, TELAIO)
    rect(h, 48, 55, 7, 23, VETRO)
    rect(h, 57, 55, 7, 23, VETRO)
    for k in range(6):
        put(h, 49 + k // 2, 57 + k, VETRO_L)
        put(h, 58 + k // 2, 57 + k, VETRO_L)
    rect(h, 44, 50, 24, 3, (88, 168, 200, 255))   # tettoia
    base.contorna(h, OUT)
    return h


# ------------------------------------------------------------------
# Interni (16x16 ciascuno)
# ------------------------------------------------------------------
LEGNO = (200, 152, 96, 255); LEGNO_D = (168, 120, 72, 255); LEGNO_L = (224, 184, 128, 255)
CARTA = (232, 216, 176, 255); CARTA_D = (208, 188, 144, 255)
BATTISCOPA = (136, 88, 56, 255); BORDO = (72, 48, 40, 255)


def pavimento(variante):
    t = img(T, T, LEGNO)
    for y in range(0, T, 4):
        rect(t, 0, y + 3, T, 1, LEGNO_D)
        off = (y // 4 * 7 + variante * 3) % 16
        put(t, off, y, LEGNO_D); put(t, off, y + 1, LEGNO_D); put(t, off, y + 2, LEGNO_D)
        put(t, (off + 9) % 16, y + 1, LEGNO_L)
    return t


def muro():
    t = img(T, T, CARTA)
    for x in range(1, T, 4):
        rect(t, x, 2, 1, 11, CARTA_D)
    rect(t, 0, 0, T, 2, BORDO)
    rect(t, 0, 13, T, 3, BATTISCOPA)
    rect(t, 0, 13, T, 1, BORDO)
    return t


def muro_alto():
    t = img(T, T, (112, 88, 72, 255))
    rect(t, 0, 0, T, 1, (152, 120, 96, 255))
    rect(t, 0, 15, T, 1, (72, 56, 48, 255))
    return t


def finestra():
    t = muro()
    rect(t, 3, 3, 10, 8, BORDO)
    rect(t, 4, 4, 8, 6, (136, 200, 240, 255))
    rect(t, 7, 4, 1, 6, BORDO)
    put(t, 5, 5, (232, 248, 255, 255)); put(t, 6, 5, (232, 248, 255, 255)); put(t, 5, 6, (232, 248, 255, 255))
    return t


def con_oggetto(disegna, variante=0):
    t = pavimento(variante)
    o = img(T, T)
    disegna(o)
    base.contorna(o, (56, 40, 40, 255))
    t.alpha_composite(o)
    return t


def tappetino(o):
    rect(o, 2, 4, 12, 9, (184, 56, 56, 255))
    rect(o, 3, 5, 10, 7, (216, 88, 72, 255))
    for x in range(3, 13, 2):
        put(o, x, 13, (232, 200, 120, 255))
    rect(o, 6, 7, 4, 1, (248, 216, 120, 255))
    rect(o, 7, 8, 2, 2, (248, 216, 120, 255))


def tappeto(o):
    rect(o, 0, 0, 16, 16, (80, 112, 184, 255))
    rect(o, 2, 2, 12, 12, (112, 144, 208, 255))
    for k in range(4, 12, 4):
        rect(o, k, 7, 2, 2, (232, 216, 120, 255))


def sedia(o):
    rect(o, 4, 3, 8, 4, LEGNO_D)
    rect(o, 4, 7, 8, 4, LEGNO)
    rect(o, 4, 11, 1, 3, LEGNO_D); rect(o, 11, 11, 1, 3, LEGNO_D)


def letto_a(o):
    rect(o, 1, 0, 14, 4, LEGNO_D)
    rect(o, 2, 4, 12, 12, (248, 248, 240, 255))
    rect(o, 4, 5, 8, 5, (232, 232, 248, 255))
    rect(o, 2, 12, 12, 4, (88, 136, 208, 255))


def letto_b(o):
    rect(o, 2, 0, 12, 13, (88, 136, 208, 255))
    for y in range(2, 12, 3):
        rect(o, 2, y, 12, 1, (120, 168, 232, 255))
    rect(o, 1, 13, 14, 3, LEGNO_D)


def libreria_a(o):
    rect(o, 1, 0, 14, 16, LEGNO_D)
    colori = [(200, 64, 64), (64, 120, 200), (72, 168, 96), (232, 184, 72), (152, 96, 168)]
    for riga, y in enumerate((2, 9)):
        for i in range(6):
            c = colori[(i + riga * 2) % 5] + (255,)
            rect(o, 2 + i * 2, y, 2, 6, c)
        rect(o, 2, y + 6, 12, 1, BORDO)


def libreria_b(o):
    rect(o, 1, 0, 14, 14, LEGNO_D)
    colori = [(72, 168, 96), (200, 64, 64), (232, 184, 72), (64, 120, 200)]
    for i in range(6):
        rect(o, 2 + i * 2, 1, 2, 6, colori[i % 4] + (255,))
    rect(o, 2, 7, 12, 1, BORDO)
    rect(o, 2, 9, 12, 4, LEGNO)


def macchina_a(o):
    rect(o, 1, 1, 14, 15, (152, 160, 176, 255))
    rect(o, 3, 3, 10, 7, (32, 48, 64, 255))
    for x in range(3, 13):
        y = 6 + int(2 * __import__("math").sin(x * 1.1))
        put(o, x, y, (120, 240, 200, 255))
    rect(o, 3, 12, 2, 2, (232, 88, 72, 255)); rect(o, 7, 12, 2, 2, (120, 232, 120, 255))


def macchina_b(o):
    rect(o, 1, 0, 14, 14, (128, 136, 152, 255))
    for y in range(2, 12, 3):
        rect(o, 3, y, 10, 1, (96, 104, 120, 255))
    rect(o, 11, 3, 2, 2, (120, 232, 208, 255))


def pianta(o):
    rect(o, 5, 10, 6, 5, (184, 104, 64, 255))
    rect(o, 5, 10, 6, 1, (144, 80, 48, 255))
    for (x, y, r) in ((8, 6, 3), (5, 7, 2), (11, 7, 2), (8, 3, 2)):
        for j in range(-r, r + 1):
            for i in range(-r, r + 1):
                if i * i + j * j <= r * r:
                    put(o, x + i, y + j, (72, 160, 80, 255) if j < 0 else (48, 120, 64, 255))


def tv(o):
    rect(o, 2, 2, 12, 9, (48, 48, 56, 255))
    rect(o, 3, 3, 10, 7, (96, 160, 216, 255))
    put(o, 4, 4, (200, 232, 255, 255)); put(o, 5, 4, (200, 232, 255, 255))
    rect(o, 3, 11, 10, 4, LEGNO_D)


def bancone(o):
    rect(o, 0, 3, 16, 12, (232, 232, 224, 255))
    rect(o, 0, 3, 16, 3, LEGNO)
    rect(o, 0, 12, 16, 3, (192, 192, 184, 255))


def scaffale_a(o):
    rect(o, 1, 0, 14, 16, (184, 184, 192, 255))
    for y, colori in ((2, [(232, 88, 72), (232, 200, 72), (88, 168, 232)]), (9, [(120, 200, 120), (232, 128, 168), (232, 200, 72)])):
        for i, c in enumerate(colori):
            rect(o, 2 + i * 4, y, 3, 5, c + (255,))
        rect(o, 2, y + 5, 12, 1, (120, 120, 136, 255))


def scaffale_b(o):
    rect(o, 1, 0, 14, 14, (184, 184, 192, 255))
    for i, c in enumerate([(88, 168, 232), (232, 88, 72), (120, 200, 120)]):
        rect(o, 2 + i * 4, 1, 3, 5, c + (255,))
    rect(o, 2, 6, 12, 1, (120, 120, 136, 255))
    rect(o, 2, 8, 12, 5, (152, 152, 168, 255))


def tavolo(o):
    rect(o, 1, 3, 14, 9, LEGNO)
    rect(o, 2, 4, 12, 7, (240, 232, 216, 255))
    rect(o, 2, 12, 2, 3, LEGNO_D); rect(o, 12, 12, 2, 3, LEGNO_D)
    rect(o, 6, 6, 4, 2, (232, 120, 96, 255))


def pc(o):
    rect(o, 0, 9, 16, 6, LEGNO_D)
    rect(o, 3, 1, 10, 8, (64, 64, 80, 255))
    rect(o, 4, 2, 8, 6, (88, 200, 176, 255))
    put(o, 5, 3, (220, 255, 240, 255))
    rect(o, 7, 9, 2, 1, (64, 64, 80, 255))


def interni():
    ts = img(256, 32)
    tiles = [
        pavimento(0), pavimento(1), muro(), muro_alto(), con_oggetto(tappetino), con_oggetto(tappeto),
        con_oggetto(sedia), con_oggetto(letto_a), con_oggetto(letto_b), con_oggetto(libreria_a),
        con_oggetto(libreria_b), con_oggetto(macchina_a), con_oggetto(macchina_b), con_oggetto(pianta),
        con_oggetto(tv), con_oggetto(bancone),
        con_oggetto(scaffale_a), con_oggetto(scaffale_b), con_oggetto(tavolo), con_oggetto(pc), finestra(),
    ]
    for i, t in enumerate(tiles):
        ts.alpha_composite(t, ((i % 16) * T, (i // 16) * T))
    return ts


# ------------------------------------------------------------------
# NPC: variazioni di colore dei modelli del protagonista
# ------------------------------------------------------------------
def P(*rgb):
    return tuple(rgb) + (255,)


NPC = [
    # 0 ELIAN: capelli grigi, camice bianco
    {"h": P(150, 150, 160), "H": P(196, 196, 204), "b": P(150, 150, 160), "j": P(244, 244, 240), "J": P(200, 200, 212),
     "w": P(112, 160, 208), "p": P(72, 64, 64), "P": P(48, 40, 40), "g": P(244, 244, 240), "G": P(200, 200, 212),
     "f": P(96, 64, 48), "F": P(64, 40, 32)},
    # 1 KAEL: capelli rossi, bandana nera, giacca blu, zaino giallo
    {"h": P(200, 64, 48), "H": P(240, 112, 80), "b": P(48, 48, 56), "j": P(64, 120, 216), "J": P(40, 80, 160),
     "p": P(96, 96, 108), "P": P(64, 64, 76), "g": P(232, 184, 48), "G": P(176, 128, 32)},
    # 2 MADRE: capelli castani, cerchietto rosa, vestito verde
    {"h": P(120, 72, 40), "H": P(168, 112, 64), "b": P(232, 128, 160), "j": P(104, 176, 112), "J": P(72, 136, 88),
     "w": P(240, 232, 208), "p": P(104, 176, 112), "P": P(72, 136, 88), "g": P(104, 176, 112), "G": P(72, 136, 88),
     "f": P(136, 88, 56), "F": P(96, 56, 40)},
    # 3 RAGAZZA: bionda, giacca rosa
    {"h": P(224, 184, 80), "H": P(248, 224, 144), "b": P(216, 64, 72), "j": P(240, 144, 184), "J": P(200, 104, 144),
     "p": P(64, 64, 120), "P": P(40, 40, 88), "g": P(240, 144, 184), "G": P(200, 104, 144)},
    # 4 UOMO: capelli neri, giacca marrone
    {"h": P(48, 40, 40), "H": P(88, 80, 80), "b": P(48, 40, 40), "j": P(152, 104, 64), "J": P(112, 72, 40),
     "w": P(232, 224, 200), "p": P(56, 72, 96), "P": P(40, 52, 72), "g": P(152, 104, 64), "G": P(112, 72, 40)},
    # 5 ANZIANO: capelli bianchi, giacca viola
    {"h": P(216, 216, 216), "H": P(248, 248, 248), "b": P(216, 216, 216), "j": P(136, 96, 168), "J": P(96, 64, 128),
     "p": P(112, 112, 120), "P": P(80, 80, 88), "s": P(232, 184, 152), "S": P(200, 144, 112),
     "g": P(136, 96, 168), "G": P(96, 64, 128)},
    # 6 BAMBINO: capelli arancioni, cappellino blu, maglia gialla
    {"h": P(216, 128, 48), "H": P(240, 168, 88), "b": P(64, 104, 200), "j": P(240, 208, 72), "J": P(200, 160, 40),
     "p": P(200, 64, 64), "P": P(152, 40, 48), "g": P(64, 104, 200), "G": P(40, 72, 152)},
    # 7 ESPLORATORE: divisa verde, berretto verde, zaino marrone
    {"h": P(72, 56, 40), "H": P(112, 88, 64), "b": P(56, 112, 64), "j": P(88, 136, 88), "J": P(56, 96, 64),
     "p": P(168, 144, 96), "P": P(128, 104, 64), "g": P(136, 96, 56), "G": P(96, 64, 40)},
]


def npc_sheet():
    pal_orig = base.PAL_G
    sheet = img(48 * len(NPC), 72)
    for i, override in enumerate(NPC):
        base.PAL_G = dict(pal_orig)
        base.PAL_G.update(override)
        sheet.alpha_composite(base.giocatore(), (i * 48, 0))
    base.PAL_G = pal_orig
    return sheet


def main():
    casa_blu().save(os.path.join(RADICE, "tilesets", "casa_blu.png"))
    negozio().save(os.path.join(RADICE, "tilesets", "negozio.png"))
    laboratorio().save(os.path.join(RADICE, "tilesets", "laboratorio.png"))
    interni().save(os.path.join(RADICE, "tilesets", "interni.png"))
    npc_sheet().save(os.path.join(RADICE, "sprites", "npc.png"))
    print("Grafica Fase 2 generata in", os.path.abspath(RADICE))


if __name__ == "__main__":
    main()
