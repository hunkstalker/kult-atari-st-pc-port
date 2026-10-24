# Nota 0001 — Inicio del proyecto

- **Sesión 1.** Se crea el repositorio `kult-atari-st-pc-port` y **toda la
  documentación inicial**.

## Qué se decidió
- Objetivo: port nativo a PC (Windows/Linux) que une **Atari ST + Amiga**, con
  **todos los idiomas**, partiendo del **C verificado** de `kult-atari-st-c`.
- El juego usa **19 servicios**; el porte sustituye al TOS/XBIOS/BIOS y al
  hardware por un backend de PC (**"nuestro TOS"**).
- Stack: **SDL3** (`SDL_Renderer` base, `SDL_GPU` para CRT).
- Fases M0–M7 + workstream Amiga (ver `docs/PLAN.md`).

## Por qué así
- El core ya es C portable y verificado (415 funciones, 499 tests): **no se
  reescribe**; se le da una capa de máquina nueva.
- Faltan en C: `_start` (arranque + menú de idioma) y el tick VBL `0x160c2`.
- El render es planar→RGBA; el audio exige emular MFP Timer A + PSG.

## Siguiente
- **T-001**: importar el core (`src/include/test/tools`) y dejar `make test` en
  499 verde (T-002). Ver `RETOMAR.md`.
