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
        ("VERBC.BIN", 39): "OMPLIR GOT",
        ("VERBC.BIN", 104): "DAGA",
        ("VERBC.BIN", 117): "HIPERVISIO",
        ("VERBC.BIN", 115): "FESOL",
        ("VERBC.BIN", 155): "DONAR\nEL FESOL PER MENJAR",
        ("MOTSC.BIN", 51): "ASH",
        ("MOTSC.BIN", 124): "COMERCIANT",
        ("MOTSC.BIN", 92): "DESGUAS",
        ("MOTSC.BIN", 129): "SCI FI",
        ("DESCC.BIN", 50): "QUEDES ATRAPAT A LA\nTERANYINA!",
        ("DESCC.BIN", 73): "UN CURIOS HOMENET AL REVES.",
        ("DESCC.BIN", 126): "EL CAMINANT\nSALVARA ALGU I INDICARA\nLA SEVA ELECCIO AL REPTIL.",
        ("DESCC.BIN", 261): "MENTRE AVANCES, ACTIVES UN MECANISME HORRIPILANT!\nEL SOSTRE ET CAU A SOBRE!",
        ("DESCC.BIN", 339): "UN GOT DE METALL.",
        ("DESCC.BIN", 350): "QUEDA 1 GLOP.",
        ("DESCC.BIN", 356): "SELECCIONA EL GOT QUE CONTE EL CRANI.",
        ("DESCC.BIN", 378): "[04][04][04][04][04]UNA\nDAGA [04][04][04][04][04][04][04][04][04][04][04][04]",
        ("DIALC.BIN", 15): "PORTA'M UN CRANI. DESPRES PODRAS PROVAR AMB EL COMERCIANT. SI EM DONES 5 EN MENYS D'UNA HORA, SERAS UN DIVO!",
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


def test_desc_126_passerby():
    expected = {
        "DESCS.BIN": "EL CAMINANTE\nSALVARA A ALGUIEN E INDICARA\nSU ELECCION AL REPTIL.",
        "DESCC.BIN": "EL CAMINANT\nSALVARA ALGU I INDICARA\nLA SEVA ELECCIO AL REPTIL.",
    }
    for bank, text in expected.items():
        source = next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()
        assert dict(_entries(source))[126] == text
        assert dict(_entries(_from_container(bank)))[126] == text
