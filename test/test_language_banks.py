"""Regresiones de texto en los packs de idioma (assets/lang)."""
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LANG = ROOT / "assets" / "language.data"

SPECIAL = {0x01: "!", 0x03: "Ö", 0x05: "Ä", 0x06: "Ü", 0x07: "'",
           0x0C: ",", 0x0D: "-", 0x0E: ".", 0x1F: "?", 0x3B: "|", 0x3C: "|"}


def _glyph(c):
    if 0x10 <= c <= 0x19:
        return chr(ord("0") + c - 0x10)
    if c == 0x20:
        return " "
    if 0x21 <= c <= 0x3A:
        return chr(ord("A") + c - 0x21)
    return SPECIAL.get(c, f"[{c:02x}]")


def _decode(data):
    out = []
    for b in data:
        c, flags = b & 0x3F, b & 0xC0
        if c == 0:
            if flags == 0:
                out.append("\n")
            elif flags == 0x40:
                out.append(" ")
            else:
                out.append(" E" if flags == 0x80 else " A")
            continue
        out.append(_glyph(c))
        if flags == 0x40:
            out.append(" ")
        elif flags == 0x80:
            out.append("E")
        elif flags == 0xC0:
            out.append("A")
    return "".join(out)


def _entries(data):
    p = n = 0
    while p < len(data) and data[p]:
        size = data[p]
        yield n, _decode(data[p + 1:p + size])
        p += size
        n += 1


def _from_container(name):
    data = LANG.read_bytes()
    assert data[:8] == b"KULTDTA1"
    _version, kind, count = struct.unpack_from("<III", data, 8)
    assert kind == 2
    for i in range(count):
        base = 20 + 72 * i
        entry = data[base:base + 64].split(b"\0", 1)[0].decode("ascii")
        off, size = struct.unpack_from("<II", data, base + 64)
        if entry == name:
            return data[off:off + size]
    raise AssertionError(f"falta {name} en language.data")


def test_verbo_die_es_el_objeto_dado_no_morir():
    for bank, expected in (("VERBS.BIN", "DADO"), ("VERBC.BIN", "DAU")):
        source = next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()
        assert dict(_entries(source))[116] == expected
        assert dict(_entries(_from_container(bank)))[116] == expected
        assert dict(_entries(source))[116] != "MORIR"


def test_erbe_es_import():
    counts = {"VERBS.BIN": 176, "DESCS.BIN": 407, "MOTSS.BIN": 131, "DIALS.BIN": 337}
    for bank, count in counts.items():
        source = next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()
        entries = dict(_entries(source))
        assert len(entries) == count
        assert dict(_entries(_from_container(bank))) == entries

    verbs = dict(_entries(_from_container("VERBS.BIN")))
    descs = dict(_entries(_from_container("DESCS.BIN")))
    mots = dict(_entries(_from_container("MOTSS.BIN")))
    dials = dict(_entries(_from_container("DIALS.BIN")))
    assert verbs[116] == "DADO"
    assert mots[11] == "LOS GEMELOS"
    assert mots[47] == "PAVO"
    assert mots[92] == "DESAGÜE"
    assert mots[129] == "SCI FI"
    assert "SCI FI" in descs[202]
    assert descs[406] == "[1a]"
    assert "VICTORIA\nDE\nZORQ." in dials[333]
    for entries in (descs, dials):
        assert not any("SHEIFED" in t or "SAI FAI" in t for t in entries.values())


def test_catalan_differential():
    expected = {
        ("VERBC.BIN", 1): "PERTINENCES",
        ("VERBC.BIN", 13): "DESPLACAR",
        ("VERBC.BIN", 17): "MOURE",
        ("VERBC.BIN", 39): "OMPLIR GOT",
        ("VERBC.BIN", 52): "ENCAIXAR LA MA",
        ("VERBC.BIN", 60): "ESTIRAR",
        ("VERBC.BIN", 71): "OFERIR",
        ("VERBC.BIN", 81): "LLAN[0b]AR SOBRE",
        ("VERBC.BIN", 83): "DONAR",
        ("VERBC.BIN", 104): "DAGA",
        ("VERBC.BIN", 117): "HIPERVISIO",
        ("VERBC.BIN", 115): "FESOL",
        ("VERBC.BIN", 145): "LLAN[0b]AR LLIGANYA",
        ("VERBC.BIN", 164): "TIRAR",
        ("VERBC.BIN", 155): "DONAR\nEL FESOL PER MENJAR",
        ("MOTSC.BIN", 51): "ASH",
        ("MOTSC.BIN", 24): "NAIXEMENT D'UNA\nRA[0b]A DIVINA",
        ("MOTSC.BIN", 77): "CAL[0b] VIVA",
        ("MOTSC.BIN", 124): "COMERCIANT",
        ("MOTSC.BIN", 92): "DESGUAS",
        ("MOTSC.BIN", 129): "SCI FI",
        ("DESCC.BIN", 0): "LES COSES HAN\nANAT\nMASSA LLUNY! UN ZAPSTIK CAU AMB FOR[0b]A...",
        ("DESCC.BIN", 18): "ET LLEN[0b]A A TERRA.",
        ("DESCC.BIN", 45): "UN ASPIRANT COM TU, AMB PRESSA PER ACONSEGUIR ELS CINC CRANIS!",
        ("DESCC.BIN", 129): "ELS ALTRES PROTOZORQ LLAN[0b]EN UN ATAC MASSIU!",
        ("DESCC.BIN", 156): "UNA ABRA[0b]ADA SALVATGE. LES SEVES UNGLES AFILADES T'OBREN L'ESQUENA. EL VERI ES RAPID...",
        ("DESCC.BIN", 173): "... LLEN[0b]AT AMB VIOLENCIA A TERRA!",
        ("DESCC.BIN", 175): "NI LA DAGA NI LA FOR[0b]A BASTEN PER DEIXAR-TE PASSAR.",
        ("DESCC.BIN", 213): "ELS GALLS DINDI SEMBLEN MALEVOLS... COMEN[0b]EN A ACOSTAR-SE AMENASSADORS...",
        ("DESCC.BIN", 227): "ELS ALTRES VAN A ABALAN[0b]AR-SE SOBRE TU...!",
        ("DESCC.BIN", 368): "QRIICH CAU A TERRA I COMEN[0b]A A CRIDAR.",
        ("DESCC.BIN", 404): "AMPLIFICADOR HITACHI-GAUSS, CONEGUT COM A GAUSS O FET. EL TEU ENLLA[0b] TELEPATIC AMB LA XARXA TUNER",
        ("DESCC.BIN", 50): "QUEDES ATRAPAT A LA\nTERANYINA!",
        ("DESCC.BIN", 73): "UN CURIOS HOMENET AL REVES.",
        ("DESCC.BIN", 126): "EL CAMINANT\nSALVARA ALGU I INDICARA\nLA SEVA ELECCIO AL REPTIL.",
        ("DESCC.BIN", 261): "MENTRE AVANCES, ACTIVES UN MECANISME HORRIPILANT!\nEL SOSTRE ET CAU A SOBRE!",
        ("DESCC.BIN", 339): "UN GOT DE METALL.",
        ("DESCC.BIN", 350): "QUEDA 1 GLOP.",
        ("DESCC.BIN", 356): "SELECCIONA EL GOT QUE CONTE EL CRANI.",
        ("DESCC.BIN", 378): "[04][04][04][04][04]UNA\nDAGA [04][04][04][04][04][04][04][04][04][04][04][04]",
        ("DIALC.BIN", 15): "PORTA'M UN CRANI. DESPRES PODRAS PROVAR AMB EL COMERCIANT. SI EM DONES 5 EN MENYS D'UNA HORA, SERAS UN DIVO!",
        ("DIALC.BIN", 23): "AVANCA, AMIC, ES LA TEVA UNICA ESPERAN[0b]A!",
        ("DIALC.BIN", 70): "CREU-ME, AMIC, LA TEVA UNICA ESPERAN[0b]A ARA...",
        ("DIALC.BIN", 227): "EL COS PROTOZORQ ES MASSA DEBIL PER A MI. LA RA[0b]A ZORQ S'EXTINGEIX. NECESSITO UNA NOVA RA[0b]A ESCLAVA.",
        ("DIALC.BIN", 282): "OH DEILOS, CONCEDEIX-ME FOR[0b]A!",
        ("DIALC.BIN", 288): "239, 240. O 250? OH, SHLUKS, COMEN[0b]O DE NOU. 1... 2...",
        ("DIALC.BIN", 143): "TU NER? SAI FAI...\nDES BRAN XAT.",
        ("DIALC.BIN", 147): "TU NER? SAI FAI...\nREKUPE RAT. IN GUT\nESTAT.",
        ("DIALC.BIN", 155): "TU NER? SAI FAI...\nLOKA LITZAT.\nAI TA UN IN SERT.",
        ("DIALC.BIN", 156): "PRE SISI.\nAI TA UN KIT TANT.",
        ("DIALC.BIN", 157): "PORTA\nRONGA.",
        ("DIALC.BIN", 162): "LASIT UAC IOESDES\nEMBOLUPA AM NORMA LYTAT",
        ("DIALC.BIN", 164): "AKETS SON KLASI FIKATS\nDEFEK TUOSOS.\nDES BRAN XEU LOS ARA.",
        ("DIALC.BIN", 165): "NO MES SA MOR\nUNA BE GADA.",
        ("DIALC.BIN", 166): "LA SITU ACIO\nIN KLE RA.",
        ("DIALC.BIN", 8): "LA SANG\nRA JARA!",
        ("DIALC.BIN", 11): "JO SOK EL MASTEL\nDELS ESKORPIONS.\nKIN FAM KIN TINK!",
        ("DIALC.BIN", 42): "RAY VUN EN KONTAKTE\nAMB DOS NOUS!",
        ("DIALC.BIN", 80): "KE! UN TU NER\nLLARDOS!",
        ("DIALC.BIN", 100): "KREK K ESTAS EN\nUN EMBOLIK, RAY VUN.",
        ("DIALC.BIN", 180): "TU NER RAY VEN\nSOTA KONTR ATAK...",
        ("DIALC.BIN", 184): "LA XARXA TU NER\nDONA GRA SIES PER\nLOKA LITSAR LOS, RAY VEN.",
        ("DIALC.BIN", 235): "EL KRICH ES UN\nPARA SIT. NE SESSI TA...",
        ("DIALC.BIN", 238): "LA XARXA TU NER\nA DE MANAR,\nNO EL KRICH.",
        ("DIALC.BIN", 242): "LA SITU ASIO\nEBOLU SIONA NORMAL MEN.",
        ("DIALC.BIN", 294): "OLA LAVEN.\nHEH HEH. JO TAMBE!",
        ("DIALC.BIN", 309): "TU NER RAY VUN\nEN POSI SIO\nFABO RABLE.",
        ("DIALC.BIN", 311): "SITU ASIO KRITI KA\nPER AL TU NER\nRAY VUN.",
        ("DIALC.BIN", 284): "QUE\nS'HI\nPUDREIXI, AQUEST!",
        ("DIALC.BIN", 327): "SERE\nJO QUI ABORTI EL COMPLOT PROTOZORQUISTA!",
        ("DIALC.BIN", 329): "I LA XARXA TUNER VOLIA\nQUE ABANDONESSIS SCI FI ENTRE\nLES GARRES D'AQUESTES CRIATURES!",
        ("DIALC.BIN", 332): "GUANYA UN CRANI PER CADA PROVA, RESOLENT ELS MISTERIS. LLIURA ELS 5 CRANIS AL GUARDIA COMERCIANT EN MENYS D'UNA HORA I SERAS UN DIVO!",
    }
    counts = {"VERBC.BIN": 176, "DESCC.BIN": 407, "MOTSC.BIN": 131, "DIALC.BIN": 337}
    for bank, count in counts.items():
        source = next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()
        entries = dict(_entries(source))
        assert len(entries) == count
        assert dict(_entries(_from_container(bank))) == entries
    for (bank, idx), text in expected.items():
        source = next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()
        assert dict(_entries(source))[idx] == text
        assert dict(_entries(_from_container(bank)))[idx] == text
    for bank in counts:
        texts = dict(_entries(next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()))
        for old in ("CALZE", "TRAGO", "BEINA", "CANVIADOR", "TELA", "REZZO"):
            assert not any(old in text for text in texts.values()), (bank, old)
    assert "XARXA TUNER" in dict(_entries(next((ROOT / "assets" / "lang").rglob("DESCC.BIN")).read_bytes()))[404]
    verbs = dict(_entries(next((ROOT / "assets" / "lang").rglob("VERBC.BIN")).read_bytes()))
    assert "LLENCAR" not in verbs.values()
    assert "LLANCAR" not in verbs.values()
    assert verbs[60] == "ESTIRAR"
    assert verbs[81] == "LLAN[0b]AR SOBRE"
    assert verbs[164] == "TIRAR"
    cedilla = ROOT / "assets" / "lang" / "ca" / "CARACC.BIN"
    assert cedilla.exists()
    assert cedilla.read_bytes()[0x0b * 6:0x0b * 6 + 6] == bytes((0x1c, 0x10, 0x10, 0x10, 0x1c, 0x04))
    assert _from_container("CARACC.BIN")
    assert any("[0b]" in text for text in dict(_entries(_from_container("DESCC.BIN"))).values())
    coded = {8, 10, 11, 31, 36, 37, 40, 41, 42, 43, 46, 48, 74, 75, 77, 80, 81,
             100, 101, 102, 105, 119, 121, 122, 123, 124, 129, 177, 180, 184,
             188, 210, 215, 219, 220, 235, 236, 237, 238, 239, 242, 243, 244,
             245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257,
             258, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271,
             272, 273, 294, 309, 310, 311}
    dials = dict(_entries(next((ROOT / "assets" / "lang").rglob("DIALC.BIN")).read_bytes()))
    for idx in coded:
        assert not any(word in dials[idx] for word in (
            "SIT YOU", "NETWERK", "KONDISH", "DEEAKTIV", "MISHUN", "WODDA",
            "IFYOO", "DOOAZ", "STAKE", "POACH", "KREECH", "YOR ORDERZ",
            "TOO NER", "TOON"
        )), (idx, dials[idx])


def test_desc_126_passerby():
    expected = {
        "DESCS.BIN": "EL CAMINANTE\nSALVARA A ALGUIEN E INDICARA\nSU ELECCION AL REPTIL.",
        "DESCC.BIN": "EL CAMINANT\nSALVARA ALGU I INDICARA\nLA SEVA ELECCIO AL REPTIL.",
    }
    for bank, text in expected.items():
        source = next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()
        assert dict(_entries(source))[126] == text
        assert dict(_entries(_from_container(bank)))[126] == text
