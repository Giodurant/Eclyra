"""
ECLYRA - Palette ufficiale (Design System v1).

Ogni colore ha un nome stabile: gli script grafici e il codice C (src/palette.h, generato)
usano solo questi valori. Le rampe vanno dal piu' scuro al piu' chiaro.
"""


def H(s):
    s = s.lstrip("#")
    return (int(s[0:2], 16), int(s[2:4], 16), int(s[4:6], 16), 255)


# --- Interfaccia: notte, Lumen, oro, pergamena -------------------------------
NOTTE = [H("070A18"), H("0D1330"), H("141D45"), H("1D2A5E"), H("2A3C7A")]   # fondo dei pannelli
LUMEN = [H("0E4D5C"), H("12808A"), H("2BC4C0"), H("7FF2E2"), H("E6FFFA")]   # cristallo, energia
AZZURRO = [H("1B3F8F"), H("2E6FD0"), H("54A6F2"), H("A6DBFF"), H("E2F4FF")]
ORO = [H("3D2709"), H("6E4A16"), H("A8772E"), H("E0B04A"), H("FFE596"), H("FFF8DE")]
PERGAMENA = [H("8C7651"), H("C2AA7E"), H("E4D3AC"), H("F3E9CF"), H("FBF6E8")]
INCHIOSTRO = H("1A1F3D")          # testo su pergamena
CREMA = H("F3EAD2")               # testo su notte
GRIGIO_UI = H("8E98BE")           # testo secondario su notte
OMBRA_TESTO = H("05071A")

# --- Mondo --------------------------------------------------------------------
ERBA = [H("143A26"), H("1F5A32"), H("2F7D3C"), H("4FA548"), H("86CF5E"), H("C9EE8A")]
TERRA = [H("3E2615"), H("6A4426"), H("9A6A3E"), H("C89A62"), H("E8CB95")]
ACQUA = [H("0B2452"), H("123C80"), H("1D5FB3"), H("3A8FDC"), H("7CC6F2"), H("D5F1FF")]
PIETRA = [H("1F2230"), H("383C50"), H("5C6178"), H("878DA3"), H("B9BED0"), H("E2E5EE")]
LEGNO = [H("2E1A0E"), H("553219"), H("7E4D27"), H("A8703A"), H("D29E5E")]
INTONACO = [H("6F6A7E"), H("A6A2B6"), H("D3D0DE"), H("EFEDF5")]
TETTO_BLU = [H("121C44"), H("1E2F6E"), H("2E4A9E"), H("4C70C8"), H("86A8EE")]
TETTO_ROSSO = [H("3A1418"), H("6A2028"), H("9A3236"), H("C8524A"), H("E88C6E")]   # case di Verdan/Ember
FIORI = [H("E85A7A"), H("F2C14A"), H("A57AF0"), H("F5F0FF")]
PELLE = [H("5A3424"), H("9A5E40"), H("D2936A"), H("F2C29A")]

# --- 12 tipi: (scuro, base, chiaro). Mai usare solo il colore: ogni tipo ha anche il suo glifo.
TIPI = {
    "NEUTRO":  (H("5C5F70"), H("9EA1B0"), H("D6D8E2")),
    "FIAMMA":  (H("8C2414"), H("E8542F"), H("FFB36B")),
    "ACQUA":   (H("12407E"), H("2F86E0"), H("8FD0FF")),
    "NATURA":  (H("1E5E2A"), H("4CAF4A"), H("A8E07A")),
    "IMPULSO": (H("8A6410"), H("F2C230"), H("FFF09A")),
    "VENTO":   (H("2A7E72"), H("6FD3BE"), H("D2FFF2")),
    "ROCCIA":  (H("5A3E20"), H("A57A4A"), H("D9BC8C")),
    "GELO":    (H("3A84A8"), H("9EE6FF"), H("F0FCFF")),
    "OMBRA":   (H("1E1530"), H("4A3678"), H("8E76C8")),
    "ASTRALE": (H("3A3290"), H("8E86FF"), H("DCD8FF")),
    "TOSSICO": (H("5C1E70"), H("B04FD0"), H("E2A6F2")),
    "PSICHE":  (H("8A1E52"), H("F0609E"), H("FFB8D6")),
    "METALLO": (H("3E4858"), H("8E9CB0"), H("DCE4EE")),
}
ORDINE_TIPI = ["NEUTRO", "FIAMMA", "ACQUA", "NATURA", "IMPULSO", "VENTO", "ROCCIA", "GELO",
               "OMBRA", "ASTRALE", "TOSSICO", "PSICHE", "METALLO"]

# --- Stati e barre -------------------------------------------------------------
HP_ALTO = (H("1E7A3A"), H("3FC25A"), H("A6F28A"))
HP_MEDIO = (H("8A5A10"), H("F0B030"), H("FFE07A"))
HP_BASSO = (H("7A1414"), H("E8402E"), H("FF9A7A"))
XP = (H("0E4D5C"), H("2BC4C0"), H("A6FFF0"))
STATI = {
    "VEL": TIPI["TOSSICO"][1], "VEL+": TIPI["TOSSICO"][0], "BRU": TIPI["FIAMMA"][1],
    "PAR": TIPI["IMPULSO"][1], "SON": H("7C84A8"), "GEL": TIPI["GELO"][1], "CNF": TIPI["PSICHE"][1],
}

TR = (0, 0, 0, 0)


def mix(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3)) + (255,)


def con_alfa(c, a):
    return (c[0], c[1], c[2], a)
