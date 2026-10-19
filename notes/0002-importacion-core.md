# Nota 0002 — Importación del core y cierre de M0

- **Sesión 2.** Se importa el core verificado y se deja el diferencial verde en
  este repo (T-001/T-002/T-003). **M0 cerrada.**

## Qué se hizo
- Copiados **verbatim** de `/app/kult-atari-st-c`: `src/`, `include/`, `test/`,
  `tools/` (incluye el kit `tools/recreate_kit/` y su oráculo Musashi prebuilt,
  gitignored).
- Copiados además `Makefile`, `project.toml`, `requirements.txt`,
  `kult_core.map` y `work/names.txt` (el Makefile hace
  `include tools/recreate_kit/kit.mk`).
- `work/disk_a/KULT.PRG` (gitignored) copiado de
  `work/disk_a/KULT.PRG` del proyecto de referencia.
- `.venv` enlazado al proyecto de referencia.
- `docs/mapa/` regenerado con `make mapa`: `test/test_mapa.py` exige que las
  fichas estén sincronizadas con `src/`+`names.txt`+`test/`.
- `.gitignore`: `.venv` (sin barra final, para enlazar también un symlink) y
  `.pytest_cache/`.

## Verificación
- `make test` → **499 passed**.
- `make test PYTEST_ARGS=-n0` → **499 passed**.
- `make regscan` → `ids_reg=0 goto=95 lineas=12472`.
- `make core` → `build/libkultcore.so` OK.

## Trampas encontradas
- No copiar `docs/` del core (colisiona con la doc de la compilación); sí hay
  que **generar** `docs/mapa/` aquí porque la suite lo compara.
- `src/` queda **intacto**: la importación no cambia el juego.

## Siguiente
- **M1 — HAL mínima headless** (T-010…T-017), ver `docs/PLAN.md`.
