#!/usr/bin/env python3
"""
ECLYRA - Mockup delle schermate (Design System v1), renderizzati a 320x180 come nel gioco.

Uso: python3 tools/design/mockups.py [cartella_uscita]
Scrive una PNG per schermata a 1x e a 3x, e un foglio riassuntivo.
"""
import math
import os
import sys

from PIL import Image

import elyon_design as D
import font as F
import mondo as W
import palette as P
import sprites as S
import ui

LW, LH = 320, 180
px, rect, hline, vline = ui.px, ui.rect, ui.hline, ui.vline

_cache = {}


def elyon(nome, retro=False, frame=0, scala=1.0):
    chiave = (nome, retro, frame, scala)
    if chiave not in _cache:
        f, m = D.SCHEDE[nome]()
        if scala != 1.0:
            f = [_scala(x, scala) for x in f]
        _cache[chiave] = S.render(f, m, S=int(64 * scala), retro=retro, frame=frame)
    return _cache[chiave]


def _scala(f, s):
    import copy
    g = copy.deepcopy(f)
    for k, v in list(g.k.items()):
        if k in ("cx", "cy", "rx", "ry", "x1", "y1", "x2", "y2", "r", "r2", "base", "h", "w"):
            g.k[k] = v * s
        if k == "punti":
            g.k[k] = [(x * s, y * s) for x, y in v]
    if g.tipo == "occhio":
        # gli occhi non crescono in proporzione: le sagome grandi restano minacciose, le piccole leggibili
        g.k["r"] = max(1, round(g.k["r"] * s)) if s < 1 else g.k["r"] * (1 + (s - 1) * 0.35)
    return g


# ================================================================ post-produzione

def grading(im, caldo=0.0, vignetta=0.35, tinta=None):
    """Illuminazione cinematografica: luce calda dall'alto a sinistra, vignetta, tinta d'ambiente"""
    w, h = im.size
    a = im.load()
    for y in range(h):
        for x in range(w):
            r, g, b, al = a[x, y]
            luce = 1 - (x / w * 0.5 + y / h * 0.5)       # 1 in alto a sinistra
            if caldo:
                r = r + (255 - r) * caldo * luce * 0.25
                g = g + (230 - g) * caldo * luce * 0.15
            dx, dy = (x - w / 2) / (w / 2), (y - h / 2) / (h / 2)
            v = max(0.0, math.hypot(dx * 0.9, dy) - 0.55) * vignetta * 1.6
            r, g, b = r * (1 - v), g * (1 - v), b * (1 - v * 0.7)
            if tinta:
                t = tinta[3] / 255
                r, g, b = r + (tinta[0] - r) * t, g + (tinta[1] - g) * t, b + (tinta[2] - b) * t
            a[x, y] = (int(min(255, r)), int(min(255, g)), int(min(255, b)), al)


def scintille(im, n, area, colore, seme=1):
    r = W.rnd("sc", seme)
    x0, y0, x1, y1 = area
    for _ in range(n):
        x, y = r.randrange(x0, x1), r.randrange(y0, y1)
        px(im, x, y, colore)
        if r.random() < 0.3:
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                px(im, x + dx, y + dy, P.con_alfa(colore, 110))


def nuovo(colore=None):
    return Image.new("RGBA", (LW, LH), colore or P.NOTTE[1])


# ================================================================ 2. OVERWORLD - LUNARIA

def scena_lunaria(frame=0):
    im = nuovo(P.ERBA[3])
    cols, righe = LW // 16 + 1, LH // 16 + 1
    piazza = lambda c, r: 5 <= c <= 14 and 6 <= r <= 9
    via = lambda c, r: (c in (9, 10) and r >= 9) or (r in (7, 8) and (2 <= c <= 5 or c >= 14)) or (c in (3, 4) and r <= 7 and r >= 4) or (c in (15, 16) and r in (5, 6))
    stagno = lambda c, r: c in (0, 1) or (c == 2 and r >= 9) or (c <= 3 and r >= 10)
    for r in range(righe):
        for c in range(cols):
            x, y = c * 16, r * 16
            if piazza(c, r):
                W.lastricato(im, x, y)
            elif stagno(c, r):
                W.acqua(im, x, y, (stagno(c, r - 1), stagno(c, r + 1), stagno(c - 1, r), stagno(c + 1, r)), frame)
            elif via(c, r):
                v = lambda cc, rr: via(cc, rr) or piazza(cc, rr)
                W.sentiero(im, x, y, (v(c, r - 1), v(c, r + 1), v(c - 1, r), v(c + 1, r)))
            elif (c, r) in ((17, 9), (18, 10), (12, 11), (6, 4), (1, 6), (19, 7)):
                W.fiori(im, x, y, c * 7 + r)
            elif (c, r) in ((14, 11), (15, 11), (16, 11), (13, 10)):
                W.erba_alta(im, x, y, frame)
            else:
                W.erba(im, x, y)
    # bordo della piazza e rosa del Lumen intarsiata attorno alla fonte
    hline(im, 80, 96, 160, P.PIETRA[4]); hline(im, 80, 159, 160, P.PIETRA[1])
    rosa_lumen(im, 160, 128)
    # ponte sul ruscello
    for c in range(0, 2):
        for rr in (7, 8):
            W.ponte(im, c * 16, rr * 16, orizzontale=True)
    oggetti = []   # (base_y, funzione)
    oggetti.append((60, lambda: W.casa(im, 38, 2, 60)))
    oggetti.append((70, lambda: W.casa(im, 112, 14, 56)))
    oggetti.append((79, lambda: W.laboratorio(im, 210, 6, 96)))
    for (tx, ty, sp) in ((-10, 64, "quercia"), (-14, 100, "abete"), (296, 92, "quercia"), (280, 130, "abete"),
                         (300, 150, "quercia"), (60, 150, "abete")):
        oggetti.append((ty + 38, lambda tx=tx, ty=ty, sp=sp: W.albero(im, tx, ty, specie=sp)))
    oggetti.append((118, lambda: W.albero(im, 250, 80, specie="lumen")))
    for (lx, ly) in ((84, 72), (228, 72), (84, 140), (228, 140)):
        oggetti.append((ly + 27, lambda lx=lx, ly=ly: W.lampione(im, lx, ly, frame)))
    oggetti.append((84, lambda: W.recinto(im, 140, 74, 40)))
    oggetti.append((112, lambda: W.cartello(im, 52, 98)))
    for (rx, ry, g) in ((182, 160, True), (70, 116, False), (288, 120, False)):
        oggetti.append((ry + 8, lambda rx=rx, ry=ry, g=g: W.roccia(im, rx, ry, g)))
    for (bx, by) in ((172, 86), (40, 78), (132, 160)):
        oggetti.append((by + 13, lambda bx=bx, by=by: W.cespuglio(im, bx, by)))
    oggetti.append((134, lambda: fontana(im, 160, 128, frame)))
    pers = [(152, 140, W.ABITO_GIOCATORE), (232, 82, W.ABITO_KAEL), (188, 118, W.ABITO_ABITANTE), (110, 112, W.ABITO_ELIAN)]
    for (cx, cy, ab) in pers:
        oggetti.append((cy + 23, lambda cx=cx, cy=cy, ab=ab: W.personaggio(im, cx, cy, ab, frame)))
    for _, f in sorted(oggetti, key=lambda o: o[0]):
        f()
    grading(im, caldo=0.5, vignetta=0.25)
    ui.bagliore(im, 168, 116, 22, P.LUMEN[3], 40)
    return im


def rosa_lumen(im, cx, cy):
    """Intarsio a stella a otto punte (oro e cristallo) sul lastricato: simbolo di Lunaria"""
    for yy in range(-30, 31):
        for xx in range(-44, 45):
            d = math.hypot(xx / 1.45, yy)
            ang = math.atan2(yy, xx / 1.45)
            punta = 22 + 8 * abs(math.cos(ang * 4)) ** 6
            if 26.5 <= d <= 28.5:
                px(im, cx + xx, cy + yy, P.ORO[2] if d < 27.6 else P.ORO[1])
            elif d < punta and d > 18 and abs(math.cos(ang * 4)) > 0.92:
                px(im, cx + xx, cy + yy, P.mix(P.PIETRA[4], P.LUMEN[2], 0.35))


def fontana(im, cx, cy, frame=0):
    """Fonte del Lumen nella piazza: vasca di pietra, acqua, pilastro con cristallo"""
    W.ombra_ellisse(im, cx + 2, cy + 6, 17, 5, 70)
    for yy in range(-8, 7):
        for xx in range(-16, 17):
            d = (xx / 16) ** 2 + (yy / 8) ** 2
            if d <= 1:
                px(im, cx + xx, cy + yy, P.PIETRA[4] if yy < 0 else P.PIETRA[3])
            if d <= 0.62:
                px(im, cx + xx, cy + yy, P.ACQUA[3] if (xx + yy + frame) % 5 else P.ACQUA[4])
    for k in range(-16, 17):
        yy = int(8 * math.sqrt(max(0, 1 - (k / 16) ** 2)))
        px(im, cx + k, cy + yy, P.PIETRA[1]); px(im, cx + k, cy + yy + 1, P.PIETRA[1])
    rect(im, cx - 3, cy - 14, 6, 14, P.PIETRA[3]); vline(im, cx - 3, cy - 14, 14, P.PIETRA[4]); vline(im, cx + 2, cy - 14, 14, P.PIETRA[2])
    rect(im, cx - 4, cy - 16, 8, 2, P.ORO[2])
    ui.cristallo(im, cx, cy - 31, 15, bagliore_r=20)
    scintille(im, 10, (cx - 14, cy - 34, cx + 14, cy), P.LUMEN[4], seme=frame)


def banner_luogo(im, nome, sottotitolo):
    w = max(F.larghezza_grassetto(nome), F.larghezza(sottotitolo)) + 30
    ui.cornice(im, 4, 4, w, 28, gemme=False)
    ui.cristallo(im, 13, 9, 9, bagliore_r=6)
    ui.titolo_oro(im, nome, 21, 9)
    F.testo(im, sottotitolo, 21, 20, P.GRIGIO_UI)


def schermata_overworld():
    im = scena_lunaria()
    banner_luogo(im, "LUNARIA", "Villaggio del Lumen")
    # suggerimento d'interazione contestuale
    ui.cornice(im, LW - 70, LH - 18, 66, 15, gemme=False)
    ui.badge_tipo  # (riferimento)
    F.micro(im, "Z", LW - 64, LH - 13, P.ORO[4])
    F.testo(im, "Parla", LW - 56, LH - 14, P.CREMA)
    return im


# ================================================================ BATTAGLIA

def sfondo_lotta(im, bioma="prato", frame=0):
    """Fondale illustrato: cielo, montagne, bosco, prato, raggi di luce, pulviscolo di Lumen"""
    temi = {
        "prato":   dict(cielo=(P.H("6FB6F0"), P.H("F4E6C2")), monti=P.H("7C8EC8"), bosco=P.ERBA[1], suolo=(P.ERBA[3], P.ERBA[4]), luce=P.H("FFF2C0")),
        "arena":   dict(cielo=(P.H("2E6A5A"), P.H("CDEBA8")), monti=P.H("3E7A5A"), bosco=P.ERBA[0], suolo=(P.H("5E8A4A"), P.H("8AB060")), luce=P.H("E8FFC8")),
        "vuoto":   dict(cielo=(P.H("05030E"), P.H("2A1450")), monti=P.H("140A2A"), bosco=P.H("0A0618"), suolo=(P.H("1A1030"), P.H("2E1C50")), luce=P.H("9C7CFF")),
        "nival":   dict(cielo=(P.H("8EC6F0"), P.H("F0F8FF")), monti=P.H("A8C4E8"), bosco=P.H("3A6A8A"), suolo=(P.H("D6E8F6"), P.H("F4FAFF")), luce=P.H("FFFFFF")),
    }
    t = temi[bioma]
    ui.gradiente_v(im, 0, 0, LW, 70, t["cielo"][0], t["cielo"][1])
    if bioma == "vuoto":
        r = W.rnd("stelle")
        for _ in range(90):
            px(im, r.randrange(LW), r.randrange(80), P.con_alfa((220, 210, 255, 255), r.randrange(60, 230)))
        for x in range(LW):   # nebulosa
            for y in range(10, 70):
                v = math.sin(x * 0.04 + y * 0.09) + math.sin(x * 0.013 - y * 0.05)
                if v > 1.2:
                    px(im, x, y, P.con_alfa(P.H("8E3AC8"), int((v - 1.2) * 70)))
    else:
        for (nx, ny, nw) in ((40, 14, 46), (190, 22, 60), (280, 10, 34)):   # nuvole
            for k in range(nw):
                hh = int(4 + 3 * math.sin(k / nw * math.pi))
                vline(im, nx + k, ny - hh, hh, P.con_alfa((255, 255, 255, 255), 140))
                px(im, nx + k, ny, P.con_alfa(t["cielo"][0], 90))
    # montagne lontane
    for x in range(LW):
        h = 46 + int(9 * math.sin(x * 0.021 + 1) + 6 * math.sin(x * 0.057))
        vline(im, x, h, 72 - h, t["monti"])
        if (x * 7) % 13 == 0:
            px(im, x, h, P.mix(t["monti"], (255, 255, 255, 255), 0.4))
    # bosco: sagome di chiome
    for x in range(LW):
        h = 60 + int(4 * math.sin(x * 0.19) + 3 * math.sin(x * 0.47 + 2))
        vline(im, x, h, 76 - h, t["bosco"])
        px(im, x, h, P.mix(t["bosco"], (255, 255, 255, 255), 0.25))
    # prato con profondita' (righe piu' fitte in lontananza)
    ui.gradiente_v(im, 0, 74, LW, LH - 74, t["suolo"][0], t["suolo"][1])
    r = W.rnd("prato", bioma)
    for _ in range(260):
        x, y = r.randrange(LW), r.randrange(76, LH)
        px(im, x, y, P.mix(t["suolo"][0], (0, 0, 0, 255), 0.25))
        px(im, x + 1, y - 1, P.mix(t["suolo"][1], (255, 255, 255, 255), 0.25)) if r.random() < 0.4 else None
    # raggi di luce dall'alto a sinistra
    for k, (x0, w0) in enumerate(((10, 18), (60, 10), (120, 24), (190, 12))):
        for y in range(0, 120):
            for i in range(w0):
                x = x0 + i + y * 0.55
                px(im, int(x), y, P.con_alfa(t["luce"], 16 if bioma != "vuoto" else 10))
    scintille(im, 16, (0, 0, LW, 130), P.con_alfa(P.LUMEN[4], 200), seme=7)


def cerchio_runico(im, cx, cy, rx, ry, colore, frame=0):
    """Pedana di lotta: disco di terra battuta con anello di pietra e rune di Lumen accese"""
    for yy in range(-ry - 2, ry + 3):
        for xx in range(-rx - 2, rx + 3):
            d = (xx / rx) ** 2 + (yy / ry) ** 2
            if d <= 1.0:
                c = P.mix(P.TERRA[2], P.TERRA[3], 0.5 + 0.5 * (-yy / ry))
                px(im, cx + xx, cy + yy, c)
            elif d <= 1.18:
                px(im, cx + xx, cy + yy, P.PIETRA[2] if yy > 0 else P.PIETRA[4])
    for k in range(24):
        a = k / 24 * math.tau + frame * 0.02
        x, y = cx + math.cos(a) * rx * 0.82, cy + math.sin(a) * ry * 0.82
        if k % 3 == 0:
            px(im, int(x), int(y), colore[4]); px(im, int(x) + 1, int(y), colore[3])
        else:
            px(im, int(x), int(y), P.con_alfa(colore[3], 150))
    for k in range(-rx + 4, rx - 4):
        yy = ry * 0.45 * math.sqrt(max(0, 1 - (k / (rx - 4)) ** 2))
        px(im, cx + k, int(cy + yy), P.con_alfa(colore[2], 90))


def targa_hud(im, x, y, w, nome, lv, tipi, hp, hpmax, numeri=False, xp=None, stato=None, specchio=False):
    h = 34 if numeri else 26
    # piastra con lato obliquo
    for j in range(h):
        taglio = (h - j) // 2 if not specchio else j // 2
        x0 = x + (0 if not specchio else taglio)
        ww = w - taglio
        hline(im, x0, y + j, ww, P.NOTTE[0] if j in (0, h - 1) else P.mix(P.NOTTE[3], P.NOTTE[1], j / h))
        px(im, x0, y + j, P.NOTTE[0]); px(im, x0 + ww - 1, y + j, P.NOTTE[0])
        px(im, x0 + ww - 2, y + j, P.ORO[1]) if not specchio else px(im, x0 + 1, y + j, P.ORO[3])
    hline(im, x + 1, y + 1, w - 2 - (h // 2 if not specchio else 0), P.ORO[3])
    hline(im, x + 1, y + h - 2, w - 2 - (0 if not specchio else 0), P.ORO[1])
    ox = x + (14 if specchio else 5)
    tx = ox
    for t in tipi:
        ui.badge_tipo(im, t, tx, y + 4)
        tx += 12
    F.testo(im, nome, tx + 2, y + 5, P.CREMA, ombra=P.OMBRA_TESTO)
    lvx = x + w - (30 if not specchio else 20) - (h // 2 if not specchio else 0)
    F.micro(im, "LV", lvx, y + 7, P.ORO[3])
    F.testo(im, str(lv), lvx + 9, y + 5, P.ORO[4], ombra=P.OMBRA_TESTO)
    f = hp / hpmax
    bx = ox
    ui.barra(im, bx, y + 18, w - 40 - (h // 2 if not specchio else 0), f, ui.rampa_hp(f), "HP")
    if stato:
        ui.pillola_stato(im, stato, x + w - 30 - (h // 2 if not specchio else 4), y + 16)
    if numeri:
        s = f"{hp}/{hpmax}"
        F.testo(im, s, x + w - 10 - F.larghezza(s) - (0 if specchio else 0), y + 24, P.CREMA, ombra=P.OMBRA_TESTO)
        if xp is not None:
            F.micro(im, "XP", ox, y + 27, P.LUMEN[3])
            ui.barra(im, ox + 10, y + 28, w - 76, xp, P.XP, h=2)
    return h


def pannello_comandi(im, messaggio, scelta=0, frame=0):
    y = 134
    ui.cornice(im, 0, y, 156, LH - y, stile="pergamena")
    for i, riga in enumerate(F.a_capo(messaggio, 138)[:3]):
        F.testo(im, riga, 10, y + 9 + i * 11, P.INCHIOSTRO)
    ui.cornice(im, 156, y, LW - 156, LH - y)
    voci = [("LOTTA", ui.icona_spada), ("ELYON", ui.icona_elyon), ("ZAINO", ui.icona_zaino), ("FUGA", ui.icona_fuga)]
    for i, (et, ic) in enumerate(voci):
        bx, by = 168 + (i % 2) * 74, y + 6 + (i // 2) * 20
        ui.pulsante(im, bx, by, 68, 17, et, ic, attivo=(i == scelta), frame=frame)


def schermata_lotta():
    im = nuovo()
    sfondo_lotta(im, "prato")
    cerchio_runico(im, 236, 82, 46, 10, P.LUMEN)
    cerchio_runico(im, 84, 132, 62, 13, P.ORO)
    im.alpha_composite(elyon("AQUIL"), (204, 22))
    im.alpha_composite(elyon("PYRIN", retro=True, scala=1.25), (40, 54))
    targa_hud(im, 6, 10, 132, "AQUIL", 5, ["ACQUA"], 15, 20)
    targa_hud(im, 178, 92, 138, "PYRIN", 5, ["FIAMMA"], 20, 20, numeri=True, xp=0.35, specchio=True)
    pannello_comandi(im, "Un AQUIL selvatico sbuca dall'acqua! Cosa farà PYRIN?")
    return im


def schermata_mosse():
    im = nuovo()
    sfondo_lotta(im, "prato")
    cerchio_runico(im, 236, 82, 46, 10, P.LUMEN)
    cerchio_runico(im, 84, 132, 62, 13, P.ORO)
    im.alpha_composite(elyon("AQUIL"), (204, 22))
    im.alpha_composite(elyon("PYRIN", retro=True, scala=1.25), (40, 54))
    targa_hud(im, 6, 10, 132, "AQUIL", 5, ["ACQUA"], 15, 20)
    targa_hud(im, 178, 92, 138, "PYRIN", 5, ["FIAMMA"], 20, 20, numeri=True, xp=0.35, specchio=True)
    y = 134
    ui.cornice(im, 0, y, 212, LH - y)
    mosse = [("BRACIAVIVA", "FIAMMA", 25, 25), ("GRAFFIO", "NEUTRO", 35, 35), ("MARCHIO FUOCO", "FIAMMA", 20, 20), ("RAPIDITÀ", "NEUTRO", 30, 30)]
    for i, (nome, tipo, pp, ppm) in enumerate(mosse):
        cx, cy = 8 + (i % 2) * 101, y + 6 + (i // 2) * 19
        carta_mossa(im, cx, cy, 97, nome, tipo, pp, ppm, attiva=(i == 0))
    ui.cornice(im, 212, y, LW - 212, LH - y)
    ui.pillola_tipo(im, "FIAMMA", 220, y + 7, 60)
    F.micro(im, "SPECIALE", 284, y + 9, P.GRIGIO_UI)
    F.micro(im, "POT", 222, y + 22, P.ORO[3]); F.testo(im, "40", 236, y + 20, P.CREMA)
    F.micro(im, "PRE", 260, y + 22, P.ORO[3]); F.testo(im, "100", 274, y + 20, P.CREMA)
    # efficacia contro il bersaglio, con simbolo oltre al colore
    rect(im, 220, y + 32, 92, 10, P.NOTTE[0])
    F.testo(im, "POCO EFFICACE", 223, y + 33, P.HP_MEDIO[2])
    for k in range(2):
        px(im, 304 + k * 0, y + 35 + k, P.HP_MEDIO[2])
    ui.hline(im, 302, y + 35, 5, P.HP_MEDIO[2]); ui.hline(im, 303, y + 36, 3, P.HP_MEDIO[2]); px(im, 304, y + 37, P.HP_MEDIO[2])
    return im


def carta_mossa(im, x, y, w, nome, tipo, pp, ppm, attiva=False):
    sc, base, chiaro = P.TIPI[tipo]
    h = 17
    rect(im, x, y, w, h, P.NOTTE[0])
    ui.gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.NOTTE[3] if attiva else P.NOTTE[2], P.NOTTE[1])
    rect(im, x + 1, y + 1, 3, h - 2, base)
    vline(im, x + 1, y + 1, h - 2, chiaro)
    if attiva:
        hline(im, x, y, w, P.ORO[4]); hline(im, x, y + h - 1, w, P.ORO[2])
        vline(im, x, y, h, P.ORO[3]); vline(im, x + w - 1, y, h, P.ORO[2])
        ui.cursore(im, x - 6, y + h // 2)
    F.testo(im, nome, x + 7, y + 2, P.ORO[4] if attiva else P.CREMA, ombra=P.OMBRA_TESTO)
    F.micro(im, "PP", x + 7, y + 11, P.GRIGIO_UI)
    F.micro(im, f"{pp}/{ppm}", x + 16, y + 11, P.CREMA)
    ui.badge_tipo(im, tipo, x + w - 13, y + 3)


# ================================================================ elementi comuni

def logo(im, cx, y, scala=3):
    """Logo ECLYRA: lettere del font corpo ingrandite a blocchi interi, oro inciso,
    contorno blu notte, cristallo e linea dell'Equilibrio."""
    testo = "ECLYRA"
    w = (F.larghezza(testo) + len(testo)) * scala
    x0 = cx - w // 2
    strato = Image.new("RGBA", im.size, (0, 0, 0, 0))
    x = x0
    for ch in testo:
        g = F.glifo(ch)
        for gy, riga in enumerate(g[:7]):
            for gx, v in enumerate(riga):
                if v:
                    for dy in range(scala):
                        for dx in range(scala + 1):
                            yy = gy * scala + dy
                            c = ui.oro_verticale(int(yy / (7 * scala) * 8))
                            if dx == 0 and dy == 0:
                                c = P.ORO[5]
                            px(strato, x + gx * scala + dx, y + yy, c)
        x += (len(g[0]) + 2) * scala
    a = strato.load()
    contorno = Image.new("RGBA", im.size, (0, 0, 0, 0))
    for yy in range(max(0, y - 3), min(im.height, y + 7 * scala + 4)):
        for xx in range(max(0, x0 - 3), min(im.width, x0 + w + 6)):
            if a[xx, yy][3] == 0:
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (-1, 1), (0, 2), (1, 2)):
                    nx, ny = xx + dx, yy + dy
                    if 0 <= nx < im.width and 0 <= ny < im.height and a[nx, ny][3]:
                        contorno.putpixel((xx, yy), P.NOTTE[0] if dy < 2 else P.ORO[0])
                        break
    ui.bagliore(im, cx, y + 10, 70, P.AZZURRO[2], 45)
    im.alpha_composite(contorno)
    im.alpha_composite(strato)
    # linea dell'Equilibrio con cristallo centrale
    ly = y - 7
    for k in range(-w // 2 - 6, w // 2 + 7):
        px(im, cx + k, ly, P.ORO[3] if abs(k) < w // 2 - 4 else P.ORO[2])
    for s in (-1, 1):
        ui.gemma(im, cx + s * (w // 2 + 8), ly)
    ui.cristallo(im, cx, ly - 9, 17, bagliore_r=18)


def menu_verticale(im, x, y, w, voci, scelta, disabilitate=(), passo=14, icone=None):
    for i, v in enumerate(voci):
        yy = y + i * passo
        if i == scelta:
            ui.riga_selezionata(im, x, yy - 2, w, passo - 2)
            ui.cursore(im, x + 3, yy + 3)
        col = P.GRIGIO_UI if i in disabilitate else (P.ORO[4] if i == scelta else P.CREMA)
        tx = x + 12
        if icone:
            icone[i](im, x + 11, yy - 2)
            tx = x + 26
        F.testo(im, v, tx, yy, col, ombra=P.OMBRA_TESTO)


def dialogo(im, nome, testo, y=124):
    ui.cornice(im, 6, y, LW - 12, LH - y - 4, stile="pergamena")
    if nome:
        ui.intestazione(im, 16, y - 7, nome)
    for i, riga in enumerate(F.a_capo(testo, LW - 36)[:3]):
        F.testo(im, riga, 18, y + 10 + i * 11, P.INCHIOSTRO)
    ui.cristallo(im, LW - 20, LH - 18, 7)


def oscura(im, a=120, colore=None):
    c = colore or P.NOTTE[0]
    strato = Image.new("RGBA", im.size, (c[0], c[1], c[2], a))
    im.alpha_composite(strato)


# ================================================================ 1. TITOLO

def schermata_titolo(frame=0):
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, LH, P.NOTTE[0], P.H("3A3A8A"))
    r = W.rnd("titolo")
    for _ in range(140):
        x, y = r.randrange(LW), r.randrange(110)
        px(im, x, y, P.con_alfa((230, 230, 255, 255), r.randrange(70, 255)))
    # aurora di Lumen
    for x in range(LW):
        for k, (col, fase, amp, base) in enumerate(((P.LUMEN[3], 0.0, 10, 38), (P.TIPI["ASTRALE"][1], 1.7, 8, 52))):
            yc = base + amp * math.sin(x * 0.018 + fase) + 4 * math.sin(x * 0.051 + fase * 2)
            for dy in range(-10, 11):
                a = int(55 * (1 - abs(dy) / 10) * (0.6 + 0.4 * math.sin(x * 0.03 + fase)))
                if a > 0:
                    px(im, x, int(yc + dy), P.con_alfa(col, a))
    # Torre dell'Equilibrio (Elys) in lontananza
    tx = 236
    for j in range(70):
        larg = max(1, int(1 + j * 0.12))
        hline(im, tx - larg, 40 + j, 2 * larg + 1, P.H("2A2E6A"))
    ui.bagliore(im, tx, 40, 26, P.LUMEN[3], 90)
    ui.cristallo(im, tx, 28, 13)
    # montagne a strati
    for strato, (colore, base, amp, f) in enumerate(((P.H("262A62"), 96, 14, 0.017), (P.H("1A1E4A"), 112, 10, 0.031), (P.H("10142E"), 130, 8, 0.05))):
        for x in range(LW):
            h = base + int(amp * math.sin(x * f + strato * 2) + amp * 0.5 * math.sin(x * f * 2.7))
            vline(im, x, h, LH - h, colore)
            if strato == 0 and (x * 5) % 9 == 0:
                px(im, x, h, P.H("6A72C8"))
    # villaggio di Lunaria con finestre calde
    for (hx, hw) in ((30, 18), (52, 14), (70, 22), (100, 16)):
        rect(im, hx, 138, hw, 14, P.H("0C1028"))
        for k in range(hw // 2 + 2):
            hline(im, hx - 2 + k, 138 - k // 2 + 0, hw + 4 - 2 * k, P.H("0C1028")) if k < hw // 2 else None
        px(im, hx + 4, 144, P.H("FFD27A")); px(im, hx + 5, 144, P.H("FFD27A"))
        ui.bagliore(im, hx + 5, 144, 5, P.H("FFC060"), 60)
    # promontorio con il protagonista e Pyrin
    for x in range(150, LW):
        h = 150 + int(6 * math.sin((x - 150) * 0.03)) - (x - 150) // 12
        vline(im, x, h, LH - h, P.H("080A1C"))
    W.personaggio(im, 266, 112, W.ABITO_GIOCATORE)
    im.alpha_composite(elyon("PYRIN", scala=0.5), (240, 108))
    logo(im, LW // 2 - 20, 30, scala=4)
    F.testo(im, "LE CRONACHE DELL'EQUILIBRIO", LW // 2 - 20 - F.larghezza("LE CRONACHE DELL'EQUILIBRIO") // 2, 64, P.AZZURRO[3], ombra=P.NOTTE[0])
    voci = ["NUOVA PARTITA", "CONTINUA", "OPZIONI", "ESCI"]
    ui.cornice(im, 86, 84, 104, 64)
    menu_verticale(im, 92, 92, 92, voci, 0)
    F.micro(im, "V 0.5.0", 4, LH - 8, P.GRIGIO_UI)
    F.micro(im, "PREMI Z", LW - 34, LH - 8, P.ORO[3])
    return im


# ================================================================ 3. DIALOGO

def schermata_dialogo():
    im = scena_lunaria()
    dialogo(im, "KAEL", "Asterra è molto più grande di quello che pensi, {G}. Questo è solo l'inizio!".replace("{G}", "ARI"))
    return im


# ================================================================ 4. MENU PRINCIPALE

def schermata_menu():
    im = scena_lunaria()
    oscura(im, 90)
    ui.cornice(im, 4, 4, 140, 30)
    F.testo(im, "ROUTE 1", 12, 11, P.ORO[4], ombra=P.OMBRA_TESTO)
    F.micro(im, "TEMPO 02:14", 12, 23, P.GRIGIO_UI)
    ui.icona_core(im, 80, 19)
    F.testo(im, "12.480", 94, 20, P.CREMA, ombra=P.OMBRA_TESTO)
    F.micro(im, "LUMEN", 118, 11, P.LUMEN[3])
    ui.cornice(im, LW - 112, 4, 108, LH - 8, titolo="MENU")
    voci = ["ELYON", "BESTIARIO", "ZAINO", "SCHEDA", "MAPPA", "SALVA", "OPZIONI"]
    icone = [ui.icona_elyon, icona_libro, ui.icona_zaino, icona_scheda, icona_mappa, icona_salva, icona_opzioni]
    menu_verticale(im, LW - 106, 22, 96, voci, 0, passo=20, icone=icone)
    return im


def icona_libro(im, x, y):
    ui._icona(im, x, y, [".aaaa.aaaa.", "abbbbabbbba", "abccbabccba", "abbbbabbbba", "abccbabccba", "abbbbabbbba",
                         "abccbabccba", "abbbbabbbba", ".aaaadaaaa.", ".....d.....", "..........."],
              {"a": P.ORO[1], "b": P.PERGAMENA[3], "c": P.PERGAMENA[1], "d": P.LUMEN[3]})


def icona_scheda(im, x, y):
    ui._icona(im, x, y, ["aaaaaaaaaaa", "abbbbbbbbba", "abccbdddbba", "abccbbbbbba", "abeebdddbba", "abeebbbbbba",
                         "abbbbdddbba", "abbbbbbbbba", "aaaaaaaaaaa", "...........", "..........."],
              {"a": P.ORO[1], "b": P.NOTTE[3], "c": P.PELLE[2], "e": P.LUMEN[2], "d": P.CREMA})


def icona_mappa(im, x, y):
    ui._icona(im, x, y, ["aaa.aaa.aaa", "abbabbbabba", "abbacbbabba", "abbabcbabba", "abbabbcabba", "abcabbbacba",
                         "abbcbbbabba", "abbabbbabba", "aaa.aaa.aaa", "...........", "..........."],
              {"a": P.TERRA[1], "b": P.PERGAMENA[3], "c": P.TIPI["FIAMMA"][1]})


def icona_salva(im, x, y):
    ui.cristallo(im, x + 5, y, 11)
    ui.hline(im, x + 1, y + 10, 9, P.ORO[2])


def icona_opzioni(im, x, y):
    ui._icona(im, x, y, ["....a.a....", "..aaaaaaa..", ".aabbbbbaa.", "aabbcccbbaa", ".abccccbba.", "aabccdccbaa",
                         ".abccccbba.", "aabbcccbbaa", ".aabbbbbaa.", "..aaaaaaa..", "....a.a...."],
              {"a": P.PIETRA[3], "b": P.PIETRA[4], "c": P.NOTTE[2], "d": P.LUMEN[3]})


# ================================================================ 5. SQUADRA

SQUADRA = [("PYRION", 16, ["FIAMMA"], 48, 48, None), ("AQUIL", 14, ["ACQUA"], 45, 45, None),
           ("MOSSEN", 13, ["NATURA"], 30, 41, "VEL"), ("LUMI", 12, ["ASTRALE"], 38, 38, None),
           ("FROLI", 11, ["GELO"], 9, 36, None), ("SHADEK", 13, ["OMBRA"], 0, 40, None)]


def schermata_squadra():
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, LH, P.NOTTE[2], P.NOTTE[0])
    for k in range(0, LW + LH, 12):   # trama a losanghe
        for t in range(LH):
            x = k - t
            if 0 <= x < LW and (t // 3) % 2 == 0:
                px(im, x, t, P.con_alfa(P.NOTTE[3], 40))
    ui.intestazione(im, 6, 4, "SQUADRA")
    F.micro(im, "6/6", LW - 22, 8, P.ORO[3])
    for i, (nome, lv, tipi, hp, hpm, st) in enumerate(SQUADRA):
        cx, cy = 6 + (i % 2) * 157, 22 + (i // 2) * 48
        carta_elyon(im, cx, cy, 151, 44, nome, lv, tipi, hp, hpm, st, attiva=(i == 0))
    ui.cornice(im, 6, LH - 18, LW - 12, 15, gemme=False)
    F.testo(im, "Scegli un Elyon.", 14, LH - 14, P.CREMA)
    F.micro(im, "Z SCEGLI   X INDIETRO", LW - 96, LH - 12, P.ORO[3])
    return im


def carta_elyon(im, x, y, w, h, nome, lv, tipi, hp, hpm, stato, attiva=False):
    esausto = hp == 0
    rect(im, x, y, w, h, P.NOTTE[0])
    ui.gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.NOTTE[3] if attiva else P.NOTTE[2], P.NOTTE[1])
    if attiva:
        ui._bordo_oro(im, x, y, w, h)
        ui.cursore(im, x - 4, y + h // 2)
    else:
        hline(im, x + 1, y + 1, w - 2, P.NOTTE[4])
    # medaglione con lo sprite
    mx, my = x + 4, y + 4
    for yy in range(36):
        for xx in range(36):
            d = math.hypot(xx - 17.5, yy - 17.5)
            if d <= 17.5:
                px(im, mx + xx, my + yy, P.mix(P.NOTTE[1], P.TIPI[tipi[0]][0], 0.35 + 0.3 * (1 - d / 17.5)))
            elif d <= 18.5:
                px(im, mx + xx, my + yy, P.ORO[2] if attiva else P.NOTTE[4])
    spr = elyon(nome, scala=0.5)
    if esausto:
        spr = spr.convert("LA").convert("RGBA")
    im.alpha_composite(spr, (mx + 2, my + 2))
    tx = x + 44
    F.testo(im, nome, tx, y + 5, P.ORO[4] if attiva else P.CREMA, ombra=P.OMBRA_TESTO)
    F.micro(im, "LV", x + w - 26, y + 7, P.ORO[3])
    F.testo(im, str(lv), x + w - 17, y + 5, P.CREMA, ombra=P.OMBRA_TESTO)
    bx = tx
    for t in tipi:
        ui.badge_tipo(im, t, bx, y + 15)
        bx += 13
    if stato:
        ui.pillola_stato(im, stato, bx + 2, y + 16)
    if esausto:
        ui.pillola_stato(im, "SON", bx + 2, y + 16)
        rect(im, bx + 3, y + 17, 18, 7, P.HP_BASSO[1]); F.micro(im, "KO", bx + 6, y + 18, (255, 255, 255, 255))
    f = hp / hpm
    ui.barra(im, tx, y + 31, w - 60, f, ui.rampa_hp(f), "HP")
    s = f"{hp}/{hpm}"
    F.micro(im, s, x + w - 6 - F.larghezza_micro(s), y + 37, P.CREMA)


# ================================================================ 6. ZAINO

OGGETTI = [("CORE", 10, ui.icona_core, "Nucleo di Lumen per catturare Elyon selvatici. Affidabile ed elegante."),
           ("ERBA VITALE", 5, None, ""), ("FRUTTO LUMEN", 3, None, ""), ("ANTIDOTO", 2, None, ""),
           ("REVITA", 1, None, ""), ("PIETRA FIAMMA", 1, None, ""), ("PIETRA ACQUA", 1, None, ""), ("PIETRA NATURA", 1, None, "")]


def icona_oggetto(im, nome, x, y):
    if nome == "CORE":
        return ui.icona_core(im, x, y)
    if nome.startswith("PIETRA"):
        tipo = nome.split()[1]
        pal = [P.TIPI[tipo][0], P.TIPI[tipo][0], P.TIPI[tipo][1], P.TIPI[tipo][2], P.H("FFFFFF")]
        return ui.cristallo(im, x + 5, y, 11, pal=pal)
    col = {"ERBA VITALE": P.ERBA, "FRUTTO LUMEN": [P.H("5A1A3A"), P.H("9A2E5A"), P.H("E8508A"), P.H("FF9AC0"), P.H("FFE0EC"), P.H("FFFFFF")],
           "ANTIDOTO": [P.H("3A1A5A"), P.H("5E2E8A"), P.H("A04FD0"), P.H("D29AF2"), P.H("F2E0FF"), P.H("FFFFFF")],
           "REVITA": P.ORO}[nome]
    if nome == "ERBA VITALE":
        ui._icona(im, x, y, ["......aa...", "....aabba..", "...abbcba..", "..abbcbba..", ".abbcbba...", ".abcbba....",
                             "..acba.....", "..dd.......", ".d.........", "...........", "..........."],
                  {"a": col[1], "b": col[3], "c": col[4], "d": P.LEGNO[2]})
    elif nome == "REVITA":
        ui._icona(im, x, y, ["....a......", "...aba.....", "..abcba....", ".abcdcba...", "..abcba....", "...aba..a..",
                             "....a..aba.", ".......abca", "........aba", ".........a.", "..........."],
                  {"a": col[1], "b": col[2], "c": col[3], "d": col[5]})
    else:
        ui._icona(im, x, y, ["....dd.....", "...d.......", "..aaaaa....", ".abbbbba...", "abbccbbba..", "abcccbbba..",
                             "abbcbbbba..", "abbbbbbba..", ".abbbbba...", "..aaaaa....", "..........."],
                  {"a": col[1], "b": col[2], "c": col[4], "d": P.ERBA[2]})


def schermata_zaino():
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, LH, P.NOTTE[2], P.NOTTE[0])
    ui.intestazione(im, 6, 4, "ZAINO")
    # schede delle categorie
    cat = [ui.icona_zaino, ui.icona_core, None, None, None]
    for i in range(5):
        x = 90 + i * 22
        rect(im, x, 4, 20, 15, P.NOTTE[0])
        ui.gradiente_v(im, x + 1, 5, 18, 13, P.AZZURRO[1] if i == 0 else P.NOTTE[3], P.NOTTE[2])
        if i == 0:
            hline(im, x + 1, 5, 18, P.ORO[4])
    ui.icona_zaino(im, 94, 6); ui.icona_core(im, 116, 6); icona_oggetto(im, "PIETRA FIAMMA", 137, 6)
    ui._icona(im, 160, 6, ["...aaaa....", "..a....a...", "..a....a...", "...aaaa....", ".....a.....", ".....aa....", ".....a.....", ".....aa....", "...........", "...........", "..........."], {"a": P.ORO[3]})
    icona_oggetto(im, "REVITA", 182, 6)
    ui.cornice(im, 6, 22, 186, 132, stile="pergamena")
    for i, (nome, q, _, _) in enumerate(OGGETTI):
        y = 30 + i * 15
        if i == 0:
            rect(im, 10, y - 2, 178, 14, P.con_alfa(P.ORO[3], 90))
            hline(im, 10, y - 2, 178, P.ORO[2]); hline(im, 10, y + 11, 178, P.ORO[2])
            ui.cursore(im, 12, y + 4)
        icona_oggetto(im, nome, 20, y)
        F.testo(im, nome, 36, y + 2, P.INCHIOSTRO)
        s = f"×{q}"
        F.testo(im, s, 182 - F.larghezza(s), y + 2, P.INCHIOSTRO)
    ui.cornice(im, 196, 22, 118, 132)
    for yy in range(-20, 21):
        for xx in range(-20, 21):
            d = math.hypot(xx, yy)
            if d < 20:
                px(im, 255 + xx, 62 + yy, P.mix(P.NOTTE[3], P.NOTTE[1], d / 20))
    # Core grande (32 px): anello d'oro e cristallo
    for yy in range(-12, 13):
        for xx in range(-12, 13):
            d = math.hypot(xx, yy)
            if d <= 12:
                if d > 9.5:
                    c = P.ORO[4] if xx + yy < -4 else P.ORO[3] if xx + yy < 6 else P.ORO[1]
                else:
                    l = -(xx + yy) / 12
                    c = P.LUMEN[4] if l > 0.7 else P.LUMEN[3] if l > 0.1 else P.LUMEN[2] if l > -0.5 else P.LUMEN[1]
                px(im, 255 + xx, 62 + yy, c)
    ui.cristallo(im, 255, 53, 13)
    ui.bagliore(im, 255, 62, 22, P.LUMEN[3], 50)
    F.testo(im, "CORE", 255 - F.larghezza("CORE") // 2, 88, P.ORO[4], ombra=P.OMBRA_TESTO)
    for i, riga in enumerate(F.a_capo(OGGETTI[0][3], 100)):
        F.testo(im, riga, 204, 102 + i * 11, P.CREMA)
    ui.cornice(im, 6, LH - 22, LW - 12, 19, gemme=False)
    F.micro(im, "Z USA   X INDIETRO   <> CATEGORIA", 14, LH - 15, P.ORO[3])
    F.micro(im, "PREZZO 200", LW - 60, LH - 15, P.GRIGIO_UI)
    return im


# ================================================================ 7. BESTIARIO

def schermata_bestiario():
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, LH, P.NOTTE[2], P.NOTTE[0])
    ui.intestazione(im, 6, 4, "BESTIARIO")
    F.micro(im, "VISTI 28   CATTURATI 9", LW - 106, 8, P.ORO[3])
    ui.cornice(im, 6, 22, 128, 154)
    voci = [(1, "PYRIN", 2), (2, "PYRION", 2), (3, "PYROVAR", 0), (4, "AQUIL", 1), (5, "AQUILON", 0), (6, "AQUARYS", 0),
            (7, "MOSSEN", 1), (8, "MOSSKA", 0), (9, "VERDANTOR", 0), (10, "FLARO", 1)]
    for i, (n, nome, stato) in enumerate(voci):
        y = 30 + i * 14
        if i == 3:
            ui.riga_selezionata(im, 10, y - 2, 120, 12)
            ui.cursore(im, 12, y + 3)
        F.micro(im, f"{n:03d}", 20, y + 1, P.GRIGIO_UI)
        if stato == 2:
            ui.icona_core(im, 36, y - 2)
        elif stato == 1:
            px(im, 41, y + 3, P.GRIGIO_UI)
        F.testo(im, nome if stato else "?????", 50, y, P.ORO[4] if i == 3 else (P.CREMA if stato else P.GRIGIO_UI), ombra=P.OMBRA_TESTO)
    ui.cornice(im, 138, 22, 176, 154)
    cerchio_runico(im, 188, 86, 38, 9, P.AZZURRO)
    im.alpha_composite(elyon("AQUIL"), (156, 30))
    F.micro(im, "N. 004", 236, 32, P.GRIGIO_UI)
    ui.titolo_oro(im, "AQUIL", 236, 40)
    ui.pillola_tipo(im, "ACQUA", 236, 54, 60)
    F.testo(im, "Lontra-pinna", 236, 69, P.GRIGIO_UI)
    stats = [("HP", 48), ("ATK", 45), ("DEF", 48), ("SPA", 58), ("SPD", 52), ("SPE", 50)]
    for i, (s, v) in enumerate(stats):
        yy = 100 + i * 7
        F.micro(im, s, 146, yy, P.ORO[3])
        ui.barra(im, 162, yy + 1, 52, v / 160, P.XP, h=3)
    desc = "Nuota nei torrenti di Lunaria seguendo i riflessi del Lumen. Le sue pinne brillano quando è felice."
    for i, riga in enumerate(F.a_capo(desc, 92)[:6]):
        F.testo(im, riga, 220, 98 + i * 11, P.CREMA)
    return im


# ================================================================ 10. STATISTICHE

def schermata_statistiche():
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, LH, P.NOTTE[2], P.NOTTE[0])
    ui.intestazione(im, 6, 4, "PYRION")
    for i, t in enumerate(["INFO", "STATISTICHE", "MOSSE"]):
        x = 110 + i * 68
        rect(im, x, 5, 64, 13, P.NOTTE[0])
        ui.gradiente_v(im, x + 1, 6, 62, 11, P.AZZURRO[1] if i == 1 else P.NOTTE[3], P.NOTTE[2])
        if i == 1:
            hline(im, x + 1, 6, 62, P.ORO[4])
        F.testo(im, t, x + 32 - F.larghezza(t) // 2, 8, P.ORO[4] if i == 1 else P.CREMA)
    ui.cornice(im, 6, 22, 128, 154)
    cerchio_runico(im, 70, 92, 40, 9, P.ORO)
    im.alpha_composite(elyon("PYRION"), (38, 32))
    F.micro(im, "LV", 14, 108, P.ORO[3]); F.testo(im, "16", 24, 106, P.CREMA, ombra=P.OMBRA_TESTO)
    ui.pillola_tipo(im, "FIAMMA", 50, 106, 60)
    righe = [("NATURA", "Ardente"), ("ABILITÀ", "Fiamma interna"), ("OGGETTO", "Nucleo caldo"), ("AMICIZIA", "128")]
    for i, (k, v) in enumerate(righe):
        F.micro(im, k, 14, 122 + i * 12, P.ORO[3])
        F.testo(im, v, 54, 120 + i * 12, P.CREMA)
    ui.cornice(im, 136, 22, 178, 154)
    stats = [("HP", 48, 48, 0), ("ATTACCO", 52, 0, 1), ("DIFESA", 40, 0, -1), ("ATT. SPEC.", 47, 0, 0), ("DIF. SPEC.", 41, 0, 0), ("VELOCITÀ", 50, 0, 0)]
    for i, (s, v, _, nat) in enumerate(stats):
        y = 32 + i * 17
        col = P.TIPI["FIAMMA"][2] if nat > 0 else P.AZZURRO[3] if nat < 0 else P.CREMA
        F.testo(im, s, 146, y, col, ombra=P.OMBRA_TESTO)
        if nat:
            freccia(im, 198, y + 1, nat, col)
        F.testo(im, str(v), 226 - F.larghezza(str(v)), y, P.CREMA, ombra=P.OMBRA_TESTO)
        ui.barra(im, 232, y + 2, 72, v / 70, ui.rampa_hp(1) if s == "HP" else P.XP, h=4)
    ui.separatore(im, 146, 136, 158)
    F.micro(im, "XP", 146, 146, P.LUMEN[3]); F.testo(im, "4.096", 160, 144, P.CREMA)
    F.micro(im, "AL PROSSIMO", 204, 146, P.GRIGIO_UI); F.testo(im, "817", 254, 144, P.CREMA)
    ui.barra(im, 146, 158, 156, 0.42, P.XP, h=3)
    return im


def freccia(im, x, y, verso, col):
    for j in range(4):
        w = j if verso > 0 else 3 - j
        hline(im, x + 3 - w, y + j, 2 * w + 1, col)


# ================================================================ 11. EVOLUZIONE

def schermata_evoluzione():
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, LH, P.NOTTE[0], P.NOTTE[2])
    cx, cy = LW // 2, 70
    for k in range(32):   # raggi dal centro
        a = k / 32 * math.tau
        for d in range(20, 220):
            x, y = cx + math.cos(a) * d, cy + math.sin(a) * d * 0.8
            larg = 1 + d // 60
            for t in range(larg):
                px(im, int(x + t * math.sin(a)), int(y), P.con_alfa(P.LUMEN[3] if k % 2 else P.ORO[3], max(0, 70 - d // 3)))
    for r in (24, 38, 54):   # anelli di Lumen
        for k in range(360):
            a = math.radians(k)
            px(im, int(cx + math.cos(a) * r), int(cy + math.sin(a) * r * 0.5 + 18), P.con_alfa(P.LUMEN[4], 160 if k % 6 else 255))
    ui.bagliore(im, cx, cy, 60, P.LUMEN[3], 120)
    sil = elyon("PYRION")
    a = sil.load()
    for y in range(sil.height):
        for x in range(sil.width):
            if a[x, y][3] > 200:
                a[x, y] = (240, 255, 252, 255)
    im.alpha_composite(sil, (cx - 32, cy - 36))
    scintille(im, 40, (cx - 70, cy - 60, cx + 70, cy + 50), P.LUMEN[4], seme=3)
    dialogo(im, "", "Cosa? PYRIN si sta evolvendo!", y=128)
    F.micro(im, "X INTERROMPI", LW - 60, 6, P.GRIGIO_UI)
    return im


# ================================================================ 12. CATTURA

def schermata_cattura():
    im = nuovo()
    sfondo_lotta(im, "prato")
    cerchio_runico(im, 236, 82, 46, 10, P.LUMEN)
    cerchio_runico(im, 84, 132, 62, 13, P.ORO)
    im.alpha_composite(elyon("PYRIN", retro=True, scala=1.25), (40, 54))
    cx, cy = 236, 72
    for r, a in ((18, 200), (26, 130), (34, 70)):
        for k in range(0, 360, 3):
            ang = math.radians(k)
            px(im, int(cx + math.cos(ang) * r), int(cy + 6 + math.sin(ang) * r * 0.32), P.con_alfa(P.LUMEN[4], a))
    ui.bagliore(im, cx, cy, 26, P.LUMEN[3], 110)
    for yy in range(-8, 9):
        for xx in range(-8, 9):
            d = math.hypot(xx, yy)
            if d <= 8:
                c = (P.ORO[4] if xx + yy < -3 else P.ORO[2]) if d > 6 else (P.LUMEN[4] if xx + yy < -4 else P.LUMEN[3] if xx + yy < 3 else P.LUMEN[2])
                px(im, cx + xx + 2, cy + yy, c)
    ui.cristallo(im, cx + 2, cy - 7, 9)
    for k, (dx, dy) in enumerate(((-16, -12), (18, -10), (-20, 6), (22, 8))):
        ui.cristallo(im, cx + dx, cy + dy, 5)
    targa_hud(im, 6, 10, 132, "AQUIL", 5, ["ACQUA"], 3, 20)
    targa_hud(im, 178, 92, 138, "PYRIN", 5, ["FIAMMA"], 14, 20, numeri=True, xp=0.35, specchio=True)
    ui.cornice(im, 0, 134, LW, LH - 134, stile="pergamena")
    F.testo(im, "Lanci un CORE!   1...   2...", 12, 146, P.INCHIOSTRO)
    for i in range(3):
        x = 260 + i * 16
        ui.gemma(im, x, 151, P.LUMEN if i < 2 else [P.PIETRA[1], P.PIETRA[2], P.PIETRA[3], P.PIETRA[4], P.PIETRA[5]])
    return im


# ================================================================ 13. SALVATAGGIO

def schermata_salva():
    im = scena_lunaria()
    oscura(im, 150)
    ui.cornice(im, 40, 26, 240, 128, titolo="SALVA")
    ui.cornice(im, 52, 40, 216, 70, stile="pergamena")
    W.personaggio(im, 62, 50, W.ABITO_GIOCATORE)
    F.testo(im, "ARI", 86, 48, P.INCHIOSTRO)
    F.micro(im, "LUNARIA  TEMPO 02:14", 86, 60, P.PERGAMENA[0])
    for i, n in enumerate(["PYRION", "AQUIL", "MOSSEN", "LUMI", "FROLI", "SHADEK"]):
        im.alpha_composite(elyon(n, scala=0.5), (56 + i * 34, 72))
    for i in range(7):
        ui.gemma(im, 188 + i * 11, 52, [P.H("1E5E2A"), P.H("2E8A3E"), P.H("4CC85A"), P.H("A8F08A"), P.H("F0FFE0")] if i == 0 else [P.PIETRA[1], P.PIETRA[2], P.PIETRA[3], P.PIETRA[3], P.PIETRA[4]])
    F.testo(im, "Vuoi salvare i tuoi progressi?", 56, 116, P.CREMA, ombra=P.OMBRA_TESTO)
    ui.pulsante(im, 84, 130, 66, 16, "SALVA", icona_salva, attivo=True)
    ui.pulsante(im, 170, 130, 66, 16, "ANNULLA")
    return im


# ================================================================ 14. SCONFITTA (nessuna penalita')

def schermata_sconfitta():
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, LH, P.NOTTE[0], P.H("1A1440"))
    r = W.rnd("sconfitta")
    for _ in range(50):
        px(im, r.randrange(LW), r.randrange(LH), P.con_alfa(P.LUMEN[3], r.randrange(30, 120)))
    cx, cy = LW // 2, 66
    ui.bagliore(im, cx, cy, 34, P.LUMEN[2], 60)
    pal = [P.H("0E2E3A"), P.H("1E5A6A"), P.H("3A8A96"), P.H("6EC2C8"), P.H("B8F0F0")]
    ui.cristallo(im, cx, cy - 16, 31, pal=pal)
    for k in range(10):   # crepa
        px(im, cx - 2 + (k % 3) - 1, cy - 12 + k * 2, P.NOTTE[0])
    ui.titolo_oro(im, "IL LUMEN SI AFFIEVOLISCE", cx, 104, centrato=True)
    for i, riga in enumerate(["I tuoi Elyon sono esausti.", "Torni a casa per farli riposare: nessun progresso andrà perso."]):
        F.testo(im, riga, cx - F.larghezza(riga) // 2, 122 + i * 12, P.CREMA)
    ui.pulsante(im, cx - 40, 150, 80, 17, "CONTINUA", attivo=True)
    return im


# ================================================================ 15. MAPPA DI ASTERRA

REGIONI = [  # nome, sottotitolo, centro, colore base, chiaro
    ("NIVAL", "Gelo", (176, 40), P.H("B8D2EE"), P.H("F2F8FF")),
    ("ARKAS", "Roccia", (76, 74), P.H("C89A5A"), P.H("E8C890")),
    ("ELYS", "Astrale", (176, 84), P.H("6A5AC8"), P.H("B8B0FF")),
    ("VULKAR", "Fiamma", (262, 76), P.H("8A3A2A"), P.H("D2683E")),
    ("VERDAN", "Natura", (150, 122), P.H("3E8A3A"), P.H("8AC85A")),
    ("TALASSA", "Acqua", (250, 132), P.H("2E7AB8"), P.H("7CC0E8")),
    ("LUNARIA", "Inizio", (112, 140), P.H("6AAA5A"), P.H("B8E08A")),
]


def schermata_mappa():
    im = nuovo()
    for y in range(LH):
        for x in range(LW):
            v = math.sin(x * 0.11 + y * 0.07) + math.sin(x * 0.05 - y * 0.13)
            px(im, x, y, P.ACQUA[1] if v < 1.3 else P.ACQUA[2])
    # continente: unione di regioni con bordo frastagliato deterministico
    terra = {}
    for y in range(LH):
        for x in range(LW):
            migliore, dmin = None, 1e9
            for i, (_, _, (rx, ry), _, _) in enumerate(REGIONI):
                d = math.hypot((x - rx) / 1.15, y - ry) + 6 * math.sin(x * 0.17 + i) * math.cos(y * 0.13 - i)
                if d < dmin:
                    migliore, dmin = i, d
            bordo = 46 + 8 * math.sin(x * 0.09) + 6 * math.cos(y * 0.11 + x * 0.03)
            if dmin < bordo and 18 < y < 172 and 22 < x < 304:
                terra[(x, y)] = migliore
    for (x, y), i in terra.items():
        _, _, _, base, chiaro = REGIONI[i]
        c = base if (x + y * 3) % 7 else chiaro
        vic = [terra.get((x + dx, y + dy)) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
        if None in vic:
            c = P.PERGAMENA[3]
        elif any(v != i for v in vic):
            c = P.mix(base, (0, 0, 0, 255), 0.35)
        px(im, x, y, c)
    for (x, y), i in terra.items():   # costa: spuma
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            if (x + dx, y + dy) not in terra:
                px(im, x + dx, y + dy, P.ACQUA[4])
    # punti di riferimento disegnati (non rumore): montagne, vulcano, foreste, isole, isole sospese
    def monte(x, y, neve=True, h=8):
        for j in range(h):
            for i in range(-j, j + 1):
                c = P.PIETRA[4] if i < 0 else P.PIETRA[2]
                if neve and j < h // 3:
                    c = P.H("FFFFFF") if i <= 0 else P.H("C8D8EE")
                px(im, x + i, y - h + j, c)
        hline(im, x - h, y, 2 * h + 1, P.PIETRA[1])
    def abete(x, y):
        for j in range(5):
            hline(im, x - j // 2, y - 5 + j, j + 1 if j else 1, P.ERBA[1] if j > 2 else P.ERBA[2])
        px(im, x, y, P.LEGNO[1])
    for (x, y) in ((150, 34), (162, 30), (176, 28), (190, 31), (204, 36), (196, 46), (160, 46)):
        monte(x, y, True, 7)
    for (x, y) in ((54, 66), (66, 60), (92, 88), (60, 92)):
        monte(x, y, False, 6)
    for (x, y) in ((70, 80), (84, 70), (98, 66)):   # dune
        hline(im, x - 4, y, 9, P.TERRA[4]); hline(im, x - 2, y - 1, 5, P.TERRA[4])
    for (x, y) in ((262, 66), (282, 92)):   # vulcani
        monte(x, y, False, 9)
        rect(im, x - 1, y - 10, 3, 2, P.TIPI["FIAMMA"][1])
        ui.bagliore(im, x, y - 11, 6, P.TIPI["FIAMMA"][2], 120)
        for k in range(4):
            px(im, x + k % 2, y - 12 - k * 2, P.con_alfa(P.PIETRA[3], 160))
    for (x, y) in ((130, 112), (138, 118), (146, 110), (160, 116), (168, 128), (126, 128), (176, 118), (152, 134), (100, 130), (122, 146)):
        abete(x, y)
    for (x, y) in ((292, 150), (300, 138), (276, 160)):   # isolette di Talassa
        for yy in range(-2, 3):
            for xx in range(-4, 5):
                if (xx / 4) ** 2 + (yy / 2.5) ** 2 <= 1:
                    px(im, x + xx, y + yy, P.TERRA[4] if yy < 0 else P.ERBA[3])
        px(im, x, y - 3, P.ERBA[2])
    for (x, y) in ((160, 70), (196, 74), (186, 98)):   # isole sospese di Elys
        hline(im, x - 4, y, 9, P.H("D6D2FF")); hline(im, x - 3, y + 1, 7, P.TIPI["ASTRALE"][1]); hline(im, x - 1, y + 2, 3, P.TIPI["ASTRALE"][0])
    ui.bagliore(im, 176, 84, 30, P.TIPI["ASTRALE"][2], 70)
    ui.cristallo(im, 176, 72, 13)
    # rotte tratteggiate tra le citta'
    citta = [(112, 140), (150, 122), (76, 74), (176, 40), (250, 132), (262, 76), (176, 84)]
    for (a, b) in zip(citta, citta[1:]):
        n = int(math.hypot(b[0] - a[0], b[1] - a[1]))
        for k in range(0, n, 4):
            t = k / n
            px(im, int(a[0] + (b[0] - a[0]) * t), int(a[1] + (b[1] - a[1]) * t), P.ORO[4])
    for (x, y) in citta:
        for dy in range(-2, 3):
            for dx in range(-2, 3):
                if abs(dx) + abs(dy) <= 2:
                    px(im, x + dx, y + dy, P.ORO[3] if abs(dx) + abs(dy) == 2 else P.CREMA)
    ui.bagliore(im, 112, 140, 10, P.ORO[4], 120)
    for (nome, sott, (x, y), base, chiaro) in REGIONI:
        w = max(F.larghezza(nome), F.larghezza_micro(sott.upper())) + 8
        lx, ly = x - w // 2, y + 5
        rect(im, lx, ly, w, 17, P.NOTTE[0])
        rect(im, lx + 1, ly + 1, w - 2, 15, P.NOTTE[2])
        hline(im, lx + 1, ly + 1, w - 2, P.ORO[3])
        F.testo(im, nome, lx + 4, ly + 2, P.CREMA)
        F.micro(im, sott.upper(), lx + 4, ly + 10, chiaro)
    # rosa dei venti
    cx, cy = 292, 28
    for k in range(8):
        a = k / 8 * math.tau - math.pi / 2
        L = 10 if k % 2 == 0 else 5
        for d in range(L):
            px(im, int(cx + math.cos(a) * d), int(cy + math.sin(a) * d), P.ORO[4] if k == 0 else P.ORO[2])
    F.micro(im, "N", cx - 1, cy - 17, P.ORO[4])
    ui.intestazione(im, 6, 4, "ASTERRA")
    # sette Sigilli
    rect(im, 0, LH - 16, LW, 16, P.NOTTE[0]); hline(im, 0, LH - 16, LW, P.ORO[2])
    F.micro(im, "SIGILLI", 6, LH - 10, P.ORO[3])
    sigilli = [("VERDE", "NATURA"), ("TUONO", "IMPULSO"), ("MAREE", "ACQUA"), ("FUOCO", "FIAMMA"), ("GHIACCIO", "GELO"), ("PIETRA", "ROCCIA"), ("STELLE", "ASTRALE")]
    for i, (s, t) in enumerate(sigilli):
        x = 46 + i * 39
        acceso = i == 0
        pal = [P.TIPI[t][0], P.TIPI[t][0], P.TIPI[t][1], P.TIPI[t][2], P.H("FFFFFF")] if acceso else [P.PIETRA[1], P.PIETRA[1], P.PIETRA[2], P.PIETRA[3], P.PIETRA[4]]
        ui.cristallo(im, x, LH - 14, 11, pal=pal, bagliore_r=6 if acceso else 0)
        F.micro(im, s, x + 6, LH - 10, P.CREMA if acceso else P.GRIGIO_UI)
    return im


# ================================================================ 16. LOTTA CON GUARDIANO

def sfondo_arena(im):
    sfondo_lotta(im, "arena")
    # colonne di pietra con stendardi verdi e il Sigillo
    for x in (18, 70, 250, 302):
        rect(im, x - 5, 20, 10, 70, P.PIETRA[3]); vline(im, x - 5, 20, 70, P.PIETRA[4]); vline(im, x + 4, 20, 70, P.PIETRA[1])
        rect(im, x - 7, 18, 14, 4, P.PIETRA[4]); rect(im, x - 7, 88, 14, 4, P.PIETRA[2])
        rect(im, x - 4, 26, 8, 22, P.TIPI["NATURA"][0]); rect(im, x - 3, 27, 6, 20, P.TIPI["NATURA"][1])
        for k in range(4):
            px(im, x - 3 + k * 2, 47 + (k % 2), P.TIPI["NATURA"][0])
        ui.cristallo(im, x, 31, 7, pal=[P.H("0E3A1A"), P.H("1E7A3A"), P.H("4CC85A"), P.H("A6F28A"), P.H("F0FFE0")])
        for j in range(20, 90, 7):   # edera
            px(im, x - 5 + (j % 3), j, P.ERBA[3]); px(im, x - 4 + (j % 3), j + 1, P.ERBA[4])


def schermata_guardiano():
    im = nuovo()
    sfondo_arena(im)
    sigillo = [P.H("0E3A1A"), P.H("1E7A3A"), P.H("4CC85A"), P.H("A6F28A"), P.H("F0FFE0")]
    cerchio_runico(im, 236, 82, 50, 11, sigillo)
    cerchio_runico(im, 84, 132, 62, 13, P.ORO)
    im.alpha_composite(elyon("LEAFORA"), (204, 18))
    im.alpha_composite(elyon("PYRION", retro=True, scala=1.25), (38, 52))
    W.personaggio(im, 286, 54, W.ABITO_LIRA)
    # targa speciale del Guardiano: titolo e Elyon rimasti
    ui.cornice(im, 92, 2, 136, 20, gemme=False)
    ui.cristallo(im, 100, 6, 11, pal=sigillo, bagliore_r=6)
    F.testo(im, "PROVA DEL SIGILLO VERDE", 108, 4, P.ORO[4], ombra=P.OMBRA_TESTO)
    F.micro(im, "GUARDIANA LIRA", 108, 13, sigillo[3])
    targa_hud(im, 6, 26, 132, "LEAFORA", 18, ["NATURA"], 41, 60)
    for i in range(3):
        ui.gemma(im, 12 + i * 8, 58, sigillo if i < 3 else None)
    targa_hud(im, 178, 92, 138, "PYRION", 21, ["FIAMMA"], 52, 66, numeri=True, xp=0.6, specchio=True)
    pannello_comandi(im, "La Guardiana Lira manda in campo LEAFORA!", scelta=0)
    return im


# ================================================================ 17. BOSS

def schermata_boss():
    im = nuovo()
    sfondo_lotta(im, "vuoto")
    for k in range(7):   # sette cristalli-Sigillo in orbita
        a = k / 7 * math.tau
        x, y = 232 + math.cos(a) * 58, 56 + math.sin(a) * 16
        ui.cristallo(im, int(x), int(y) - 5, 9, bagliore_r=7)
    cerchio_runico(im, 232, 92, 58, 12, [P.H("1A0A3A"), P.H("4A1E8A"), P.H("8A4AE0"), P.H("C8A0FF"), P.H("FFFFFF")])
    cerchio_runico(im, 84, 138, 62, 13, P.ORO)
    im.alpha_composite(elyon("AURION", scala=2.0), (182, -22))
    im.alpha_composite(elyon("PYROVAR", retro=True, scala=1.25), (36, 60))
    # barra del boss a tutta larghezza con le soglie di fase
    rect(im, 6, 4, 172, 26, P.NOTTE[0])
    ui.gradiente_v(im, 7, 5, 170, 24, P.H("2A1450"), P.NOTTE[0])
    hline(im, 7, 5, 170, P.ORO[4]); hline(im, 7, 28, 170, P.ORO[1])
    ui.badge_tipo(im, "ASTRALE", 10, 8); ui.badge_tipo(im, "OMBRA", 22, 8)
    F.testo(im, "AURION", 36, 8, P.ORO[4], ombra=P.OMBRA_TESTO, grassetto=True)
    F.micro(im, "PRIMORDIALE", 92, 10, P.TIPI["ASTRALE"][2])
    F.micro(im, "LV", 146, 10, P.ORO[3]); F.testo(im, "65", 156, 8, P.CREMA)
    ui.barra(im, 10, 20, 162, 0.62, P.HP_ALTO, h=4)
    for soglia in (0.5, 0.2):
        x = 11 + int(162 * soglia)
        vline(im, x, 18, 8, P.ORO[4]); px(im, x - 1, 17, P.ORO[4]); px(im, x + 1, 17, P.ORO[4])
    F.micro(im, "FASE 1 DI 3", 8, 32, P.TIPI["ASTRALE"][2])
    targa_hud(im, 178, 99, 138, "PYROVAR", 64, ["FIAMMA", "METALLO"], 151, 203, numeri=True, xp=0.2, specchio=True)
    pannello_comandi(im, "AURION libera la FRATTURA DEL VUOTO! Il campo diventa instabile.", scelta=0)
    return im


# ================================================================ 18. FINALE

def schermata_finale():
    im = nuovo()
    ui.gradiente_v(im, 0, 0, LW, 110, P.H("3A2A6A"), P.H("FFB070"))
    cx, cy = 200, 104
    ui.bagliore(im, cx, cy, 90, P.H("FFE0A0"), 110)
    for yy in range(-16, 1):
        for xx in range(-16, 17):
            if xx * xx + yy * yy <= 256:
                px(im, cx + xx, cy + yy, P.H("FFF2C8"))
    for k in range(14):
        a = math.pi + k / 13 * math.pi
        for d in range(20, 110, 2):
            px(im, int(cx + math.cos(a) * d), int(cy + math.sin(a) * d), P.con_alfa(P.H("FFF2C8"), max(0, 90 - d)))
    for strato, (col, base, amp) in enumerate(((P.H("6A4A7A"), 92, 10), (P.H("4A3A6A"), 104, 8), (P.H("2A2244"), 118, 6))):
        for x in range(LW):
            h = base + int(amp * math.sin(x * 0.03 + strato) + amp * 0.5 * math.sin(x * 0.08))
            vline(im, x, h, LH - h, col)
    # torre dell'Equilibrio con il Lumen ripristinato
    for j in range(60):
        larg = max(1, int(1 + j * 0.1))
        hline(im, 236 - larg, 40 + j, 2 * larg + 1, P.H("1E1838"))
    ui.cristallo(im, 236, 28, 13, bagliore_r=16)
    for x in range(0, 140):
        h = 140 - x // 10
        vline(im, x, h, LH - h, P.H("120E24"))
    W.personaggio(im, 72, 116, W.ABITO_GIOCATORE)
    im.alpha_composite(elyon("PYROVAR", scala=0.75), (88, 104))
    scintille(im, 30, (0, 0, LW, 110), P.con_alfa(P.LUMEN[4], 220), seme=11)
    ui.titolo_oro(im, "FINE", LW // 2, 12, centrato=True)
    F.testo(im, "Hai scelto di SIGILLARE. Il Lumen torna a scorrere.", LW // 2 - F.larghezza("Hai scelto di SIGILLARE. Il Lumen torna a scorrere.") // 2, 26, P.CREMA, ombra=P.NOTTE[0])
    ui.cornice(im, 180, 140, 134, 34, gemme=False)
    F.micro(im, "TEMPO 31:07", 188, 147, P.ORO[3])
    F.micro(im, "BESTIARIO 74 / 100", 188, 156, P.ORO[3])
    F.micro(im, "SIGILLI 7 / 7", 188, 165, P.LUMEN[3])
    return im


# ================================================================ esportazione

SCHERMATE = [
    ("01_titolo", schermata_titolo), ("02_overworld", schermata_overworld), ("03_dialogo", schermata_dialogo),
    ("04_menu", schermata_menu), ("05_squadra", schermata_squadra), ("06_zaino", schermata_zaino),
    ("07_bestiario", schermata_bestiario), ("08_lotta", schermata_lotta), ("09_mosse", schermata_mosse),
    ("10_statistiche", schermata_statistiche), ("11_evoluzione", schermata_evoluzione), ("12_cattura", schermata_cattura),
    ("13_salvataggio", schermata_salva), ("14_sconfitta", schermata_sconfitta), ("15_mappa", schermata_mappa),
    ("16_guardiano", schermata_guardiano), ("17_boss", schermata_boss), ("18_finale", schermata_finale),
]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "..", "docs", "design", "mockup")
    os.makedirs(out, exist_ok=True)
    for nome, fn in SCHERMATE:
        im = fn().convert("RGB")
        im.save(os.path.join(out, f"{nome}.png"))
        im.resize((LW * 3, LH * 3), Image.NEAREST).save(os.path.join(out, f"{nome}_x3.png"))
        print("scritto", nome)
    foglio_riassuntivo(out)


def foglio_riassuntivo(out):
    """Tutte le schermate in una griglia 3x6 (a 2x) per la revisione"""
    imgs = [Image.open(os.path.join(out, f"{n}.png")) for n, _ in SCHERMATE]
    g = Image.new("RGB", (3 * (LW + 6) + 6, 6 * (LH + 18) + 6), (7, 10, 24))
    for i, (im, (n, _)) in enumerate(zip(imgs, SCHERMATE)):
        x, y = 6 + (i % 3) * (LW + 6), 6 + (i // 3) * (LH + 18)
        etichetta = Image.new("RGBA", (LW, 12), (7, 10, 24, 255))
        F.testo(etichetta, n.replace("_", " ").upper(), 2, 2, P.ORO[4])
        g.paste(etichetta.convert("RGB"), (x, y))
        g.paste(im, (x, y + 12))
    g.resize((g.width * 2, g.height * 2), Image.NEAREST).save(os.path.join(out, "00_tutte_le_schermate.png"))


if __name__ == "__main__":
    main()
