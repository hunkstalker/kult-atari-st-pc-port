# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 5 — T-024 (causa raíz)**: **el loader C `boot/prg.c` aplicaba una
  relocación de más tras cada `RELOC_SKIP`** (byte 1 del stream DRI), corrompiendo
  544 bytes de la imagen (p. ej. un largo de la tabla ICONE). Eso desviaba el
  guion de `_start` (`0xc001`) y reventaba. **Corregido**: la imagen del port
  coincide byte a byte con el loader del oráculo (`loader.py`). El port **ejecuta
  el arranque y `game_loop`** (probado 1200+ frames). Prueba de regresión del
  loader en `make test-port`.
- **Sesión 4 — M2/T-021**: tick de VBL `0x160c2` en `boot/vbl.c`.
- **Sesión 3 — M1**: `platform/` + `boot/` + `app/`; HAL headless.
- **Sesión 2 — M0**: core importado; `make test` **499 verde**.

## Estado actual
- **M0 y M1 hechas**; T-021 (tick) y T-024 (loader) resueltas.
- **El juego arranca y avanza frames en headless** (`make port` →
  `KULT_FRAMES=N ./build/kult` → PPM coherente). Sin bloqueantes conocidos.
- `make test` 499 verde; `make test-port` verde.

## Siguiente tarea
**T-020 — validar/cerrar `_start`** (menú de idioma fiel con espera de tecla, en
vez del auto-tecla headless) y, después, **M3**: primera ventana SDL
(`SDL_Renderer`, planar→RGBA, entrada real). Ver `docs/PLAN.md`.

## Comandos
- Core/diferencial: `make test` (499), `make test PYTEST_ARGS=-n0`,
  `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (→ `build/kult`), `make test-port` (HAL + tick, sin oráculo).
- Ejecutar headless: `KULT_DATA_DIR=/tmp/kult_data ./build/kult`
  (junta `disk_a`+`disk_b` del usuario en `/tmp/kult_data`).

## M1: qué hay y cómo se prueba
- `platform/include/{os,hw,psg,sched}.h` **sombrean** al kit en el build del
  ejecutable: el core incluye los mismos nombres, pero responden la HAL de PC.
  `platform/{fs,video,input,time,hw,psg,os,audio}.c` los implementan.
- `boot/prg.c` (loader GEMDOS: cabecera + relocación DRI, espejo de `loader.py`) +
  `boot/boot.c` (`_start` provisional, corte por `frames` con `hook_loop_top` +
  `longjmp`) + `boot/vbl.c` (tick 0x160c2).
- `app/main.c`: `KULT_PRG`, `KULT_DATA_DIR` (admite varios con `:`),
  `KULT_FRAMES`, `KULT_DUMP`, `KULT_PLANAR`, `KULT_KEYS`/`KULT_KEY`,
  `KULT_SKIP_DRAP`, `KULT_DEBUG`.
- Datos de prueba: **hay que juntar** `disk_a`+`disk_b` (el juego necesita
  además `FOND.PI1`, `PUZZL.BIN`, `ECHE*.BIN`… que están en `disk_b`). Los
  recursos "empotrados" (CARAC/TEMPL/ICONE/ARPLA…) van **dentro del PRG**.

## Decisiones cerradas (no re-litigar; ver `docs/adr/`)
- **SDL3** para ventana/entrada/audio/GPU.
- **`SDL_Renderer`** para lo básico; **`SDL_GPU` + shaders** solo para CRT.
- **Audio** en hito propio (no al principio).
- **Menú de idioma**: mantener el fiel (espera de tecla) al inicio.
- **`boot/`** fuera de `src/` (no se toca el core verificado).
- **Headless primero**, luego ventana SDL.

## Trampas / notas
- `src/` (el juego) **no se modifica**; todo lo de la máquina va en `platform/`.
- El arranque `_start` (0x10000) y el tick `0x160c2` **no existen en C** en el
  core; se reconstruyen aquí.
- El juego avanza **1 paso por frame**; separar simulación (ritmo del original)
  de presentación (refresh) para no acelerar animaciones.
- No tocar `/app/kult-port-legacy` ni `/app/kult-atari-st-decomp`.
