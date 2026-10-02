"""
ECLYRA - Motore degli sprite degli Elyon (Design System v1).

Ogni Elyon e' descritto da FORME (ellissi, capsule, poligoni, fiamme emissive) con un
materiale (rampa di 5 colori) e una profondita'. Il renderer:
  1. rasterizza le forme in ordine di profondita' (mappa di materiali + normali);
  2. ombreggia con luce dall'alto a sinistra in 4 toni (cel shading, niente sfumature);
  3. aggiunge linee interne tra parti sovrapposte e un contorno selettivo
     (scuro in basso a destra, piu' tenue sul lato illuminato);
  4. disegna occhi espressivi, riflessi e i dettagli emissivi (fiamme, cristalli di Lumen).
La vista da dietro si ottiene specchiando le FORME (non l'immagine), cosi' la luce resta
coerente; le forme con tag 'viso' non compaiono da dietro, quelle con 'retro' solo da dietro.
"""
import math

import numpy as np
from PIL import Image
from scipy import ndimage

import palette as P

LUCE = np.array([-0.55, -0.65, 0.52])
LUCE = LUCE / np.linalg.norm(LUCE)


def rampa(*esadecimali):
    return [P.H(h) for h in esadecimali]


# ---------------------------------------------------------------- forme

class Forma:
    def __init__(self, tipo, mat, z=0, tag=(), **k):
        self.tipo, self.mat, self.z, self.tag, self.k = tipo, mat, z, set(tag), k


def E(cx, cy, rx, ry, mat, z=0, tag=(), rot=0.0):
    return Forma("ellisse", mat, z, tag, cx=cx, cy=cy, rx=rx, ry=ry, rot=rot)


def C(x1, y1, x2, y2, r, mat, z=0, tag=(), r2=None):
    return Forma("capsula", mat, z, tag, x1=x1, y1=y1, x2=x2, y2=y2, r=r, r2=r2 if r2 is not None else r)


def Poly(punti, mat, z=0, tag=(), piatto=False):
    return Forma("poligono", mat, z, tag, punti=punti, piatto=piatto)


def Fiamma(cx, base, h, w, z=5, tag=(), pal=None, curva=0.0):
    return Forma("fiamma", "fiamma", z, tag, cx=cx, base=base, h=h, w=w, pal=pal, curva=curva)


def Occhio(cx, cy, r, iride, sguardo=-1, espressione="deciso", z=50, tag=("viso",)):
    return Forma("occhio", "occhio", z, set(tag) | {"viso"}, cx=cx, cy=cy, r=r, iride=iride, sguardo=sguardo,
                 espressione=espressione)


def Bocca(punti, z=51, tag=("viso",), zanna=False):
    return Forma("bocca", "bocca", z, set(tag) | {"viso"}, punti=punti, zanna=zanna)


def Gemma(cx, cy, h, z=60, pal=None, tag=()):
    return Forma("gemma", "gemma", z, tag, cx=cx, cy=cy, h=h, pal=pal or P.LUMEN)


def Macchia(cx, cy, rx, ry, mat, z=40, tag=()):
    """Macchia piatta (segni sul manto): prende la luce del corpo sotto ma col proprio colore"""
    return Forma("macchia", mat, z, tag, cx=cx, cy=cy, rx=rx, ry=ry)


# ---------------------------------------------------------------- rasterizzazione

def _griglia(S):
    y, x = np.mgrid[0:S, 0:S].astype(float)
    return x + 0.5, y + 0.5


def _maschera_normale(f, S, X, Y):
    k = f.k
    if f.tipo in ("ellisse", "macchia"):
        dx, dy = X - k["cx"], Y - k["cy"]
        rot = k.get("rot", 0.0)
        if rot:
            c, s = math.cos(rot), math.sin(rot)
            dx, dy = dx * c + dy * s, -dx * s + dy * c
        u, v = dx / k["rx"], dy / k["ry"]
        d2 = u * u + v * v
        m = d2 <= 1.0
        nz = np.sqrt(np.clip(1 - d2, 0, 1))
        if rot:
            c, s = math.cos(-rot), math.sin(-rot)
            u, v = u * c + v * s, -u * s + v * c
        return m, np.stack([u, v, nz], -1)
    if f.tipo == "capsula":
        x1, y1, x2, y2 = k["x1"], k["y1"], k["x2"], k["y2"]
        vx, vy = x2 - x1, y2 - y1
        L2 = max(1e-6, vx * vx + vy * vy)
        t = np.clip(((X - x1) * vx + (Y - y1) * vy) / L2, 0, 1)
        r = k["r"] + (k["r2"] - k["r"]) * t
        px_, py_ = x1 + t * vx, y1 + t * vy
        dx, dy = (X - px_) / r, (Y - py_) / r
        d2 = dx * dx + dy * dy
        m = d2 <= 1.0
        return m, np.stack([dx, dy, np.sqrt(np.clip(1 - d2, 0, 1))], -1)
    if f.tipo == "poligono":
        im = Image.new("L", (S, S), 0)
        from PIL import ImageDraw
        ImageDraw.Draw(im).polygon([(x, y) for x, y in k["punti"]], fill=255)
        m = np.array(im) > 0
        if k.get("piatto"):
            n = np.zeros((S, S, 3)); n[..., 2] = 1
            return m, n
        d = ndimage.distance_transform_edt(m)
        h = np.sqrt(np.clip(d / max(1.0, d.max()), 0, 1))
        gy, gx = np.gradient(ndimage.gaussian_filter(h, 0.8))
        n = np.stack([-gx * 6, -gy * 6, np.ones_like(h)], -1)
        n /= np.linalg.norm(n, axis=-1, keepdims=True)
        return m, n
    raise ValueError(f.tipo)


def _specchia(f, S):
    import copy
    g = copy.deepcopy(f)
    k = g.k
    for chiave in ("cx", "x1", "x2"):
        if chiave in k:
            k[chiave] = S - k[chiave]
    if "rot" in k:
        k["rot"] = -k["rot"]
    if "punti" in k:
        k["punti"] = [(S - x, y) for x, y in k["punti"]]
    if "sguardo" in k:
        k["sguardo"] = -k["sguardo"]
    if "curva" in k:
        k["curva"] = -k["curva"]
    return g


def render(forme, materiali, S=64, retro=False, frame=0, outline_scuro=None, ombra_terra=True):
    """forme: lista di Forma; materiali: nome -> rampa di 5 colori (scuro -> chiaro).
    frame 1 = respiro (le forme con tag 'su' salgono di 1 px), frame 2 = occhi chiusi."""
    if retro:
        forme = [_specchia(f, S) for f in forme if "viso" not in f.tag]
    else:
        forme = [f for f in forme if "retro" not in f.tag]
    X, Y = _griglia(S)
    if frame == 1:
        Y = Y + 0.0
    col = np.zeros((S, S, 4), np.uint8)
    idmat = np.full((S, S), -1, int)
    zbuf = np.full((S, S), -1e9)
    ramp_idx = np.zeros((S, S), int)
    nomi = list(materiali.keys())
    solide = [f for f in forme if f.tipo in ("ellisse", "capsula", "poligono", "macchia")]
    for indice, f in enumerate(sorted(solide, key=lambda f: f.z)):
        Yf = Y + (1.0 if (frame == 1 and "su" in f.tag) else 0.0)
        m, n = _maschera_normale(f, S, X, Yf)
        if f.tipo == "macchia":
            m = m & (idmat >= 0)
        intens = np.clip((n * LUCE).sum(-1), -1, 1)
        livello = np.select([intens > 0.88, intens > 0.55, intens > 0.12], [4, 3, 2], 1)
        if f.tipo == "macchia":
            # i segni sul manto seguono la luce del corpo sotto, un tono piu' scuri: niente toppe piatte
            livello = np.where(m, np.clip(ramp_idx - (ramp_idx >= 3), 1, 4), livello)
        sel = m & (f.z >= zbuf)
        mi = nomi.index(f.mat)
        # linea interna: dove questa forma copre una parte diversa, il bordo della parte coperta si scurisce
        idmat[sel] = mi * 1000 + indice
        zbuf[sel] = f.z
        ramp_idx[sel] = livello[sel]
    # colore
    for mi, nome in enumerate(nomi):
        r = materiali[nome]
        sel = (idmat // 1000 == mi) & (idmat >= 0)
        for lv in range(5):
            s2 = sel & (ramp_idx == lv)
            col[s2] = r[lv]
    pieno = idmat >= 0
    # linee interne (bordo tra parti diverse: si scurisce il lato dietro)
    for dx, dy in ((1, 0), (0, 1), (-1, 0), (0, -1)):
        vic = np.roll(np.roll(idmat, dy, 0), dx, 1)
        zv = np.roll(np.roll(zbuf, dy, 0), dx, 1)
        bordo = pieno & (vic >= 0) & (vic != idmat) & (zv > zbuf + 0.5)
        for mi, nome in enumerate(nomi):
            s2 = bordo & (idmat // 1000 == mi)
            col[s2] = materiali[nome][0]
    # contorno selettivo
    esterno = ~pieno
    vicini = np.zeros((S, S), bool)
    sorgente = np.full((S, S), -1, int)
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        v = np.roll(np.roll(idmat, dy, 0), dx, 1)
        nuovi = esterno & (v >= 0) & ~vicini
        sorgente[nuovi] = v[nuovi]
        vicini |= nuovi
    for mi, nome in enumerate(nomi):
        r = materiali[nome]
        s2 = vicini & (sorgente // 1000 == mi)
        ys, xs = np.nonzero(s2)
        for y, x in zip(ys, xs):
            # lato illuminato (alto/sinistra) = contorno piu' chiaro
            lit = (y < S - 1 and x < S - 1 and pieno[min(S - 1, y + 1), x] and not pieno[max(0, y - 1), x]) or \
                  (x < S - 1 and pieno[y, min(S - 1, x + 1)] and not pieno[y, max(0, x - 1)])
            c = P.mix(r[0], (10, 8, 24, 255), 0.35) if not lit else r[0]
            col[y, x] = outline_scuro or c
    im = Image.fromarray(col, "RGBA")
    # dettagli
    for f in sorted([f for f in forme if f.tipo not in ("ellisse", "capsula", "poligono", "macchia")], key=lambda f: f.z):
        dy = -1 if (frame == 1 and "su" in f.tag) else 0
        if f.tipo == "occhio":
            _occhio(im, f, dy, chiuso=(frame == 2))
        elif f.tipo == "bocca":
            _bocca(im, f, dy)
        elif f.tipo == "gemma":
            _gemma(im, f, dy)
        elif f.tipo == "fiamma":
            _fiamma(im, f, dy, frame)
    if ombra_terra:
        _ombra(im, S)
    return im


def _px(im, x, y, c):
    x, y = int(math.floor(x + 0.5)), int(math.floor(y + 0.5))
    if 0 <= x < im.width and 0 <= y < im.height:
        if len(c) == 4 and c[3] < 255:
            b = im.getpixel((x, y))
            a = c[3] / 255
            im.putpixel((x, y), tuple(int(b[i] * (1 - a) + c[i] * a) for i in range(3)) + (max(b[3], c[3]),))
        else:
            im.putpixel((x, y), c)


def _occhio(im, f, dy, chiuso=False):
    """Occhio grande e leggibile: sclera con ombra, iride a 2 toni, pupilla, due riflessi,
    palpebra superiore spessa che da' l'espressione."""
    k = f.k
    cx, cy = int(math.floor(k["cx"] + 0.5)), int(math.floor(k["cy"] + dy + 0.5))
    r = int(round(k["r"]))
    s = k["sguardo"]
    contorno = P.H("140F26")
    if chiuso:
        for i in range(-r, r + 1):
            _px(im, cx + i, cy + (1 if abs(i) < r else 0), contorno)
        return
    rx, ry = r + 0.2, r + 0.7
    for y in range(-r - 2, r + 3):
        for x in range(-r - 1, r + 2):
            d = (x / rx) ** 2 + (y / ry) ** 2
            if d <= 1.0:
                _px(im, cx + x, cy + y, P.H("F7F5FF") if y < r - 1 else P.H("C6CCE6"))
            elif d <= 1.55:
                _px(im, cx + x, cy + y, contorno)
    iride = k["iride"]
    ir = max(1, r - 1)
    ix = cx + s
    for y in range(-ir - 1, ir + 2):
        for x in range(-ir, ir + 1):
            if (x / (ir + 0.5)) ** 2 + (y / (ir + 0.9)) ** 2 <= 1.0:
                c = iride[0] if y < -ir + 1 else iride[1] if y < 1 else iride[2]
                _px(im, ix + x, cy + y + 1, c)
    # pupilla 2x2 (1x2 negli occhi piccoli)
    for y in range(0, 2):
        _px(im, ix, cy + y, contorno)
        if r >= 3:
            _px(im, ix + s, cy + y, contorno)
    _px(im, ix - 1, cy - 1, P.H("FFFFFF"))
    if r >= 3:
        _px(im, ix - 1, cy - 2, P.H("FFFFFF"))
        _px(im, ix + 1, cy + 2, P.mix(iride[2], P.H("FFFFFF"), 0.6))
    e = k["espressione"]
    for x in range(-r - 1, r + 2):
        if e == "deciso":
            yy = cy - r - 1 + (x * s) * 0.45
        elif e == "felice":
            yy = cy - r - 1 - (1 if abs(x) < r else 0)
        elif e == "fiero":
            yy = cy - r + (x * s) * 0.6
        else:
            yy = cy - r - 1
        _px(im, cx + x, yy, contorno)
        if r >= 3 and e in ("deciso", "fiero") and abs(x) < r:
            _px(im, cx + x, yy - 1, contorno)


def _bocca(im, f, dy):
    pts = f.k["punti"]
    c = P.H("2A0E18")
    for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
        n = int(max(abs(x2 - x1), abs(y2 - y1))) + 1
        for i in range(n):
            t = i / max(1, n - 1)
            _px(im, x1 + (x2 - x1) * t, y1 + (y2 - y1) * t + dy, c)
    if f.k["zanna"]:
        x, y = pts[-1]
        _px(im, x - 1, y + 1 + dy, P.H("FFFFFF"))


def _gemma(im, f, dy):
    k = f.k
    cx, cy, h, pal = k["cx"], k["cy"] + dy, max(3, int(round(k["h"]))), k["pal"]
    meta = h // 2
    # alone
    for y in range(-h, h + 1):
        for x in range(-h, h + 1):
            d = math.hypot(x, y) / h
            if d < 1:
                b = im.getpixel((int(cx + x) % im.width, int(cy + y) % im.height))
                if 0 <= cx + x < im.width and 0 <= cy + y < im.height:
                    a = (1 - d) ** 2 * 0.35
                    im.putpixel((int(cx + x), int(cy + y)), tuple(min(255, int(b[i] + pal[3][i] * a)) for i in range(3)) + (max(b[3], int(140 * (1 - d))),))
    for j in range(h):
        larg = (j if j <= meta else h - 1 - j) // 2 + (1 if h >= 7 and 0 < j < h - 1 else 0)
        for i in range(-larg, larg + 1):
            c = pal[4] if (i < 0 and j < meta) else pal[3] if i <= 0 else pal[2] if j < meta else pal[1]
            _px(im, cx + i, cy - meta + j, c)
    _px(im, cx, cy - meta - 1, P.mix(pal[1], (0, 0, 0, 255), 0.4))
    _px(im, cx, cy - meta + h, P.mix(pal[1], (0, 0, 0, 255), 0.4))


def _fiamma(im, f, dy, frame):
    """Fiamma emissiva: nucleo chiaro, bordo arancio-rosso, guizzo animato"""
    k = f.k
    pal = k["pal"] or [P.H("8C2414"), P.H("E8542F"), P.H("FF9A3C"), P.H("FFD45A"), P.H("FFF6C8")]
    cx, base, h, w, curva = k["cx"], k["base"] + dy, k["h"], k["w"], k["curva"]
    guizzo = [0, 1, -1][frame % 3]
    for j in range(int(h)):
        t = j / h                         # 0 = base, 1 = punta
        larg = w * (1 - t) ** 0.8 * (0.6 + 0.4 * math.sin(t * math.pi * 0.9 + 0.3))
        x0 = cx + curva * t * t * h + guizzo * t
        for i in range(-int(larg), int(larg) + 1):
            dd = abs(i) / max(0.5, larg)
            if dd > 1:
                continue
            if dd > 0.75:
                c = pal[1]
            elif dd > 0.45 or t > 0.7:
                c = pal[2]
            elif t > 0.45:
                c = pal[3]
            else:
                c = pal[4] if dd < 0.25 else pal[3]
            _px(im, x0 + i, base - j, c)
    # alone caldo
    for y in range(int(base - h - 3), int(base + 3)):
        for x in range(int(cx - w - 4), int(cx + w + 4)):
            if 0 <= x < im.width and 0 <= y < im.height:
                b = im.getpixel((x, y))
                if b[3] == 0:
                    d = math.hypot((x - cx) / (w + 4), (y - (base - h / 2)) / (h / 2 + 3))
                    if d < 1:
                        a = int(70 * (1 - d))
                        if a > 8:
                            im.putpixel((x, y), (pal[2][0], pal[2][1], pal[2][2], a))


def _ombra(im, S):
    """Ombra al suolo: ellisse scura semitrasparente sotto i piedi"""
    a = np.array(im)
    ys = np.nonzero(a[..., 3] > 200)[0]
    xs = np.nonzero(a[..., 3] > 200)[1]
    if len(ys) == 0:
        return
    fondo = ys.max()
    cx = (xs.min() + xs.max()) / 2
    rx = (xs.max() - xs.min()) / 2 * 0.8
    sh = Image.new("RGBA", im.size, (0, 0, 0, 0))
    for y in range(fondo - 2, min(S, fondo + 3)):
        for x in range(int(cx - rx), int(cx + rx) + 1):
            if ((x - cx) / max(1, rx)) ** 2 + ((y - fondo) / 2.5) ** 2 <= 1 and 0 <= x < S:
                sh.putpixel((x, y), (8, 10, 30, 90))
    sh.alpha_composite(im)
    im.paste(sh)


def foglio(sprites, colonne, scala=1, sfondo=(20, 26, 60, 255), margine=4):
    w = sprites[0].width
    righe = (len(sprites) + colonne - 1) // colonne
    out = Image.new("RGBA", (colonne * (w + margine) + margine, righe * (w + margine) + margine), sfondo)
    for i, s in enumerate(sprites):
        out.alpha_composite(s, (margine + (i % colonne) * (w + margine), margine + (i // colonne) * (w + margine)))
    if scala > 1:
        out = out.resize((out.width * scala, out.height * scala), Image.NEAREST)
    return out
