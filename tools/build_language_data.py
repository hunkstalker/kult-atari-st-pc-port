#!/usr/bin/env python3
"""Empaqueta assets/lang/ en language.data (KULTDTA1 tipo LANG)."""
import argparse, struct
from pathlib import Path

MAGIC=b"KULTDTA1"
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--root", default="assets/lang")
    ap.add_argument("--out", default="assets/language.data")
    a=ap.parse_args()
    root=Path(a.root)
    files=[]
    for p in sorted(root.rglob("*")):
        if p.is_file() and p.suffix.lower() in (".bin",".pi1"):
            files.append((p.name.upper(),p))
    seen=set()
    for n,_ in files:
        if n in seen: raise SystemExit(f"nombre duplicado: {n}")
        seen.add(n)
    blobs=[(n,p.read_bytes()) for n,p in files]
    off=20+72*len(blobs)
    hdr=MAGIC+struct.pack("<III",1,2,len(blobs))
    idx=bytearray(); body=bytearray()
    for n,data in blobs:
        nb=n.encode("ascii","replace")[:63]; nb+=b"\0"*(64-len(nb))
        idx+=nb+struct.pack("<II",off,len(data)); body+=data; off+=len(data)
    Path(a.out).write_bytes(hdr+bytes(idx)+bytes(body))
    print(f"{a.out}: {len(blobs)} entradas, {20+len(idx)+len(body)} bytes")
    for n,_ in files: print(" ",n)
if __name__=="__main__": main()
