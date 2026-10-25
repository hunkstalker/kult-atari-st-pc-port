#!/usr/bin/env python3
"""Genera assets/kult.icns desde assets/kult_icon.ico (PNG inside ICNS)."""
from pathlib import Path
from PIL import Image
import struct

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "assets" / "kult_icon.ico"
OUT = ROOT / "assets" / "kult.icns"
# tipo ICNS -> lado en pixeles
SIZES = {
    "icp4": 16, "icp5": 32, "icp6": 64,
    "ic07": 128, "ic08": 256, "ic09": 512, "ic10": 1024,
}

def main():
    image = Image.open(SRC).convert("RGBA")
    chunks = []
    for kind, side in SIZES.items():
        img = image.resize((side, side), Image.LANCZOS)
        from io import BytesIO
        buf = BytesIO()
        img.save(buf, format="PNG")
        data = buf.getvalue()
        chunks.append(kind.encode("ascii") + struct.pack(">I", len(data) + 8) + data)
    body = b"".join(chunks)
    OUT.write_bytes(b"icns" + struct.pack(">I", len(body) + 8) + body)
    print(f"OK: {OUT} ({OUT.stat().st_size} bytes)")

if __name__ == "__main__":
    main()
