"""
ECLYRA - Schede visive degli Elyon (Design System v1).

Ogni funzione ritorna (forme, materiali) in coordinate 64x64 (vista frontale di 3/4, muso a
sinistra, suolo a y = 58). Le famiglie evolutive condividono materiali, segni e "nodo di Lumen":
- PYRIN  -> PYRION  -> PYROVAR  : lince di brace -> lince con criniera -> leone-fucina corazzato
- AQUIL  -> AQUILON -> AQUARYS  : lontra-pinna -> lontra slanciata -> lontra di marea stellata
- MOSSEN -> MOSSKA  -> VERDANTOR: cinghialino di muschio -> cinghiale -> colosso di roccia e bosco
"""
import copy

from sprites import E, C, Poly, Fiamma, Occhio, Bocca, Gemma, Macchia, rampa
import palette as P

# ---------------------------------------------------------------- materiali condivisi
M_PYR = {
    "pelo": rampa("4A140E", "9A2E1A", "D9502A", "F2843E", "FFC27A"),
    "crema": rampa("7A4A30", "B88258", "E2BC8E", "F5DDB6", "FFF2DC"),
    "fuliggine": rampa("140C14", "2A1A24", "3E2A34", "56393F", "735052"),
    "ottone": rampa("3E2408", "7E5016", "BC842C", "E6B44C", "FFE08E"),
    "acciaio": rampa("1E2232", "3C4458", "66708A", "98A2BA", "D6DCEA"),
}
IRIDE_BRACE = (P.H("7A3A08"), P.H("E8941E"), P.H("FFD45A"))
GEMMA_BRACE = [P.H("6A1A08"), P.H("C8401A"), P.H("FF8A2E"), P.H("FFD27A"), P.H("FFF6D8")]

M_AQU = {
    "pelo": rampa("0E2A5C", "1D4F9E", "2F7FD6", "5CABF0", "A8DCFF"),
    "ventre": rampa("4A6E8E", "86AFC8", "BFDDEC", "E2F2FA", "FFFFFF"),
    "pinna": rampa("0C3A5A", "136C8E", "1FA3B8", "56D2D6", "B2F6F0"),
    "stelle": rampa("2A2470", "4E46B8", "8E86FF", "C8C2FF", "F2F0FF"),
}
IRIDE_MAREA = (P.H("0E3A5A"), P.H("1FA3B8"), P.H("8AF2E8"))

M_MOS = {
    "pelo": rampa("2A2216", "54442A", "806640", "A88A5A", "CDB383"),
    "muschio": rampa("143A1C", "245E2A", "3A8A38", "62B44A", "A6DE6A"),
    "pietra": rampa("23242E", "434656", "6A6E80", "9498A8", "C6C9D4"),
    "zanna": rampa("8A7A5A", "C0B08A", "E2D6B6", "F4ECD8", "FFFFFF"),
    "foglia": rampa("1A4A1E", "2E7A30", "4CAF4A", "86D45E", "C8F08A"),
}
IRIDE_BOSCO = (P.H("2A4A0E"), P.H("7AB82A"), P.H("D2F07A"))
GEMMA_BOSCO = [P.H("0E3A1A"), P.H("1E7A3A"), P.H("4CC85A"), P.H("A6F28A"), P.H("F0FFE0")]


# ================================================================ PYRIN (#001)
def pyrin():
    f = [
        # coda con brace in punta (dietro al corpo)
        C(41, 45, 50, 33, 4.0, "pelo", z=1, r2=3.2, tag=("su",)),
        Fiamma(51, 33, 11, 3.6, z=2, curva=0.25),
        # zampe lontane
        C(43, 48, 44, 56.5, 2.8, "pelo", z=2),
        C(25, 48, 25, 56.5, 2.5, "pelo", z=2),
        # corpo
        E(34, 45, 12.5, 8.5, "pelo", z=5, tag=("su",)),
        Macchia(30, 49.5, 7.5, 3.5, "crema"),
        Macchia(38, 40, 2.2, 1.4, "fuliggine"), Macchia(42, 42, 2.0, 1.3, "fuliggine"),
        # zampe vicine
        E(41, 49, 5.5, 6.0, "pelo", z=7),
        C(42, 52, 41, 57, 3.0, "pelo", z=8),
        C(29, 47, 28.5, 56.5, 3.1, "pelo", z=8),
        E(28, 57, 3.6, 1.8, "fuliggine", z=9),
        E(41, 57.2, 3.6, 1.7, "fuliggine", z=9),
        # orecchio lontano (dietro la testa)
        Poly([(27, 26), (33, 12), (35, 27)], "pelo", z=9, tag=("su",)),
        Poly([(29.5, 24), (33, 15), (33.5, 25)], "fuliggine", z=9.5, tag=("su",)),
        # testa
        E(24, 33, 11.5, 10.0, "pelo", z=10, tag=("su",)),
        Poly([(13.5, 34), (8, 40.5), (14, 40), (12.5, 43), (19, 40)], "crema", z=11, tag=("su",)),
        E(17.5, 37.5, 5.5, 3.6, "crema", z=11, tag=("su",)),
        E(13.2, 35.6, 1.7, 1.3, "fuliggine", z=12, tag=("su",)),
        # orecchio vicino
        Poly([(15, 27), (15.5, 11), (23.5, 24)], "pelo", z=12, tag=("su",)),
        Poly([(16.5, 25), (16.2, 14.5), (21, 23)], "fuliggine", z=12.5, tag=("su",)),
        Fiamma(15.6, 13, 6, 1.6, z=13, tag=("su",), curva=-0.1),
        Fiamma(33.2, 14, 5, 1.4, z=13, tag=("su",), curva=0.1),
        # stemma d'ottone con nodo di Lumen (marchio di famiglia)
        Poly([(21, 25.5), (24, 22), (27, 25.5), (24, 29)], "ottone", z=13, tag=("su", "viso")),
        Gemma(24, 25.5, 5, pal=GEMMA_BRACE, tag=("su", "viso")),
        Occhio(19, 32.5, 3, IRIDE_BRACE, sguardo=-1, espressione="deciso", tag=("su",)),
        Occhio(28, 31.5, 2, IRIDE_BRACE, sguardo=-1, espressione="deciso", tag=("su",)),
        Bocca([(14.5, 39.5), (17, 40.5), (19.5, 39.8)], zanna=True, tag=("su",)),
        # dietro: tre strisce scure sulla nuca
        Macchia(27, 26, 1.2, 2.5, "fuliggine", tag=("retro",)), Macchia(30, 27, 1.2, 2.5, "fuliggine", tag=("retro",)),
        Macchia(33, 28.5, 1.1, 2.2, "fuliggine", tag=("retro",)),
    ]
    return f, M_PYR


# ================================================================ PYRION (#002)
def pyrion():
    f = [
        C(44, 40, 56, 22, 4.5, "pelo", z=1, r2=3.4, tag=("su",)),
        Fiamma(57, 22, 15, 4.5, z=2, curva=0.3),
        C(46, 45, 48, 57, 3.4, "pelo", z=2),
        C(22, 44, 21, 57, 3.0, "fuliggine", z=2),
        E(36, 41, 15, 10, "pelo", z=5, tag=("su",)),
        Macchia(31, 46, 9, 4, "crema"),
        Macchia(40, 35, 2.6, 1.5, "fuliggine"), Macchia(45, 37, 2.4, 1.4, "fuliggine"), Macchia(49, 40, 2, 1.3, "fuliggine"),
        E(45, 46, 6.5, 7.5, "pelo", z=7),
        C(46, 50, 45, 57, 3.4, "fuliggine", z=8),
        C(27, 44, 26, 57, 3.6, "fuliggine", z=8),
        # bracciale d'ottone (anticipa la corazza di PYROVAR)
        E(26.5, 51, 4.2, 1.8, "ottone", z=9),
        E(26, 57.2, 4.2, 1.9, "fuliggine", z=9), E(45, 57.4, 4.2, 1.8, "fuliggine", z=9),
        # criniera di fiamme
        Fiamma(30, 30, 13, 5, z=4, curva=0.4, tag=("su",)),
        Fiamma(35, 32, 11, 4, z=4, curva=0.5, tag=("su",)),
        Poly([(26, 20), (31, 6), (34, 22)], "pelo", z=9, tag=("su",)),
        Poly([(28.5, 19), (31, 9), (32.5, 20)], "fuliggine", z=9.5, tag=("su",)),
        E(21, 28, 12, 10.5, "pelo", z=10, tag=("su",)),
        Poly([(10, 29), (4, 36), (10, 36), (8, 40), (16, 36)], "crema", z=11, tag=("su",)),
        E(13.5, 33, 6.5, 4, "crema", z=11, tag=("su",)),
        E(8.6, 31, 1.9, 1.4, "fuliggine", z=12, tag=("su",)),
        Poly([(11, 22), (11, 5), (20, 19)], "pelo", z=12, tag=("su",)),
        Poly([(12.5, 20), (12.3, 8.5), (17.5, 18)], "fuliggine", z=12.5, tag=("su",)),
        Fiamma(11.3, 7, 7, 1.8, z=13, tag=("su",)),
        Fiamma(31.2, 8, 6, 1.6, z=13, tag=("su",)),
        Poly([(17.5, 20), (21, 16), (24.5, 20), (21, 24)], "ottone", z=13, tag=("su", "viso")),
        Gemma(21, 20, 6, pal=GEMMA_BRACE, tag=("su", "viso")),
        Occhio(15.5, 27, 3, IRIDE_BRACE, sguardo=-1, espressione="fiero", tag=("su",)),
        Occhio(25, 26, 2, IRIDE_BRACE, sguardo=-1, espressione="fiero", tag=("su",)),
        Bocca([(9.5, 35), (13, 36.4), (16, 35.5)], zanna=True, tag=("su",)),
    ]
    return f, M_PYR


# ================================================================ PYROVAR (#003) Fiamma/Metallo
def pyrovar():
    f = [
        C(46, 34, 58, 14, 5, "pelo", z=1, r2=3.5, tag=("su",)),
        Fiamma(59, 15, 14, 5, z=2, curva=0.25),
        C(49, 42, 51, 57, 4.4, "acciaio", z=2),
        C(20, 42, 19, 57, 4.0, "acciaio", z=2),
        E(37, 38, 17, 12, "pelo", z=5, tag=("su",)),
        # piastre di corazza
        Poly([(30, 28), (48, 27), (52, 37), (34, 39)], "acciaio", z=6, tag=("su",)),
        Poly([(36, 30), (46, 29.5), (48, 34), (38, 35)], "ottone", z=6.5, tag=("su",)),
        Macchia(31, 45, 10, 4.5, "crema"),
        E(47, 44, 8, 9, "pelo", z=7),
        Poly([(42, 38), (54, 39), (53, 46), (44, 47)], "acciaio", z=7.5),
        C(48, 49, 47, 57, 4.4, "acciaio", z=8),
        C(26, 42, 25, 57, 4.6, "acciaio", z=8),
        E(25.5, 49, 5.2, 2.2, "ottone", z=9),
        E(25, 57.5, 5.5, 2.2, "fuliggine", z=9), E(47, 57.6, 5.5, 2.1, "fuliggine", z=9),
        # criniera ardente ampia
        Fiamma(28, 26, 18, 7, z=4, curva=0.45, tag=("su",)),
        Fiamma(36, 28, 15, 6, z=4, curva=0.55, tag=("su",)),
        Fiamma(22, 24, 14, 5, z=4, curva=0.2, tag=("su",)),
        E(20, 25, 12.5, 11, "pelo", z=10, tag=("su",)),
        Poly([(8, 25), (2, 33), (8, 33), (6, 37), (15, 33)], "crema", z=11, tag=("su",)),
        E(12, 30, 7, 4.5, "crema", z=11, tag=("su",)),
        E(6.6, 28, 2.0, 1.5, "fuliggine", z=12, tag=("su",)),
        # elmo d'acciaio con corna
        Poly([(12, 18), (20, 12), (30, 16), (28, 20), (14, 21)], "acciaio", z=12, tag=("su",)),
        Poly([(13, 17), (8, 4), (17, 14)], "ottone", z=12.5, tag=("su",)),
        Poly([(25, 14), (30, 2), (31, 16)], "ottone", z=11.5, tag=("su",)),
        Gemma(20, 16.5, 7, pal=GEMMA_BRACE, tag=("su",)),
        # nucleo di Lumen sul petto
        Gemma(24, 41, 6, pal=GEMMA_BRACE, tag=("su", "viso")),
        Occhio(14.5, 24.5, 3, IRIDE_BRACE, sguardo=-1, espressione="fiero", tag=("su",)),
        Occhio(24, 23.5, 2, IRIDE_BRACE, sguardo=-1, espressione="fiero", tag=("su",)),
        Bocca([(7, 32.5), (11, 34), (15, 33)], zanna=True, tag=("su",)),
    ]
    return f, M_PYR


# ================================================================ AQUIL (#004)
def aquil():
    f = [
        # coda a pinna
        C(42, 50, 52, 44, 3.2, "pelo", z=1, r2=2.2),
        Poly([(50, 46), (60, 36), (57, 46), (61, 52), (51, 48)], "pinna", z=2),
        Macchia(56.5, 42, 0.9, 0.9, "stelle"), Macchia(57.5, 48.5, 0.8, 0.8, "stelle"),
        C(44, 50, 45, 56.5, 2.6, "pelo", z=2),
        C(26, 50, 25, 56.5, 2.4, "pelo", z=2),
        E(35, 47, 11, 8.0, "pelo", z=5, tag=("su",)),
        Macchia(30, 50, 7, 4.2, "ventre"),
        E(42, 51, 5, 5, "pelo", z=7),
        C(42, 53, 41.5, 57, 2.8, "pelo", z=8),
        C(29, 49, 28, 56.5, 2.9, "pelo", z=8),
        E(28, 57, 3.3, 1.6, "pinna", z=9), E(41.5, 57.2, 3.3, 1.6, "pinna", z=9),
        # pinna dorsale
        Poly([(33, 40), (40, 33), (43, 41)], "pinna", z=4, tag=("su",)),
        # orecchie a pinna
        Poly([(26, 26), (33, 18), (33, 28)], "pinna", z=9, tag=("su",)),
        E(24, 35, 11, 9.5, "pelo", z=10, tag=("su",)),
        E(17.5, 39.5, 6, 4.2, "ventre", z=11, tag=("su",)),
        E(12.6, 37.6, 1.8, 1.3, "pelo", z=12, tag=("su",)),
        Poly([(14, 29), (9, 18), (20, 26)], "pinna", z=12, tag=("su",)),
        Macchia(13.3, 24, 1.0, 1.0, "stelle"),
        # baffi-pinna sulle guance
        Poly([(12, 42), (6, 44), (12, 44.5)], "pinna", z=12, tag=("su",)),
        Gemma(24, 27.5, 5, pal=P.AZZURRO, tag=("su", "viso")),
        Occhio(19, 34.5, 3, IRIDE_MAREA, sguardo=-1, espressione="felice", tag=("su",)),
        Occhio(28, 33.5, 2, IRIDE_MAREA, sguardo=-1, espressione="felice", tag=("su",)),
        Bocca([(14, 41.4), (16.5, 42.6), (19, 41.6)], tag=("su",)),
        Macchia(28, 30, 3, 2, "pinna", tag=("retro",)),
    ]
    return f, M_AQU


# ================================================================ AQUARYS (#006) Acqua/Astrale
def aquarys():
    f = [
        C(42, 44, 54, 30, 4.5, "pelo", z=1, r2=3),
        Poly([(52, 33), (63, 16), (61, 30), (64, 40), (54, 36)], "stelle", z=2),
        Macchia(59, 24, 1, 1, "ventre"), Macchia(61, 33, 1, 1, "ventre"), Macchia(57, 30, 0.8, 0.8, "ventre"),
        C(46, 46, 48, 57, 3.6, "pelo", z=2),
        C(23, 45, 22, 57, 3.2, "pelo", z=2),
        E(36, 42, 15, 10, "pelo", z=5, tag=("su",)),
        Macchia(30, 46, 9.5, 4.5, "ventre"),
        Macchia(40, 37, 1.1, 1.1, "stelle"), Macchia(45, 39, 1, 1, "stelle"), Macchia(36, 38, 0.9, 0.9, "stelle"),
        E(45, 47, 6.5, 7, "pelo", z=7),
        C(46, 51, 45, 57, 3.4, "pelo", z=8),
        C(28, 45, 27, 57, 3.6, "pelo", z=8),
        E(27, 57.3, 4.2, 1.9, "pinna", z=9), E(45, 57.4, 4.2, 1.8, "pinna", z=9),
        Poly([(32, 33), (44, 18), (48, 35)], "stelle", z=4, tag=("su",)),
        Poly([(38, 32), (44, 22), (45, 33)], "pinna", z=4.5, tag=("su",)),
        Poly([(24, 20), (34, 9), (34, 23)], "stelle", z=9, tag=("su",)),
        E(21, 29, 12, 10.5, "pelo", z=10, tag=("su",)),
        E(13.5, 34, 7, 4.5, "ventre", z=11, tag=("su",)),
        E(8, 32, 2, 1.5, "pelo", z=12, tag=("su",)),
        Poly([(10, 22), (2, 8), (17, 18)], "stelle", z=12, tag=("su",)),
        Poly([(11, 21), (5, 11.5), (15, 18.5)], "pinna", z=12.5, tag=("su",)),
        Poly([(8, 37), (0, 40), (8, 40)], "pinna", z=12, tag=("su",)),
        # alone stellare
        Gemma(21, 19.5, 8, pal=P.AZZURRO, tag=("su",)),
        Gemma(31, 15, 4, pal=[P.TIPI["ASTRALE"][0], P.TIPI["ASTRALE"][1], P.TIPI["ASTRALE"][2], P.H("F2F0FF"), P.H("FFFFFF")], tag=("su",)),
        Gemma(12, 14, 4, pal=[P.TIPI["ASTRALE"][0], P.TIPI["ASTRALE"][1], P.TIPI["ASTRALE"][2], P.H("F2F0FF"), P.H("FFFFFF")], tag=("su",)),
        Occhio(15.5, 28, 3, IRIDE_MAREA, sguardo=-1, espressione="fiero", tag=("su",)),
        Occhio(25, 27, 2, IRIDE_MAREA, sguardo=-1, espressione="fiero", tag=("su",)),
        Bocca([(9, 35.8), (12, 37), (15, 36)], tag=("su",)),
    ]
    return f, M_AQU


# ================================================================ MOSSEN (#007)
def mossen():
    f = [
        C(43, 50, 50, 46, 2.4, "pelo", z=1),
        E(51, 45, 2.6, 2.4, "muschio", z=1.5),
        C(44, 51, 45, 57, 3.0, "pelo", z=2),
        C(25, 51, 24.5, 57, 2.8, "pelo", z=2),
        E(35, 47, 13, 9.0, "pelo", z=5, tag=("su",)),
        # manto di muschio sul dorso, con sassolini
        E(38, 41.5, 11, 5.5, "muschio", z=6, tag=("su",)),
        E(31, 41, 4, 3.0, "muschio", z=6.2, tag=("su",)),
        E(42, 39, 2.2, 1.7, "pietra", z=6.5, tag=("su",)),
        E(35, 38.5, 1.6, 1.2, "pietra", z=6.5, tag=("su",)),
        Macchia(30, 51, 8, 3.5, "zanna"),
        E(42, 51, 5.5, 5.5, "pelo", z=7),
        C(42, 53, 41.5, 57.2, 3.2, "pelo", z=8),
        C(29, 50, 28.5, 57.2, 3.2, "pelo", z=8),
        E(28.5, 57.5, 3.6, 1.6, "pietra", z=9), E(41.5, 57.6, 3.6, 1.5, "pietra", z=9),
        # germoglio sul capo
        C(26, 26, 26.5, 21, 0.9, "foglia", z=9, tag=("su",)),
        Poly([(26.5, 22), (33, 16), (31, 23)], "foglia", z=9.2, tag=("su",)),
        Poly([(26, 22.5), (20, 18), (22, 24)], "foglia", z=9.2, tag=("su",)),
        E(23, 36, 11, 9.5, "pelo", z=10, tag=("su",)),
        E(15.5, 39.5, 6.5, 4.6, "pelo", z=11, tag=("su",)),
        E(11, 39, 2.6, 2.3, "pietra", z=12, tag=("su",)),
        E(10.4, 38.3, 0.7, 0.9, "pelo", z=12.5, tag=("su",)),
        # zannette di pietra
        Poly([(14, 43), (12.5, 47), (16, 44)], "zanna", z=13, tag=("su",)),
        Poly([(29, 30), (34, 26), (33, 32)], "pelo", z=9.5, tag=("su",)),
        Poly([(17, 30), (14, 24), (21, 28)], "pelo", z=12, tag=("su",)),
        Macchia(18, 27.5, 1.5, 1.2, "muschio"),
        Gemma(27, 26.5, 4, pal=GEMMA_BOSCO, tag=("su",)),
        Occhio(18, 35.5, 3, IRIDE_BOSCO, sguardo=-1, espressione="felice", tag=("su",)),
        Occhio(26.5, 35, 2, IRIDE_BOSCO, sguardo=-1, espressione="felice", tag=("su",)),
        Bocca([(13, 43), (16, 43.6)], tag=("su",)),
    ]
    return f, M_MOS


# ================================================================ VERDANTOR (#009) Natura/Roccia
def verdantor():
    f = [
        C(47, 44, 58, 38, 4, "pelo", z=1),
        E(59, 37, 4, 3.5, "muschio", z=1.5),
        C(50, 45, 52, 57, 5, "pietra", z=2),
        C(20, 44, 19, 57, 4.6, "pietra", z=2),
        E(37, 40, 18, 13, "pelo", z=5, tag=("su",)),
        # dorso di roccia muschiosa con alberelli
        Poly([(22, 34), (30, 22), (46, 20), (56, 30), (54, 38), (28, 38)], "pietra", z=6, tag=("su",)),
        E(38, 25.5, 12, 5, "muschio", z=6.5, tag=("su",)),
        E(27, 30, 5, 3, "muschio", z=6.6, tag=("su",)),
        C(42, 22, 43, 12, 1.1, "pelo", z=7, tag=("su",)),
        E(43, 10, 5.5, 4.5, "foglia", z=7.5, tag=("su",)),
        E(46, 8, 3.5, 3, "foglia", z=7.6, tag=("su",)),
        C(33, 23, 32, 16, 1.0, "pelo", z=7, tag=("su",)),
        E(32, 14.5, 4, 3.4, "foglia", z=7.5, tag=("su",)),
        Macchia(30, 47, 10, 4.5, "zanna"),
        E(48, 46, 8.5, 9, "pelo", z=7),
        C(49, 50, 48, 57, 5, "pietra", z=8),
        C(26, 44, 25, 57, 5, "pietra", z=8),
        E(25, 57.6, 6, 2.2, "pietra", z=9), E(48, 57.6, 6, 2.2, "pietra", z=9),
        E(20, 33, 12.5, 11, "pelo", z=10, tag=("su",)),
        E(11, 37, 7.5, 5.5, "pelo", z=11, tag=("su",)),
        E(5.6, 36, 3, 2.8, "pietra", z=12, tag=("su",)),
        # zanne di roccia
        Poly([(9, 41), (3, 33), (6, 43)], "zanna", z=13, tag=("su",)),
        Poly([(15, 42), (12, 35), (12.5, 44)], "zanna", z=12.8, tag=("su",)),
        Poly([(12, 25), (8, 17), (18, 23)], "pietra", z=12, tag=("su",)),
        Gemma(23, 24, 6, pal=GEMMA_BOSCO, tag=("su",)),
        Occhio(14.5, 31, 3, IRIDE_BOSCO, sguardo=-1, espressione="fiero", tag=("su",)),
        Occhio(23, 30.5, 2, IRIDE_BOSCO, sguardo=-1, espressione="fiero", tag=("su",)),
        Bocca([(8, 42), (12, 42.6)], tag=("su",)),
    ]
    return f, M_MOS


SCHEDE = {
    "PYRIN": pyrin, "PYRION": pyrion, "PYROVAR": pyrovar,
    "AQUIL": aquil, "AQUARYS": aquarys,
    "MOSSEN": mossen, "VERDANTOR": verdantor,
}


# ================================================================ altre specie usate nei mockup
M_LUMI = {
    "corpo": rampa("3A3290", "6A62D8", "A49CFF", "D6D2FF", "FFFFFF"),
    "petalo": rampa("6A4A10", "B88A2A", "E8C25A", "FFE69A", "FFFBE6"),
}
IRIDE_STELLA = (P.H("2A2470"), P.H("6A62D8"), P.H("C8C2FF"))
FUMO_OMBRA = [P.H("120A20"), P.H("2E1E50"), P.H("4A3678"), P.H("7A5CB8"), P.H("B8A2F0")]
FUOCO_GELO = [P.H("3A84A8"), P.H("6EC8EE"), P.H("A8E8FF"), P.H("DDF6FF"), P.H("FFFFFF")]


def lumi():
    """#028 LUMI (Astrale): spiritello stellare che fluttua, petali di luce come ali"""
    f = [
        Poly([(34, 34), (52, 22), (50, 34), (58, 38), (40, 40)], "petalo", z=1, tag=("su",)),
        Poly([(26, 36), (14, 26), (16, 38), (22, 42)], "petalo", z=1, tag=("su",)),
        E(32, 38, 12, 11, "corpo", z=5, tag=("su",)),
        Poly([(32, 16), (35, 25), (44, 26), (37, 31), (40, 40), (32, 34), (24, 40), (27, 31), (20, 26), (29, 25)], "corpo", z=6, tag=("su",)),
        E(32, 44, 6, 3, "petalo", z=7, tag=("su",)),
        C(32, 49, 32, 56, 1.4, "corpo", z=2, r2=0.6),
        Gemma(32, 25, 6, pal=M_LUMI["petalo"], tag=("su",)),
        Occhio(27, 37, 3, IRIDE_STELLA, sguardo=-1, espressione="felice", tag=("su",)),
        Occhio(36, 37, 3, IRIDE_STELLA, sguardo=-1, espressione="felice", tag=("su",)),
        Bocca([(30, 43), (32, 44), (34, 43)], tag=("su",)),
    ]
    return f, M_LUMI


M_SHA = {
    "pelo": rampa("0E0A1A", "221A3A", "3A2E5E", "584886", "8270B8"),
    "segni": rampa("4A0E1A", "8A1E2E", "D03A4A", "FF7A7A", "FFD0D0"),
    "ventre": rampa("2A2440", "4A4268", "6E6690", "948CB4", "C2BCDA"),
}
IRIDE_OMBRA = (P.H("6A0E1A"), P.H("E03A3A"), P.H("FFB0A0"))


def shadek():
    """#026 SHADEK (Ombra): furetto d'ombra con mezzelune rosse e coda di fumo"""
    f = [
        Fiamma(51, 46, 16, 4.5, z=1, pal=FUMO_OMBRA, curva=0.35),
        C(43, 50, 44, 57, 2.3, "pelo", z=2), C(25, 50, 24.5, 57, 2.2, "pelo", z=2),
        E(35, 47, 13, 7.5, "pelo", z=5, tag=("su",)),
        Macchia(31, 50.5, 7, 2.6, "ventre"),
        Macchia(38, 43, 2.2, 1.1, "segni"), Macchia(43, 44.5, 2, 1, "segni"),
        E(42, 50, 5, 5, "pelo", z=7),
        C(42, 53, 41.5, 57.2, 2.5, "pelo", z=8), C(29, 50, 28.5, 57.2, 2.6, "pelo", z=8),
        Poly([(25, 30), (31, 17), (34, 31)], "pelo", z=9, tag=("su",)),
        E(23, 37, 10.5, 8.5, "pelo", z=10, tag=("su",)),
        Poly([(14, 37), (6, 41), (15, 43), (19, 40)], "ventre", z=11, tag=("su",)),
        E(7.5, 40.6, 1.6, 1.2, "segni", z=12, tag=("su",)),
        Poly([(13, 31), (12, 15), (21, 28)], "pelo", z=12, tag=("su",)),
        Poly([(14.5, 29), (13.8, 19), (18.5, 27.5)], "segni", z=12.5, tag=("su",)),
        Poly([(22, 29), (26, 27), (24, 31.5)], "segni", z=13, tag=("su", "viso")),
        Occhio(17.5, 36, 3, IRIDE_OMBRA, sguardo=-1, espressione="fiero", tag=("su",)),
        Occhio(26, 35.5, 2, IRIDE_OMBRA, sguardo=-1, espressione="fiero", tag=("su",)),
        Bocca([(10, 42), (13, 43), (16, 42.4)], zanna=True, tag=("su",)),
    ]
    return f, M_SHA


M_FRO = {
    "piuma": rampa("3A6A9A", "6A9ED2", "A6D2F2", "DDF0FF", "FFFFFF"),
    "mantello": rampa("1A3A6A", "2A5AA0", "4A86D2", "86B8EE", "C6E2FF"),
    "becco": rampa("6A4A10", "B88A2A", "E8C25A", "FFE69A", "FFFBE6"),
}
IRIDE_GELO = (P.H("1A3A6A"), P.H("4A9EE0"), P.H("C6F0FF"))


def froli():
    """#024 FROLI (Gelo): civettina delle nevi con cresta di ghiaccioli e mantellina di brina"""
    f = [
        C(40, 54, 44, 57, 1.4, "becco", z=1), C(26, 54, 22, 57, 1.4, "becco", z=1),
        E(32, 42, 14, 14, "piuma", z=5, tag=("su",)),
        Poly([(18, 40), (10, 52), (22, 50)], "mantello", z=6, tag=("su",)),
        Poly([(46, 40), (55, 52), (42, 50)], "mantello", z=4, tag=("su",)),
        Macchia(32, 47, 8, 7, "mantello"),
        Macchia(32, 47, 6, 5.5, "piuma"),
        Poly([(22, 30), (20, 18), (27, 28)], "mantello", z=7, tag=("su",)),
        Poly([(29, 28), (32, 14), (35, 28)], "mantello", z=7, tag=("su",)),
        Poly([(37, 28), (44, 18), (42, 30)], "mantello", z=7, tag=("su",)),
        Fiamma(32, 16, 6, 1.4, z=8, pal=FUOCO_GELO, tag=("su",)),
        Poly([(29, 39), (32, 44), (35, 39)], "becco", z=9, tag=("su",)),
        Occhio(26, 36, 4, IRIDE_GELO, sguardo=0, espressione="felice", tag=("su",)),
        Occhio(38, 36, 4, IRIDE_GELO, sguardo=0, espressione="felice", tag=("su",)),
    ]
    return f, M_FRO


M_LEA = {
    "pelo": rampa("2A3A16", "4A6A26", "6E9A3A", "9AC85A", "CDEB8A"),
    "foglia": M_MOS["foglia"],
    "crema": M_PYR["crema"],
    "petalo": rampa("6A1A3A", "B83A6A", "E8709A", "FFB0C8", "FFE8F0"),
}


def leafora():
    """#017 LEAFORA (Natura): cerva di foglie, orecchie a foglia lanceolata, coda a corolla"""
    f = [
        Poly([(46, 34), (58, 26), (57, 36), (62, 40), (50, 40)], "petalo", z=1, tag=("su",)),
        C(46, 44, 48, 57, 2.2, "pelo", z=2), C(26, 44, 24, 57, 2.1, "pelo", z=2),
        E(37, 39, 13, 8.5, "pelo", z=5, tag=("su",)),
        Macchia(33, 43, 8, 3.2, "crema"),
        Macchia(40, 34.5, 1.4, 1, "crema"), Macchia(44, 36, 1.3, 1, "crema"), Macchia(36, 35, 1.2, 0.9, "crema"),
        C(44, 44, 45, 57, 2.4, "pelo", z=7), C(30, 44, 29, 57, 2.4, "pelo", z=8),
        E(29, 57.3, 2.6, 1.3, "pelo", z=9), E(45, 57.3, 2.6, 1.3, "pelo", z=9),
        C(28, 37, 25, 30, 4.4, "pelo", z=9, tag=("su",), r2=3.8),
        Poly([(27, 22), (39, 10), (34, 24)], "foglia", z=9.5, tag=("su",)),
        E(22, 26, 8, 7, "pelo", z=10, tag=("su",)),
        E(15, 29, 5, 3.6, "crema", z=11, tag=("su",)),
        E(10.6, 28, 1.5, 1.2, "pelo", z=12, tag=("su",)),
        Poly([(17, 21), (5, 12), (15, 24)], "foglia", z=12, tag=("su",)),
        Gemma(23, 19, 5, pal=GEMMA_BOSCO, tag=("su",)),
        Occhio(18.5, 25, 3, IRIDE_BOSCO, sguardo=-1, espressione="neutro", tag=("su",)),
        Occhio(26, 24.5, 2, IRIDE_BOSCO, sguardo=-1, espressione="neutro", tag=("su",)),
        Bocca([(12, 31.5), (15, 32)], tag=("su",)),
    ]
    return f, M_LEA


M_AUR = {
    "corpo": rampa("06040E", "140C2A", "2A1A50", "463080", "7A5CC8"),
    "oro": rampa("3D2709", "7E5016", "C8922E", "F0C55A", "FFF0B0"),
    "astrale": rampa("2A2470", "5A50D0", "9C94FF", "D6D2FF", "FFFFFF"),
    "ventre": rampa("1A1A3A", "2E3060", "4A5090", "7A84C2", "B8C0EE"),
}
IRIDE_AURION = (P.H("6A4A08"), P.H("F0C55A"), P.H("FFFFFF"))
NEBULA = [P.H("1A0A3A"), P.H("4A1E8A"), P.H("8A4AE0"), P.H("C8A0FF"), P.H("FFFFFF")]


def aurion():
    """#100 AURION (Astrale/Ombra): primordiale. Leone-drago cosmico, ali di nebulosa,
    corna d'oro, anello di sette cristalli (i Sigilli). Disegnato per 128x128 (scala 2)."""
    f = [
        # ali di nebulosa (dietro)
        Poly([(40, 26), (58, 2), (63, 14), (60, 30), (50, 36)], "astrale", z=0, tag=("su",)),
        Poly([(44, 28), (56, 10), (58, 26), (50, 33)], "corpo", z=0.5, tag=("su",)),
        Poly([(22, 24), (6, 4), (4, 18), (10, 30), (20, 34)], "astrale", z=0, tag=("su",)),
        Fiamma(56, 40, 18, 5, z=1, pal=NEBULA, curva=0.45),
        C(46, 46, 50, 58, 4.4, "corpo", z=2), C(20, 46, 18, 58, 4, "corpo", z=2),
        E(35, 42, 16, 11, "corpo", z=5, tag=("su",)),
        Macchia(31, 47, 10, 4.5, "ventre"),
        Macchia(40, 37, 1, 1, "astrale"), Macchia(45, 40, 1, 1, "astrale"), Macchia(36, 38, 0.8, 0.8, "astrale"), Macchia(48, 43, 0.8, 0.8, "astrale"),
        E(45, 47, 7, 8, "corpo", z=7),
        C(46, 51, 45, 58, 4.4, "corpo", z=8), C(26, 45, 25, 58, 4.6, "corpo", z=8),
        E(25, 58.2, 5.6, 2, "oro", z=9), E(45, 58.2, 5.6, 2, "oro", z=9),
        # criniera di cosmo
        Fiamma(30, 30, 18, 8, z=4, pal=NEBULA, curva=0.5, tag=("su",)),
        Fiamma(38, 32, 14, 6, z=4, pal=NEBULA, curva=0.6, tag=("su",)),
        E(21, 27, 11, 10, "corpo", z=10, tag=("su",)),
        E(13, 31, 6.5, 4.5, "ventre", z=11, tag=("su",)),
        Poly([(15, 20), (6, 4), (19, 17)], "oro", z=12, tag=("su",)),
        Poly([(25, 18), (30, 2), (29, 19)], "oro", z=9.5, tag=("su",)),
        Gemma(20, 19, 6, pal=M_AUR["astrale"], tag=("su",)),
        Gemma(32, 44, 8, pal=M_AUR["oro"], tag=("su", "viso")),
        Occhio(15.5, 26, 3, IRIDE_AURION, sguardo=-1, espressione="fiero", tag=("su",)),
        Occhio(24, 25, 2, IRIDE_AURION, sguardo=-1, espressione="fiero", tag=("su",)),
        Bocca([(8, 33), (12, 34.4), (16, 33.5)], zanna=True, tag=("su",)),
    ]
    return f, M_AUR


SCHEDE.update({"LUMI": lumi, "SHADEK": shadek, "FROLI": froli, "LEAFORA": leafora, "AURION": aurion})


# ================================================================ AQUILON (#005) - stadio intermedio
def aquilon():
    """Lontra slanciata: pinne piu' ampie di AQUIL, prime due stelle sulla coda (anticipa AQUARYS)"""
    f = [
        C(42, 47, 53, 38, 3.8, "pelo", z=1, r2=2.6),
        Poly([(51, 40), (62, 27), (60, 38), (63, 47), (53, 43)], "pinna", z=2),
        Macchia(58.5, 33, 0.9, 0.9, "stelle"), Macchia(59.5, 42, 0.9, 0.9, "stelle"),
        C(45, 48, 46, 57, 3.0, "pelo", z=2),
        C(24, 48, 23, 57, 2.8, "pelo", z=2),
        E(35, 44, 13, 8.8, "pelo", z=5, tag=("su",)),
        Macchia(30, 48, 8.5, 4.4, "ventre"),
        E(44, 49, 5.8, 6, "pelo", z=7),
        C(44, 52, 43.5, 57, 3.1, "pelo", z=8),
        C(28, 47, 27, 57, 3.2, "pelo", z=8),
        E(27, 57.2, 3.8, 1.7, "pinna", z=9), E(43.5, 57.3, 3.8, 1.7, "pinna", z=9),
        Poly([(32, 37), (41, 26), (45, 38)], "pinna", z=4, tag=("su",)),
        Poly([(25, 23), (34, 13), (34, 26)], "pinna", z=9, tag=("su",)),
        E(22.5, 32, 11.5, 10, "pelo", z=10, tag=("su",)),
        E(15, 37, 6.6, 4.4, "ventre", z=11, tag=("su",)),
        E(9.8, 35, 1.9, 1.4, "pelo", z=12, tag=("su",)),
        Poly([(12, 25), (5, 12), (18, 22)], "pinna", z=12, tag=("su",)),
        Macchia(9.5, 17.5, 1, 1, "stelle"),
        Poly([(10, 40), (2, 42.5), (10, 43)], "pinna", z=12, tag=("su",)),
        Gemma(22.5, 23, 6, pal=P.AZZURRO, tag=("su", "viso")),
        Occhio(17.5, 31.5, 3, IRIDE_MAREA, sguardo=-1, espressione="deciso", tag=("su",)),
        Occhio(26.5, 30.5, 2, IRIDE_MAREA, sguardo=-1, espressione="deciso", tag=("su",)),
        Bocca([(11, 38.6), (14, 39.8), (17, 39)], tag=("su",)),
        Macchia(27, 28, 3, 2, "pinna", tag=("retro",)),
    ]
    return f, M_AQU


# ================================================================ MOSSKA (#008) - stadio intermedio
def mosska():
    """Cinghiale di muschio: il manto copre tutto il dorso, spalle di pietra, due germogli, zanne piu' lunghe"""
    f = [
        C(45, 48, 54, 43, 2.8, "pelo", z=1),
        E(55, 42, 3.2, 3, "muschio", z=1.5),
        C(47, 49, 48, 57, 3.8, "pelo", z=2),
        C(22, 48, 21, 57, 3.6, "pelo", z=2),
        E(36, 44, 15.5, 10.5, "pelo", z=5, tag=("su",)),
        E(39, 37, 13.5, 6.5, "muschio", z=6, tag=("su",)),
        E(28, 37, 5, 3.6, "muschio", z=6.2, tag=("su",)),
        E(44, 34.5, 2.6, 2, "pietra", z=6.5, tag=("su",)),
        E(36, 33.5, 2, 1.5, "pietra", z=6.5, tag=("su",)),
        C(41, 33, 42, 27, 0.9, "foglia", z=6.8, tag=("su",)),
        Poly([(42, 28), (48, 23), (46, 30)], "foglia", z=6.9, tag=("su",)),
        Macchia(30, 49, 9, 4, "zanna"),
        E(46, 48, 7, 7.5, "pelo", z=7),
        Poly([(40, 41), (51, 40), (52, 46), (42, 47)], "pietra", z=7.5),
        C(46.5, 51, 45.5, 57.3, 3.8, "pelo", z=8),
        C(28, 47, 27, 57.3, 3.9, "pelo", z=8),
        E(27, 57.6, 4.5, 1.8, "pietra", z=9), E(45.5, 57.7, 4.5, 1.7, "pietra", z=9),
        C(24, 24, 24.5, 18, 1.0, "foglia", z=9, tag=("su",)),
        Poly([(24.5, 19), (32, 12), (30, 20)], "foglia", z=9.2, tag=("su",)),
        Poly([(24, 19.5), (17, 14), (19, 21)], "foglia", z=9.2, tag=("su",)),
        E(21, 33, 11.5, 10, "pelo", z=10, tag=("su",)),
        E(13, 37, 7, 5, "pelo", z=11, tag=("su",)),
        E(8, 36.5, 2.9, 2.6, "pietra", z=12, tag=("su",)),
        E(7.3, 35.8, 0.8, 1.0, "pelo", z=12.5, tag=("su",)),
        Poly([(11, 41), (6.5, 35), (13, 43)], "zanna", z=13, tag=("su",)),
        Poly([(28, 27), (34, 23), (32, 29)], "pelo", z=9.5, tag=("su",)),
        Poly([(15, 27), (11, 20), (19, 25)], "pelo", z=12, tag=("su",)),
        Gemma(25, 23.5, 5, pal=GEMMA_BOSCO, tag=("su",)),
        Occhio(16, 32, 3, IRIDE_BOSCO, sguardo=-1, espressione="deciso", tag=("su",)),
        Occhio(24.5, 31.5, 2, IRIDE_BOSCO, sguardo=-1, espressione="deciso", tag=("su",)),
        Bocca([(10, 41), (14, 41.6)], tag=("su",)),
    ]
    return f, M_MOS


SCHEDE.update({"AQUILON": aquilon, "MOSSKA": mosska})
LINEE_STARTER = [["PYRIN", "PYRION", "PYROVAR"], ["AQUIL", "AQUILON", "AQUARYS"], ["MOSSEN", "MOSSKA", "VERDANTOR"]]


# ================================================================ taglia per stadio
# Le forme base occupano meno spazio delle finali: la crescita si legge a colpo d'occhio.
# La scala e' applicata alle FORME attorno al punto d'appoggio (32, 58): i pixel restano nitidi.
TAGLIA_STADIO = {"PYRIN": 0.84, "AQUIL": 0.84, "MOSSEN": 0.84, "PYRION": 0.93, "AQUILON": 0.93, "MOSSKA": 0.93}


def _scala_attorno(forme, s, ax=32.0, ay=58.0):
    import copy
    out = []
    for f in forme:
        g = copy.deepcopy(f)
        k = g.k
        for cx, cy in (("cx", "cy"), ("x1", "y1"), ("x2", "y2")):
            if cx in k:
                k[cx] = ax + (k[cx] - ax) * s
            if cy in k:
                k[cy] = ay + (k[cy] - ay) * s
        if "base" in k:
            k["base"] = ay + (k["base"] - ay) * s
            k["h"] = k["h"] * s
            k["w"] = k["w"] * s
        for chiave in ("rx", "ry", "r2"):
            if chiave in k:
                k[chiave] *= s
        if "r" in k and g.tipo != "occhio":
            k["r"] *= s
        if g.tipo == "gemma":
            k["h"] = k["h"] * s
        if "punti" in k:
            k["punti"] = [(ax + (x - ax) * s, ay + (y - ay) * s) for x, y in k["punti"]]
        out.append(g)
    return out


def _con_taglia(nome, fn):
    def scheda():
        f, m = fn()
        s = TAGLIA_STADIO.get(nome, 1.0)
        return (_scala_attorno(f, s), m) if s != 1.0 else (f, m)
    return scheda


for _n in list(TAGLIA_STADIO):
    SCHEDE[_n] = _con_taglia(_n, SCHEDE[_n])
