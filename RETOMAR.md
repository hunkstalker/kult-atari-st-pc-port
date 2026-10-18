# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 6 — M3/T-030 (ventana SDL3)**: `platform/video_sdl.c` (ventana,
  `SDL_Renderer`, textura RGBA del framebuffer planar, `SDL_Renderer` con
  escalado entero y vsync, ESC para salir), `platform_fb_to_rgba`,
  `kult_set_frame_cb`/`kult_request_stop` y `make port-sdl` → `build/kult-sdl`.
  SDL3 (`sdl3-dev`) instalado. Probado con `SDL_VIDEODRIVER=dummy`.
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
- **M0, M1 y M2 hechas** (T-021 tick, T-024 loader corregido).
- **El juego arranca y avanza frames**: headless (`make port`) y con **ventana
  SDL3** (`make port-sdl` + `KULT_WINDOW=1`). Sin bloqueantes conocidos.
- `make test` 499 verde; `make test-port` verde.

## Siguiente tarea
**M3/T-031 — entrada real** (teclado → `os_crawio/cnecin`; ratón → paquete IKBD y
`A_CURSOR_X/Y`), y **T-032 — ritmo** (simulación a cadencia fija, presentación al
refresco). Después, T-020 (menú fiel) y M4 (audio). Ver `docs/PLAN.md`.

## Comandos
- Core/diferencial: `make test` (499), `make test PYTEST_ARGS=-n0`,
  `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (→ `build/kult`, headless), `make port-sdl` (→ `build/kult-sdl`,
  ventana SDL3), `make test-port` (HAL, sin oráculo).
- Ejecutar headless: `KULT_DATA_DIR=/tmp/kult_data ./build/kult`
  (junta `disk_a`+`disk_b` del usuario en `/tmp/kult_data`).
- Con ventana: `KULT_WINDOW=1 KULT_DATA_DIR=/tmp/kult_data ./build/kult-sdl`
  (`KULT_SCALE=3`, ESC para salir). Sin display: `SDL_VIDEODRIVER=dummy`.

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
