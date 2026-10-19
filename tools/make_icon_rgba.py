#!/usr/bin/env python3
"""Genera el RGBA embebido del icono del port a partir de assets/kult_icon.ico.

Entrada:  assets/kult_icon.ico   (versionado; arte propio del proyecto)
Salida:   assets/kult_icon_rgba.h  (48x48 RGBA para SDL_SetWindowIcon)

El .ico es el icono de la app (K de Kult) con tamanos 16..256; este script saca
la entrada de 48x48 (BMP 32bpp, filas de abajo arriba, BGRA; sin mascara AND
porque el canal alfa ya trae la transparencia) y la reescribe como array C.

El .h generado se versiona: el build no necesita Python. Regenerar con:
    python3 tools/make_icon_rgba.py
"""
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ICO = os.path.join(ROOT, "assets", "kult_icon.ico")
OUT = os.path.join(ROOT, "assets", "kult_icon_rgba.h")
SIZE = 48


def read_ico_entry(path, want):
    """Devuelve (w, h, rgba) de la entrada `want`x`want` del .ico."""
    data = open(path, "rb").read()
    reserved, _type, count = struct.unpack_from("<HHH", data, 0)
    if reserved != 0 or count == 0:
        raise SystemExit(f"{path}: no parece un .ico")
    for i in range(count):
        (w, h, _colors, _r, _planes, bpp,
         size, off) = struct.unpack_from("<BBBBHHII", data, 6 + i * 16)
        w = w or 256
        h = h or 256
        if (w, h) != (want, want):
            continue
        block = data[off:off + size]
        if block[:8] == b"\x89PNG\r\n\x1a\n":
            raise SystemExit(f"{path}: la entrada {want}x{want} es PNG; "
                             "se esperaba BMP 32bpp")
        bw, bh, _planes, _bpp = struct.unpack_from("<iiHH", block, 4)
        if bh != 2 * bw:
            raise SystemExit(f"{path}: cabecera BMP inesperada {bw}x{bh}")
        pixels = block[40:40 + w * h * 4]
        if len(pixels) != w * h * 4:
            raise SystemExit(f"{path}: faltan pixeles en la entrada {want}px")
        # BMP guarda de abajo arriba y en BGRA; el array C es de arriba abajo RGBA.
        out = bytearray(w * h * 4)
        for y in range(h):
            src = (h - 1 - y) * w * 4
            dst = y * w * 4
            for x in range(w):
                b, g, r, a = pixels[src + x * 4:src + x * 4 + 4]
                out[dst + x * 4:dst + x * 4 + 4] = bytes((r, g, b, a))
        return w, h, bytes(out)
    raise SystemExit(f"{path}: no hay entrada {want}x{want}")


def main():
    w, h, rgba = read_ico_entry(ICO, SIZE)
    lines = []
    for i in range(0, len(rgba), 12):
        chunk = ", ".join(f"0x{b:02x}" for b in rgba[i:i + 12])
        lines.append("    " + chunk + ",")
    with open(OUT, "w") as f:
        f.write("/* Generado por tools/make_icon_rgba.py a partir de\n"
                " * assets/kult_icon.ico (K de Kult, 48x48 RGBA): NO editar a mano. */\n")
        f.write("#ifndef KULT_ICON_RGBA_H_INCLUDED\n#define KULT_ICON_RGBA_H_INCLUDED\n\n")
        f.write(f"#define KULT_ICON_W {w}\n#define KULT_ICON_H {h}\n\n")
        f.write(f"static const unsigned char kult_icon_rgba[{len(rgba)}] = {{\n")
        f.write("\n".join(lines))
        f.write("\n};\n\n#endif /* KULT_ICON_RGBA_H_INCLUDED */\n")
    print(f"{OUT}: {w}x{h}, {len(rgba)} bytes")


if __name__ == "__main__":
    sys.exit(main())
