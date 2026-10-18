# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 3 — M1 (HAL headless)**: implementada `platform/` (los 19 servicios),
  `boot/` (carga del PRG + `_start` provisional) y `app/` (driver headless).
  **`make test-port` verde**; **`make port`** genera `build/kult`, que arranca el
  juego y vuelca la sala cargada (framebuffer coherente, 14 colores).
- **Sesión 2 — M0**: importado el core; `make test` **499 verde**.
- **Sesión 1**: repositorio + documentación inicial.

## Estado actual
- **M0 hecha**. **M1 hecha** (T-010…T-017): HAL headless + arranque + volcado.
- **Bloqueo del bucle principal (observado)**: `run_command(0xc001)` de `_start`
  revienta en `elem_command` con un índice de glifo inválido (escribe fuera de la
  imagen). **Hipótesis a confirmar en T-021**: falta el trabajo por frame del tick
  de VBL `0x160c2` (rotación de paleta + blits MURSM/TEMPL) que el original sí
  corre entre medias. El driver M1 se detiene **antes** de 0xc001 por defecto
  (vuelca la sala cargada). Con `KULT_RUN_LOOP=1` se reproduce el fallo.
- Sin otros bloqueantes.

## Siguiente tarea
**M2 — T-021: portar el tick `0x160c2`** (callea `0x16210`=core, `0x10768`=rts,
`0x1618a` masked blit MURSM/TEMPL y la rotación de paleta `$ff8258..`). Con él,
`kult_boot` debe poder seguir a 0xc001 y al `game_loop`. Después, T-020 (validar
`_start`, menú fiel) y M3 (SDL).

## M1: qué hay y cómo se prueba
- `platform/include/{os,hw,psg,sched}.h` **sombrean** al kit en el build del
  ejecutable: el core incluye los mismos nombres, pero responden la HAL de PC.
  `platform/{fs,video,input,time,hw,psg,os,audio}.c` los implementan.
- `boot/prg.c` (loader del PRG) + `boot/boot.c` (`_start` provisional, corte por
  `frames` con `hook_loop_top` + `longjmp`).
- `app/main.c`: `KULT_PRG`, `KULT_DATA_DIR` (admite varios con `:`),
  `KULT_FRAMES`, `KULT_DUMP`, `KULT_PLANAR`, `KULT_KEYS`/`KULT_KEY`,
  `KULT_SKIP_DRAP`, `KULT_RUN_LOOP`, `KULT_DEBUG`.
- Datos de prueba: **hay que juntar** `disk_a`+`disk_b` (el juego necesita
  además `FOND.PI1`, `PUZZL.BIN`, `ECHE*.BIN`… que están en `disk_b`). Los
  recursos "empotrados" (CARAC/TEMPL/ICONE/ARPLA…) van **dentro del PRG**.

## Comandos
- Core/diferencial: `make test` (499), `make test PYTEST_ARGS=-n0`,
  `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (→ `build/kult`), `make test-port` (HAL sin oráculo).
- Ejecutar headless (ejemplo con datos de prueba):
  `KULT_DATA_DIR=/tmp/kult_data ./build/kult`
  (crea `/tmp/kult_data` copiando `disk_a`+`disk_b` del usuario).

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
