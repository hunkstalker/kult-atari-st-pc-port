#!/usr/bin/env python3
"""Resume las lineas `R` de una captura KULT_MARKS (anillo de acciones).

Uso:
  python3 tools/ringreport.py kult-marcas.log
  python3 tools/ringreport.py kult-marcas.log --verbs ../kult-port-legacy/work/texts/VERBE.BIN.json
  python3 tools/ringreport.py kult-marcas.log --all

La traza la escribe platform/cpuwork.c (`platform_diag_ev`) y la emiten
lib/kult-core/src/commands.c (anillo) y lib/kult-core/src/dispatch.c / platform/dispatch_trace.c (despacho).
Solo aparece con KULT_MARKS activo. No depende de F12: recoge todos los anillos.
"""
import argparse
import json
import os
import re
import sys

KV = re.compile(r"(\w+)=([0-9a-fA-F]+)")


def load_verbs(path):
    if not path:
        for cand in (
            "../kult-port-legacy/work/texts/VERBE.BIN.json",
            "../../kult-port-legacy/work/texts/VERBE.BIN.json",
            "kult-port-legacy/work/texts/VERBE.BIN.json",
        ):
            if os.path.exists(cand):
                path = cand
                break
    if not path:
        return []
    try:
        with open(path, encoding="utf-8") as f:
            return [e.get("text", "?") for e in json.load(f)]
    except (OSError, ValueError):
        return []


def label(verbs, icon):
    if 0 <= icon < len(verbs):
        return verbs[icon].replace("\n", " ")
    return "?"


def parse(line):
    if not line.startswith("R "):
        return None
    parts = line.split()
    if len(parts) < 3:
        return None
    try:
        vbl = int(parts[1])
    except ValueError:
        return None
    ev = parts[2]
    fields = {k: int(v, 16) for k, v in KV.findall(line)}
    fields["vbl"] = vbl
    fields["ev"] = ev
    return fields


def fmt_state(f):
    return (
        "cr2=%05x abf8=%04x ace7=%02x acf2=%02x ace8=%02x ad09=%02x "
        "w19316=%04x a826=%05x sel=%06x cls=%06x"
        % (
            f.get("cr2", 0),
            f.get("abf8", 0),
            f.get("ace7", 0),
            f.get("acf2", 0),
            f.get("ace8", 0),
            f.get("ad09", 0),
            f.get("w19316", 0),
            f.get("a826", 0),
            f.get("sel", 0),
            f.get("cls", 0),
        )
    )


def main():
    ap = argparse.ArgumentParser(description="Resumen de la traza R del anillo")
    ap.add_argument("log", help="log con lineas R (KULT_MARKS)")
    ap.add_argument("--verbs", help="JSON de textos VERBE.BIN.json")
    ap.add_argument("--all", action="store_true", help="mostrar tambien eventos de despacho")
    args = ap.parse_args()

    verbs = load_verbs(args.verbs)
    rings = []          # [vbl, script, mask, entries]
    cur = None
    last_pick = None
    stats = {"opens": 0, "reopens": 0, "picks": 0, "tails": 0, "pending": 0, "suspicious": 0}
    warnings = []

    def flush_ring():
        nonlocal cur
        if cur is not None:
            rings.append(cur)
            cur = None

    with open(args.log, encoding="utf-8", errors="replace") as f:
        for raw in f:
            fld = parse(raw)
            if not fld:
                continue
            ev = fld["ev"]
            if ev == "run_boundary_in":
                flush_ring()
                print("\n=== RUN @ VBL %d  cmd=%04x  a826=%05x ===" %
                      (fld["vbl"], fld.get("b", 0), fld.get("a826", 0)))
                stats = {"opens": 0, "reopens": 0, "picks": 0, "tails": 0, "pending": 0, "suspicious": 0}
                warnings = []
                last_pick = None
                continue
            if ev == "ring_open":
                flush_ring()
                stats["opens"] += 1
                script = fld.get("a", 0)
                mask = fld.get("b", 0)
                nbits = fld.get("c", 0)
                cur = [fld["vbl"], script, mask, []]
                print("\n[VBL %d] ring_open A6=%05x mask=%02x n=%d" %
                      (fld["vbl"], script, mask, nbits))
                print("          " + fmt_state(fld))
                continue
            if ev == "ring_entry" and cur is not None:
                icon = fld.get("a", 0)
                cmd = fld.get("b", 0)
                bit = fld.get("c", 0)
                cur[3].append((bit, icon, cmd))
                print("          bit %d icon=%02x %-24s cmd=%04x nib=%01x" %
                      (bit, icon, label(verbs, icon), cmd, cmd >> 12))
                continue
            if ev == "ring_pick":
                stats["picks"] += 1
                last_pick = (fld.get("a", 0), fld.get("b", 0))
                print("          pick icon=%02x %-24s cmd=%04x" %
                      (last_pick[0], label(verbs, last_pick[0]), last_pick[1]))
                continue
            if ev == "ring_return":
                print("          ring_return A6=%05x cmd=%04x" %
                      (fld.get("b", 0), fld.get("c", 0)))
                continue
            if ev == "loop_in":
                print("          loop_in script=%05x tmpl_end=%05x" %
                      (fld.get("a", 0), fld.get("b", 0)))
                continue
            if ev in ("loop_sel", "loop_cmd", "loop_after_cmd", "loop_threshold",
                      "loop_pending", "loop_pending_tail", "loop_reopen",
                      "loop_clear_flags", "loop_exit_ffff", "loop_tail_ac"):
                if ev == "loop_reopen":
                    stats["reopens"] += 1
                if ev == "loop_tail_ac":
                    stats["tails"] += 1
                if ev == "loop_pending_tail":
                    stats["pending"] += 1
                if ev in ("loop_exit_ffff", "loop_tail_ac", "loop_pending_tail",
                          "loop_reopen", "loop_pending", "loop_clear_flags"):
                    print("          %-18s %s" % (ev, fmt_state(fld)))
                elif args.all:
                    print("          %-18s a=%05x b=%05x c=%05x" %
                          (ev, fld.get("a", 0), fld.get("b", 0), fld.get("c", 0)))
                if ev == "loop_reopen" and cur is not None:
                    if cur[3] and last_pick and (last_pick[1] >> 12) not in (0xa, 0xc):
                        stats["suspicious"] += 1
                        warnings.append(
                            "VBL %d: reapertura tras pick cmd=%04x nib=%x"
                            % (fld["vbl"], last_pick[1], last_pick[1] >> 12))
                continue
            if ev in ("run_cmd", "run_f", "run_f_ret", "walk4d", "walk4d_ret", "vm_tail",
                       "run_unwind", "run_unwind_out"):
                if args.all:
                    print("          %-18s a=%05x b=%05x c=%05x" %
                          (ev, fld.get("a", 0), fld.get("b", 0), fld.get("c", 0)))

    flush_ring()
    print("\n--- resumen ---")
    print("anillos abiertos: %d  reaperturas: %d  picks: %d  tail a/c: %d  pending: %d  sospechosas: %d"
          % (stats["opens"], stats["reopens"], stats["picks"], stats["tails"],
             stats["pending"], stats["suspicious"]))
    for w in warnings:
        print("  ! " + w)
    return 0


if __name__ == "__main__":
    sys.exit(main())
