#!/usr/bin/env python3
"""
ECLYRA - Mockup v2 (Design System definitivo, in revisione).

Novita' rispetto alla v1: cornici sottili, menu ufficiale (ELYON, ZAINO, BESTIARIO, MAPPA, SCHEDA,
SALVA, OPZIONI), battaglia ECLYRA (Elyon di profilo uno di fronte all'altro, HUD in alto, "rosa dei
comandi" sui quattro tasti frontali), Bestiario a codice, mappa con nebbia e posizione del giocatore,
controlli touch e prompt del controller, varianti per telefono (390x180) e Steam Deck (320x200).

Uso: python3 tools/design/mockups_v2.py [cartella]
"""
import math
import os
import sys

from PIL import Image

import font as F
import mockups as M
import mondo as W
import palette as P
import ui

px, rect, hline, vline = ui.px, ui.rect, ui.hline, ui.vline


def dimensioni(w, h):
    """Le funzioni di scena di mockups.py leggono LW/LH al momento della chiamata"""
    M.LW, M.LH = w, h


# ================================================================ glifi dei tasti e touch

def glifo_tasto(im, x, y, lettera, attivo=False):
    """Prompt del tasto (9x9): cerchio scuro, anello d'oro, lettera. Il carattere cambia col dispositivo:
    tastiera Z/X/TAB, Xbox A/B/X/Y, PlayStation croce/cerchio/quadrato/triangolo, nulla su touch."""
    for dy in range(-4, 5):
        for dx in range(-4, 5):
            d = math.hypot(dx, dy)
            if d <= 4.5:
                px(im, x + 4 + dx, y + 4 + dy, P.ORO[3] if d > 3.5 else (P.NOTTE[3] if attivo else P.NOTTE[1]))
    F.micro(im, lettera, x + 3 if len(lettera) == 1 else x + 1, y + 2, P.ORO[5])


def touch_dpad(im, cx, cy, r=22):
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            d = math.hypot(dx, dy)
            if d <= r:
                px(im, cx + dx, cy + dy, (8, 12, 30, 70) if d < r - 1 else (230, 240, 255, 90))
    for (dx, dy) in ((0, -1), (0, 1), (-1, 0), (1, 0)):
        bx, by = cx + dx * (r - 9), cy + dy * (r - 9)
        for k in range(5):
            for t in range(-k, k + 1):
                x = bx + (t if dy else -dx * (k - 2))
                y = by + (-dy * (k - 2) if dy else t)
                px(im, x, y, (240, 246, 255, 150))
    for dy in range(-3, 4):
        for dx in range(-3, 4):
            if dx * dx + dy * dy <= 9:
                px(im, cx + dx, cy + dy, (240, 246, 255, 60))


def touch_tasto(im, cx, cy, r, etichetta):
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            d = math.hypot(dx, dy)
            if d <= r:
                px(im, cx + dx, cy + dy, (8, 12, 30, 80) if d < r - 1 else (255, 229, 150, 140))
    w = F.larghezza(etichetta)
    F.testo(im, etichetta, cx - w // 2, cy - 3, (255, 248, 222, 200))


def touch_pillola(im, x, y, etichetta):
    w = F.larghezza(etichetta) + 10
    for j in range(11):
        for i in range(w):
            bordo = j in (0, 10) or i in (0, w - 1)
            px(im, x + i, y + j, (255, 229, 150, 140) if bordo else (8, 12, 30, 90))
    F.testo(im, etichetta, x + 5, y + 2, (255, 248, 222, 210))


def controlli_touch_overworld(im):
    W_, H_ = im.size
    touch_dpad(im, 30, H_ - 30)
    touch_tasto(im, W_ - 22, H_ - 34, 12, "A")
    touch_tasto(im, W_ - 46, H_ - 18, 10, "B")
    touch_pillola(im, W_ - 44, 6, "MENU")


# ================================================================ battaglia ECLYRA

def hud_lotta(im, x, y, w, nome, lv, tipi, hp, hpmax, giocatore=True, xp=None, stato=None):
    """Targa HUD: sottile, una riga per nome e livello, una per la barra HP. Il lato del giocatore
    mostra anche i numeri e l'XP; quello avversario solo la barra (come da leggibilita')."""
    h = 30 if giocatore else 23
    rect(im, x, y, w, h, P.con_alfa(P.NOTTE[0], 210))
    ui.gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.con_alfa(P.NOTTE[3], 210), P.con_alfa(P.NOTTE[1], 210))
    hline(im, x + 1, y + h - 1, w - 2, P.ORO[2])
    hline(im, x + 1, y, w - 2, P.ORO[3] if giocatore else P.LUMEN[2])
    tx = x + 4
    for t in tipi:
        ui.badge_tipo(im, t, tx, y + 3)
        tx += 12
    F.testo(im, nome, tx + 1, y + 4, P.CREMA, ombra=P.OMBRA_TESTO)
    s = str(lv)
    F.micro(im, "LV", x + w - 16 - F.larghezza(s), y + 6, P.ORO[3])
    F.testo(im, s, x + w - 5 - F.larghezza(s), y + 4, P.ORO[4], ombra=P.OMBRA_TESTO)
    f = hp / hpmax
    if giocatore:
        num = f"{hp}/{hpmax}"
        ui.barra(im, x + 4, y + 17, w - 26 - F.larghezza(num), f, ui.rampa_hp(f), "HP")
        F.testo(im, num, x + w - 4 - F.larghezza(num), y + 15, P.CREMA, ombra=P.OMBRA_TESTO)
        if xp is not None:
            ui.barra(im, x + 4, y + 26, w - 10, xp, P.XP, h=1)
    else:
        ui.barra(im, x + 4, y + 16, w - 24, f, ui.rampa_hp(f), "HP")
    if stato:
        ui.pillola_stato(im, stato, tx + F.larghezza(nome) + 5, y + 4)
    return h


def chip_campo(im, cx, y, testo, colore=None):
    w = F.larghezza_micro(testo) + 12
    x = cx - w // 2
    rect(im, x, y, w, 9, P.con_alfa(P.NOTTE[0], 200))
    hline(im, x, y + 8, w, P.ORO[1])
    ui.gemma(im, x + 4, y + 4, None)
    F.micro(im, testo, x + 8, y + 2, colore or P.CREMA)


def nastro(im, x, y, w, h, testo):
    """Nastro dei messaggi: fascia scura sottile, testo crema, cristallo di avanzamento"""
    rect(im, x, y, w, h, P.con_alfa(P.NOTTE[0], 220))
    hline(im, x, y, w, P.ORO[2]); hline(im, x, y + h - 1, w, P.NOTTE[0])
    for i, riga in enumerate(F.a_capo(testo, w - 16)[:2]):
        F.testo(im, riga, x + 6, y + 5 + i * 11, P.CREMA, ombra=P.OMBRA_TESTO)
    ui.cristallo(im, x + w - 7, y + h - 10, 7)


ROSA = [("ELYON", "Y", ui.icona_elyon, (0, -1)), ("ZAINO", "X", ui.icona_zaino, (-1, 0)),
        ("FUGA", "B", ui.icona_fuga, (1, 0)), ("LOTTA", "A", ui.icona_spada, (0, 1))]


def rosa_comandi(im, cx, cy, scelta="LOTTA", controller=True, disattiva=False):
    """Rosa dei comandi: 4 pulsanti a rombo attorno a un cristallo. Ogni posizione corrisponde
    al tasto frontale del controller nella stessa posizione (A in basso = LOTTA, conferma).
    Il prompt del tasto sta sul lato esterno del pulsante; su touch non compare."""
    for (nome, tasto, icona, (dx, dy)) in ROSA:
        w, h = 46, 16
        x = cx - w // 2 + dx * 29
        y = cy - h // 2 + dy * 18
        attivo = (nome == scelta) and not disattiva
        rect(im, x, y, w, h, P.NOTTE[0])
        if attivo:
            ui.gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.AZZURRO[1], P.NOTTE[3])
            hline(im, x + 1, y + 1, w - 2, P.ORO[4]); hline(im, x + 1, y + h - 2, w - 2, P.ORO[2])
            vline(im, x + 1, y + 1, h - 2, P.ORO[3]); vline(im, x + w - 2, y + 1, h - 2, P.ORO[2])
        else:
            ui.gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.con_alfa(P.NOTTE[3], 230), P.con_alfa(P.NOTTE[2], 230))
            hline(im, x + 1, y + 1, w - 2, P.NOTTE[4])
        icona(im, x + 3, y + 3)
        col = P.ORO[4] if attivo else (P.GRIGIO_UI if disattiva else P.CREMA)
        F.testo(im, nome, x + 16, y + 5, col, ombra=P.OMBRA_TESTO)
        if controller:
            gx = {(0, -1): x + w - 10, (0, 1): x + w - 10, (-1, 0): x - 10, (1, 0): x + w + 1}[(dx, dy)]
            gy = {(0, -1): y - 10, (0, 1): y + h + 1, (-1, 0): y + 4, (1, 0): y + 4}[(dx, dy)]
            if (dx, dy) == (0, 1):
                gx, gy = x - 10, y + 4
            if (dx, dy) == (0, -1):
                gx, gy = x - 10, y + 4
            glifo_tasto(im, gx, gy, tasto, attivo)
    ui.cristallo(im, cx, cy - 6, 13, bagliore_r=10)


def campo_lotta(im, bioma="prato", sfondo=None):
    if sfondo:
        sfondo(im)
    else:
        M.sfondo_lotta(im, bioma)


def schieramento(im, mio, suo, cerchio_suo=P.LUMEN, scala_mio=1.25, scala_suo=1.1, extra=None):
    """Elyon uno di fronte all'altro: il giocatore a sinistra (rivolto a destra), l'avversario a destra
    piu' indietro (prospettiva). Entrambi mostrano il muso: niente vista da dietro."""
    o = (M.LW - 320) // 2          # schermi piu' larghi: il campo resta centrato, le HUD ai bordi
    M.cerchio_runico(im, 226 + o, 112, 46, 10, cerchio_suo)
    M.cerchio_runico(im, 92 + o, 130, 54, 12, P.ORO)
    if extra:
        extra(im)
    spr_suo = M.elyon(suo, scala=scala_suo)
    im.alpha_composite(spr_suo, (226 + o - spr_suo.width // 2, int(112 - 58 * scala_suo)))
    spr_mio = M.elyon(mio, scala=scala_mio).transpose(Image.FLIP_LEFT_RIGHT)
    im.alpha_composite(spr_mio, (92 + o - spr_mio.width // 2, int(130 - 58 * scala_mio)))


def schermata_lotta():
    im = M.nuovo()
    campo_lotta(im)
    schieramento(im, "PYRIN", "AQUIL")
    hud_lotta(im, 4, 4, 148, "PYRIN", 5, ["FIAMMA"], 20, 20, giocatore=True, xp=0.35)
    hud_lotta(im, M.LW - 152, 4, 148, "AQUIL", 5, ["ACQUA"], 15, 20, giocatore=False)
    chip_campo(im, M.LW // 2, 38, "TURNO 1")
    nastro(im, 4, 146, M.LW - 114, 30, "Un AQUIL selvatico sbuca dall'acqua! Cosa farà PYRIN?")
    rosa_comandi(im, M.LW - 54, 150, controller=M.LW == 320)
    return im


TIPI_MOSSE = [("BRACIAVIVA", "FIAMMA", 25, 25, "SPECIALE", 40, 100, -1),
              ("GRAFFIO", "NEUTRO", 35, 35, "FISICA", 40, 100, 0),
              ("MARCHIO FUOCO", "FIAMMA", 20, 20, "STATO", 0, 100, None),
              ("RAPIDITÀ", "NEUTRO", 30, 30, "STATO", 0, 100, None)]


def schermata_mosse():
    im = M.nuovo()
    campo_lotta(im)
    schieramento(im, "PYRIN", "AQUIL")
    hud_lotta(im, 4, 4, 148, "PYRIN", 5, ["FIAMMA"], 20, 20, giocatore=True, xp=0.35)
    hud_lotta(im, 168, 4, 148, "AQUIL", 5, ["ACQUA"], 15, 20, giocatore=False)
    sel = 1
    for i, (nome, tipo, pp, ppm, cat, pot, pre, eff) in enumerate(TIPI_MOSSE):
        x, y = 4 + i * 79, 148 - (4 if i == sel else 0)
        carta_mossa_v2(im, x, y, 75, nome, tipo, pp, ppm, i == sel)
    # dettaglio della mossa selezionata, sopra la carta
    nome, tipo, pp, ppm, cat, pot, pre, eff = TIPI_MOSSE[sel]
    dx, dy, dw = 4 + sel * 79 - 10, 110, 96
    rect(im, dx, dy, dw, 30, P.con_alfa(P.NOTTE[0], 225))
    hline(im, dx, dy, dw, P.ORO[3]); hline(im, dx, dy + 29, dw, P.ORO[1])
    F.micro(im, cat, dx + 5, dy + 4, P.GRIGIO_UI)
    F.micro(im, "POT", dx + 5, dy + 13, P.ORO[3]); F.testo(im, str(pot), dx + 19, dy + 11, P.CREMA)
    F.micro(im, "PRE", dx + 45, dy + 13, P.ORO[3]); F.testo(im, str(pre), dx + 59, dy + 11, P.CREMA)
    F.micro(im, "EFFICACIA NORMALE", dx + 5, dy + 22, P.CREMA)
    for k in range(3):
        px(im, dx + dw // 2 - 2 + k, dy + 30, P.ORO[1])
    px(im, dx + dw // 2 - 1 + 1, dy + 31, P.ORO[1])
    glifo_tasto(im, M.LW - 56, 133, "B")
    F.micro(im, "INDIETRO", M.LW - 44, 135, P.ORO[3])
    return im


def carta_mossa_v2(im, x, y, w, nome, tipo, pp, ppm, attiva):
    """Carta della mossa: fascia del tipo in alto (colore + glifo + nome del tipo), nome, PP"""
    sc, base, chiaro = P.TIPI[tipo]
    h = 30
    rect(im, x, y, w, h, P.NOTTE[0])
    ui.gradiente_v(im, x + 1, y + 1, w - 2, h - 2, P.NOTTE[3] if attiva else P.NOTTE[2], P.NOTTE[1])
    rect(im, x + 1, y + 1, w - 2, 9, base)
    hline(im, x + 1, y + 1, w - 2, chiaro); hline(im, x + 1, y + 9, w - 2, sc)
    lum = 0.3 * base[0] + 0.59 * base[1] + 0.11 * base[2]
    F.micro(im, tipo, x + 14, y + 3, sc if lum > 150 else P.mix(chiaro, (255, 255, 255, 255), 0.7))
    ui.badge_tipo(im, tipo, x + 1, y)
    F.testo(im, nome, x + 4, y + 12, P.ORO[4] if attiva else P.CREMA, ombra=P.OMBRA_TESTO)
    F.micro(im, "PP", x + 4, y + 23, P.GRIGIO_UI)
    s = f"{pp}/{ppm}"
    F.micro(im, s, x + 14, y + 23, P.CREMA)
    ui.barra(im, x + 32, y + 24, w - 38, pp / ppm, P.XP, h=1)
    if attiva:
        hline(im, x, y, w, P.ORO[4]); hline(im, x, y + h - 1, w, P.ORO[2])
        vline(im, x, y, h, P.ORO[3]); vline(im, x + w - 1, y, h, P.ORO[2])


def schermata_cattura():
    im = M.nuovo()
    campo_lotta(im)
    M.cerchio_runico(im, 226, 112, 46, 10, P.LUMEN)
    M.cerchio_runico(im, 92, 130, 54, 12, P.ORO)
    spr = M.elyon("PYRIN", scala=1.25).transpose(Image.FLIP_LEFT_RIGHT)
    im.alpha_composite(spr, (92 - spr.width // 2, int(130 - 58 * 1.25)))
    cx, cy = 226, 100
    for r, a in ((14, 220), (22, 140), (30, 80)):
        for k in range(0, 360, 3):
            ang = math.radians(k)
            px(im, int(cx + math.cos(ang) * r), int(cy + 8 + math.sin(ang) * r * 0.3), P.con_alfa(P.LUMEN[4], a))
    ui.bagliore(im, cx, cy, 24, P.LUMEN[3], 110)
    for yy in range(-7, 8):
        for xx in range(-7, 8):
            d = math.hypot(xx, yy)
            if d <= 7:
                c = (P.ORO[4] if xx + yy < -3 else P.ORO[2]) if d > 5 else (P.LUMEN[4] if xx + yy < -3 else P.LUMEN[3] if xx + yy < 3 else P.LUMEN[2])
                px(im, cx + xx, cy + yy, c)
    ui.cristallo(im, cx, cy - 7, 9)
    hud_lotta(im, 4, 4, 148, "PYRIN", 5, ["FIAMMA"], 14, 20, giocatore=True, xp=0.35)
    hud_lotta(im, 168, 4, 148, "AQUIL", 5, ["ACQUA"], 3, 20, giocatore=False)
    nastro(im, 4, 146, 312, 30, "Lanci un CORE!   1...   2...")
    for i in range(3):
        ui.gemma(im, 266 + i * 14, 161, P.LUMEN if i < 2 else [P.PIETRA[1], P.PIETRA[2], P.PIETRA[3], P.PIETRA[4], P.PIETRA[5]])
    return im


def schermata_guardiano():
    im = M.nuovo()
    sigillo = [P.H("0E3A1A"), P.H("1E7A3A"), P.H("4CC85A"), P.H("A6F28A"), P.H("F0FFE0")]
    campo_lotta(im, sfondo=M.sfondo_arena)
    schieramento(im, "PYRION", "LEAFORA", cerchio_suo=sigillo, extra=lambda i: W.personaggio(i, 266, 66, W.ABITO_LIRA))
    hud_lotta(im, 4, 4, 148, "PYRION", 21, ["FIAMMA"], 52, 66, giocatore=True, xp=0.6)
    hud_lotta(im, 168, 4, 148, "LEAFORA", 18, ["NATURA"], 41, 60, giocatore=False)
    for i in range(3):   # Elyon rimasti al Guardiano
        ui.cristallo(im, 300 - i * 8, 29, 7, pal=sigillo if i < 3 else None)
    # targa della prova
    w = F.larghezza("PROVA DEL SIGILLO VERDE") + 26
    x = 160 - w // 2
    rect(im, x, 36, w, 20, P.con_alfa(P.NOTTE[0], 220)); hline(im, x, 36, w, sigillo[2]); hline(im, x, 55, w, P.ORO[1])
    ui.cristallo(im, x + 8, 40, 11, pal=sigillo, bagliore_r=6)
    F.testo(im, "PROVA DEL SIGILLO VERDE", x + 16, 38, P.ORO[4], ombra=P.OMBRA_TESTO)
    F.micro(im, "GUARDIANA LIRA", x + 16, 48, sigillo[3])
    nastro(im, 4, 146, 206, 30, "La Guardiana Lira manda in campo LEAFORA!")
    rosa_comandi(im, 266, 150)
    return im


# ================================================================ menu ufficiale

MENU = [("ELYON", ui.icona_elyon, "La tua squadra"), ("ZAINO", ui.icona_zaino, "Oggetti e Core"),
        ("BESTIARIO", M.icona_libro, "28 scoperti"), ("MAPPA", M.icona_mappa, "Asterra"),
        ("SCHEDA", M.icona_scheda, "Esploratore del Lumen"), ("SALVA", M.icona_salva, "Ultimo: 2 min fa"),
        ("OPZIONI", M.icona_opzioni, "Audio e controlli")]


def schermata_menu():
    im = M.scena_lunaria()
    M.oscura(im, 110)
    x, y, w = 6, 6, 128
    ui.cornice(im, x, y, w, 168)
    F.testo(im, "ARI", x + 8, y + 7, P.ORO[4], ombra=P.OMBRA_TESTO)
    F.micro(im, "LUNARIA  02:14", x + 8, y + 18, P.GRIGIO_UI)
    ui.icona_core(im, x + 82, y + 6)
    F.testo(im, "12.480", x + 82 - F.larghezza("12.480") - 3, y + 8, P.CREMA) if False else F.micro(im, "12480", x + 96, y + 9, P.LUMEN[3])
    hline(im, x + 6, y + 27, w - 12, P.ORO[1])
    for i, (nome, icona, sotto) in enumerate(MENU):
        yy = y + 32 + i * 19
        if i == 0:
            ui.riga_selezionata(im, x + 3, yy - 1, w - 6, 18)
            ui.cursore(im, x + 6, yy + 8)
        icona(im, x + 14, yy + 3)
        F.testo(im, nome, x + 30, yy + 2, P.ORO[4] if i == 0 else P.CREMA, ombra=P.OMBRA_TESTO)
        F.micro(im, sotto.upper()[:22], x + 30, yy + 11, P.LUMEN[3] if i == 0 else P.GRIGIO_UI)
    # anteprima della squadra a destra (contesto della voce selezionata)
    for k, n in enumerate(["PYRION", "AQUIL", "MOSSEN"]):
        im.alpha_composite(M.elyon(n, scala=0.5), (150 + k * 34, 128))
    rect(im, 140, 165, 176, 12, P.con_alfa(P.NOTTE[0], 200)); hline(im, 140, 165, 176, P.ORO[1])
    glifo_tasto(im, 146, 166, "A"); F.micro(im, "APRI", 158, 168, P.ORO[3])
    glifo_tasto(im, 186, 166, "B"); F.micro(im, "CHIUDI", 198, 168, P.ORO[3])
    glifo_tasto(im, 238, 166, "TB"); F.micro(im, "MENU RAPIDO", 250, 168, P.ORO[3])
    return im


# ================================================================ bestiario a codice

def icona_occhio(im, x, y, c):
    for (dx, dy) in ((1, 0), (2, 0), (3, 0), (0, 1), (4, 1), (1, 2), (2, 2), (3, 2)):
        px(im, x + dx, y + dy, c)
    px(im, x + 2, y + 1, c)


def schermata_bestiario():
    im = M.nuovo()
    ui.gradiente_v(im, 0, 0, M.LW, M.LH, P.NOTTE[2], P.NOTTE[0])
    ui.intestazione(im, 6, 3, "BESTIARIO")
    F.micro(im, "SCOPERTI 28   CATTURATI 9   DI 100", 172, 7, P.ORO[3])
    # copertina e due pagine di pergamena (il codice dei Custodi)
    rect(im, 4, 19, 312, 159, P.H("0E1638"))
    hline(im, 5, 20, 310, P.ORO[2]); hline(im, 5, 176, 310, P.ORO[1])
    for pagina in ((9, 23, 148, 150), (163, 23, 148, 150)):
        x, y, w, h = pagina
        ui.gradiente_v(im, x, y, w, h, P.PERGAMENA[4], P.PERGAMENA[3])
    for j in range(23, 173):   # dorso con ombra
        px(im, 158, j, P.PERGAMENA[0]); px(im, 159, j, P.H("0E1638")); px(im, 160, j, P.H("0E1638")); px(im, 162, j, P.PERGAMENA[0])
    for j in range(20, 177, 6):
        px(im, 160, j, P.ORO[2])
    # pagina sinistra: filtri e lista
    filtri = ["TUTTI", "VERDAN", "ARKAS", "NIVAL"]
    fx = 13
    for i, t in enumerate(filtri):
        w = F.larghezza_micro(t) + 6
        rect(im, fx, 27, w, 9, P.TERRA[2] if i == 0 else P.PERGAMENA[2])
        F.micro(im, t, fx + 3, 29, P.PERGAMENA[4] if i == 0 else P.PERGAMENA[0])
        fx += w + 3
    voci = [(1, "PYRIN", 2), (2, "PYRION", 2), (3, "", 0), (4, "AQUIL", 2), (5, "AQUILON", 1), (6, "", 0),
            (7, "MOSSEN", 1), (8, "", 0), (9, "", 0)]
    for i, (n, nome, st) in enumerate(voci):
        y = 40 + i * 13
        if i == 3:
            rect(im, 11, y - 1, 144, 12, P.con_alfa(P.ORO[3], 110))
            hline(im, 11, y - 1, 144, P.ORO[2]); hline(im, 11, y + 10, 144, P.ORO[2])
            ui.cursore(im, 13, y + 5)
        F.micro(im, f"{n:03d}", 21, y + 3, P.PERGAMENA[0])
        if st == 2:
            ui.icona_core(im, 36, y)
        elif st == 1:
            icona_occhio(im, 39, y + 4, P.TERRA[1])
        else:
            F.micro(im, "?", 40, y + 3, P.PERGAMENA[1])
        F.testo(im, nome if st else "- - -", 52, y + 2, P.INCHIOSTRO if st else P.PERGAMENA[1])
    F.micro(im, "CATTURATO", 18, 166, P.TERRA[1]); ui.icona_core(im, 54, 163)
    icona_occhio(im, 70, 167, P.TERRA[1]); F.micro(im, "VISTO", 78, 166, P.TERRA[1])
    F.micro(im, "?  IGNOTO", 106, 166, P.PERGAMENA[1])
    # pagina destra: voce
    x0 = 167
    F.micro(im, "N. 004", x0 + 2, 28, P.PERGAMENA[0])
    F.testo(im, "AQUIL", x0 + 2, 36, P.INCHIOSTRO, grassetto=True)
    ui.pillola_tipo(im, "ACQUA", x0 + 2, 47, 56)
    F.testo(im, "Lontra-pinna", x0 + 64, 48, P.TERRA[1])
    # medaglione con lo sprite
    for yy in range(-24, 25):
        for xx in range(-34, 35):
            d = (xx / 34) ** 2 + (yy / 24) ** 2
            if d <= 1:
                px(im, x0 + 108 + xx, 88 + yy, P.mix(P.PERGAMENA[2], P.PERGAMENA[4], 1 - d))
    for k in range(0, 360, 4):
        a = math.radians(k)
        px(im, int(x0 + 108 + math.cos(a) * 33), int(88 + math.sin(a) * 23), P.ORO[2])
    spr = M.elyon("AQUIL")
    im.alpha_composite(spr, (x0 + 108 - 32, 88 - 40))
    righe = [("HABITAT", "Route 1"), ("ALTEZZA", "0,5 m"), ("PESO", "6,8 kg")]
    for i, (k, v) in enumerate(righe):
        F.micro(im, k, x0 + 2, 66 + i * 12, P.TERRA[1])
        F.testo(im, v, x0 + 2, 72 + i * 12 - 1, P.INCHIOSTRO) if False else None
        F.testo(im, v, x0 + 34, 64 + i * 12, P.INCHIOSTRO)
    hline(im, x0 + 2, 113, 140, P.PERGAMENA[1])
    desc = "Nuota nei torrenti seguendo i riflessi del Lumen. Quando è felice, le sue pinne si illuminano."
    for i, riga in enumerate(F.a_capo(desc, 138)[:5]):
        F.testo(im, riga, x0 + 2, 117 + i * 11, P.INCHIOSTRO)
    # sigillo "catturato"
    sx, sy = x0 + 128, 36
    for k in range(0, 360, 6):
        a = math.radians(k)
        px(im, int(sx + math.cos(a) * 10), int(sy + math.sin(a) * 10), P.LUMEN[1])
        px(im, int(sx + math.cos(a) * 8), int(sy + math.sin(a) * 8), P.LUMEN[1])
    ui.icona_core(im, sx - 5, sy - 5)
    return im


# ================================================================ mappa

def schermata_mappa():
    im = M.schermata_mappa(visitate={"LUNARIA", "VERDAN"}, giocatore=(130, 132))
    x, y, w = 214, 4, 102
    ui.cornice(im, x, y, w, 70)
    F.testo(im, "VERDAN", x + 7, y + 6, P.ORO[4], ombra=P.OMBRA_TESTO)
    F.micro(im, "REGIONE NATURA", x + 7, y + 16, P.TIPI["NATURA"][2])
    luoghi = [("Lunaria", True), ("Route 1", True), ("Bosco Verdalia", False), ("Verdan", False)]
    for i, (l, v) in enumerate(luoghi):
        yy = y + 26 + i * 10
        if v:
            ui.gemma(im, x + 10, yy + 3)
        else:
            px(im, x + 10, yy + 3, P.GRIGIO_UI)
        F.testo(im, l, x + 16, yy, P.CREMA if v else P.GRIGIO_UI)
    glifo_tasto(im, 6, 154, "A"); F.micro(im, "DETTAGLI", 18, 156, P.ORO[3])
    return im


# ================================================================ overworld: PC, telefono, Steam Deck

def overworld_pc():
    dimensioni(320, 180)
    im = M.scena_lunaria()
    M.banner_luogo(im, "LUNARIA", "Villaggio del Lumen")
    ui.cornice(im, 252, 162, 64, 15, gemme=False)
    glifo_tasto(im, 256, 165, "Z"); F.testo(im, "Parla", 268, 166, P.CREMA)
    return im


def overworld_telefono():
    dimensioni(390, 180)
    im = M.scena_lunaria()
    M.banner_luogo(im, "LUNARIA", "Villaggio del Lumen")
    controlli_touch_overworld(im)
    dimensioni(320, 180)
    return im


def overworld_deck():
    dimensioni(320, 200)
    im = M.scena_lunaria()
    M.banner_luogo(im, "LUNARIA", "Villaggio del Lumen")
    ui.cornice(im, 252, 182, 64, 15, gemme=False)
    glifo_tasto(im, 256, 185, "A"); F.testo(im, "Parla", 268, 186, P.CREMA)
    dimensioni(320, 180)
    return im


def lotta_telefono():
    """La battaglia su telefono (390x180): niente croce direzionale, si toccano direttamente i comandi
    della rosa; i prompt del controller spariscono."""
    dimensioni(390, 180)
    im = schermata_lotta()
    dimensioni(320, 180)
    return im


SCHERMATE = [
    ("01_titolo", M.schermata_titolo), ("02_overworld", overworld_pc), ("02b_overworld_telefono", overworld_telefono),
    ("02c_overworld_steamdeck", overworld_deck), ("03_menu", schermata_menu), ("04_lotta", schermata_lotta),
    ("04b_lotta_mosse", schermata_mosse), ("04c_lotta_telefono", lotta_telefono), ("05_bestiario", schermata_bestiario), ("06_mappa", schermata_mappa),
    ("07_scheda_elyon", M.schermata_statistiche), ("08_zaino", M.schermata_zaino), ("09_dialogo", M.schermata_dialogo),
    ("10_evoluzione", M.schermata_evoluzione), ("11_cattura", schermata_cattura), ("12_guardiano", schermata_guardiano),
]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "..", "docs", "design", "mockup_v2")
    os.makedirs(out, exist_ok=True)
    for nome, fn in SCHERMATE:
        dimensioni(320, 180)
        im = fn().convert("RGB")
        im.save(os.path.join(out, f"{nome}.png"))
        im.resize((im.width * 3, im.height * 3), Image.NEAREST).save(os.path.join(out, f"{nome}_x3.png"))
        print("scritto", nome, im.size)


if __name__ == "__main__":
    main()
