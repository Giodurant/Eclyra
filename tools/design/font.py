"""
ECLYRA - Font pixel originali (Design System v1).

"Eclyra Corpo": proporzionale, maiuscole alte 7 px, minuscole con discendenti (cella 9 px),
accenti italiani. "Eclyra Micro": maiuscole e cifre 3x5 per le etichette (LV, HP, PP).
I glifi sono disegnati a mano qui sotto ('#' = pixel acceso). Nessun font esterno.
"""
from PIL import Image

CORPO_ALTEZZA = 9      # righe 0-6 maiuscole, 7-8 discendenti
CORPO_SPAZIO = 1       # spaziatura tra i caratteri
CORPO_INTERLINEA = 11

_C = {}


def _g(ch, righe, alto=0):
    """righe: lista di stringhe; alto = riga di partenza (0 = cima della cella)"""
    w = max(len(r) for r in righe)
    m = [[0] * w for _ in range(CORPO_ALTEZZA)]
    for y, r in enumerate(righe):
        for x, c in enumerate(r):
            if c == "#":
                m[alto + y][x] = 1
    _C[ch] = m


def _split(s):
    return s.split()


# ---------------------------------------------------------------- maiuscole (7 righe)
MAIUSCOLE = {
    "A": ".###. #...# #...# ##### #...# #...# #...#",
    "B": "####. #...# ####. #...# #...# #...# ####.",
    "C": ".###. #...# #.... #.... #.... #...# .###.",
    "D": "####. #...# #...# #...# #...# #...# ####.",
    "E": "##### #.... ####. #.... #.... #.... #####",
    "F": "##### #.... ####. #.... #.... #.... #....",
    "G": ".###. #...# #.... #.### #...# #...# .####",
    "H": "#...# #...# ##### #...# #...# #...# #...#",
    "I": "### .#. .#. .#. .#. .#. ###",
    "J": "..### ...#. ...#. ...#. #..#. #..#. .##..",
    "K": "#...# #..#. ###.. #..#. #...# #...# #...#",
    "L": "#.... #.... #.... #.... #.... #.... #####",
    "M": "#...# ##.## #.#.# #.#.# #...# #...# #...#",
    "N": "#...# ##..# #.#.# #..## #...# #...# #...#",
    "O": ".###. #...# #...# #...# #...# #...# .###.",
    "P": "####. #...# #...# ####. #.... #.... #....",
    "Q": ".###. #...# #...# #...# #.#.# #..#. .##.#",
    "R": "####. #...# #...# ####. #..#. #...# #...#",
    "S": ".###. #...# #.... .###. ....# #...# .###.",
    "T": "##### ..#.. ..#.. ..#.. ..#.. ..#.. ..#..",
    "U": "#...# #...# #...# #...# #...# #...# .###.",
    "V": "#...# #...# #...# #...# .#.#. .#.#. ..#..",
    "W": "#...# #...# #...# #.#.# #.#.# ##.## #...#",
    "X": "#...# #...# .#.#. ..#.. .#.#. #...# #...#",
    "Y": "#...# #...# .#.#. ..#.. ..#.. ..#.. ..#..",
    "Z": "##### ....# ...#. ..#.. .#... #.... #####",
    "0": ".###. #...# #..## #.#.# ##..# #...# .###.",
    "1": "..#.. .##.. ..#.. ..#.. ..#.. ..#.. .###.",
    "2": ".###. #...# ....# ...#. ..#.. .#... #####",
    "3": ".###. #...# ....# ..##. ....# #...# .###.",
    "4": "...#. ..##. .#.#. #..#. ##### ...#. ...#.",
    "5": "##### #.... ####. ....# ....# #...# .###.",
    "6": ".###. #.... #.... ####. #...# #...# .###.",
    "7": "##### ....# ...#. ..#.. ..#.. ..#.. ..#..",
    "8": ".###. #...# #...# .###. #...# #...# .###.",
    "9": ".###. #...# #...# .#### ....# ....# .###.",
    "?": ".###. #...# ....# ...#. ..#.. ..... ..#..",
    "!": "# # # # # . #",
    "/": "....# ...#. ...#. ..#.. .#... .#... #....",
    "%": "##..# ##..# ...#. ..#.. .#... #..## #..##",
    "<": "...# ..#. .#.. #... .#.. ..#. ...#",
    ">": "#... .#.. ..#. ...# ..#. .#.. #...",
    "(": ".# #. #. #. #. #. .#",
    ")": "#. .# .# .# .# .# #.",
    "#": "..... .#.#. ##### .#.#. ##### .#.#. .....",
    "&": ".##.. #..#. .##.. .#... #.#.# #..#. .##.#",
}
for ch, s in MAIUSCOLE.items():
    _g(ch, _split(s))

# ---------------------------------------------------------------- minuscole (9 righe)
MINUSCOLE = {
    "a": "..... ..... .###. ....# .#### #...# .####",
    "b": "#.... #.... ####. #...# #...# #...# ####.",
    "c": ".... .... .### #... #... #... .###",
    "d": "....# ....# .#### #...# #...# #...# .####",
    "e": "..... ..... .###. #...# ##### #.... .###.",
    "f": "..## .#.. ###. .#.. .#.. .#.. .#..",
    "g": "..... ..... .#### #...# #...# #...# .#### ....# .###.",
    "h": "#.... #.... ####. #...# #...# #...# #...#",
    "i": "# . # # # # #",
    "j": "..# ... ..# ..# ..# ..# ..# ..# ##.",
    "k": "#... #... #..# #.#. ##.. #.#. #..#",
    "l": "#. #. #. #. #. #. .#",
    "m": "..... ..... ##.#. #.#.# #.#.# #.#.# #.#.#",
    "n": "..... ..... ####. #...# #...# #...# #...#",
    "o": "..... ..... .###. #...# #...# #...# .###.",
    "p": "..... ..... ####. #...# #...# #...# ####. #.... #....",
    "q": "..... ..... .#### #...# #...# #...# .#### ....# ....#",
    "r": ".... .... #.## ##.. #... #... #...",
    "s": "..... ..... .#### #.... .###. ....# ####.",
    "t": ".#.. .#.. ###. .#.. .#.. .#.. ..##",
    "u": "..... ..... #...# #...# #...# #...# .####",
    "v": "..... ..... #...# #...# #...# .#.#. ..#..",
    "w": "..... ..... #...# #...# #.#.# #.#.# .#.#.",
    "x": "..... ..... #...# .#.#. ..#.. .#.#. #...#",
    "y": "..... ..... #...# #...# #...# #...# .#### ....# .###.",
    "z": "..... ..... ##### ...#. ..#.. .#... #####",
}
for ch, s in MINUSCOLE.items():
    _g(ch, _split(s))

# ---------------------------------------------------------------- punteggiatura
_g(".", ["#"], 6)
_g(",", [".#", "#."], 6)
_g(":", ["#", ".", ".", "#"], 3)
_g(";", [".#", "..", "..", ".#", "#."], 3)
_g("'", ["#", "#"], 0)
_g("’", ["#", "#"], 0)
_g('"', ["#.#", "#.#"], 0)
_g("-", ["####"], 3)
_g("_", ["#####"], 6)
_g("+", ["..#..", "..#..", "#####", "..#..", "..#.."], 1)
_g("=", ["####", "....", "####"], 2)
_g("*", ["#.#.#", ".###.", "#####", ".###.", "#.#.#"], 1)
_g("«", ["..#.#", ".#.#.", "#.#..", ".#.#.", "..#.#"], 1)   # «
_g("»", ["#.#..", ".#.#.", "..#.#", ".#.#.", "#.#.."], 1)   # »
_g("—", ["#######"], 3)                                       # —
_g("…", ["#.#.#"], 6)                                         # …
_g("×", ["#...#", ".#.#.", "..#..", ".#.#.", "#...#"], 1)   # ×
_C[" "] = [[0] * 3 for _ in range(CORPO_ALTEZZA)]

# ---------------------------------------------------------------- accenti
_vocali = {"a": MINUSCOLE["a"], "e": MINUSCOLE["e"], "o": MINUSCOLE["o"], "u": MINUSCOLE["u"]}
for v, s in _vocali.items():
    corpo = _split(s)[2:]
    _g({"a": "à", "e": "è", "o": "ò", "u": "ù"}[v], [".#...", "..#.."] + corpo)
_g("é", ["...#.", "..#.."] + _split(MINUSCOLE["e"])[2:])
_g("ì", ["#.", ".#", ".#", ".#", ".#", ".#", ".#"])
_g("í", [".#", "#.", "#.", "#.", "#.", "#.", "#."])


def _maiuscola_accentata(base, accento):
    righe = _split(MAIUSCOLE[base])
    # lettera compressa a 6 righe + accento in cima; si toglie una riga ripetuta, mai la traversa
    togli = {"A": 1, "E": 4, "I": 2, "O": 2, "U": 2}[base]
    righe = righe[:togli] + righe[togli + 1:]
    return [accento] + righe


_g("À", _maiuscola_accentata("A", ".#..."))
_g("È", _maiuscola_accentata("E", ".#..."))
_g("É", _maiuscola_accentata("E", "...#."))
_g("Ì", _maiuscola_accentata("I", "#.."))
_g("Ò", _maiuscola_accentata("O", ".#..."))
_g("Ù", _maiuscola_accentata("U", ".#..."))

# ---------------------------------------------------------------- micro 3x5
MICRO = {
    "A": ".#. #.# ### #.# #.#", "B": "##. #.# ##. #.# ##.", "C": ".## #.. #.. #.. .##",
    "D": "##. #.# #.# #.# ##.", "E": "### #.. ##. #.. ###", "F": "### #.. ##. #.. #..",
    "G": ".## #.. #.# #.# .##", "H": "#.# #.# ### #.# #.#", "I": "### .#. .#. .#. ###",
    "J": "..# ..# ..# #.# .#.", "K": "#.# #.# ##. #.# #.#", "L": "#.. #.. #.. #.. ###",
    "M": "#.# ### #.# #.# #.#", "N": "##. #.# #.# #.# #.#", "O": ".#. #.# #.# #.# .#.",
    "P": "##. #.# ##. #.. #..", "Q": ".#. #.# #.# ##. .##", "R": "##. #.# ##. #.# #.#",
    "S": ".## #.. .#. ..# ##.", "T": "### .#. .#. .#. .#.", "U": "#.# #.# #.# #.# ###",
    "V": "#.# #.# #.# .#. .#.", "W": "#.# #.# ### ### #.#", "X": "#.# #.# .#. #.# #.#",
    "Y": "#.# #.# .#. .#. .#.", "Z": "### ..# .#. #.. ###",
    "0": "### #.# #.# #.# ###", "1": ".#. ##. .#. .#. ###", "2": "##. ..# .#. #.. ###",
    "3": "##. ..# .#. ..# ##.", "4": "#.# #.# ### ..# ..#", "5": "### #.. ##. ..# ##.",
    "6": ".## #.. ### #.# ###", "7": "### ..# .#. .#. .#.", "8": "### #.# ### #.# ###",
    "9": "### #.# ### ..# ##.", "/": "..# ..# .#. #.. #..", ":": ". # . # .", ".": ". . . . #",
    "-": "... ... ### ... ...", "+": "... .#. ### .#. ...", " ": "... ... ... ... ...",
    "%": "#.# ..# .#. #.. #.#", "'": "# # . . .",
}
_M = {}
for ch, s in MICRO.items():
    righe = s.split()
    _M[ch] = [[1 if c == "#" else 0 for c in r] for r in righe]
_M["À"] = _M["A"]
_M["È"] = _M["E"]
_M["Ì"] = _M["I"]


def glifo(ch):
    return _C.get(ch) or _C.get("?")


def larghezza(testo):
    w = 0
    for i, ch in enumerate(testo):
        w += len(glifo(ch)[0]) + (CORPO_SPAZIO if i < len(testo) - 1 else 0)
    return w


def larghezza_micro(testo):
    return sum(len(_M.get(c.upper(), _M[" "])[0]) + 1 for c in testo) - 1 if testo else 0


def _px(im, x, y, c):
    if 0 <= x < im.width and 0 <= y < im.height:
        if c[3] == 255:
            im.putpixel((x, y), c)
        else:
            im.alpha_composite(Image.new("RGBA", (1, 1), c), (x, y))


def testo(im, s, x, y, colore, ombra=None, grassetto=False, contorno=None):
    """Disegna testo del corpo. ombra: colore dell'ombra in basso (1 px). contorno: bordo 1 px."""
    if contorno:
        for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (1, -1), (-1, 1), (1, 1)):
            testo(im, s, x + dx, y + dy, contorno, grassetto=grassetto)
    if ombra:
        testo(im, s, x, y + 1, ombra, grassetto=grassetto)
        if grassetto:
            testo(im, s, x + 1, y + 1, ombra)
    cx = x
    for ch in s:
        g = glifo(ch)
        for gy, riga in enumerate(g):
            for gx, v in enumerate(riga):
                if v:
                    c = colore(gy) if callable(colore) else colore
                    _px(im, cx + gx, y + gy, c)
                    if grassetto:
                        _px(im, cx + gx + 1, y + gy, c)
        cx += len(g[0]) + CORPO_SPAZIO + (1 if grassetto else 0)
    return cx - x


def larghezza_grassetto(s):
    return larghezza(s) + len(s)


def micro(im, s, x, y, colore, ombra=None):
    if ombra:
        micro(im, s, x, y + 1, ombra)
    cx = x
    for ch in s.upper():
        g = _M.get(ch, _M[" "])
        for gy, riga in enumerate(g):
            for gx, v in enumerate(riga):
                if v:
                    _px(im, cx + gx, y + gy, colore)
        cx += len(g[0]) + 1
    return cx - x


def a_capo(s, larg):
    """Spezza un testo in righe che entrano nella larghezza (pixel)."""
    righe = []
    for paragrafo in s.split("\n"):
        corrente = ""
        for parola in paragrafo.split(" "):
            prova = (corrente + " " + parola).strip()
            if larghezza(prova) <= larg or not corrente:
                corrente = prova
            else:
                righe.append(corrente)
                corrente = parola
        righe.append(corrente)
    return righe


CARATTERI_ATLANTE = (
    " !\"#%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
    "àèéìíòùÀÈÉÌÒÙ«»×—…’"
)


def atlante_corpo():
    """Atlante per raylib: glifi su una riga, separati da 1 colonna magenta (LoadFontFromImage)."""
    chiave = (255, 0, 255, 255)
    w = 1 + sum(len(glifo(c)[0]) + 1 for c in CARATTERI_ATLANTE)
    im = Image.new("RGBA", (w, CORPO_ALTEZZA + 2), chiave)
    x = 1
    for ch in CARATTERI_ATLANTE:
        g = glifo(ch)
        for gy in range(CORPO_ALTEZZA):
            for gx in range(len(g[0])):
                im.putpixel((x + gx, 1 + gy), (255, 255, 255, 255) if g[gy][gx] else (0, 0, 0, 0))
        x += len(g[0]) + 1
    return im
