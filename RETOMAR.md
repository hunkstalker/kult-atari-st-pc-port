# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 4 — M2/T-021 (tick de VBL)**: portado `0x160c2` a `boot/vbl.c`
  (contador, rotación de paleta, strip GAUSS cada 64 VBLs y blit enmascarado
  MURSM/TEMPL en VBLs impares), conectado con `platform_set_tick` y con prueba en
  `make test-port`. **No desbloquea el bucle** (hipótesis falsada).
- **Sesión 3 — M1 (HAL headless)**: `platform/` + `boot/` (loader + `_start`) +
  `app/`; `make test-port` verde y `build/kult` vuelca la sala cargada.
- **Sesión 2 — M0**: core importado; `make test` **499 verde**.

## Estado actual
- **M0 y M1 hechas** (HAL, arranque provisional, driver headless).
- **Bloqueo del bucle principal (observado, sin resolver)**: `run_command(0xc001)`
  de `_start` revienta en `elem_command`: el guion salta (por `script_expr` a
  través de `script_jump_zero`) a `TEMPL+0x6a20`, que pide el elem idx 73 → glifo
  17, **fuera de la tabla `0x1a252`** (que solo tiene ~8-10 entradas). Es una
  divergencia de **estado** (o del evaluador de expresión) con el original, no del
  tick: el tick T-021 ya está puesto y no lo evita. El driver M1 se detiene **antes**
  de 0xc001 por defecto; `KULT_RUN_LOOP=1` reproduce el fallo.
- Sin otros bloqueantes.

## Siguiente tarea
**T-024 — diagnosticar el bloqueo de `0xc001` con el oráculo**: correr el original
(Musashi, `tools/recreate_kit`) y comparar, en el dispatch de `0xc001`, la imagen
(tabla `0x1a252`, operandos de `script_arg` en `0x1a752`, contadores `ACD4/AB60/ACD7/
1A579`) contra la nuestra. Alternativa: instrumentar `script_expr`/`script_arg`.
Después, T-020 (validar `_start`, menú fiel) y M3 (SDL).

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
- `boot/prg.c` (loader del PRG) + `boot/boot.c` (`_start` provisional, corte por
  `frames` con `hook_loop_top` + `longjmp`).
- `app/main.c`: `KULT_PRG`, `KULT_DATA_DIR` (admite varios con `:`),
  `KULT_FRAMES`, `KULT_DUMP`, `KULT_PLANAR`, `KULT_KEYS`/`KULT_KEY`,
  `KULT_SKIP_DRAP`, `KULT_RUN_LOOP`, `KULT_DEBUG`.
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
