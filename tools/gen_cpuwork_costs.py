#!/usr/bin/env python3
"""Genera platform/cpuwork_costs.inc: coste de CPU medido por función (T-073b).

Entrada:
  - tools/cpuwork/funcost_st_reference.txt: "<addr_hex> <ciclos_totales> <calls>"
    medido del oráculo (Musashi) por el predecesor sobre el binario original.
  - lib/kult-core/docs/mapa/*.md: tabla `| 0xADDR | nombre | ...` (dirección ST ↔ símbolo C).
  - símbolos globales del build del port (`nm -g --defined-only <lib>`), para no
    referenciar funciones que no existen o no son enlazables.

Salida: tabla `{ (void *)nombre, 0xADDR, ciclos_por_llamada }` que usa
platform/cpuwork.c con `-finstrument-functions`.

Exclusiones (igual que el predecesor):
  - 0x13594/0x13698: esperas de VBL/frame (su tiempo lo pone el reloj de VBL).
  - 0x15da6/0x12e2e/0x12f28: sus `dbf` los cobra `cpu_delay` (evita doble).
  - audio 0x19xxx/0x3axxx: lo mueve el reloj de audio.

Uso:
  python3 tools/gen_cpuwork_costs.py [--nm build-cmake/libkultport.a] [-o ...]
"""
import argparse
import glob
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REF = os.path.join(ROOT, "tools", "cpuwork", "funcost_st_reference.txt")
OUT = os.path.join(ROOT, "platform", "cpuwork_costs.inc")
MAP = os.path.join(ROOT, "lib", "kult-core", "docs", "mapa", "*.md")

# Se cobran por el reloj de VBL, por cpu_delay o por audio (no aquí).
EXCLUDE = {0x13594, 0x13698, 0x15DA6, 0x12E2E, 0x12F28}


def map_names():
    m = {}
    for f in sorted(glob.glob(MAP)):
        for line in open(f, errors="ignore"):
            r = re.match(r"\|\s*`?(0x[0-9a-fA-F]+)`?\s*\|\s*`?([A-Za-z_][A-Za-z0-9_]*)`?\s*\|", line)
            if r:
                m.setdefault(int(r.group(1), 16), r.group(2))
    return m


def nm_symbols(path):
    out = subprocess.check_output(["nm", "-g", "--defined-only", path], text=True)
    syms = set()
    for line in out.splitlines():
        p = line.split()
        if len(p) >= 3 and p[-2] == "T":      # texto global
            syms.add(p[-1])
    return syms


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--nm", default=os.path.join(ROOT, "build-cmake", "libkultport.a"))
    ap.add_argument("-o", "--out", default=OUT)
    args = ap.parse_args()

    if not os.path.exists(args.nm):
        sys.exit(f"falta {args.nm}; compila antes (`make cmake-native`) para tener símbolos")
    syms = nm_symbols(args.nm)
    names = map_names()

    rows = []
    for line in open(REF):
        p = line.split()
        if len(p) != 3:
            continue
        addr, units, calls = int(p[0], 16), int(p[1]), int(p[2])
        if calls <= 0 or units <= 0 or addr in EXCLUDE:
            continue
        if 0x19000 <= addr < 0x19200 or 0x3A000 <= addr < 0x3B000:
            continue
        name = names.get(addr)
        if not name or name not in syms:
            continue
        rows.append((addr, name, units // calls))

    rows.sort()
    # Tabla de marcas: TODAS las funciones globales con direccion en lib/kult-core/docs/mapa
    # (aunque no tengan coste). Permite que la ventana F12 trace todo el core.
    marks = sorted((a, n) for a, n in names.items() if n in syms)
    lines = [
        "/* GENERADO por tools/gen_cpuwork_costs.py — NO editar a mano.",
        " * Coste = ciclos del 68000 por llamada, medidos del oráculo (referencia",
        " * del predecesor) y mapeados a símbolos C del port. Excluidas esperas,",
        " * retardos dbf ya cobrados por cpu_delay y audio. */",
    ]
    externs = sorted({n for _a, n, _c in rows} | {n for _a, n in marks})
    for n in externs:
        lines.append(f"extern void {n}(void);")
    lines.append("")
    lines.append("static const cpuwork_sym k_cpuwork_costs[] = {")
    for a, n, c in rows:
        lines.append(f"  {{ (void *){n}, 0x{a:x}, {c} }},")
    lines.append("};")
    lines.append(f"static const unsigned k_cpuwork_ncosts = {len(rows)};")
    lines.append("")
    lines.append("/* Marcas (F12): todas las funciones mapeadas, coste 0. */")
    lines.append("static const cpuwork_sym k_cpuwork_marks[] = {")
    for a, n in marks:
        lines.append(f"  {{ (void *){n}, 0x{a:x}, 0 }},")
    lines.append("};")
    lines.append(f"static const unsigned k_cpuwork_nmarks = {len(marks)};")
    open(args.out, "w").write("\n".join(lines) + "\n")
    print(f"{args.out}: {len(rows)} con coste, {len(marks)} para marcas")


if __name__ == "__main__":
    main()
