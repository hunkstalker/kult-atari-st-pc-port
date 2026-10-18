# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Fecha/estado:** sesión 1 — creado el repositorio y **toda la documentación
  inicial** (fases, decisiones/ADRs, frontera de port, verificación).
- **No hay código todavía**: no se ha importado el C del core.

## Estado actual
- Fase **M0**. Documentación lista; el siguiente paso es **importar el core**.
- Sin bloqueantes.

## Siguiente tarea
**T-001 — Importar el core** desde `/app/kult-atari-st-c`:
```
cp -r /app/kult-atari-st-c/src      /app/kult-port-pc-compilation/
cp -r /app/kult-atari-st-c/include  /app/kult-port-pc-compilation/
cp -r /app/kult-atari-st-c/test     /app/kult-port-pc-compilation/
cp -r /app/kult-atari-st-c/tools    /app/kult-port-pc-compilation/
# project.toml, Makefile y work/disk_a/KULT.PRG (gitignored) según T-002/T-003
```
Después, T-002: dejar `make test` en **499 verde** en este repo (kit diferencial).

## Cómo verificar (tras T-002)
`make test` (499), `make test PYTEST_ARGS=-n0`, `make regscan` (`ids_reg=0`),
`make core`. El aráculo necesita `work/disk_a/KULT.PRG` (copia del usuario;
gitignored).

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
