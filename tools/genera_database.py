#!/usr/bin/env python3
"""
ECLYRA - generatore del database di gioco (Fase 3).

Fonte unica: le tabelle canoniche del documento di design sono copiate qui sotto
cosi' come sono. Lo script valida i dati e genera i file C:

  src/data/species_data.c    100 specie, abilita', rarita', crescita, resa XP, learnset, descrizioni
  src/data/moves_data.c      100 mosse con effetti strutturati
  src/data/abilities_data.c  abilita' 001..046 (alias inclusi)
  src/data/evolution_data.c  evoluzioni canoniche
  src/data/trainers_data.c   allenatori, Guardiani, Nexus, Lega, boss
  src/data/encounters_data.c habitat e incontri

Uso: python3 tools/genera_database.py
Regole decise in questa fase (documentate anche in PROJECT_STATE.md):
  rarita'   : #100 primordiale; #074-080 e #099 leggendari; starter (001-009) e specie con
              statistiche totali >= 480 rari; le altre comuni.
              catchRate: comune 0.20, raro 0.08, leggendario 0.015, primordiale 0.0
  crescita  : decisa dalla forma base della famiglia evolutiva (cosi' l'XP resta coerente
              quando un Elyon evolve): i tre starter 1.0; totale < 300 -> 0.8;
              leggendari/primordiale o totale >= 560 -> 1.2; altrimenti 1.0
  resa XP   : baseYield = max(4, round(totale / 50))
  learnset  : regola generativa del documento (Lv 1, 5, 9, 13, 18, 24, 30, 38, 46, 55),
              eccezioni esplicite per la linea di PYRIN e per AURION
"""
import os
import re

RADICE = os.path.join(os.path.dirname(__file__), "..")
OUT = os.path.join(RADICE, "src", "data")

TIPI = ["FIAMMA", "ACQUA", "NATURA", "IMPULSO", "VENTO", "ROCCIA", "GELO", "OMBRA",
        "ASTRALE", "TOSSICO", "PSICHE", "METALLO"]

# ------------------------------------------------------------------
# DATABASE SPECIE CANONICO (ID NOME TIPO HP ATK DEF SPA SPD SPE)
# ------------------------------------------------------------------
SPECIE_TXT = """
001 PYRIN FIAMMA 44 52 39 60 43 62
002 PYRION FIAMMA 62 68 54 76 58 74
003 PYROVAR FIAMMA/METALLO 88 105 92 108 82 101
004 AQUIL ACQUA 48 45 48 58 52 50
005 AQUILON ACQUA 64 59 63 76 68 64
006 AQUARYS ACQUA/ASTRALE 90 82 88 118 102 95
007 MOSSEN NATURA 52 48 55 42 52 41
008 MOSSKA NATURA 69 62 73 54 68 50
009 VERDANTOR NATURA/ROCCIA 105 101 115 70 96 63
010 FLARO FIAMMA 55 56 38 50 42 65
011 FLAROK FIAMMA 72 78 51 66 54 82
012 FLARAGON FIAMMA/VENTO 96 112 78 88 72 108
013 NERUBI ACQUA 51 43 45 57 48 58
014 NERULON ACQUA 70 61 62 76 65 75
015 NERUTIDE ACQUA/VENTO 94 83 80 112 91 105
016 LEAFIN NATURA 48 49 42 54 47 62
017 LEAFORA NATURA 78 72 64 86 71 91
018 ZAPPI IMPULSO 43 42 35 61 39 78
019 ZAPPOLT IMPULSO 65 62 51 91 57 108
020 WINDI VENTO 40 38 31 54 43 82
021 WINDRA VENTO 65 61 50 82 64 112
022 ROCKIT ROCCIA 58 62 73 29 45 31
023 GRANITON ROCCIA 91 105 116 40 69 38
024 FROLI GELO 42 36 34 61 56 67
025 FROSTEL GELO 70 59 55 94 84 93
026 SHADEK OMBRA 45 54 34 58 48 72
027 SHADEVOR OMBRA 78 94 58 88 74 109
028 LUMI ASTRALE 38 32 31 65 61 79
029 LUMIRA ASTRALE 68 51 55 111 95 102
030 TOXIL TOSSICO 47 54 43 48 52 49
031 TOXORA TOSSICO/ROCCIA 82 92 84 69 76 55
032 MINDI PSICHE 40 30 35 67 59 70
033 MINDORA PSICHE 67 51 59 109 94 99
034 FERROK METALLO 51 68 71 33 51 28
035 FERROX METALLO 84 110 122 48 83 44
036 EMBERA FIAMMA 42 57 34 61 39 76
037 CORALU ACQUA 48 41 54 58 66 42
038 MOSSBUG NATURA 39 45 41 31 36 69
039 THUNDERN IMPULSO 62 71 44 82 48 88
040 SKYLO VENTO 35 34 30 48 42 85
041 BOULDERN ROCCIA 68 70 81 32 44 28
042 ICELO GELO 45 36 41 70 65 61
043 NOCTIL OMBRA 42 57 37 52 51 73
044 STELLI ASTRALE 34 29 28 71 67 76
045 VENOMA TOSSICO 51 62 52 45 58 51
046 PSYRA PSICHE 39 31 39 75 62 68
047 IRONEL METALLO 52 69 75 31 54 34
048 SANDOR ROCCIA/VENTO 55 63 61 35 44 58
049 DUSTRA VENTO/ROCCIA 78 87 79 52 61 91
050 MAGMOR FIAMMA/ROCCIA 63 79 72 64 45 43
051 AQUALIS ACQUA/NATURA 61 47 58 79 67 55
052 THORVAN IMPULSO/METALLO 89 108 82 94 74 91
053 GLACIRA GELO/ASTRALE 68 44 61 112 101 74
054 NIGHTOR OMBRA/VENTO 71 91 59 78 67 113
055 MYSTARA PSICHE/ASTRALE 73 42 58 119 108 91
056 VENOROK TOSSICO/ROCCIA 84 99 96 57 69 49
057 STEELYN METALLO/NATURA 82 92 108 63 91 42
058 FLAMORA FIAMMA/ASTRALE 69 62 55 115 78 102
059 TIDEL ACQUA/VENTO 59 46 51 91 71 103
060 ROOTAN NATURA/ROCCIA 91 89 105 62 83 44
061 CRAGOR ROCCIA/METALLO 105 119 133 43 82 35
062 FROSTOR GELO/METALLO 87 82 101 89 104 59
063 SHADRA OMBRA/TOSSICO 75 98 63 105 78 91
064 LUNEL ASTRALE/PSICHE 61 37 51 121 110 96
065 PSYVOR PSICHE/OMBRA 79 71 63 127 87 102
066 DRAVEN OMBRA/FIAMMA 91 117 79 108 72 101
067 AERION VENTO/ASTRALE 79 65 61 124 91 128
068 TERRAGON ROCCIA 108 125 137 53 76 49
069 HYDRELL ACQUA/METALLO 98 83 106 119 108 71
070 SOLVIA ASTRALE/FIAMMA 83 71 67 137 102 115
071 UMBRAX OMBRA/METALLO 97 123 116 86 103 72
072 CYRIEL PSICHE/GELO 74 52 63 132 119 109
073 VERDRA NATURA/ASTRALE 102 76 101 121 115 84
074 IGNAR FIAMMA 110 129 97 101 88 111
075 MARIEL ACQUA 115 87 102 128 117 93
076 ZERON IMPULSO 91 106 81 137 93 143
077 NIVALIS GELO/ASTRALE 107 74 97 139 129 108
078 AEROS VENTO 94 82 75 121 99 151
079 TERRAX ROCCIA 129 143 151 52 92 58
080 UMBRIEL OMBRA 101 132 89 141 104 124
081 PYRELISK FIAMMA/ROCCIA 103 127 109 83 74 82
082 CORVANE VENTO/OMBRA 81 104 61 91 69 132
083 SYLPHRA VENTO/NATURA 74 59 66 116 98 139
084 GRAVOLT IMPULSO/ROCCIA 109 118 125 89 83 76
085 MIREON ACQUA/TOSSICO 91 72 84 123 107 79
086 OBSCURA OMBRA/PSICHE 77 93 69 128 102 118
087 ASTRION ASTRALE/METALLO 102 86 112 134 125 89
088 TOXMARA TOSSICO/NATURA 96 108 89 107 92 76
089 MINDREL PSICHE/VENTO 79 54 62 136 111 125
090 FERRADON METALLO/ROCCIA 116 137 148 58 101 51
091 VOLCRAG FIAMMA/ROCCIA 119 141 126 91 78 69
092 SEVRIN VENTO/IMPULSO 83 72 61 129 86 148
093 AQUALITH ACQUA/ROCCIA 121 91 132 101 117 62
094 FLORUNE NATURA/PSICHE 97 68 82 133 126 93
095 ZEPHRAX VENTO/ASTRALE 88 63 69 141 106 153
096 CRYONOR GELO/ROCCIA 113 101 137 93 116 67
097 NOCTARA OMBRA/VENTO 92 118 71 113 83 141
098 LUMINOX ASTRALE/IMPULSO 89 69 67 149 121 137
099 VESPERA OMBRA/ASTRALE 98 81 79 151 130 119
100 AURION ASTRALE/OMBRA 160 150 140 165 145 130
"""

EVOLUZIONI_TXT = ("001>002@16;002>003@34;004>005@16;005>006@34;007>008@16;008>009@34;010>011@18;011>012@34;"
                  "013>014@18;014>015@35;016>017@20;018>019@20;020>021@20;022>023@22;024>025@22;026>027@24;"
                  "028>029@24;030>031@23;032>033@25;034>035@25;037>093@27;038>060@18;039>052@32;040>095@26;"
                  "041>061@30;042>096@30;043>097@28;044>098@30;045>056@29;046>089@30;047>090@30;048>049@28;"
                  "050>091@34")

# ------------------------------------------------------------------
# DATABASE MOSSE CANONICO (ID nome|tipo|categoria|potenza|precisione|PP|effetto)
# ------------------------------------------------------------------
MOSSE_TXT = """
001 COLPO|NEUTRO|F|40|100|35|nessuno
002 GRAFFIO|NEUTRO|F|40|100|35|nessuno
003 AZIONE RAPIDA|NEUTRO|F|40|100|30|priorita+1
004 IMPATTO|NEUTRO|F|60|100|25|nessuno
005 CARICA|NEUTRO|F|80|90|20|stordimento10
006 COLPO PESANTE|NEUTRO|F|90|90|15|nessuno
007 IMPATTO LUMEN|NEUTRO|S|80|100|15|nessuno
008 RITORNO|NEUTRO|F|variabile|100|20|potenza=20+amicizia/3 max105
009 DOPPIO COLPO|NEUTRO|F|30|90|20|2colpi
010 RAPIDITÀ|NEUTRO|T|0|100|30|velocita+1
011 BRACIAVIVA|FIAMMA|S|40|100|25|bruciatura10
012 FIAMMATA|FIAMMA|S|70|95|20|bruciatura10
013 FIAMMATA ROSSA|FIAMMA|S|85|90|15|bruciatura15
014 INFERNO CREMISI|FIAMMA|S|100|85|10|bruciatura20
015 NUCLEO ARDENTE|FIAMMA|S|120|80|5|potenza dimezzata sotto 30%HP
016 ARTIGLIO ARDENTE|FIAMMA|F|65|95|20|bruciatura10
017 MARCHIO DEL FUOCO|FIAMMA|T|0|100|20|attacco+1
018 ONDATA MAGMATICA|FIAMMA|S|90|90|10|multi-target
019 GETTOBLU|ACQUA|S|40|100|25|nessuno
020 ONDAFLUSSO|ACQUA|S|70|100|20|nessuno
021 MAREA PROFONDA|ACQUA|S|100|85|10|nessuno
022 IDROPOMPA LUMEN|ACQUA|S|120|80|5|nessuno
023 MORSO MARINO|ACQUA|F|60|100|25|stordimento10
024 CORRENTE|ACQUA|T|0|100|20|velocita+1
025 RIFLESSO D'ACQUA|ACQUA|T|0|100|15|difesa+1
026 ONDA CURATIVA|ACQUA|T|0|100|10|cura50%
027 FRUSTA VERDE|NATURA|F|45|100|30|nessuno
028 LAMAFOGLIA|NATURA|F|65|95|25|critico12%
029 RADICE SELVAGGIA|NATURA|F|90|95|15|nessuno
030 ESPLOSIONE VEGETALE|NATURA|S|120|80|5|nessuno
031 SPORA|NATURA|T|0|100|20|sonno30
032 RADICAMENTO|NATURA|T|0|100|15|cura1/8 per turno
033 POLLINE|NATURA|S|60|100|20|confusione20
034 VIGORE NATURALE|NATURA|T|0|100|10|attacco+1 difesa+1
035 SCINTILLA|IMPULSO|S|40|100|30|paralisi10
036 IMPULSO|IMPULSO|S|70|100|20|paralisi10
037 FULMINE|IMPULSO|S|90|90|15|paralisi20
038 IMPULSO ZERO|IMPULSO|S|110|85|10|nessuno
039 ARTIGLIO ELETTRICO|IMPULSO|F|65|95|20|paralisi10
040 SOVRACCARICO|IMPULSO|S|130|80|5|SPA-2 dopo uso
041 CAMPO ELETTRICO|IMPULSO|T|0|100|10|impulso+15% 5 turni
042 SOFFIO|VENTO|S|40|100|30|nessuno
043 TAGLIOVENTO|VENTO|S|65|95|25|nessuno
044 LAMA D'ARIA|VENTO|S|80|95|20|critico12%
045 URAGANO|VENTO|S|105|80|10|confusione20
046 TURBINE|VENTO|T|0|100|20|SPE nemico-1
047 CORRENTE ASCENDENTE|VENTO|T|0|100|20|SPE+2
048 SASSATA|ROCCIA|F|45|95|25|nessuno
049 FRANTUMA|ROCCIA|F|70|95|20|nessuno
050 MACIGNO|ROCCIA|F|100|85|10|nessuno
051 IMPATTO TITANICO|ROCCIA|F|120|75|5|nessuno
052 CORAZZA DI PIETRA|ROCCIA|T|0|100|20|DEF+1
053 FRANA|ROCCIA|F|75|90|15|SPE nemico-1 al 20%
054 GELOPUNTA|GELO|S|45|100|30|gelo10
055 VENTO GELIDO|GELO|S|65|95|20|gelo10
056 VALANGA BIANCA|GELO|F|90|90|10|raddoppia se colpito prima
057 GLACIAZIONE|GELO|S|110|80|5|gelo20
058 MANTO GELIDO|GELO|T|0|100|15|DEF+1 SPD+1
059 ARTIGLIO OSCURO|OMBRA|F|45|100|30|critico12%
060 MORSO NOTTURNO|OMBRA|F|70|100|20|stordimento20
061 ABISSO|OMBRA|S|100|85|10|nessuno
062 ECLISSI|OMBRA|S|120|75|5|nessuno
063 VELO D'OMBRA|OMBRA|T|0|100|15|evasione+1
064 PAURA|OMBRA|T|0|100|15|ATK nemico-1
065 LUCE LUMEN|ASTRALE|S|45|100|30|nessuno
066 RAGGIO STELLARE|ASTRALE|S|75|100|20|nessuno
067 NOVA|ASTRALE|S|110|85|10|nessuno
068 SUPERNOVA|ASTRALE|S|140|70|5|recoil25%HP
069 AURA LUMEN|ASTRALE|T|0|100|10|SPA+1 SPD+1
070 RIFLESSO COSMICO|ASTRALE|T|0|100|10|danno prossimo colpo dimezzato
071 SPORA TOSSICA|TOSSICO|T|0|100|25|veleno40
072 MORSO VENEFICO|TOSSICO|F|60|100|20|veleno20
073 VORTICE TOSSICO|TOSSICO|S|90|90|10|veleno30
074 VELENO PROFONDO|TOSSICO|T|0|100|10|veleno grave
075 ACIDO|TOSSICO|S|60|100|25|SPD nemico-1 al20%
076 CORROSIONE|TOSSICO|T|0|100|15|DEF nemico-2
077 ONDA MENTALE|PSICHE|S|50|100|25|nessuno
078 PULSAZIONE|PSICHE|S|75|100|20|nessuno
079 COLPO PSIONICO|PSICHE|S|105|85|10|nessuno
080 FRATTURA MENTALE|PSICHE|S|120|80|5|confusione30
081 CONCENTRAZIONE|PSICHE|T|0|100|20|SPA+1
082 MENTE FERMA|PSICHE|T|0|100|20|SPD+1
083 CONTROLLO|PSICHE|T|0|100|10|stordimento prossimo turno50
084 SCAGLIA ACCIAIO|METALLO|F|45|100|30|nessuno
085 ARTIGLIO METALLICO|METALLO|F|65|95|20|ATK+1 al10%
086 IMPATTO FERROSO|METALLO|F|100|85|10|nessuno
087 CANNONE FERROSO|METALLO|S|110|80|5|nessuno
088 ARMATURA|METALLO|T|0|100|15|DEF+2
089 RIFLESSIONE|METALLO|T|0|100|10|riflette25% fisico 3 turni
090 CURA LUMEN|ASTRALE|T|0|100|10|cura50%
091 SACRIFICIO|NEUTRO|F|100|100|10|recoil25%HP
092 DOPPIO IMPATTO|NEUTRO|F|50|90|15|2colpi
093 FURIA|NEUTRO|T|0|100|15|prossima mossa fisica+20% cumulabile max60%
094 SIGILLO|ASTRALE|T|0|100|10|blocca switch2 turni
095 ROTTURA LUMEN|ASTRALE|S|125|90|5|ignora boost difensivi
096 VUOTO|OMBRA|S|120|85|5|ignora abilita difensive
097 EQUILIBRIO|ASTRALE|T|0|100|5|HP entrambi media limitata al proprio maxHP
098 FRATTURA DEL VUOTO|OMBRA|S|150|75|3|solo Aurion
099 RITORNO DEL LUMEN|ASTRALE|T|0|100|1|solo evento
100 SIGILLO DI AURION|ASTRALE|S|160|100|1|solo evento
"""

# Effetto testuale -> (effetto, prob, p1, p2, p3, p4, priorita, flag, sul_nemico)
# Boost: ATK DEF SPA SPD SPE PRECISIONE EVASIONE
EFFETTI = {
    "nessuno": ("EFF_NESSUNO", 0, 0, 0, 0, 0, 0, 0),
    "priorita+1": ("EFF_NESSUNO", 0, 0, 0, 0, 0, 1, 0),
    "stordimento10": ("EFF_STORDIMENTO", 10, 0, 0, 0, 0, 0, 0),
    "stordimento20": ("EFF_STORDIMENTO", 20, 0, 0, 0, 0, 0, 0),
    "potenza=20+amicizia/3 max105": ("EFF_POTENZA_AMICIZIA", 100, 105, 0, 0, 0, 0, 0),
    "2colpi": ("EFF_MULTICOLPO", 100, 2, 0, 0, 0, 0, 0),
    "velocita+1": ("EFF_BOOST_SE", 100, "BOOST_SPE", 1, 0, 0, 0, 0),
    "bruciatura10": ("EFF_STATUS", 10, "STATUS_BRUCIATURA", 0, 0, 0, 0, 0),
    "bruciatura15": ("EFF_STATUS", 15, "STATUS_BRUCIATURA", 0, 0, 0, 0, 0),
    "bruciatura20": ("EFF_STATUS", 20, "STATUS_BRUCIATURA", 0, 0, 0, 0, 0),
    "potenza dimezzata sotto 30%HP": ("EFF_DIMEZZATA_HP_BASSI", 100, 30, 0, 0, 0, 0, 0),
    "attacco+1": ("EFF_BOOST_SE", 100, "BOOST_ATK", 1, 0, 0, 0, 0),
    "multi-target": ("EFF_MULTI_BERSAGLIO", 100, 0, 0, 0, 0, 0, 0),
    "difesa+1": ("EFF_BOOST_SE", 100, "BOOST_DEF", 1, 0, 0, 0, 0),
    "cura50%": ("EFF_CURA_PERC", 100, 50, 0, 0, 0, 0, 0),
    "critico12%": ("EFF_CRITICO_ALTO", 100, 12, 0, 0, 0, 0, 0),
    "sonno30": ("EFF_STATUS", 30, "STATUS_SONNO", 0, 0, 0, 0, 0),
    "cura1/8 per turno": ("EFF_RADICAMENTO", 100, 8, 0, 0, 0, 0, 0),
    "confusione20": ("EFF_STATUS", 20, "STATUS_CONFUSIONE", 0, 0, 0, 0, 0),
    "confusione30": ("EFF_STATUS", 30, "STATUS_CONFUSIONE", 0, 0, 0, 0, 0),
    "attacco+1 difesa+1": ("EFF_BOOST_SE", 100, "BOOST_ATK", 1, "BOOST_DEF", 1, 0, 0),
    "paralisi10": ("EFF_STATUS", 10, "STATUS_PARALISI", 0, 0, 0, 0, 0),
    "paralisi20": ("EFF_STATUS", 20, "STATUS_PARALISI", 0, 0, 0, 0, 0),
    "SPA-2 dopo uso": ("EFF_BOOST_SE", 100, "BOOST_SPA", -2, 0, 0, 0, 0),
    "impulso+15% 5 turni": ("EFF_CAMPO_TIPO", 100, "TYPE_IMPULSO", 15, 5, 0, 0, 0),
    "SPE nemico-1": ("EFF_BOOST_NEMICO", 100, "BOOST_SPE", -1, 0, 0, 0, 0),
    "SPE+2": ("EFF_BOOST_SE", 100, "BOOST_SPE", 2, 0, 0, 0, 0),
    "DEF+1": ("EFF_BOOST_SE", 100, "BOOST_DEF", 1, 0, 0, 0, 0),
    "SPE nemico-1 al 20%": ("EFF_BOOST_NEMICO", 20, "BOOST_SPE", -1, 0, 0, 0, 0),
    "gelo10": ("EFF_STATUS", 10, "STATUS_GELO", 0, 0, 0, 0, 0),
    "gelo20": ("EFF_STATUS", 20, "STATUS_GELO", 0, 0, 0, 0, 0),
    "raddoppia se colpito prima": ("EFF_RADDOPPIA_SE_COLPITO", 100, 0, 0, 0, 0, 0, 0),
    "DEF+1 SPD+1": ("EFF_BOOST_SE", 100, "BOOST_DEF", 1, "BOOST_SPD", 1, 0, 0),
    "evasione+1": ("EFF_BOOST_SE", 100, "BOOST_EVASIONE", 1, 0, 0, 0, 0),
    "ATK nemico-1": ("EFF_BOOST_NEMICO", 100, "BOOST_ATK", -1, 0, 0, 0, 0),
    "recoil25%HP": ("EFF_CONTRACCOLPO", 100, 25, 0, 0, 0, 0, 0),
    "SPA+1 SPD+1": ("EFF_BOOST_SE", 100, "BOOST_SPA", 1, "BOOST_SPD", 1, 0, 0),
    "danno prossimo colpo dimezzato": ("EFF_DIMEZZA_PROSSIMO", 100, 0, 0, 0, 0, 0, 0),
    "veleno40": ("EFF_STATUS", 40, "STATUS_VELENO", 0, 0, 0, 0, 0),
    "veleno20": ("EFF_STATUS", 20, "STATUS_VELENO", 0, 0, 0, 0, 0),
    "veleno30": ("EFF_STATUS", 30, "STATUS_VELENO", 0, 0, 0, 0, 0),
    "veleno grave": ("EFF_STATUS", 100, "STATUS_VELENO_GRAVE", 0, 0, 0, 0, 0),
    "SPD nemico-1 al20%": ("EFF_BOOST_NEMICO", 20, "BOOST_SPD", -1, 0, 0, 0, 0),
    "DEF nemico-2": ("EFF_BOOST_NEMICO", 100, "BOOST_DEF", -2, 0, 0, 0, 0),
    "SPA+1": ("EFF_BOOST_SE", 100, "BOOST_SPA", 1, 0, 0, 0, 0),
    "SPD+1": ("EFF_BOOST_SE", 100, "BOOST_SPD", 1, 0, 0, 0, 0),
    "stordimento prossimo turno50": ("EFF_STORDIMENTO_PROSSIMO", 50, 0, 0, 0, 0, 0, 0),
    "ATK+1 al10%": ("EFF_BOOST_SE", 10, "BOOST_ATK", 1, 0, 0, 0, 0),
    "DEF+2": ("EFF_BOOST_SE", 100, "BOOST_DEF", 2, 0, 0, 0, 0),
    "riflette25% fisico 3 turni": ("EFF_RIFLESSIONE", 100, 25, 3, 0, 0, 0, 0),
    "prossima mossa fisica+20% cumulabile max60%": ("EFF_FURIA", 100, 20, 60, 0, 0, 0, 0),
    "blocca switch2 turni": ("EFF_BLOCCA_CAMBIO", 100, 2, 0, 0, 0, 0, 0),
    "ignora boost difensivi": ("EFF_IGNORA_BOOST_DIFESA", 100, 0, 0, 0, 0, 0, 0),
    "ignora abilita difensive": ("EFF_IGNORA_ABILITA_DIFESA", 100, 0, 0, 0, 0, 0, 0),
    "HP entrambi media limitata al proprio maxHP": ("EFF_EQUILIBRIO", 100, 0, 0, 0, 0, 0, 0),
    "solo Aurion": ("EFF_NESSUNO", 0, 0, 0, 0, 0, 0, "MOSSA_SOLO_AURION"),
    "solo evento": ("EFF_NESSUNO", 0, 0, 0, 0, 0, 0, "MOSSA_SOLO_EVENTO"),
}

# ------------------------------------------------------------------
# ABILITA' (ID, NOME, effetto strutturato, descrizione)
# (effetto, condizione, tipo1, tipo2, moltiplicatore, param, aliasDi)
# ------------------------------------------------------------------
AB = "AB_MOLT_TIPO_ATTACCO"
ABILITA = [
    (1, "FIAMMA_INTERNA", (AB, "COND_HP_SOTTO_30", "FIAMMA", None, 1.25, 0, 0), "Sotto il 30% degli HP, mosse Fiamma x1,25."),
    (2, "PELLE_UMIDA", ("AB_MOLT_DANNO_SUBITO", "COND_SEMPRE", "FIAMMA", None, 0.70, 0, 0), "Danni Fiamma subiti x0,70."),
    (3, "RADICE_VIVA", ("AB_CURA_FINE_TURNO", "COND_SEMPRE", None, None, 2.0, 0, 0), "A fine turno cura il 2% degli HP."),
    (4, "CONDUTTORE", ("AB_IMMUNITA_TIPO", "COND_SEMPRE", "IMPULSO", None, 1.0, "BOOST_SPA", 0), "Immune a Impulso; SPA +1 quando ne viene colpito."),
    (5, "CORAZZA", ("AB_MOLT_DANNO_FISICO_SUBITO", "COND_SEMPRE", None, None, 0.90, 0, 0), "Danni fisici subiti x0,90."),
    (6, "GELO_ETERNO", ("AB_IMMUNITA_STATUS", "COND_SEMPRE", None, None, 1.0, "STATUS_GELO", 0), "Immune al congelamento."),
    (7, "OMBRA_DENSA", ("AB_MOLT_DANNO_SUBITO", "COND_SEMPRE", "ASTRALE", None, 0.70, 0, 0), "Danni Astrale subiti x0,70."),
    (8, "MENTE_LUCIDA", ("AB_IMMUNITA_STATUS", "COND_SEMPRE", None, None, 1.0, "STATUS_CONFUSIONE", 0), "Immune alla confusione."),
    (9, "AURA_LUMEN", (AB, "COND_SEMPRE", "ASTRALE", None, 1.15, 0, 0), "Mosse Astrale x1,15."),
    (10, "CORRENTE_RAPIDA", ("AB_MOLT_STAT", "COND_PIOGGIA", None, None, 1.15, "STAT_SPE", 0), "Con la pioggia SPE x1,15."),
    (11, "FOTOSINTESI", ("AB_CURA_FINE_TURNO", "COND_SOLE", None, None, 3.0, 0, 0), "Al sole cura il 3% degli HP a ogni turno."),
    (12, "CORAZZA_FERROSA", ("AB_CRITICO_RIDOTTO", "COND_SEMPRE", None, None, 0.75, 0, 0), "Bonus dei critici avversari ridotto del 25%."),
    (13, "SCUDO_LUMEN", ("AB_RESISTE_UN_COLPO", "COND_SEMPRE", None, None, 1.0, 0, 0), "Una volta per battaglia resta a 1 HP."),
    (14, "NUCLEO_ARDENTE", (AB, "COND_SEMPRE", "FIAMMA", None, 1.20, 0, 0), "Mosse Fiamma x1,20."),
    (15, "VELENO_ATTIVO", (AB, "COND_SEMPRE", "TOSSICO", None, 1.15, 0, 0), "Mosse Tossico x1,15."),
    (16, "MENTE_OSCURA", (AB, "COND_HP_SOTTO_50", "PSICHE", None, 1.15, 0, 0), "Sotto il 50% degli HP, mosse Psiche x1,15."),
    (17, "MAGMA_INTERNO", ("AB_IMMUNITA_STATUS", "COND_SEMPRE", None, None, 1.0, "STATUS_BRUCIATURA", 0), "Immune alla bruciatura."),
    (18, "ARMATURA_MARINA", ("AB_MOLT_DANNO_SUBITO", "COND_SEMPRE", "ACQUA", None, 0.80, 0, 0), "Danni Acqua subiti x0,80."),
    (19, "CORRENTE_CELESTE", ("AB_MOLT_STAT", "COND_SEMPRE", None, None, 1.20, "STAT_SPE", 0), "SPE x1,20."),
    (20, "EQUILIBRIO_NOTTURNO", (AB, "COND_NOTTE", "OMBRA", None, 1.20, 0, 0), "Di notte mosse Ombra x1,20."),
    (21, "SOVRACCARICO", (AB, "COND_HP_SOTTO_25", "IMPULSO", None, 1.30, 0, 0), "Sotto il 25% degli HP, mosse Impulso x1,30."),
    (22, "BREZZA_VITALE", ("AB_CURA_FINE_TURNO", "COND_SEMPRE", None, None, 2.0, 0, 0), "Cura il 2% degli HP a ogni turno."),
    (23, "CUORE_VEGETALE", (AB, "COND_SEMPRE", "NATURA", None, 1.15, 0, 0), "Mosse Natura x1,15."),
    (24, "CORAZZA_TITANICA", ("AB_MOLT_DANNO_FISICO_SUBITO", "COND_SEMPRE", None, None, 0.85, 0, 0), "Danni fisici subiti x0,85."),
    (25, "MAREA_LUMEN", (AB, "COND_SEMPRE", "ACQUA", None, 1.20, 0, 0), "Mosse Acqua x1,20."),
    (26, "OMBRA_ETERNA", ("AB_IMMUNE_CALO_ATK", "COND_SEMPRE", None, None, 1.0, 0, 0), "Immune ai cali di ATK causati da abilità."),
    (27, "LUCE_ARDENTE", (AB, "COND_SEMPRE", "ASTRALE", "FIAMMA", 1.15, 0, 0), "Mosse Astrale e Fiamma x1,15."),
    (28, "VELENO_OSCURO", ("AB_STATUS_SU_ATTACCO", "COND_SEMPRE", "OMBRA", None, 1.0, 10, 0), "Le mosse Ombra avvelenano nel 10% dei casi."),
    (29, "NUCLEO_DEL_VUOTO", (AB, "COND_SEMPRE", "OMBRA", None, 1.25, 0, 0), "Mosse Ombra x1,25."),
    (30, "EQUILIBRIO_DEL_VUOTO", ("AB_EFFETTO_CAMPO", "COND_SEMPRE", None, None, 1.0, 3, 0), "Ogni tre turni modifica il campo. Solo Aurion."),
    (31, "CORRENTE_LUMEN", (AB, "COND_SEMPRE", "ACQUA", None, 1.15, 0, 0), "Mosse Acqua x1,15."),
    (32, "VENTO_ARDENTE", (AB, "COND_SEMPRE", "FIAMMA", "VENTO", 1.10, 0, 0), "Mosse Fiamma e Vento x1,10."),
    (33, "CORRENTE_ASCENDENTE", (AB, "COND_SEMPRE", "VENTO", None, 1.15, 0, 0), "Mosse Vento x1,15."),
    (34, "VELENO_OSCURO", None, "Alias di VELENO_OSCURO (028)."),
    (35, "OMBRA_MOBILE", ("AB_MOLT_STAT", "COND_NOTTE", None, None, 1.10, "STAT_SPE", 0), "Di notte SPE x1,10."),
    (36, "AURA_ARDENTE", None, "Alias di LUCE_ARDENTE (027)."),
    (37, "CORRENTE_VITALE", ("AB_CURA_FINE_TURNO", "COND_PIOGGIA", None, None, 2.0, 0, 0), "Con la pioggia cura il 2% degli HP a ogni turno."),
    (38, "TEMPESTA_DI_SABBIA", (AB, "COND_TEMPESTA", "ROCCIA", None, 1.15, 0, 0), "In tempesta mosse Roccia x1,15."),
    (39, "MENTE_GELIDA", (AB, "COND_SEMPRE", "GELO", None, 1.15, 0, 0), "Mosse Gelo x1,15."),
    (40, "FIAMMA_OSCURA", (AB, "COND_SEMPRE", "OMBRA", "FIAMMA", 1.10, 0, 0), "Mosse Ombra e Fiamma x1,10."),
    (41, "ARMATURA_TITANICA", None, "Alias di CORAZZA_TITANICA (024)."),
    (42, "CALORE_PRIMORDIALE", (AB, "COND_SEMPRE", "FIAMMA", None, 1.15, 0, 0), "Mosse Fiamma x1,15."),
    (43, "CACCIATORE_NOTTURNO", ("AB_CRITICO_ALTO", "COND_NOTTE", None, None, 1.0, 12, 0), "Di notte probabilità di critico 12%."),
    (44, "VELENO_ACQUATICO", ("AB_STATUS_SU_ATTACCO", "COND_SEMPRE", "ACQUA", None, 1.0, 10, 0), "Le mosse Acqua avvelenano nel 10% dei casi."),
    (45, "EQUILIBRIO_NOTTURNO", None, "Alias di EQUILIBRIO_NOTTURNO (020)."),
    (46, "FRATTURA_DEL_VUOTO", ("AB_EFFETTO_CAMPO", "COND_SEMPRE", None, None, 1.0, 3, 0), "Forma boss: effetto di campo per tre turni."),
]
ALIAS = {34: 28, 36: 27, 41: 24, 45: 20}

ABILITA_PER_TIPO = {"FIAMMA": 1, "ACQUA": 2, "NATURA": 3, "IMPULSO": 4, "VENTO": 33, "ROCCIA": 5, "GELO": 6,
                    "OMBRA": 7, "ASTRALE": 9, "TOSSICO": 15, "PSICHE": 8, "METALLO": 12}
ECCEZIONI_ABILITA_TXT = ("#003 014, #006 031, #009 023, #012 032, #015 010, #017 011, #019 004, #021 033, #023 005, "
    "#025 006, #027 007, #029 009, #031 015, #033 008, #035 012, #049 038, #052 004, #053 006, #054 035, #055 008, "
    "#056 015, #057 012, #058 027, #059 010, #060 003, #061 005, #062 006, #063 028, #064 009, #065 016, #066 040, "
    "#067 019, #068 024, #069 018, #070 027, #071 007, #072 039, #073 023, #074 014, #075 025, #076 004, #077 006, "
    "#078 019, #079 024, #080 026, #081 042, #082 043, #083 022, #084 004, #085 044, #086 016, #087 013, #088 015, "
    "#089 008, #090 012, #091 017, #092 021, #093 018, #094 023, #095 019, #096 006, #097 035, #098 009, #099 020, #100 030")

# ------------------------------------------------------------------
# ALLENATORI (dal documento). Il formato e' "NOME" + livello.
# ------------------------------------------------------------------
ALLENATORI = [
    ("KAEL_INIZIALE_VS_PYRIN", "KAEL", "Rivale", 100, "AI_INTRO", "AQUIL5", "", "Kael sceglie Aquil se il giocatore sceglie Pyrin."),
    ("KAEL_INIZIALE_VS_AQUIL", "KAEL", "Rivale", 100, "AI_INTRO", "MOSSEN5", "", "Kael sceglie Mossen se il giocatore sceglie Aquil."),
    ("KAEL_INIZIALE_VS_MOSSEN", "KAEL", "Rivale", 100, "AI_INTRO", "PYRIN5", "", "Kael sceglie Pyrin se il giocatore sceglie Mossen."),
    # Route 1 (non canonici: definiti in Fase 4 per i due Esploratori della mappa, premi bassi)
    ("ROUTE1_TEO", "TEO", "Esploratore", 80, "AI_SIMPLE", "LEAFIN5, ZAPPI6", "", "Allenatore di Route 1 (Fase 4)."),
    ("ROUTE1_NINA", "NINA", "Esploratrice", 90, "AI_SIMPLE", "NERUBI6, LUMI5", "", "Allenatrice di Route 1 (Fase 4)."),
    ("NEXUS_RECLUTA_01", "RECLUTA NEXUS", "Recluta Nexus", 280, "AI_SIMPLE", "FLARO8, ZAPPI7", "", ""),
    ("NEXUS_RECLUTA_02", "RECLUTA NEXUS", "Recluta Nexus", 360, "AI_SIMPLE", "SHADEK10, TOXIL9", "", ""),
    ("NEXUS_SCIENZIATO", "SCIENZIATO NEXUS", "Scienziato Nexus", 1200, "AI_STANDARD", "MINDI15, FERROK15, LUMI14", "", ""),
    ("NEXUS_COMMANDER", "COMANDANTE NEXUS", "Comandante Nexus", 3000, "AI_SMART", "SHADEVOR25, VENOROK26, UMBRAX27", "",
     "Boss: può avere specie rare non catturabili dal giocatore a quel punto."),
    ("GUARDIANA_LIRA", "LIRA", "Guardiana", 2100, "AI_STANDARD", "LEAFORA18, MOSSKA19, STEELYN21", "SIGILLO_VERDE", ""),
    ("GUARDIANO_KAIN", "KAIN", "Guardiano", 2600, "AI_STANDARD", "ZAPPOLT23, THUNDERN24, THORVAN26", "SIGILLO_TUONO", ""),
    ("GUARDIANA_MIRA", "MIRA", "Guardiana", 3000, "AI_STANDARD", "NERULON26, AQUALIS27, HYDRELL29", "SIGILLO_MAREE", ""),
    ("GUARDIANO_ROVAN", "ROVAN", "Guardiano", 3500, "AI_SMART", "FLAROK31, MAGMOR32, PYROVAR34", "SIGILLO_FUOCO", ""),
    ("GUARDIANA_SELENE", "SELENE", "Guardiana", 4000, "AI_SMART", "FROSTEL35, GLACIRA36, NIVALIS38", "SIGILLO_GHIACCIO", ""),
    ("GUARDIANO_DAREN", "DAREN", "Guardiano", 4500, "AI_SMART", "GRANITON39, CRAGOR40, TERRAGON42", "SIGILLO_PIETRA", ""),
    ("GUARDIANA_ELARA", "ELARA", "Guardiana", 6000, "AI_SMART", "MYSTARA44, LUMIRA45, AERION46, VERDRA47", "SIGILLO_STELLE", ""),
    ("VAEL_INTERMEZZO", "VAEL", "Ordine Nexus", 0, "AI_SMART", "SHADEVOR34, HYDRELL35, DRAVEN36", "",
     "Interruzione narrativa al 50% HP dell'ultimo Elyon, senza sconfitta forzata."),
    ("LEGA_RHEON", "RHEON", "Lega di Elys", 0, "AI_SMART", "FLAMORA52, PYRELISK53, VOLCRAG54, IGNAR56", "", ""),
    ("LEGA_ILIA", "ILIA", "Lega di Elys", 0, "AI_SMART", "NERUTIDE52, TIDEL53, HYDRELL54, MARIEL56", "", ""),
    ("LEGA_TOR", "TOR", "Lega di Elys", 0, "AI_SMART", "CRAGOR53, FERRADON54, TERRAGON55, FERROX56", "", ""),
    ("LEGA_NYRA", "NYRA", "Lega di Elys", 0, "AI_SMART", "MINDREL54, CYRIEL55, MYSTARA56, LUNEL57", "", ""),
    ("CAMPIONE_KAEL_VS_PYRIN", "KAEL", "Campione", 12000, "AI_BOSS",
     "AQUARYS58, ZEPHRAX59, UMBRIEL60, ASTRION60, VERDRA61, HYDRELL62", "TORRE_ACCESSO", "Se il giocatore ha scelto Pyrin."),
    ("CAMPIONE_KAEL_VS_AQUIL", "KAEL", "Campione", 12000, "AI_BOSS",
     "PYROVAR58, ZEPHRAX59, UMBRIEL60, ASTRION60, VERDRA61, VOLCRAG62", "TORRE_ACCESSO", "Se il giocatore ha scelto Aquil."),
    ("CAMPIONE_KAEL_VS_MOSSEN", "KAEL", "Campione", 12000, "AI_BOSS",
     "PYROVAR58, AQUARYS59, UMBRIEL60, ASTRION60, LUNEL61, HYDRELL62", "TORRE_ACCESSO", "Se il giocatore ha scelto Mossen."),
    ("VAEL_FINALE", "VAEL", "Ordine Nexus", 0, "AI_BOSS", "UMBRAX60, HYDRELL60, SOLVIA61, PSYVOR62, DRAVEN64", "", "Dopo la Lega."),
    ("AURION_FASE1", "AURION", "Primordiale", 0, "AI_BOSS", "AURION65", "",
     "Fase 1: RAGGIO STELLARE, ABISSO, ROTTURA LUMEN, EQUILIBRIO. Fase 2 al 50%: forma VOID. Fase 3 al 20%: campo instabile +20%."),
    ("AURION_FASE2_VOID", "AURION", "Primordiale", 0, "AI_BOSS", "AURION65", "", "Forma VOID: FRATTURA DEL VUOTO, SUPERNOVA, ECLISSI, NOVA."),
    ("POSTGAME_VESPERA", "VESPERA", "Leggendario", 0, "AI_BOSS", "VESPERA75", "", "Incontro post-game."),
    ("POSTGAME_AURION_ORIGINALE", "AURION", "Primordiale", 0, "AI_BOSS", "AURION80", "", "Forma originale, post-game."),
]
MOSSE_SPECIALI = {
    "AURION_FASE1": ["RAGGIO STELLARE", "ABISSO", "ROTTURA LUMEN", "EQUILIBRIO"],
    "AURION_FASE2_VOID": ["FRATTURA DEL VUOTO", "SUPERNOVA", "ECLISSI", "NOVA"],
}
# Squadre canoniche con un Elyon sotto il livello della sua evoluzione: eccezioni esplicite
ECCEZIONI_LIVELLO = {
    "GUARDIANA_LIRA": "Decisione canonica: primo Guardiano Lv 16-21 con MOSSKA/LEAFORA (LEAFORA al Lv 18, evolve al 20).",
    "GUARDIANO_KAIN": "Eccezione canonica confermata: THORVAN al Lv 26 (THUNDERN evolve al Lv 32). Non modificare il livello.",
    "NEXUS_COMMANDER": "Eccezione canonica confermata: VENOROK al Lv 26 (VENOMA evolve al Lv 29). Non modificare il livello.",
}

# ------------------------------------------------------------------
# HABITAT (dal documento)
# ------------------------------------------------------------------
HABITAT = [
    ("LUNARIA", "Lunaria", 0, 0, ""),
    ("ROUTE1", "Route 1", 2, 7, "FLARO30%, NERUBI30%, LEAFIN25%, ZAPPI10%, LUMI5%"),
    ("BOSCO_VERDALIA", "Bosco Verdalia", 6, 14, "MOSSBUG25%, LEAFIN25%, WINDI20%, MINDI15%, FROLI10%, LUMI5%"),
    ("ARKAS", "Arkas", 20, 32, "ROCKIT25%, SANDOR25%, TOXIL20%, VENOMA15%, MAGMOR10%, DUSTRA5%"),
    ("NIVAL", "Nival", 30, 40, "FROLI30%, ICELO25%, FROSTEL15%, GLACIRA15%, CYRIEL10%, LUMI5%"),
    ("TALASSA", "Talassa", 35, 45, "NERUBI30%, CORALU25%, TIDEL20%, AQUALIS15%, HYDRELL10%"),
    ("VULKAR", "Vulkar", 40, 50, "FLARO30%, MAGMOR25%, PYRELISK20%, VOLCRAG10%, FERROK15%"),
    ("ELYS", "Elys", 45, 55, "LUMI25%, STELLI25%, MINDI20%, MYSTARA10%, LUNEL10%, ASTRION10%"),
]

# ------------------------------------------------------------------
# BESTIARIO: descrizioni originali
# ------------------------------------------------------------------
DESCRIZIONI = {
    1: "Cucciolo di fiamma. La brace sulla coda brilla più forte quando si fida di qualcuno.",
    2: "Le sue zampe lasciano impronte calde. Corre tra le rocce senza mai stancarsi.",
    3: "Corazza di metallo fuso sul dorso. Il suo ruggito fa vibrare le fucine di Ember.",
    4: "Piccolo nuotatore curioso. Le pinne a goccia raccolgono il Lumen dei ruscelli.",
    5: "Disegna onde nell'aria con la coda. Gioca con le correnti dei fiumi di Verdan.",
    6: "Le sue squame riflettono le stelle. Si dice che guidi i naviganti nelle notti senza luna.",
    7: "Una zolla di muschio con due occhi vispi. Dorme al sole e cresce di un dito ogni estate.",
    8: "Sul dorso spuntano felci e piccoli fiori. Protegge i germogli dagli Elyon affamati.",
    9: "Metà foresta e metà montagna. Dove cammina, la terra smossa si riempie di radici.",
    10: "Volpino scattante con la punta della coda accesa. Ama inseguire le scintille.",
    11: "Il pelo crepita di calore. In branco accende fuochi di segnalazione sulle colline.",
    12: "Ali di brace e corna ricurve. Quando vola lascia una scia di cenere luminosa.",
    13: "Girino timido che si nasconde tra le ninfee. Sputa bolle quando si spaventa.",
    14: "Nuota controcorrente per giorni. Le sue branchie filtrano il Lumen dall'acqua.",
    15: "Cavalca le onde come fossero vento. Il suo canto annuncia le tempeste in arrivo.",
    16: "Una foglia con le zampe. Si mimetizza nel sottobosco e salta tra i rami.",
    17: "Elegante e silenziosa. Le foglie della criniera ruotano verso il sole.",
    18: "Batuffolo elettrico. Se lo accarezzi contropelo ti dà una scossa affettuosa.",
    19: "Corre più veloce di un fulmine. Le sue strisce gialle si accendono nel buio.",
    20: "Uccellino leggero come una piuma. Si lascia trasportare dalle brezze del mattino.",
    21: "Signore delle correnti alte. Plana per ore senza muovere le ali.",
    22: "Un sasso con le zampette. Rotola giù dai pendii e si ferma dove trova il sole.",
    23: "Massiccio come una frana. Le rocce sul dorso crescono strato dopo strato.",
    24: "Fiocco di neve vivente. Lascia brina sulle finestre per salutare.",
    25: "Danza nelle bufere di Nival. Il suo respiro trasforma la nebbia in cristalli.",
    26: "Ombra curiosa che segue i viandanti. Sparisce appena ti volti a guardarla.",
    27: "Predatore delle notti senza stelle. I suoi occhi rossi sono l'ultima cosa che si vede.",
    28: "Lucciola di Lumen puro. Uno sciame di Lumi illumina un intero sentiero.",
    29: "Il suo corpo è una piccola costellazione. Dove passa, i fiori notturni si aprono.",
    30: "Lumaca dal guscio violaceo. La bava è irritante, ma lui è molto gentile.",
    31: "Il guscio si è indurito in pietra velenosa. Vive nelle miniere abbandonate.",
    32: "Sogna a occhi aperti e i suoi sogni si vedono come bolle di luce.",
    33: "Legge i pensieri di chi ha accanto. Preferisce la compagnia dei silenziosi.",
    34: "Coleottero corazzato. Mastica rottami di ferro e li trasforma in scaglie.",
    35: "La sua corazza resiste a qualsiasi colpo. Fa la guardia alle fonderie di Ember.",
    36: "Minuscola salamandra di brace. Vive dentro i camini e scalda le case d'inverno.",
    37: "Rametto di corallo che cammina sul fondale. Ogni anno aggiunge un nuovo ramo.",
    38: "Insetto verde e veloce. Si nasconde sotto le foglie larghe quando piove.",
    39: "Felino nervoso con la coda biforcuta. Prima di un temporale non riesce a stare fermo.",
    40: "Uccello minuscolo che vola a velocità incredibile. Il suo battito d'ali sibila.",
    41: "Un macigno sonnolento. Se lo disturbi si limita a rotolare un po' più in là.",
    42: "Cristallo di ghiaccio che fluttua. Brilla di azzurro nelle grotte più profonde.",
    43: "Pipistrello d'ombra. Caccia in silenzio sotto la luna nuova.",
    44: "Stellina caduta dal cielo. Di notte cerca di tornare lassù saltando sempre più in alto.",
    45: "Serpente dalle scaglie viola. Avverte i nemici scuotendo la coda a sonaglio.",
    46: "Farfalla dalle ali che ipnotizzano. Chi la fissa troppo a lungo si addormenta.",
    47: "Piccolo armadillo di ferro. Si chiude a palla e rotola via dai pericoli.",
    48: "Scorpione delle dune. Si seppellisce nella sabbia e aspetta il tramonto.",
    49: "Turbine di sabbia con gli artigli. Le tempeste del deserto di Arkas portano il suo nome.",
    50: "Lucertola di lava. Il suo dorso cola magma che si raffredda in pietra.",
    51: "Rana delle paludi con una ninfea sulla testa. Canta per far piovere.",
    52: "Destriero di metallo e fulmini. Ogni passo scaglia scintille sul terreno.",
    53: "Gazzella di ghiaccio stellato. Appare solo nelle notti più fredde di Nival.",
    54: "Rapace notturno dalle ali silenziose. Vola sopra le nubi per non farsi vedere.",
    55: "Fata di luce e pensiero. Parla con i sogni di chi dorme vicino a lei.",
    56: "Cobra corazzato. Il veleno delle sue zanne scava la roccia.",
    57: "Albero d'acciaio. Le sue foglie metalliche tintinnano al vento.",
    58: "Uccello di fiamma azzurra. Il suo fuoco non brucia, ma scalda il cuore.",
    59: "Medusa che vola sulle correnti marine. Le sue frange danzano nel vento salato.",
    60: "Grosso insetto coperto di radici e pietre. Scava tunnel che diventano sorgenti.",
    61: "Fortezza vivente di roccia e acciaio. Non arretra mai di un passo.",
    62: "Orso polare corazzato di ghiaccio duro come il metallo.",
    63: "Serpente d'ombra dal respiro tossico. Si muove solo dove non arriva la luce.",
    64: "Gatto lunare con gli occhi pieni di stelle. Vede il futuro per pochi secondi.",
    65: "Mente oscura e brillante. I suoi pensieri pesano come piombo su chi gli sta vicino.",
    66: "Drago d'ombra e fuoco nero. Le leggende lo dicono nato da un'eclissi.",
    67: "Uccello celeste dalle ali trasparenti. Vola così in alto da toccare le aurore.",
    68: "Gigante di pietra antica. Sulle sue spalle crescono piccoli boschi.",
    69: "Serpente marino corazzato. Custodisce i relitti dell'arcipelago di Talassa.",
    70: "Leone solare. La sua criniera di fiamma astrale illumina la notte come un'alba.",
    71: "Lupo d'ombra rivestito d'acciaio scuro. Ubbidisce solo a chi lo ha battuto.",
    72: "Cervo di cristallo che pensa in silenzio. Il gelo segue ogni sua parola.",
    73: "Spirito della foresta stellata. Fa fiorire i semi che toccano la sua luce.",
    74: "Leggendario del fuoco. Il suo risveglio, dicono, accende i vulcani di Vulkar.",
    75: "Leggendaria degli oceani. Il suo canto calma le maree di tutta Asterra.",
    76: "Leggendario del fulmine. Attraversa il cielo di Asterra in un battito di ciglia.",
    77: "Leggendaria dei ghiacci eterni. Il suo respiro ferma il tempo per un istante.",
    78: "Leggendario dei venti. Si dice che nessuno l'abbia mai visto fermo.",
    79: "Leggendario della roccia. Le montagne di Asterra sarebbero le sue ossa.",
    80: "Leggendario dell'ombra. Custodisce i confini tra il giorno e la notte.",
    81: "Basilisco di lava e pietra. Il suo sguardo fa crepitare il terreno.",
    82: "Corvo dalle piume taglienti come vento notturno. Ruba ciò che luccica.",
    83: "Silfide delle brezze. Dove danza spuntano erbe aromatiche.",
    84: "Colosso magnetico. Le rocce intorno a lui fluttuano a mezz'aria.",
    85: "Anguilla delle paludi velenose. Le sue scariche d'acqua sono tossiche.",
    86: "Figura incappucciata d'ombra. Svanisce e ricompare nei pensieri altrui.",
    87: "Cavaliere di metallo stellare. Si dice sia stato creato dai Custodi.",
    88: "Pianta carnivora gigante. I suoi fiori profumano di miele e di veleno.",
    89: "Farfalla psichica dalle ali di vento. Comunica con i sogni degli altri Elyon.",
    90: "Rinoceronte di ferro e roccia. La sua carica sfonda le pareti delle miniere.",
    91: "Montagna di fuoco che cammina. Il calore del suo corpo scioglie la pietra.",
    92: "Rondine elettrica. Lascia scie di luce che restano nel cielo per minuti.",
    93: "Tartaruga corallina enorme. Sul guscio vivono interi banchi di pesci.",
    94: "Fiore che pensa. I suoi petali si chiudono quando percepisce cattive intenzioni.",
    95: "Rondine celeste velocissima. Attraversa Asterra dall'alba al tramonto.",
    96: "Gigante di ghiaccio e granito. Riposa per secoli nelle caverne di Nival.",
    97: "Pantera notturna che corre sul vento. Caccia solo durante le tempeste.",
    98: "Stella vivente carica di energia. La sua luce alimenta i cristalli di Lumen.",
    99: "Leggendaria del crepuscolo. Appare dove il giorno e la notte si incontrano.",
    100: "Creatura primordiale legata al Vuoto. Luce e ombra convivono nel suo corpo.",
}


# ==================================================================
def tipo_c(t):
    return "TYPE_NONE" if t in (None, "NEUTRO") else "TYPE_" + t


def cstr(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def carica_specie():
    specie = {}
    for riga in SPECIE_TXT.strip().splitlines():
        p = riga.split()
        sid, nome, tipi = int(p[0]), p[1], p[2].split("/")
        stats = list(map(int, p[3:9]))
        assert len(p) == 9, riga
        for t in tipi:
            assert t in TIPI, f"tipo non valido {t} in {riga} (TERRA non e' un tipo)"
        t1 = tipi[0]
        t2 = tipi[1] if len(tipi) > 1 else None
        specie[sid] = {"id": sid, "nome": nome, "t1": t1, "t2": t2, "stats": stats, "tot": sum(stats)}
    assert sorted(specie) == list(range(1, 101)), "servono esattamente gli ID 1..100"
    nomi = [s["nome"] for s in specie.values()]
    assert len(set(nomi)) == 100, "nomi duplicati"
    return specie


def carica_evoluzioni(specie):
    evo = []
    for parte in EVOLUZIONI_TXT.split(";"):
        m = re.fullmatch(r"(\d+)>(\d+)@(\d+)", parte.strip())
        assert m, parte
        da, a, liv = map(int, m.groups())
        assert da in specie and a in specie and 1 < liv <= 100
        evo.append((da, a, liv))
    da_set = [e[0] for e in evo]
    assert len(set(da_set)) == len(da_set), "una specie evolve in un solo modo"
    return evo


def carica_mosse():
    mosse = {}
    for riga in MOSSE_TXT.strip().splitlines():
        sid, resto = riga.split(" ", 1)
        nome, tipo, cat, pot, prec, pp, eff = resto.split("|")
        sid = int(sid)
        assert tipo == "NEUTRO" or tipo in TIPI, riga
        assert cat in "FST" and eff in EFFETTI, f"effetto sconosciuto: {eff}"
        mosse[sid] = {"id": sid, "nome": nome, "tipo": None if tipo == "NEUTRO" else tipo, "cat": cat,
                      "pot": 0 if pot == "variabile" else int(pot), "prec": int(prec), "pp": int(pp), "eff": eff}
    assert sorted(mosse) == list(range(1, 101))
    return mosse


def id_mossa(mosse, nome):
    for m in mosse.values():
        if m["nome"] == nome:
            return m["id"]
    raise KeyError(nome)


def classifica(specie, evo):
    pre = {a: da for da, a, _ in evo}

    def radice(sid):
        while sid in pre:
            sid = pre[sid]
        return sid

    for s in specie.values():
        sid = s["id"]
        if sid == 100:
            r = "RARITA_PRIMORDIALE"
        elif 74 <= sid <= 80 or sid == 99:
            r = "RARITA_LEGGENDARIO"
        elif sid <= 9 or s["tot"] >= 480:
            r = "RARITA_RARO"
        else:
            r = "RARITA_COMUNE"
        s["rarita"] = r
        s["catch"] = {"RARITA_COMUNE": 0.20, "RARITA_RARO": 0.08, "RARITA_LEGGENDARIO": 0.015, "RARITA_PRIMORDIALE": 0.0}[r]
        s["yield"] = max(4, round(s["tot"] / 50))
    for s in specie.values():
        rad = specie[radice(s["id"])]
        if rad["id"] in (1, 4, 7):
            s["crescita"] = 1.0          # i tre starter crescono allo stesso ritmo
        elif rad["rarita"] in ("RARITA_LEGGENDARIO", "RARITA_PRIMORDIALE") or rad["tot"] >= 560:
            s["crescita"] = 1.2
        elif rad["tot"] < 300:
            s["crescita"] = 0.8
        else:
            s["crescita"] = 1.0


def assegna_abilita(specie):
    ecc = {}
    for parte in ECCEZIONI_ABILITA_TXT.split(","):
        m = re.fullmatch(r"#(\d+) (\d+)", parte.strip())
        assert m, parte
        ecc[int(m.group(1))] = int(m.group(2))
    ids = {a[0] for a in ABILITA}
    for s in specie.values():
        s["abilita"] = ecc.get(s["id"], ABILITA_PER_TIPO[s["t1"]])
        assert s["abilita"] in ids
    return ecc


# ------------------------------------------------------------------
# Learnset
# ------------------------------------------------------------------
LIVELLI = [1, 5, 9, 13, 18, 24, 30, 38, 46, 55]
ESCLUSIVE = {98, 99, 100}


def genera_learnset(s, mosse):
    fisico = s["stats"][1] >= s["stats"][3]
    cat_pref = "F" if fisico else "S"

    def danno(tipo):
        c = [m for m in mosse.values() if m["tipo"] == tipo and m["cat"] != "T" and m["id"] not in ESCLUSIVE]
        pref = [m for m in c if m["cat"] == cat_pref]
        altri = [m for m in c if m["cat"] != cat_pref]
        return sorted(pref, key=lambda m: m["pot"]) + sorted(altri, key=lambda m: m["pot"])

    def stato(tipo):
        return [m for m in mosse.values() if m["tipo"] == tipo and m["cat"] == "T" and m["id"] not in ESCLUSIVE]

    usate = []

    def prendi(lista):
        for m in lista:
            if m["id"] not in [u[1] for u in usate]:
                return m["id"]
        return None

    prim_sorted = sorted(danno(s["t1"]), key=lambda m: m["pot"])
    sec = sorted(danno(s["t2"]), key=lambda m: m["pot"]) if s["t2"] else []
    neutre = [mosse[i] for i in ((2 if fisico else 1), 3, 4, 5, 6 if fisico else 7)]

    def aggiungi(liv, mid):
        if mid is not None and mid not in [u[1] for u in usate]:
            usate.append((liv, mid))

    aggiungi(1, mosse[2]["id"] if fisico else mosse[1]["id"])          # mossa neutra base

    # potenza crescente del tipo primario, con un tetto per livello
    def prossima_primaria(tetto):
        restanti = [m for m in prim_sorted if m["id"] not in [u[1] for u in usate] and m["pot"] <= tetto]
        pref = [m for m in restanti if m["cat"] == cat_pref]
        scelta = (pref or restanti or [None])[0]
        return scelta["id"] if scelta else None

    aggiungi(1, prossima_primaria(50))                                 # STAB base
    aggiungi(5, prendi(stato(s["t1"])) or 10)                          # stato pertinente (o RAPIDITA')
    aggiungi(9, prossima_primaria(70) or prendi([mosse[4]]))            # ripiego: IMPATTO
    aggiungi(13, prendi(sec) if sec else 3)
    aggiungi(18, prossima_primaria(85) or prendi([mosse[4], mosse[92]]))
    st24 = prendi(stato(s["t2"])) if s["t2"] else None
    aggiungi(24, st24 or prendi(stato(s["t1"])) or (93 if fisico else 81 if s["t1"] == "PSICHE" else 10))
    aggiungi(30, prossima_primaria(100) or prendi([mosse[6], mosse[7]] if fisico else [mosse[7], mosse[6]]))
    sec_forti = [m for m in sec if 70 <= m["pot"] <= 110]
    aggiungi(38, prendi(sec_forti) if sec_forti else prendi(neutre[3:]))
    # ai livelli alti solo mosse forti: la piu' potente non ancora usata (>= 80), altrimenti neutre forti
    def forte():
        restanti = [m for m in prim_sorted if m["id"] not in [u[1] for u in usate] and m["pot"] >= 80]
        if restanti:
            return max(restanti, key=lambda m: (m["pot"], m["cat"] == cat_pref))["id"]
        return prendi([mosse[7], mosse[6]] if not fisico else [mosse[6], mosse[7]])
    aggiungi(46, forte())
    aggiungi(55, forte())
    return sorted(usate)


def learnset_speciali(specie, mosse):
    pyrin = [(1, "BRACIAVIVA"), (5, "GRAFFIO"), (9, "MARCHIO DEL FUOCO"), (13, "FIAMMATA"), (17, "RAPIDITÀ"),
             (22, "ARTIGLIO METALLICO"), (28, "INFERNO CREMISI"), (35, "NUCLEO ARDENTE")]
    base = [(l, id_mossa(mosse, n)) for l, n in pyrin]
    specie[1]["learnset"] = base
    specie[2]["learnset"] = base
    specie[3]["learnset"] = base + [(42, id_mossa(mosse, "IMPATTO FERROSO")), (50, id_mossa(mosse, "CANNONE FERROSO"))]
    aurion = ["RAGGIO STELLARE", "ABISSO", "ROTTURA LUMEN", "EQUILIBRIO", "NOVA", "ECLISSI", "SUPERNOVA", "FRATTURA DEL VUOTO"]
    livelli = [1, 1, 1, 1, 30, 45, 60, 65]
    specie[100]["learnset"] = [(l, id_mossa(mosse, n)) for l, n in zip(livelli, aurion)]


# ==================================================================
# Scrittura dei file C
# ==================================================================
TESTATA = "/* FILE GENERATO da tools/genera_database.py - modifica lo script, non questo file. */\n"


def scrivi(nome, testo):
    with open(os.path.join(OUT, nome), "w", encoding="utf-8") as f:
        f.write(TESTATA + testo)


def gen_specie(specie, mosse):
    righe = ['#include "species_data.h"', "#include <stddef.h>", "#include <string.h>", "",
             "static const SpeciesData SPECIE[NUM_SPECIE] = {"]
    for sid in range(1, 101):
        s = specie[sid]
        ls = s["learnset"]
        assert len(ls) <= 16
        lst = ", ".join(f"{{{l}, {m}}}" for l, m in ls)
        st = ", ".join(map(str, s["stats"]))
        righe.append(f"    {{ {sid}, {cstr(s['nome'])}, {tipo_c(s['t1'])}, {tipo_c(s['t2'])}, {{ {st} }}, "
                     f"{s['abilita']}, {s['rarita']}, {s['catch']}f, {s['crescita']}f, {s['yield']},\n"
                     f"      {cstr(DESCRIZIONI[sid])},\n      {{ {lst} }}, {len(ls)} }},")
    righe.append("};")
    righe.append('''
static const FormData FORME[] = {
    { 100, 1, "AURION VOID", 46, "Forma boss della fase 2: statistiche di base, abilità FRATTURA_DEL_VUOTO." },
    { 100, 2, "AURION ORIGINALE", 30, "Forma post-game: statistiche di base, abilità EQUILIBRIO_DEL_VUOTO." },
};

const SpeciesData *GetSpecies(int id) {
    if (id < 1 || id > NUM_SPECIE) return NULL;
    return &SPECIE[id - 1];
}

const SpeciesData *GetSpeciesByName(const char *nome) {
    if (!nome) return NULL;
    for (int i = 0; i < NUM_SPECIE; i++)
        if (strcmp(SPECIE[i].nome, nome) == 0) return &SPECIE[i];
    return NULL;
}

const FormData *GetForm(int specie, int forma) {
    for (size_t i = 0; i < sizeof(FORME) / sizeof(FORME[0]); i++)
        if (FORME[i].specie == specie && FORME[i].forma == forma) return &FORME[i];
    return NULL;
}

int NumForme(void) { return (int)(sizeof(FORME) / sizeof(FORME[0])); }

const char *Rarita_Nome(Rarita r) {
    switch (r) {
        case RARITA_COMUNE: return "COMUNE";
        case RARITA_RARO: return "RARO";
        case RARITA_LEGGENDARIO: return "LEGGENDARIO";
        case RARITA_PRIMORDIALE: return "PRIMORDIALE";
    }
    return "?";
}
''')
    scrivi("species_data.c", "\n".join(righe))


def gen_mosse(mosse):
    righe = ['#include "moves_data.h"', "#include <stddef.h>", "#include <string.h>", "",
             "static const MoveData MOSSE[NUM_MOSSE] = {"]
    for mid in range(1, 101):
        m = mosse[mid]
        eff, prob, p1, p2, p3, p4, prio, flag = EFFETTI[m["eff"]]
        cat = {"F": "CAT_FISICA", "S": "CAT_SPECIALE", "T": "CAT_STATO"}[m["cat"]]
        flags = [flag] if flag else []
        if m["cat"] == "T" and eff in ("EFF_BOOST_SE", "EFF_CURA_PERC", "EFF_RADICAMENTO", "EFF_DIMEZZA_PROSSIMO",
                                       "EFF_RIFLESSIONE", "EFF_FURIA", "EFF_CAMPO_TIPO", "EFF_EQUILIBRIO"):
            flags.append("MOSSA_PRECISIONE_AUTOMATICA")
        fl = " | ".join(flags) if flags else "0"
        righe.append(f"    {{ {mid}, {cstr(m['nome'])}, {tipo_c(m['tipo'])}, {cat}, {m['pot']}, {m['prec']}, {m['pp']}, "
                     f"{prio}, {eff}, {prob}, {p1}, {p2}, {p3}, {p4}, {fl}, {cstr(m['eff'])} }},")
    righe.append("};")
    righe.append('''
const MoveData *GetMove(int id) {
    if (id < 1 || id > NUM_MOSSE) return NULL;
    return &MOSSE[id - 1];
}

const MoveData *GetMoveByName(const char *nome) {
    if (!nome) return NULL;
    for (int i = 0; i < NUM_MOSSE; i++)
        if (strcmp(MOSSE[i].nome, nome) == 0) return &MOSSE[i];
    return NULL;
}

const char *Categoria_Nome(CategoriaMossa c) {
    switch (c) {
        case CAT_FISICA: return "FISICA";
        case CAT_SPECIALE: return "SPECIALE";
        case CAT_STATO: return "STATO";
    }
    return "?";
}
''')
    scrivi("moves_data.c", "\n".join(righe))


def gen_abilita():
    righe = ['#include "abilities_data.h"', '#include "moves_data.h"', '#include "species_data.h"', "#include <stddef.h>", "",
             "static const AbilityData ABILITA[NUM_ABILITA] = {"]
    for aid, nome, eff, descr in ABILITA:
        if eff is None:
            righe.append(f"    {{ {aid}, {cstr(nome)}, {ALIAS[aid]}, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 0, {cstr(descr)} }},")
        else:
            e, cond, t1, t2, molt, par, _ = eff
            righe.append(f"    {{ {aid}, {cstr(nome)}, 0, {e}, {cond}, {tipo_c(t1)}, {tipo_c(t2)}, {molt}f, {par}, {cstr(descr)} }},")
    righe.append("};")
    righe.append('''
const AbilityData *GetAbility(int id) {
    if (id < 1 || id > NUM_ABILITA) return NULL;
    return &ABILITA[id - 1];
}

const AbilityData *GetAbilityResolved(int id) {
    const AbilityData *a = GetAbility(id);
    for (int passi = 0; a && a->aliasDi != 0 && passi < 4; passi++) a = GetAbility(a->aliasDi);
    return a;
}
''')
    scrivi("abilities_data.c", "\n".join(righe))


def gen_evoluzioni(evo):
    righe = ['#include "evolution_data.h"', "#include <stddef.h>", "", "static const EvolutionData EVOLUZIONI[] = {"]
    for da, a, liv in evo:
        righe.append(f"    {{ {da}, {a}, {liv} }},")
    righe.append("};")
    righe.append('''
#define NUM_EVO ((int)(sizeof(EVOLUZIONI) / sizeof(EVOLUZIONI[0])))

const EvolutionData *GetEvolution(int specie) {
    for (int i = 0; i < NUM_EVO; i++) if (EVOLUZIONI[i].da == specie) return &EVOLUZIONI[i];
    return NULL;
}

const EvolutionData *GetPreEvolution(int specie) {
    for (int i = 0; i < NUM_EVO; i++) if (EVOLUZIONI[i].a == specie) return &EVOLUZIONI[i];
    return NULL;
}

int Evolution_LivelloMinimo(int specie) {
    const EvolutionData *e = GetPreEvolution(specie);
    return e ? e->livello : 1;
}

int NumEvoluzioni(void) { return NUM_EVO; }

const EvolutionData *GetEvolutionIndex(int i) {
    return (i >= 0 && i < NUM_EVO) ? &EVOLUZIONI[i] : NULL;
}
''')
    scrivi("evolution_data.c", "\n".join(righe))


def gen_allenatori(specie, mosse, evo):
    per_nome = {s["nome"]: s for s in specie.values()}
    pre = {a: liv for _, a, liv in evo}
    righe = ['#include "trainers_data.h"', "#include <stddef.h>", "#include <string.h>", "",
             "static const TrainerData ALLENATORI[] = {"]
    for tid, nome, classe, premio, ia, squadra, ric, nota in ALLENATORI:
        membri = []
        sotto = False
        for voce in squadra.split(","):
            m = re.fullmatch(r"\s*([A-Z]+)(\d+)\s*", voce)
            assert m, voce
            s = per_nome[m.group(1)]
            liv = int(m.group(2))
            if s["id"] in pre and liv < pre[s["id"]]:
                sotto = True
            ms = MOSSE_SPECIALI.get(tid, [])
            ids = [id_mossa(mosse, n) for n in ms] + [0] * (4 - len(ms))
            membri.append(f"{{ {s['id']}, {liv}, {{ {', '.join(map(str, ids))} }} }}")
        assert len(membri) <= 6
        if sotto:
            assert tid in ECCEZIONI_LIVELLO, f"{tid}: livello sotto l'evoluzione senza eccezione esplicita"
            nota = (nota + " " if nota else "") + ECCEZIONI_LIVELLO[tid]
        interr = "true" if tid == "VAEL_INTERMEZZO" else "false"
        righe.append(f"    {{ {cstr(tid)}, {cstr(nome)}, {cstr(classe)}, {premio}, {ia},\n      {{ {', '.join(membri)} }}, "
                     f"{len(membri)}, {cstr(ric)}, {interr}, {'true' if sotto else 'false'}, {cstr(nota)} }},")
    righe.append("};")
    righe.append('''
#define NUM_ALL ((int)(sizeof(ALLENATORI) / sizeof(ALLENATORI[0])))

const TrainerData *GetTrainer(const char *id) {
    if (!id) return NULL;
    for (int i = 0; i < NUM_ALL; i++) if (strcmp(ALLENATORI[i].id, id) == 0) return &ALLENATORI[i];
    return NULL;
}

int NumAllenatori(void) { return NUM_ALL; }

const TrainerData *GetTrainerIndex(int i) {
    return (i >= 0 && i < NUM_ALL) ? &ALLENATORI[i] : NULL;
}
''')
    scrivi("trainers_data.c", "\n".join(righe))


def gen_habitat(specie):
    per_nome = {s["nome"]: s for s in specie.values()}
    righe = ['#include "encounters_data.h"', "#include <stddef.h>", "#include <string.h>", "",
             "static const HabitatData HABITAT[] = {"]
    for hid, nome, lmin, lmax, voci in HABITAT:
        el = []
        if voci:
            tot = 0
            for v in voci.split(","):
                m = re.fullmatch(r"\s*([A-Z]+)(\d+)%\s*", v)
                assert m, v
                el.append(f"{{ {per_nome[m.group(1)]['id']}, {m.group(2)} }}")
                tot += int(m.group(2))
            assert tot == 100, f"{hid}: percentuali = {tot}"
        corpo = ", ".join(el) if el else "{ 0, 0 }"
        righe.append(f"    {{ {cstr(hid)}, {cstr(nome)}, {lmin}, {lmax}, {{ {corpo} }}, {len(el)} }},")
    righe.append("};")
    righe.append('''
#define NUM_HAB ((int)(sizeof(HABITAT) / sizeof(HABITAT[0])))

const HabitatData *GetHabitat(const char *id) {
    if (!id) return NULL;
    for (int i = 0; i < NUM_HAB; i++) if (strcmp(HABITAT[i].id, id) == 0) return &HABITAT[i];
    return NULL;
}

int NumHabitat(void) { return NUM_HAB; }

const HabitatData *GetHabitatIndex(int i) {
    return (i >= 0 && i < NUM_HAB) ? &HABITAT[i] : NULL;
}
''')
    scrivi("encounters_data.c", "\n".join(righe))


def gen_bestiario_md(specie, mosse, evo):
    os.makedirs(os.path.join(RADICE, "docs"), exist_ok=True)
    nome_ab = {a[0]: a[1] for a in ABILITA}
    evo_da = {da: (a, l) for da, a, l in evo}
    r = ["# ECLYRA — Bestiario e database (generato da tools/genera_database.py)", "",
         "Rarità: comune 0.20 · raro 0.08 · leggendario 0.015 · primordiale 0.0. "
         "Crescita: XP per il livello L = floor(crescita × L³).", "",
         "| # | Nome | Tipo | HP | ATK | DEF | SPA | SPD | SPE | Tot | Abilità | Rarità | Crescita | Resa XP | Evolve |",
         "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for sid in range(1, 101):
        x = specie[sid]
        t = x["t1"] + ("/" + x["t2"] if x["t2"] else "")
        ev = f"{specie[evo_da[sid][0]]['nome']} al Lv {evo_da[sid][1]}" if sid in evo_da else "—"
        r.append(f"| {sid:03d} | {x['nome']} | {t} | " + " | ".join(map(str, x["stats"])) +
                 f" | {x['tot']} | {nome_ab[x['abilita']]} | {x['rarita'][7:].lower()} | {x['crescita']} | {x['yield']} | {ev} |")
    r += ["", "## Descrizioni e mosse apprese", ""]
    for sid in range(1, 101):
        x = specie[sid]
        ls = ", ".join(f"Lv{l} {mosse[m]['nome']}" for l, m in x["learnset"])
        r += [f"**{sid:03d} {x['nome']}** — {DESCRIZIONI[sid]}  ", f"Mosse: {ls}", ""]
    r += ["## Mosse", "", "| # | Nome | Tipo | Cat. | Pot. | Prec. | PP | Effetto |", "|---|---|---|---|---|---|---|---|"]
    for mid in range(1, 101):
        m = mosse[mid]
        r.append(f"| {mid:03d} | {m['nome']} | {m['tipo'] or 'NEUTRO'} | {m['cat']} | {m['pot'] or '—'} | {m['prec']} | {m['pp']} | {m['eff']} |")
    r += ["", "## Allenatori", ""]
    for tid, nome, classe, premio, ia, squadra, ric, nota in ALLENATORI:
        extra = f" — {ECCEZIONI_LIVELLO[tid]}" if tid in ECCEZIONI_LIVELLO else ""
        r.append(f"- **{tid}** ({classe}, premio {premio}, {ia}): {squadra}{' → ' + ric if ric else ''}{extra}")
    with open(os.path.join(RADICE, "docs", "BESTIARIO.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(r) + "\n")


def main():
    specie = carica_specie()
    evo = carica_evoluzioni(specie)
    mosse = carica_mosse()
    classifica(specie, evo)
    assegna_abilita(specie)
    for s in specie.values():
        s["learnset"] = genera_learnset(s, mosse)
    learnset_speciali(specie, mosse)
    assert len(DESCRIZIONI) == 100
    gen_specie(specie, mosse)
    gen_mosse(mosse)
    gen_abilita()
    gen_evoluzioni(evo)
    gen_allenatori(specie, mosse, evo)
    gen_habitat(specie)
    gen_bestiario_md(specie, mosse, evo)
    from collections import Counter
    print("Database generato:", Counter(s["rarita"] for s in specie.values()))


if __name__ == "__main__":
    main()
