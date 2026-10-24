#!/usr/bin/env python3
"""Build a language.data bundle (KULTDTA1 type LANG).

Ejemplo español:
  python3 tools/make_language.py --code S \
      --drap assets/lang/DRAP.PI1 \
      --lang-dir assets/lang/es \
      --out data/language.data

Incluye DRAP.PI1 + VERBx/MOTSx/DESCx/DIALx + CARACx.BIN (si existe).
"""
import argparse
import struct
from pathlib import Path

MAGIC = b"KULTDTA1"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--code", required=True, choices=["S", "C"])
    ap.add_argument("--drap", required=True)
    ap.add_argument("--lang-dir", required=True)
    ap.add_argument("--out", required=True)
    a = ap.parse_args()
    code = a.code
    files = [("DRAP.PI1", Path(a.drap))]
    for base in ("VERB", "MOTS", "DESC", "DIAL", "CARAC"):
        p = Path(a.lang_dir) / f"{base}{code}.BIN"
        if p.exists():
            files.append((p.name, p))
    if not any(n.startswith("VERB") for n, _ in files):
        raise SystemExit(f"no hay VERB{code}.BIN en {a.lang_dir}")
    blobs = [(n, p.read_bytes()) for n, p in files]
    names = [n for n, _ in blobs]
    off = 20 + 72 * len(blobs)
    hdr = MAGIC + struct.pack("<III", 1, 2, len(blobs))
    idx = bytearray()
    body = bytearray()
    for n, data in blobs:
        nb = n.encode("ascii", "replace")[:63]
        nb = nb + b"\0" * (64 - len(nb))
        idx += nb + struct.pack("<II", off, len(data))
        body += data
        off += len(data)
    Path(a.out).write_bytes(hdr + bytes(idx) + bytes(body))
    print(f"{a.out}: {len(blobs)} entradas ({len(hdr)+len(idx)+len(body)} bytes)")

if __name__ == "__main__":
    main()
