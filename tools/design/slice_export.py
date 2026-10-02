#!/usr/bin/env python3
"""
ECLYRA - Esportazione degli asset della VERTICAL SLICE (Design System v2, STYLE BIBLE).

Genera, a partire dagli stessi generatori dei mockup approvati:
  slice/assets/*.png         atlanti (UI, Elyon, tile, oggetti, personaggi, lotta) e fondali
  slice/src/gen/*.h          font pixel originali e coordinate degli atlanti per il codice C

Nessuno stile nuovo: tutto passa da palette.py, ui.py, font.py, sprites.py, elyon_design.py,
mondo.py e mockups*.py. Uso: python3 tools/design/slice_export.py
"""
import math
import os
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
import elyon_design as D   # noqa: E402
import font as F           # noqa: E402
import mockups as M        # noqa: E402
import mockups_v2 as M2    # noqa: E402
import mondo as W          # noqa: E402
import palette as P        # noqa: E402
import sprites as S        # noqa: E402
import ui                  # noqa: E402

RADICE = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(RADICE, "slice", "assets")
GEN = os.path.join(RADICE, "slice", "src", "gen")
T = 16
px, rect, hline, vline = ui.px, ui.rect, ui.hline, ui.vline

SPECIE_SLICE = ["PYRIN", "PYRION", "PYROVAR", "AQUIL", "AQUILON", "AQUARYS", "MOSSEN", "MOSSKA", "VERDANTOR"]


def nuovo(w, h, c=(0, 0, 0, 0)):
    return Image.new("RGBA", (w, h), c)


def cid(nome):
    return "".join(ch if ch.isalnum() else "_" for ch in nome.upper())


# ================================================================ atlante (impacchettamento a scaffali)

class Atlante:
    def __init__(self, nome, larghezza=512):
        self.nome, self.larghezza, self.voci = nome, larghezza, []

    def aggiungi(self, nome, im):
        self.voci.append((nome, im))

    def costruisci(self):
        voci = sorted(self.voci, key=lambda v: (-v[1].height, v[0]))
        x = y = riga = 0
        pos = {}
        for nome, im in voci:
            if x + im.width > self.larghezza:
                x, y, riga = 0, y + riga + 1, 0
            pos[nome] = (x, y, im.width, im.height)
            x += im.width + 1
            riga = max(riga, im.height)
        h = y + riga
        out = nuovo(self.larghezza, h)
        for nome, im in voci:
            out.alpha_composite(im, pos[nome][:2])
        out.save(os.path.join(OUT, self.nome + ".png"))
        righe = [f"/* GENERATO da tools/design/slice_export.py: atlante {self.nome}.png. Non modificare. */",
                 f"#ifndef GEN_ATLANTE_{cid(self.nome)}_H", f"#define GEN_ATLANTE_{cid(self.nome)}_H", ""]
        for nome, im in self.voci:
            x, y, w, hh = pos[nome]
            righe.append(f"#define {cid(self.nome)}_{cid(nome)} ((Rectangle){{ {x}, {y}, {w}, {hh} }})")
        righe += ["", "#endif", ""]
        with open(os.path.join(GEN, f"atlante_{self.nome}.h"), "w") as f:
            f.write("\n".join(righe))
        return out


# ================================================================ font

def esporta_font():
    car = F.CARATTERI_ATLANTE
    righe = ["/* GENERATO da tools/design/slice_export.py dai font originali di font.py. Non modificare. */",
             "#ifndef GEN_FONT_H", "#define GEN_FONT_H", "", "#include <stdint.h>", "",
             "typedef struct { uint32_t cp; uint8_t w; uint16_t righe[9]; } GlifoGen;", "",
             f"#define FONT_CORPO_ALTEZZA {F.CORPO_ALTEZZA}", f"#define FONT_CORPO_SPAZIO {F.CORPO_SPAZIO}",
             f"#define FONT_CORPO_INTERLINEA {F.CORPO_INTERLINEA}", "#define FONT_MICRO_ALTEZZA 5", "",
             "static const GlifoGen FONT_CORPO[] = {"]
    for ch in car:
        g = F.glifo(ch)
        w = len(g[0])
        bits = [sum(1 << x for x in range(w) if g[y][x]) for y in range(F.CORPO_ALTEZZA)]
        righe.append(f"    {{ {ord(ch)}u, {w}, {{ {', '.join(str(b) for b in bits)} }} }},")
    righe += ["};", f"#define FONT_CORPO_N {len(car)}", "", "static const GlifoGen FONT_MICRO[] = {"]
    for ch, g in sorted(F._M.items()):
        w = len(g[0])
        bits = [sum(1 << x for x in range(w) if g[y][x]) for y in range(5)] + [0] * 4
        righe.append(f"    {{ {ord(ch)}u, {w}, {{ {', '.join(str(b) for b in bits)} }} }},")
    righe += ["};", f"#define FONT_MICRO_N {len(F._M)}", "", "#endif", ""]
    with open(os.path.join(GEN, "font.h"), "w") as f:
        f.write("\n".join(righe))


# ================================================================ UI

def icona(fn, w=11, h=11, *args):
    im = nuovo(w, h)
    fn(im, 0, 0, *args)
    return im


def icona_oggetto(nome):
    im = nuovo(11, 11)
    if nome in ("CORE", "CORE PLUS", "CORE MAX", "CORE VOID"):
        ui.icona_core(im, 0, 0)
        if nome != "CORE":   # anello di colore diverso per le versioni potenziate
            col = {"CORE PLUS": P.AZZURRO[3], "CORE MAX": P.ORO[4], "CORE VOID": P.TIPI["OMBRA"][2]}[nome]
            for (x, y) in ((4, 0), (5, 0), (6, 0), (4, 10), (5, 10), (6, 10), (0, 4), (0, 5), (0, 6), (10, 4), (10, 5), (10, 6)):
                px(im, x, y, col)
    elif nome.startswith("PIETRA"):
        tipo = {"FIAMMA": "FIAMMA", "ACQUA": "ACQUA", "NATURA": "NATURA", "LUMEN": None}[nome.split()[1]]
        pal = [P.TIPI[tipo][0], P.TIPI[tipo][0], P.TIPI[tipo][1], P.TIPI[tipo][2], P.H("FFFFFF")] if tipo else None
        ui.cristallo(im, 5, 0, 11, pal=pal)
    elif nome in ("ERBA VITALE", "FRUTTO LUMEN", "ANTIDOTO", "REVITA"):
        M.icona_oggetto(im, nome, 0, 0)
    else:   # boccetta generica: colore dalla categoria
        tinte = {"ELISIR": P.AZZURRO, "ELISIR MAX": P.AZZURRO, "STABILIZZATORE": P.ORO, "RISVEGLIO": P.LUMEN,
                 "FIAMMIFUGA": [P.TIPI["FIAMMA"][0]] * 2 + list(P.TIPI["FIAMMA"]) + [P.H("FFFFFF")],
                 "GELORISCALDO": P.AZZURRO, "REVITA MAX": P.ORO}
        col = tinte.get(nome, P.PIETRA)
        ui._icona(im, 0, 0, ["....dd.....", "....dd.....", "...aaaa....", "...abba....", "..abbbba...", ".abccbbba..",
                             ".abcbbbba..", ".abbbbbba..", "..abbbba...", "...aaaa....", "..........."],
                  {"a": P.NOTTE[0], "b": col[2], "c": col[4], "d": P.LEGNO[2]})
    return im


def esporta_ui():
    A = Atlante("ui", 256)
    for nome, fn in (("spada", ui.icona_spada), ("elyon", ui.icona_elyon), ("zaino", ui.icona_zaino), ("fuga", ui.icona_fuga),
                     ("core", ui.icona_core), ("libro", M.icona_libro), ("scheda", M.icona_scheda), ("mappa", M.icona_mappa),
                     ("salva", M.icona_salva), ("opzioni", M.icona_opzioni)):
        A.aggiungi("icona_" + nome, icona(fn))
    for t in P.ORDINE_TIPI:
        A.aggiungi("badge_" + t, icona(lambda im, x, y, t=t: ui.badge_tipo(im, t, x, y)))
    for fr in range(2):
        im = nuovo(6, 7)
        ui.cursore(im, 0, 3, fr)
        A.aggiungi(f"cursore_{fr}", im)
    for h in (7, 11, 13, 17):
        im = nuovo(h // 2 + 3, h)
        ui.cristallo(im, (h // 2 + 3) // 2, 0, h)
        A.aggiungi(f"cristallo_{h}", im)
        im = nuovo(h // 2 + 3, h)
        ui.cristallo(im, (h // 2 + 3) // 2, 0, h, pal=[P.PIETRA[1], P.PIETRA[1], P.PIETRA[2], P.PIETRA[3], P.PIETRA[4]])
        A.aggiungi(f"cristallo_spento_{h}", im)
    im = nuovo(5, 5)
    ui.gemma(im, 2, 2)
    A.aggiungi("gemma", im)
    im = nuovo(5, 3)
    M2.icona_occhio(im, 0, 0, P.TERRA[1])
    A.aggiungi("occhio", im)
    # sigilli delle regioni per la barra della mappa (accesi e spenti)
    for t in ("NATURA", "IMPULSO", "ACQUA", "FIAMMA", "GELO", "ROCCIA", "ASTRALE"):
        im = nuovo(9, 11)
        ui.cristallo(im, 4, 0, 11, pal=[P.TIPI[t][0], P.TIPI[t][0], P.TIPI[t][1], P.TIPI[t][2], P.H("FFFFFF")])
        A.aggiungi("sigillo_" + t, im)
    # oggetti
    for nome in ("ERBA VITALE", "FRUTTO LUMEN", "ELISIR", "ELISIR MAX", "ANTIDOTO", "STABILIZZATORE", "RISVEGLIO",
                 "FIAMMIFUGA", "GELORISCALDO", "REVITA", "REVITA MAX", "CORE", "CORE PLUS", "CORE MAX", "CORE VOID",
                 "PIETRA FIAMMA", "PIETRA ACQUA", "PIETRA NATURA", "PIETRA LUMEN"):
        A.aggiungi("ogg_" + nome, icona_oggetto(nome))
    A.costruisci()
    # logo del titolo (trasparente)
    im = nuovo(220, 64)
    M.logo(im, 110, 24, scala=4)
    im.crop(im.getbbox()).save(os.path.join(OUT, "logo.png"))


# ================================================================ Elyon (solo i 9 di riferimento)

def fotogrammi(nome, scala=1.0):
    base = [M.elyon(nome, frame=k, scala=scala) for k in range(3)]
    idle = base[0]
    affondo = nuovo(*idle.size)
    affondo.alpha_composite(idle, (-max(3, int(4 * scala)), 0))
    colpito = idle.copy()
    a = colpito.load()
    for y in range(colpito.height):
        for x in range(colpito.width):
            r_, g_, b_, al = a[x, y]
            if al > 200:
                a[x, y] = (min(255, r_ + 150), min(255, g_ + 150), min(255, b_ + 150), al)
    return base + [affondo, colpito]


def striscia(frames):
    w, h = frames[0].size
    out = nuovo(w * len(frames), h)
    for i, f in enumerate(frames):
        out.alpha_composite(f, (i * w, 0))
    return out


def esporta_elyon():
    A = Atlante("elyon", 1024)
    for nome in SPECIE_SLICE:
        A.aggiungi(nome + "_fronte", striscia(fotogrammi(nome)))                                   # 64 px (Bestiario, Scheda)
        A.aggiungi(nome + "_nemico", striscia(fotogrammi(nome, 1.1)))                              # 70 px, lato avversario
        A.aggiungi(nome + "_mio", striscia([f.transpose(Image.FLIP_LEFT_RIGHT) for f in fotogrammi(nome, 1.25)]))   # 80 px
        ico = M.elyon(nome, scala=0.5)
        hop = nuovo(*ico.size)
        hop.alpha_composite(ico, (0, -1))
        A.aggiungi(nome + "_icona", striscia([ico, hop]))                                          # 32 px, 2 fotogrammi
    A.costruisci()


# ================================================================ tile e oggetti del mondo

def lastricato_bordi(im, x, y, vicini):
    W.lastricato(im, x, y)
    su, giu, sx_, dx_ = vicini
    for i in range(T):
        o = W.rnd("bordo_l", x, y, i)
        if not su:
            hline(im, x + i, y, 1, P.ERBA[3]); px(im, x + i, y + 1, P.PIETRA[4])
        if not giu:
            px(im, x + i, y + T - 1, P.PIETRA[1]); px(im, x + i, y + T - 2, P.PIETRA[2])
        if not sx_:
            px(im, x, y + i, P.ERBA[3] if o.random() < 0.5 else P.PIETRA[4]); px(im, x + 1, y + i, P.PIETRA[4])
        if not dx_:
            px(im, x + T - 1, y + i, P.PIETRA[1]); px(im, x + T - 2, y + i, P.PIETRA[2])


def esporta_tile():
    """Tileset 16x16 a griglia: indice = colonna + 16 * riga. Le maschere a 4 vicini sono
    su=1, destra=2, giu=4, sinistra=8 (stesso ordine del renderer del gioco)."""
    cols = 16
    righe = 9
    im = nuovo(cols * T, righe * T)

    def cella(i):
        return (i % cols) * T, (i // cols) * T

    def vic(m):
        return (bool(m & 1), bool(m & 4), bool(m & 8), bool(m & 2))   # (su, giu, sinistra, destra)

    meta = {}
    # riga 0: erba (4), erba alta (2), fiori (2x2), sottobosco, sentiero pieno di riserva
    for k in range(4):
        x, y = cella(k)
        W.erba(im, x, y, seme=k * 17)
    meta["ERBA"] = 0
    for k in range(2):
        x, y = cella(4 + k)
        W.erba_alta(im, x, y, frame=k * 2)
    meta["ERBA_ALTA"] = 4
    for k in range(4):
        x, y = cella(6 + k)
        W.fiori(im, x, y, seme=(k // 2) * 31)
        if k % 2:   # secondo fotogramma: i petali si piegano di un pixel
            sub = im.crop((x, y, x + T, y + T))
            im.paste(Image.new("RGBA", (T, T), P.ERBA[3]), (x, y))
            W.erba(im, x, y, seme=(k // 2) * 31)
            fi = sub.load()
            for yy in range(T):
                for xx in range(T):
                    c = fi[xx, yy]
                    if c[:3] in [f[:3] for f in P.FIORI] or c[:3] == P.ORO[4][:3]:
                        px(im, min(x + T - 1, x + xx + 1), y + yy, c)
    meta["FIORI"] = 6
    x, y = cella(10)
    rect(im, x, y, T, T, P.ERBA[2])
    r = W.rnd("sottobosco")
    for _ in range(14):
        px(im, x + r.randrange(T), y + r.randrange(T), P.ERBA[1])
    meta["SOTTOBOSCO"] = 10
    # riga 1 e 2: sentiero (16 maschere x 2 varianti)
    for v in range(2):
        for m in range(16):
            x, y = cella(16 + v * 16 + m)
            sub = nuovo(T, T)
            W.sentiero(sub, 0, 0, vic(m)) if v == 0 else None
            if v == 1:   # variante: stessa forma, sassolini diversi
                big = nuovo(T * 3, T)
                W.sentiero(big, T * 2, 0, vic(m))
                sub = big.crop((T * 2, 0, T * 3, T))
            im.alpha_composite(sub, (x, y))
    meta["SENTIERO"] = 16
    # riga 3: lastricato di Lunaria (16 maschere)
    for m in range(16):
        x, y = cella(48 + m)
        lastricato_bordi(im, x, y, vic(m))
    meta["LASTRICATO"] = 48
    # righe 4-5: acqua (16 maschere x 2 fotogrammi)
    for fr in range(2):
        for m in range(16):
            x, y = cella(64 + fr * 16 + m)
            W.acqua(im, x, y, vic(m), frame=fr * 2)
    meta["ACQUA"] = 64
    # riga 6: varianti di decoro sull'erba (sassolini, ciuffo, fiore singolo)
    for k in range(4):
        x, y = cella(96 + k)
        W.erba(im, x, y, seme=200 + k)
        if k == 0:
            W.roccia(im, x + 3, y + 7)
        elif k == 1:
            for (dx, dy) in ((4, 12), (9, 10), (12, 13)):
                px(im, x + dx, y + dy, P.ERBA[5]); px(im, x + dx, y + dy + 1, P.ERBA[2])
        elif k == 2:
            px(im, x + 7, y + 8, P.FIORI[1]); px(im, x + 6, y + 8, P.FIORI[1]); px(im, x + 8, y + 8, P.FIORI[1])
            px(im, x + 7, y + 7, P.FIORI[1]); px(im, x + 7, y + 9, P.ERBA[1])
        else:
            rect(im, x + 5, y + 9, 5, 2, P.TERRA[2]); hline(im, x + 5, y + 9, 5, P.TERRA[3])
    meta["DECORO"] = 96
    im.save(os.path.join(OUT, "tiles.png"))
    righe = ["/* GENERATO da tools/design/slice_export.py: indici di tiles.png (colonna + 16 * riga). */",
             "#ifndef GEN_TILES_H", "#define GEN_TILES_H", ""]
    for k, v in meta.items():
        righe.append(f"#define TILE_{k} {v}")
    righe += ["", "#endif", ""]
    with open(os.path.join(GEN, "tiles.h"), "w") as f:
        f.write("\n".join(righe))


def alberello(im, x, y):
    """Albero piccolo 16x24 (per le tile d'albero isolate): stessa luce e contorno delle querce"""
    W.ombra_ellisse(im, x + 8, y + 22, 6, 2, 90)
    rect(im, x + 7, y + 15, 3, 7, P.LEGNO[2]); vline(im, x + 7, y + 15, 7, P.LEGNO[3])
    can = nuovo(18, 18)
    for (cx, cy, r_) in ((9, 9, 7), (5, 11, 4), (13, 11, 4)):
        for yy in range(-r_, r_ + 1):
            for xx in range(-r_, r_ + 1):
                d = (xx * xx + yy * yy) / (r_ * r_)
                if d <= 1:
                    l = -(xx / r_) * 0.55 - (yy / r_) * 0.7 + math.sqrt(max(0, 1 - d)) * 0.6
                    can.putpixel((cx + xx, cy + yy), P.ERBA[4] if l > 0.85 else P.ERBA[3] if l > 0.35 else P.ERBA[2] if l > -0.2 else P.ERBA[1])
    a = can.load()
    out = can.copy()
    for yy in range(18):
        for xx in range(18):
            if a[xx, yy][3] == 0 and any(0 <= xx + dx < 18 and 0 <= yy + dy < 18 and a[xx + dx, yy + dy][3]
                                         for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                out.putpixel((xx, yy), P.ERBA[0])
    im.alpha_composite(out, (x - 1, y - 1))


def esporta_oggetti():
    A = Atlante("oggetti", 512)
    for k in range(3):
        im = nuovo(34, 42)
        W.albero(im, 1, 1, seme=k * 7)
        A.aggiungi(f"albero_{k}", im)
    im = nuovo(34, 42)
    W.albero(im, 1, 1, seme=3, specie="lumen")
    A.aggiungi("albero_lumen", im)
    im = nuovo(18, 26)
    alberello(im, 1, 1)
    A.aggiungi("alberello", im)
    im = nuovo(16, 16)
    W.cespuglio(im, 0, 0)
    A.aggiungi("cespuglio", im)
    im = nuovo(16, 17)
    W.cartello(im, 0, 1)
    A.aggiungi("cartello", im)
    im = nuovo(91, 63)
    M.rosa_lumen(im, 45, 31)
    A.aggiungi("rosa_lumen", im)
    im = nuovo(8, 30)
    W.lampione(im, 0, 3)
    A.aggiungi("lampione", im)
    # edifici: impronta in tile (5x4 o 7x5); lo sprite include tetto e pinnacolo sopra l'impronta
    for nome, tetto, insegna in (("casa", P.TETTO_BLU, None), ("casa_blu", P.TETTO_ROSSO, None), ("negozio", P.TETTO_BLU, True)):
        im = nuovo(80, 64 + 18)
        W.casa(im, 4, 18 + 2, w=72, tetto=tetto, insegna=insegna)
        A.aggiungi(nome, im)
    im = nuovo(112, 80 + 18)
    W.laboratorio(im, 8, 18 + 3, w=96)
    A.aggiungi("lab", im)
    A.costruisci()


# ================================================================ personaggi 16x24 in 4 direzioni

FRONTE = [
    "......oooo......", "....oohhhhoo....", "...ohhHHHHhho...", "..ohhHHhhHHhho..", "..ohhhhhhhhhho..",
    "..ohggggggggho..", "..ohgLLggLLgho..", "..ohssssssssho..", "..ohsessssesho..", "..ohsessssesho..",
    "...ossrssrsso...", "....otTTTTto....", "...otTTttTTto...", "..ojjjjggjjjjo..", ".ojjjjjggjjjjjo.",
    ".osjjjjggjjjjso.", "..ojjjjggjjjjo..", "..okkkkGGkkkko..", "..opppppppppo...", "..opppo.oppo....",
    "..opppo.oppo....", "..obbbo.obbo....", ".obbbbo.obbbo...", ".oooooo.ooooo...",
]
RETRO = [
    "......oooo......", "....oohhhhoo....", "...ohhHHHHhho...", "..ohhHHhhHHhho..", "..ohhhhhhhhhho..",
    "..ohggggggggho..", "..ohhhhhhhhhho..", "..ohhhhhhhhhho..", "..ohhhHHHhhhho..", "..ohhhhhhhhhho..",
    "...ohhhhhhhho...", "....otTTTTto....", "...ojTTTTTTjo...", "..ojjjjtTjjjjo..", ".ojjjjjtTjjjjjo.",
    ".osjjjjtTjjjjso.", "..ojjjjjjjjjjo..", "..okkkkkkkkkko..", "..opppppppppo...", "..opppo.oppo....",
    "..opppo.oppo....", "..obbbo.obbo....", ".obbbbo.obbbo...", ".oooooo.ooooo...",
]
LATO = [   # rivolto a sinistra
    "......oooo......", ".....ohhhhoo....", "....ohhHHhhho...", "...ohhHHhhhhho..", "...ohhhhhhhhho..",
    "...oggggggghho..", "...ogLggggghho..", "...osssshhhhho..", "...oessshhhho...", "...ossssshhho...",
    "....osrssho.....", ".....otTTto.....", ".....otTTTto....", ".....ojjjjjo....", "....ojjjjjjo....",
    "....ojjjjsjo....", "....ojjjjjjo....", "....okkkkGko....", ".....oppppo.....", ".....opppo......",
    ".....opppo......", ".....obbbo......", "....obbbbo......", "....oooooo......",
]
GAMBE_PASSO_LATO = [".....oppppo.....", "....opp.ppo.....", "...opp..opo.....", "...obo..obo.....",
                    "..obbo.obbo.....", "..oooo.oooo....."]


def abiti_slice():
    A = dict
    return [
        W.ABITO_ELIAN,                                                                  # 0 Elian
        W.ABITO_KAEL,                                                                   # 1 Kael
        A(capelli=[P.H("4A2A18"), P.H("7A4A2A"), P.H("A86A3E")], giacca=[P.H("2A6A3A"), P.H("3E8A4E"), P.H("7ACB78")],
          sciarpa=[P.H("8A2A4A"), P.H("C84A6A"), P.H("F28AA0")], pantaloni=P.H("2E4A32"), stivali=P.LEGNO[1], fascia=P.LEGNO),   # 2 Marta
        A(capelli=[P.H("A8782A"), P.H("E0B84A"), P.H("F8E090")], giacca=[P.H("8A3A5A"), P.H("C8648A"), P.H("F2A0C0")],
          sciarpa=P.LUMEN[1:4], pantaloni=P.H("2A2E58"), stivali=P.LEGNO[2], fascia=P.NOTTE),                                   # 3 Lina
        A(capelli=[P.H("1A1418"), P.H("2E2830"), P.H("4A4250")], giacca=[P.H("5A3A1E"), P.H("8A5E32"), P.H("B8864E")],
          sciarpa=P.ORO[1:4], pantaloni=P.H("2E3A52"), stivali=P.LEGNO[1], fascia=P.LEGNO),                                     # 4 Bruno, Ugo
        A(capelli=[P.H("A8A8B0"), P.H("D8D8E0"), P.H("F6F6FA")], giacca=[P.H("4A2E6A"), P.H("6E4A98"), P.H("A07ACB")],
          sciarpa=P.ORO[2:5], pantaloni=P.H("3A3A48"), stivali=P.LEGNO[1], fascia=P.NOTTE),                                     # 5 viaggiatore
        A(capelli=[P.H("8A3A10"), P.H("D2782A"), P.H("F2A85A")], giacca=[P.H("A8862A"), P.H("E8C040"), P.H("FFE68A")],
          sciarpa=P.AZZURRO[1:4], pantaloni=P.H("8A2A2A"), stivali=P.LEGNO[2], fascia=P.AZZURRO),                               # 6 Pip
        A(capelli=[P.H("3A2A1A"), P.H("5E4630"), P.H("8A6A48")], giacca=[P.H("2A4A2E"), P.H("3E6E44"), P.H("6A9A68")],
          sciarpa=P.TERRA[1:4], pantaloni=P.H("6A5A3A"), stivali=P.LEGNO[1], fascia=P.ERBA),                                    # 7 esploratori
        W.ABITO_GIOCATORE,                                                              # 8 giocatore
    ]


def personaggio_dir(im, x, y, abito, direzione, passo):
    """direzione: 0 giu (fronte), 1 su (retro), 2 sinistra. passo: 0 fermo, 1 e 2 i due passi."""
    col = {
        "o": P.H("140F26"), "h": abito["capelli"][1], "H": abito["capelli"][2],
        "s": P.PELLE[2], "e": P.H("140F26"), "r": P.H("E8907A"),
        "g": abito.get("fascia", P.ORO)[2], "L": P.LUMEN[3], "G": P.ORO[4],
        "t": abito["sciarpa"][1], "T": abito["sciarpa"][2],
        "j": abito["giacca"][1], "k": P.LEGNO[1], "p": abito["pantaloni"], "b": abito["stivali"],
    }
    tpl = [FRONTE, RETRO, LATO][direzione][:]
    if passo and direzione == 2:
        tpl = tpl[:18] + GAMBE_PASSO_LATO
    W.ombra_ellisse(im, x + 8, y + 23, 6, 2, 90)
    dy = -1 if passo else 0
    for j, riga in enumerate(tpl):
        for i, ch in enumerate(riga):
            if ch not in col:
                continue
            c = col[ch]
            if ch == "j" and i in (2, 3, 12, 13):
                c = abito["giacca"][0]
            if ch == "j" and i in (5, 6) and direzione == 0:
                c = abito["giacca"][2]
            if ch == "j" and direzione == 2 and i in (5, 6):
                c = abito["giacca"][2]
            if ch == "s" and direzione == 0 and j < 11 and i >= 11:
                c = P.PELLE[1]
            if ch == "h" and direzione == 1 and i >= 10:
                c = abito["capelli"][0]
            yy = y + j + (dy if j < 18 else 0)
            # passo davanti/dietro: le gambe si alternano
            if passo and direzione != 2 and j >= 19:
                lato_gamba = 0 if i < 8 else 1
                if (passo == 1 and lato_gamba == 1) or (passo == 2 and lato_gamba == 0):
                    if j == 23:
                        continue
                    yy -= 1
            px(im, x + i, yy, c)
    # dettagli asimmetrici: coda della sciarpa e borsa (come nei mockup)
    if direzione == 0:
        for j in range(11, 17):
            px(im, x + 12 + (j - 11) // 3, y + j + dy, abito["sciarpa"][1])
        rect(im, x + 1, y + 15 + dy, 3, 3, P.LEGNO[2]); px(im, x + 1, y + 15 + dy, P.LEGNO[3])
    elif direzione == 1:
        rect(im, x + 12, y + 14 + dy, 3, 3, P.LEGNO[2]); px(im, x + 12, y + 14 + dy, P.LEGNO[3])
    else:
        for j in range(12, 18):
            px(im, x + 11 + (j - 12) // 3, y + j + dy, abito["sciarpa"][1])
        rect(im, x + 9, y + 15 + dy, 3, 3, P.LEGNO[2]); px(im, x + 9, y + 15 + dy, P.LEGNO[3])


def esporta_personaggi():
    abiti = abiti_slice()
    im = nuovo(16 * 9, 24 * len(abiti))
    for k, ab in enumerate(abiti):
        for d in range(3):
            for p_ in range(3):
                personaggio_dir(im, (d * 3 + p_) * 16, k * 24, ab, d, p_)
    im.save(os.path.join(OUT, "personaggi.png"))


# ================================================================ lotta e fondali

def esporta_lotta():
    # fondale: progettato a 400x180 e allungato sopra (cielo) e sotto (prato) fino a 400x240
    M.LW, M.LH = 400, 180
    base = nuovo(400, 180, P.NOTTE[1])
    M.sfondo_lotta(base, "prato")
    im = nuovo(400, 240)
    cielo_top = base.getpixel((0, 0))
    rect(im, 0, 0, 400, 30, cielo_top)
    im.alpha_composite(base, (0, 30))
    for y in range(210, 240):   # prato: si ripetono le ultime righe con un leggero scurimento
        for x in range(400):
            c = base.getpixel((x, 150 + (y - 210)))
            im.putpixel((x, y), P.mix(c, P.ERBA[2], 0.15 + (y - 210) / 120))
    im.save(os.path.join(OUT, "lotta_prato.png"))
    M.LW, M.LH = 320, 180
    # cerchi runici animati (8 fotogrammi) per avversario (46x10) e giocatore (54x12)
    A = Atlante("lotta", 512)
    for nome, (rx, ry) in (("piccolo", (46, 10)), ("grande", (54, 12))):
        for col_nome, col in (("oro", P.ORO), ("lumen", P.LUMEN), ("rosso", [P.TIPI["FIAMMA"][0]] * 2 + list(P.TIPI["FIAMMA"]))):
            for fr in range(8):
                im = nuovo(2 * rx + 7, 2 * ry + 7)
                M.cerchio_runico(im, rx + 3, ry + 3, rx, ry, col, frame=fr * 39)
                A.aggiungi(f"cerchio_{nome}_{col_nome}_{fr}", im)
    A.costruisci()


def esporta_titolo():
    M.LW, M.LH = 400, 240
    im = nuovo(400, 240, P.NOTTE[0])
    ui.gradiente_v(im, 0, 0, 400, 240, P.NOTTE[0], P.H("3A3A8A"))
    r = W.rnd("titolo")
    for _ in range(190):
        x, y = r.randrange(400), r.randrange(140)
        px(im, x, y, P.con_alfa((230, 230, 255, 255), r.randrange(70, 255)))
    for x in range(400):
        for k, (col, fase, amp, base) in enumerate(((P.LUMEN[3], 0.0, 10, 68), (P.TIPI["ASTRALE"][1], 1.7, 8, 82))):
            yc = base + amp * math.sin(x * 0.018 + fase) + 4 * math.sin(x * 0.051 + fase * 2)
            for dy in range(-10, 11):
                a = int(55 * (1 - abs(dy) / 10) * (0.6 + 0.4 * math.sin(x * 0.03 + fase)))
                if a > 0:
                    px(im, x, int(yc + dy), P.con_alfa(col, a))
    tx = 300
    for j in range(80):
        larg = max(1, int(1 + j * 0.12))
        hline(im, tx - larg, 70 + j, 2 * larg + 1, P.H("2A2E6A"))
    ui.bagliore(im, tx, 70, 26, P.LUMEN[3], 90)
    ui.cristallo(im, tx, 58, 13)
    for strato, (colore, base, amp, f) in enumerate(((P.H("262A62"), 126, 14, 0.017), (P.H("1A1E4A"), 142, 10, 0.031), (P.H("10142E"), 160, 8, 0.05))):
        for x in range(400):
            h = base + int(amp * math.sin(x * f + strato * 2) + amp * 0.5 * math.sin(x * f * 2.7))
            vline(im, x, h, 240 - h, colore)
            if strato == 0 and (x * 5) % 9 == 0:
                px(im, x, h, P.H("6A72C8"))
    for (hx, hw) in ((40, 18), (62, 14), (80, 22), (110, 16), (134, 12)):
        rect(im, hx, 168, hw, 14, P.H("0C1028"))
        for k in range(hw // 2):
            hline(im, hx - 2 + k, 168 - k // 2, hw + 4 - 2 * k, P.H("0C1028"))
        px(im, hx + 4, 174, P.H("FFD27A")); px(im, hx + 5, 174, P.H("FFD27A"))
        ui.bagliore(im, hx + 5, 174, 5, P.H("FFC060"), 60)
    for x in range(200, 400):
        h = 186 + int(6 * math.sin((x - 200) * 0.03)) - (x - 200) // 12
        vline(im, x, h, 240 - h, P.H("080A1C"))
    W.personaggio(im, 336, 140, W.ABITO_GIOCATORE)
    im.alpha_composite(M.elyon("PYRIN", scala=0.5), (310, 136))
    im.save(os.path.join(OUT, "titolo.png"))
    M.LW, M.LH = 320, 180


def esporta_mappa():
    """Mappa di Asterra senza etichette ne' nebbia + mappa delle regioni (indice nel canale rosso)."""
    base = M.schermata_mappa(visitate=None, giocatore=None)
    # ricostruisce solo il disegno del territorio: si rifa' la parte dei pixel di terra/mare
    M.LW, M.LH = 320, 180
    im = nuovo(320, 180)
    regioni = nuovo(320, 180)
    # stessa geometria di mockups.schermata_mappa
    terra = {}
    for y in range(180):
        for x in range(320):
            migliore, dmin = None, 1e9
            for i, (_, _, (rx, ry), _, _) in enumerate(M.REGIONI):
                d = math.hypot((x - rx) / 1.15, y - ry) + 6 * math.sin(x * 0.17 + i) * math.cos(y * 0.13 - i)
                if d < dmin:
                    migliore, dmin = i, d
            bordo = 46 + 8 * math.sin(x * 0.09) + 6 * math.cos(y * 0.11 + x * 0.03)
            if dmin < bordo and 18 < y < 172 and 22 < x < 304:
                terra[(x, y)] = migliore
    # copia dalla mappa completa, togliendo le targhe delle regioni (ridisegnate dal codice) e la barra dei sigilli
    im.alpha_composite(base)
    for (nome, sott, (x, y), b, c) in M.REGIONI:
        w = max(F.larghezza(nome), F.larghezza_micro(sott.upper())) + 8
        lx, ly = x - w // 2, y + 5
        for yy in range(ly, ly + 17):
            for xx in range(lx, lx + w):
                if (xx, yy) in terra:
                    i = terra[(xx, yy)]
                    bb, cc = M.REGIONI[i][3], M.REGIONI[i][4]
                    col = bb if (xx + yy * 3) % 7 else cc
                    vic = [terra.get((xx + dx, yy + dy)) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
                    if None in vic:
                        col = P.PERGAMENA[3]
                    elif any(v != i for v in vic):
                        col = P.mix(bb, (0, 0, 0, 255), 0.35)
                    im.putpixel((xx, yy), col)
                else:
                    v = math.sin(xx * 0.11 + yy * 0.07) + math.sin(xx * 0.05 - yy * 0.13)
                    im.putpixel((xx, yy), P.ACQUA[1] if v < 1.3 else P.ACQUA[2])
    for yy in range(0, 20):        # intestazione "ASTERRA": la disegna il codice
        for xx in range(0, 120):
            v = math.sin(xx * 0.11 + yy * 0.07) + math.sin(xx * 0.05 - yy * 0.13)
            if (xx, yy) not in terra:
                im.putpixel((xx, yy), P.ACQUA[1] if v < 1.3 else P.ACQUA[2])
    for yy in range(164, 180):     # barra dei sigilli: la disegna il codice
        for xx in range(320):
            v = math.sin(xx * 0.11 + yy * 0.07) + math.sin(xx * 0.05 - yy * 0.13)
            im.putpixel((xx, yy), P.ACQUA[1] if v < 1.3 else P.ACQUA[2])
    for (x, y), i in terra.items():
        regioni.putpixel((x, y), (i + 1, 0, 0, 255))
    im.save(os.path.join(OUT, "mappa_asterra.png"))
    regioni.save(os.path.join(OUT, "mappa_regioni.png"))
    righe = ["/* GENERATO da tools/design/slice_export.py: regioni della mappa di Asterra. */",
             "#ifndef GEN_MAPPA_H", "#define GEN_MAPPA_H", "",
             "typedef struct { const char *nome, *sottotitolo; int cx, cy; unsigned char chiaro[3]; } RegioneGen;", "",
             "static const RegioneGen REGIONI_GEN[] = {"]
    for (nome, sott, (x, y), b, c) in M.REGIONI:
        righe.append(f'    {{ "{nome}", "{sott}", {x}, {y}, {{ {c[0]}, {c[1]}, {c[2]} }} }},')
    righe += ["};", f"#define NUM_REGIONI_GEN {len(M.REGIONI)}", "", "#endif", ""]
    with open(os.path.join(GEN, "mappa.h"), "w") as f:
        f.write("\n".join(righe))


def esporta_palette():
    def c(col):          # inizializzatore costante (C99: niente letterali composti negli array statici)
        return f"{{ {col[0]}, {col[1]}, {col[2]}, 255 }}"

    def cm(col):
        return f"(Color){{ {col[0]}, {col[1]}, {col[2]}, 255 }}"
    righe = ["/* GENERATO da tools/design/slice_export.py da palette.py (palette ufficiale). Non modificare. */",
             "#ifndef GEN_PALETTE_H", "#define GEN_PALETTE_H", "", '#include "raylib.h"', ""]
    for nome in ("NOTTE", "LUMEN", "AZZURRO", "ORO", "PERGAMENA", "ERBA", "TERRA", "ACQUA", "PIETRA", "LEGNO", "INTONACO", "FIORI", "PELLE"):
        rampa = getattr(P, nome)
        righe.append(f"static const Color PAL_{nome}[{len(rampa)}] = {{ {', '.join(c(x) for x in rampa)} }};")
    for nome in ("INCHIOSTRO", "CREMA", "GRIGIO_UI", "OMBRA_TESTO"):
        righe.append(f"#define PAL_{nome} {cm(getattr(P, nome))}")
    for nome in ("HP_ALTO", "HP_MEDIO", "HP_BASSO", "XP"):
        righe.append(f"static const Color PAL_{nome}[3] = {{ {', '.join(c(x) for x in getattr(P, nome))} }};")
    ordine = ["NEUTRO", "FIAMMA", "ACQUA", "NATURA", "IMPULSO", "VENTO", "ROCCIA", "GELO", "OMBRA", "ASTRALE", "TOSSICO", "PSICHE", "METALLO"]
    righe.append("/* indice = ElementType (TYPE_NONE = NEUTRO): scuro, base, accento */")
    righe.append("static const Color PAL_TIPI[13][3] = {")
    for t in ordine:
        righe.append(f"    {{ {', '.join(c(x) for x in P.TIPI[t])} }},   /* {t} */")
    righe.append("};")
    righe.append("static const char *const NOMI_TIPI_GEN[13] = { " + ", ".join(f'"{t}"' for t in ordine) + " };")
    righe += ["", "#endif", ""]
    with open(os.path.join(GEN, "palette.h"), "w") as f:
        f.write("\n".join(righe))


def main():
    os.makedirs(OUT, exist_ok=True)
    os.makedirs(GEN, exist_ok=True)
    passi = [("palette", esporta_palette), ("font", esporta_font), ("ui", esporta_ui), ("elyon", esporta_elyon), ("tile", esporta_tile),
             ("oggetti", esporta_oggetti), ("personaggi", esporta_personaggi), ("lotta", esporta_lotta),
             ("titolo", esporta_titolo), ("mappa", esporta_mappa)]
    solo = sys.argv[1:]
    for nome, fn in passi:
        if solo and nome not in solo:
            continue
        fn()
        print("  esportato:", nome)


if __name__ == "__main__":
    main()
