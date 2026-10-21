"""Prueba de la herramienta `tools/ringreport.py` con una traza R sintetica."""
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOOL = ROOT / "tools" / "ringreport.py"


def _r(vbl, ev, a=0, b=0, c=0, cr2=0, abf8=0, ace7=0, acf2=0,
       ace8=0, ad09=0, w19316=0x2F3C, a826=0, sel=0x1B5F0, cls=0x1AA81):
    return (
        f"R {vbl} {ev} a={a:05x} b={b:05x} c={c:05x} cr2={cr2:05x} "
        f"abf8={abf8:04x} ace7={ace7:02x} acf2={acf2:02x} ace8={ace8:02x} "
        f"ad09={ad09:02x} w19316={w19316:04x} a826={a826:05x} "
        f"sel={sel:06x} cls={cls:06x}"
    )


def test_ringreport_detecta_reapertura_sospechosa(tmp_path):
    verbs = [{"text": f"V{i:02x}"} for i in range(0x100)]
    verbs[0x1A]["text"] = "CONTEMPLATE"
    verbs[0x82]["text"] = "CRAWL"
    verbs[0x83]["text"] = "RED SPIDER"
    verbs[0x94]["text"] = "LIE DOWN"
    verbs_path = tmp_path / "verbs.json"
    verbs_path.write_text(json.dumps(verbs), encoding="utf-8")

    lines = [
        _r(100, "run_boundary_in", b=0xC1CE, cr2=0x21F1A),
        _r(117, "ring_open", a=0x21F1A, b=0xB8, c=4, cr2=0x21F1A),
        _r(117, "ring_entry", a=0x1A, b=0xC1C2, c=7, cr2=0x21F1A),
        _r(117, "ring_entry", a=0x82, b=0xC1C3, c=5, cr2=0x21F1A),
        _r(117, "ring_entry", a=0x83, b=0xC1C4, c=4, cr2=0x21F1A),
        _r(117, "ring_entry", a=0x94, b=0xE1FC, c=3, cr2=0x21F1A),
        _r(119, "ring_pick", a=0x94, b=0xE1FC, cr2=0x21F1A),
        _r(121, "loop_pending", a=0x21F28, b=0xC357, cr2=0x21F1A,
           abf8=0xC357, ace7=2, acf2=1),
        _r(121, "loop_reopen", a=0x21F1A, b=1, cr2=0x21F1A,
           abf8=0xC357, ace7=2, acf2=1, ace8=1),
        _r(121, "ring_open", a=0x21F1A, b=0xB8, c=4, cr2=0x21F1A,
           abf8=0xC357, ace7=2, acf2=1, ace8=1),
        _r(130, "loop_pending_tail", a=0x21F28, b=0xC357, cr2=0x21F1A,
           abf8=0xC357, ace7=2, acf2=1, ace8=1),
    ]
    log = tmp_path / "ring.log"
    log.write_text("\n".join(lines) + "\n", encoding="utf-8")

    out = subprocess.run(
        [sys.executable, str(TOOL), str(log), "--verbs", str(verbs_path)],
        capture_output=True, text=True, check=True,
    ).stdout

    assert "LIE DOWN" in out
    assert "anillos abiertos: 2" in out
    assert "reaperturas: 1" in out
    assert "picks: 1" in out
    assert "pending: 1" in out
    assert "sospechosas: 1" in out
    assert "reapertura tras pick cmd=e1fc" in out
