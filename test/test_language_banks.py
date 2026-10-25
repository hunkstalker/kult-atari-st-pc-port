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


def test_desc_126_passerby():
    expected = {
        "DESCS.BIN": "EL CAMINANTE SALVA A ALGUNOS E INDICA SU ELECCION AL REPTIL.",
        "DESCC.BIN": "EL CAMINANT SALVA ALGUNS I INDICA LA SEVA ELECCIO AL REPTIL.",
    }
    for bank, text in expected.items():
        source = next((ROOT / "assets" / "lang").rglob(bank)).read_bytes()
        assert dict(_entries(source))[126] == text
        assert dict(_entries(_from_container(bank)))[126] == text
