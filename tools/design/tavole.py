#!/usr/bin/env python3
"""ECLYRA - Tavole di riferimento della Design System (palette, tipografia, componenti, sprite).
Uso: python3 tools/design/tavole.py [cartella]"""
import os
import sys

from PIL import Image

import elyon_design as D
import font as F
import palette as P
import sprites as S
import ui


def tavola_palette():
    gruppi = [("NOTTE", P.NOTTE), ("LUMEN", P.LUMEN), ("AZZURRO", P.AZZURRO), ("ORO", P.ORO), ("PERGAMENA", P.PERGAMENA),
              ("ERBA", P.ERBA), ("TERRA", P.TERRA), ("ACQUA", P.ACQUA), ("PIETRA", P.PIETRA), ("LEGNO", P.LEGNO),
              ("TETTO BLU", P.TETTO_BLU), ("TETTO ROSSO", P.TETTO_ROSSO)]
    im = Image.new("RGBA", (320, 30 + 14 * len(gruppi) + 20 + 13 * 14), P.NOTTE[1])
    ui.intestazione(im, 4, 4, "PALETTE")
    y = 24
    for nome, rampa in gruppi:
        F.micro(im, nome, 6, y + 4, P.CREMA)
        for i, c in enumerate(rampa):
            ui.rect(im, 70 + i * 22, y, 20, 11, c)
        y += 14
    y += 8
    F.micro(im, "TIPI: SCURO  BASE  CHIARO  + GLIFO", 6, y, P.ORO[3])
    y += 10
    for t in P.ORDINE_TIPI:
        ui.badge_tipo(im, t, 6, y)
        F.micro(im, t, 22, y + 3, P.CREMA)
        for i, c in enumerate(P.TIPI[t]):
            ui.rect(im, 70 + i * 22, y, 20, 11, c)
        y += 13
    return im


def tavola_tipografia():
    im = Image.new("RGBA", (320, 140), P.NOTTE[1])
    ui.intestazione(im, 4, 4, "TIPOGRAFIA")
    ui.titolo_oro(im, "TITOLO SCHERMATA", 8, 24)
    F.micro(im, "H2  GRASSETTO ORO  9 PX", 150, 26, P.GRIGIO_UI)
    F.testo(im, "ETICHETTA DI SEZIONE", 8, 40, P.ORO[3], ombra=P.OMBRA_TESTO)
    F.micro(im, "H3  CORPO MAIUSCOLO ORO", 150, 42, P.GRIGIO_UI)
    F.testo(im, "Testo del corpo su pannello notte.", 8, 54, P.CREMA, ombra=P.OMBRA_TESTO)
    F.testo(im, "àèéìòù ÀÈÉÌÒÙ «» 0123456789 ?!", 8, 66, P.CREMA, ombra=P.OMBRA_TESTO)
    ui.cornice(im, 4, 80, 312, 26, stile="pergamena")
    F.testo(im, "Testo dei dialoghi: inchiostro su pergamena.", 12, 89, P.INCHIOSTRO)
    F.micro(im, "MICRO 3X5: LV 16  HP 48/48  PP 25/25  SPECIALE", 8, 114, P.ORO[3])
    F.micro(im, "ETICHETTE HUD, NUMERI SECONDARI, SUGGERIMENTI DEI TASTI", 8, 124, P.GRIGIO_UI)
    return im


def tavola_componenti():
    im = Image.new("RGBA", (320, 180), P.NOTTE[1])
    ui.cornice(im, 6, 10, 150, 64, titolo="PANNELLO")
    ui.riga_selezionata(im, 10, 30, 140); ui.cursore(im, 13, 35)
    F.testo(im, "Voce selezionata", 22, 32, P.ORO[4], ombra=P.OMBRA_TESTO)
    F.testo(im, "Voce normale", 22, 45, P.CREMA, ombra=P.OMBRA_TESTO)
    F.testo(im, "Voce disattivata", 22, 57, P.GRIGIO_UI, ombra=P.OMBRA_TESTO)
    ui.cornice(im, 162, 10, 152, 40, stile="pergamena")
    F.testo(im, "Pergamena (dialoghi,", 170, 20, P.INCHIOSTRO)
    F.testo(im, "elenchi chiari)", 170, 31, P.INCHIOSTRO)
    ui.barra(im, 10, 86, 80, 0.8, ui.rampa_hp(0.8), "HP"); ui.barra(im, 10, 96, 80, 0.4, ui.rampa_hp(0.4), "HP")
    ui.barra(im, 10, 106, 80, 0.12, ui.rampa_hp(0.12), "HP"); ui.barra(im, 10, 116, 80, 0.6, P.XP, "XP", h=2)
    for i, s in enumerate(["VEL", "BRU", "PAR", "SON", "GEL", "CNF"]):
        ui.pillola_stato(im, s, 120 + (i % 3) * 26, 84 + (i // 3) * 12)
    ui.pillola_tipo(im, "FIAMMA", 210, 84, 56); ui.pillola_tipo(im, "ASTRALE", 210, 98, 56)
    ui.pulsante(im, 10, 132, 70, 18, "LOTTA", ui.icona_spada, attivo=True)
    ui.pulsante(im, 86, 132, 70, 18, "ELYON", ui.icona_elyon)
    ui.pulsante(im, 162, 132, 70, 18, "ZAINO", ui.icona_zaino)
    ui.pulsante(im, 238, 132, 70, 18, "FUGA", ui.icona_fuga)
    for i, h in enumerate((7, 11, 15)):
        ui.cristallo(im, 20 + i * 18, 156, h, bagliore_r=h)
    ui.icona_core(im, 80, 158)
    ui.separatore(im, 110, 166, 190)
    return im


def tavola_sprite():
    nomi = ["PYRIN", "PYRION", "PYROVAR", "AQUIL", "AQUARYS", "MOSSEN", "VERDANTOR", "LUMI", "SHADEK", "FROLI", "LEAFORA", "AURION"]
    fronte = [S.render(*D.SCHEDE[n]()) for n in nomi]
    retro = [S.render(*D.SCHEDE[n](), retro=True) for n in nomi[:7]]
    anim = [S.render(*D.SCHEDE["PYRIN"](), frame=k) for k in range(3)]
    return S.foglio(fronte, 6), S.foglio(retro, 7), S.foglio(anim, 3)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "..", "docs", "design")
    os.makedirs(out, exist_ok=True)
    for nome, im in (("tavola_palette", tavola_palette()), ("tavola_tipografia", tavola_tipografia()), ("tavola_componenti", tavola_componenti())):
        im.convert("RGB").resize((im.width * 3, im.height * 3), Image.NEAREST).save(os.path.join(out, nome + ".png"))
    f, r, a = tavola_sprite()
    for nome, im in (("tavola_sprite_fronte", f), ("tavola_sprite_retro", r), ("tavola_sprite_animazione", a)):
        im.convert("RGB").resize((im.width * 3, im.height * 3), Image.NEAREST).save(os.path.join(out, nome + ".png"))
    F.atlante_corpo().save(os.path.join(out, "font_eclyra_corpo.png"))
    print("tavole scritte in", os.path.abspath(out))


if __name__ == "__main__":
    main()
