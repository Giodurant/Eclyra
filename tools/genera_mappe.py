#!/usr/bin/env python3
"""
ECLYRA - genera le mappe della Fase 2 in assets/maps/*.map (file di testo).
Le coordinate seguono il documento di design. Dopo la generazione i file
si possono modificare a mano: il gioco legge solo i .map.

Uso: python3 tools/genera_mappe.py
"""
import os

CARTELLA = os.path.join(os.path.dirname(__file__), "..", "assets", "maps")


class Mappa:
    def __init__(self, id_, nome, w, h, tipo="ESTERNO", riempi="."):
        self.id, self.nome, self.w, self.h, self.tipo = id_, nome, w, h, tipo
        self.g = [[riempi] * w for _ in range(h)]
        self.righe = []          # direttive extra (PORTA, NPC, ...)

    def set(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.g[y][x] = c

    def rett(self, x, y, w, h, c):
        for j in range(y, y + h):
            for i in range(x, x + w):
                self.set(i, j, c)

    def edificio(self, x, y, c, w, h):
        self.rett(x, y, w, h, c)

    def da_righe(self, righe):
        assert len(righe) == self.h, (self.id, len(righe))
        for y, r in enumerate(righe):
            assert len(r) == self.w, (self.id, y, len(r), r)
            self.g[y] = list(r)

    def add(self, riga):
        self.righe.append(riga)

    def salva(self):
        os.makedirs(CARTELLA, exist_ok=True)
        with open(os.path.join(CARTELLA, self.id + ".map"), "w", encoding="utf-8") as f:
            f.write(f"# ECLYRA - mappa {self.id} (generata da tools/genera_mappe.py, modificabile)\n")
            f.write(f"NOME {self.nome}\nTIPO {self.tipo}\nDIMENSIONI {self.w} {self.h}\nTILES\n")
            for r in self.g:
                f.write("".join(r) + "\n")
            for r in self.righe:
                f.write(r + "\n")


def bordo_alberi(m, spessore=2):
    m.rett(0, 0, m.w, spessore, "T")
    m.rett(0, m.h - spessore, m.w, spessore, "T")
    m.rett(0, 0, spessore, m.h, "T")
    m.rett(m.w - spessore, 0, spessore, m.h, "T")


# ------------------------------------------------------------------
# LUNARIA 40x30
# casa giocatore (8,18) · piazza (19,15) · laboratorio (28,8) · negozio (6,7) · uscita Route 1 (19,0)
# ------------------------------------------------------------------
def lunaria():
    m = Mappa("lunaria", "LUNARIA", 40, 30)
    bordo_alberi(m)
    m.rett(18, 0, 4, 2, "=")              # varco nord verso la Route 1
    m.rett(18, 2, 4, 11, "=")             # strada principale
    m.rett(15, 13, 10, 6, "=")            # piazza
    m.rett(19, 15, 2, 2, "*")             # aiuola al centro della piazza
    m.rett(18, 19, 4, 5, "=")             # strada verso sud
    # laboratorio 7x5 in (28,8), porta (31,12)
    m.edificio(28, 8, "L", 7, 5)
    m.rett(31, 13, 1, 1, "=")
    m.rett(22, 13, 10, 1, "=")
    # negozio 5x4 in (6,7), porta (8,10)
    m.edificio(6, 7, "N", 5, 4)
    m.rett(8, 11, 1, 2, "=")
    m.rett(8, 12, 10, 1, "=")
    # casa del giocatore 5x4 in (8,18), porta (10,21)
    m.edificio(8, 18, "H", 5, 4)
    m.rett(10, 22, 1, 1, "=")
    m.rett(10, 23, 8, 1, "=")
    m.rett(11, 22, 1, 1, ".")
    # casa dei vicini 5x4 in (26,19), porta (28,22)
    m.edificio(26, 19, "K", 5, 4)
    m.rett(28, 23, 1, 1, "=")
    m.rett(22, 23, 7, 1, "=")
    # laghetto, alberi e aiuole
    m.rett(31, 24, 5, 3, "~")
    m.rett(4, 3, 2, 2, "T"); m.rett(34, 3, 2, 2, "T"); m.rett(4, 25, 2, 1, "T")
    m.rett(13, 26, 3, 1, "*"); m.rett(24, 4, 3, 1, "*"); m.rett(3, 14, 2, 2, "*"); m.rett(35, 16, 2, 2, "*")
    m.rett(12, 3, 2, 2, "T"); m.rett(26, 26, 2, 1, "*")
    # cartelli
    m.set(17, 3, "C"); m.set(27, 12, "C"); m.set(13, 22, "C"); m.set(23, 15, "C")

    m.add("SPAWN 10 22 GIU")
    m.add("USCITA 18 0 4 1 route1 28 23 SU")
    m.add("PORTA 31 12 laboratorio 7 7 SU")
    m.add("PORTA 8 10 negozio_lunaria 5 5 SU")
    m.add("PORTA 10 21 casa_giocatore 5 6 SU")
    m.add("PORTA 28 22 casa_vicini 4 5 SU")
    m.add("CARTELLO 17 3 CARTELLO_LUNARIA")
    m.add("CARTELLO 27 12 CARTELLO_LAB")
    m.add("CARTELLO 13 22 CARTELLO_CASA")
    m.add("CARTELLO 23 15 CARTELLO_PIAZZA")
    m.add("NPC kael 1 30 14 GIU FERMO KAEL_LUNARIA Kael")
    m.add("ALLENATORE kael KAEL_INIZIALE_VS_{STARTER} SCONFITTO_KAEL_1")
    m.add("NPC lina 3 22 17 SINISTRA VAGA LUNARIA_LINA Lina")
    m.add("NPC bruno 4 13 12 GIU GIRA LUNARIA_BRUNO Bruno")
    m.add("NPC pip 6 29 27 DESTRA VAGA LUNARIA_PIP Pip")
    m.add("NPC giardiniera 2 5 15 DESTRA GIRA LUNARIA_GIARDINIERA Marta")
    m.salva()


# ------------------------------------------------------------------
# ROUTE 1 - Sentiero dei Germogli 60x25
# entrata Lunaria (30,24) · uscita Bosco (30,0) · 2 allenatori · 4 zone d'erba · lago a est · CORE (45,14)
# ------------------------------------------------------------------
def route1():
    m = Mappa("route1", "ROUTE 1", 60, 25)
    bordo_alberi(m)
    # sentiero a S dall'ingresso sud al varco nord
    m.rett(28, 19, 4, 6, "=")      # ingresso sud (colonne 28-31, riga 24)
    m.rett(22, 16, 10, 3, "=")
    m.rett(22, 10, 4, 6, "=")
    m.rett(22, 10, 12, 3, "=")
    m.rett(30, 2, 4, 8, "=")
    m.rett(30, 0, 1, 2, "=")       # varco del Bosco Verdalia (presidiato, vedi NPC guardiano)
    m.rett(31, 1, 1, 1, "=")
    # quattro zone d'erba alta
    m.rett(6, 15, 10, 6, "w")
    m.rett(36, 17, 9, 5, "w")
    m.rett(8, 4, 11, 6, "w")
    m.rett(35, 3, 8, 6, "w")
    # lago a est
    m.rett(48, 7, 9, 10, "~")
    m.rett(47, 9, 1, 6, "~")
    # alberi e fiori
    for (x, y) in ((18, 13), (19, 13), (40, 12), (41, 12), (4, 10), (5, 10), (52, 19), (53, 19), (26, 4), (27, 4)):
        m.set(x, y, "T")
    m.rett(14, 22, 3, 1, "*"); m.rett(44, 4, 2, 2, "*"); m.rett(35, 13, 3, 1, "*")
    m.set(26, 22, "C"); m.set(29, 3, "C")

    m.add("SPAWN 29 23 SU")
    m.add("USCITA 28 24 4 1 lunaria 18 1 GIU")
    m.add("CARTELLO 26 22 CARTELLO_ROUTE1")
    m.add("CARTELLO 29 3 CARTELLO_BOSCO")
    m.add("NPC guardiano 7 30 1 GIU FERMO ROUTE1_GUARDIANO Esploratore Dario")
    m.add("NPC allenatore1 7 21 14 DESTRA GIRA ROUTE1_ALLENATORE_1 Esploratore Teo")
    m.add("NPC allenatore2 3 34 7 SINISTRA GIRA ROUTE1_ALLENATORE_2 Esploratrice Nina")
    m.add("NPC pescatore 4 46 12 DESTRA FERMO ROUTE1_PESCATORE Pescatore Ugo")
    m.add("NPC viaggiatore 5 12 22 SU VAGA ROUTE1_VIAGGIATORE Viaggiatore")
    m.add("OGGETTO 45 14 12 1 OGGETTO_R1_CORE")
    m.add("HABITAT ROUTE1")
    m.add("ALLENATORE allenatore1 ROUTE1_TEO SCONFITTO_ROUTE1_TEO")
    m.add("ALLENATORE allenatore2 ROUTE1_NINA SCONFITTO_ROUTE1_NINA")
    m.salva()


# ------------------------------------------------------------------
# Interni
# ------------------------------------------------------------------
def casa_giocatore():
    m = Mappa("casa_giocatore", "CASA", 12, 9, "INTERNO")
    m.da_righe([
        "############",
        "############",
        "#1_3____v_p#",
        "#2_4_______#",
        "#___hooh___#",
        "#__________#",
        "#rr________#",
        "#____x_____#",
        "############",
    ])
    m.add("SPAWN 2 4 GIU")
    m.add("USCITA 5 7 1 1 lunaria 10 22 GIU")
    m.add("CARTELLO 3 3 CASA_LIBRI")
    m.add("CARTELLO 8 2 CASA_TV")
    m.add("CARTELLO 1 3 CASA_LETTO")
    m.add("NPC mamma 2 8 5 SINISTRA FERMO CASA_MAMMA Mamma")
    m.add("CURATORE mamma")
    m.salva()


def laboratorio():
    m = Mappa("laboratorio", "LABORATORIO", 14, 10, "INTERNO")
    m.da_righe([
        "##############",
        "##############",
        "#3355_____55p#",
        "#4466_____66_#",
        "#____________#",
        "#____ooo_____#",
        "#____________#",
        "#g__________g#",
        "#______x_____#",
        "##############",
    ])
    m.add("SPAWN 7 7 SU")
    m.add("USCITA 7 8 1 1 lunaria 31 13 GIU")
    m.add("CARTELLO 1 3 LAB_LIBRI")
    m.add("CARTELLO 2 3 LAB_LIBRI")
    m.add("CARTELLO 3 3 LAB_MACCHINA")
    m.add("CARTELLO 4 3 LAB_MACCHINA")
    m.add("CARTELLO 10 3 LAB_MACCHINA_2")
    m.add("CARTELLO 11 3 LAB_MACCHINA_2")
    m.add("CARTELLO 5 5 LAB_CAPSULE")
    m.add("CARTELLO 6 5 LAB_CAPSULE")
    m.add("CARTELLO 7 5 LAB_CAPSULE")
    m.add("PC 1 7")
    m.add("PC 12 7")
    # scelta provvisoria dello starter (la scena con Elian e Kael arriva nella Fase 5)
    m.add("STARTER 5 5 PYRIN")
    m.add("STARTER 6 5 AQUIL")
    m.add("STARTER 7 5 MOSSEN")
    m.add("NPC elian 0 6 4 GIU FERMO ELIAN_LAB Prof. Elian")
    m.add("NPC assistente 4 10 6 SINISTRA VAGA LAB_ASSISTENTE Assistente Sora")
    m.salva()


def negozio():
    m = Mappa("negozio_lunaria", "EMPORIO", 11, 8, "INTERNO")
    m.da_righe([
        "###########",
        "###########",
        "#7_7_7___p#",
        "#8_8_8____#",
        "#_____ccc_#",
        "#_________#",
        "#____x____#",
        "###########",
    ])
    m.add("SPAWN 5 5 SU")
    m.add("USCITA 5 6 1 1 lunaria 8 11 GIU")
    m.add("CARTELLO 1 3 NEGOZIO_SCAFFALE")
    m.add("CARTELLO 3 3 NEGOZIO_SCAFFALE")
    m.add("CARTELLO 5 3 NEGOZIO_SCAFFALE")
    m.add("NPC commesso 4 7 3 GIU FERMO NEGOZIO_COMMESSO Commesso")
    m.add("NPC cliente 2 2 5 DESTRA GIRA NEGOZIO_CLIENTE Cliente")
    m.salva()


def casa_vicini():
    m = Mappa("casa_vicini", "CASA DEI VICINI", 10, 8, "INTERNO")
    m.da_righe([
        "##########",
        "##########",
        "#v__1__3p#",
        "#___2__4_#",
        "#__hoh___#",
        "#________#",
        "#___x____#",
        "##########",
    ])
    m.add("SPAWN 4 5 SU")
    m.add("USCITA 4 6 1 1 lunaria 28 23 GIU")
    m.add("CARTELLO 7 3 VICINI_LIBRI")
    m.add("CARTELLO 1 2 VICINI_TV")
    m.add("NPC nonno 5 6 5 SINISTRA FERMO VICINI_NONNO Nonno Aldo")
    m.add("NPC mia 6 2 5 DESTRA VAGA VICINI_MIA Mia")
    m.salva()


if __name__ == "__main__":
    lunaria()
    route1()
    casa_giocatore()
    laboratorio()
    negozio()
    casa_vicini()
    print("Mappe generate in", os.path.abspath(CARTELLA))
