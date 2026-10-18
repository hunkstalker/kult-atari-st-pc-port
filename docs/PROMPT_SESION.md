# PROMPT de arranque (copiar y pegar en una sesión nueva)

---

Trabajamos en el **port nativo a PC** de **Kult: The Temple of Flying Saucers**
(Exxos/ERE, 1989) que **une las versiones Atari ST y Amiga**, con todos los
idiomas, en **`/app/kult-port-pc-compilation`**.

Contexto: el juego **no se reescribe**. Parte del **C verificado** de
`/app/kult-atari-st-c` (415 funciones, 499 pruebas diferenciales) y se le añade un
**backend de PC con SDL3**. El juego solo depende de **19 servicios**; el port
implementa esos servicios (la HAL o "nuestro TOS").

**Antes de tocar nada, lee por este orden:**
1. `RETOMAR.md` — estado exacto, siguiente tarea, comandos y trampas.
2. `PROYECTO.md` — contexto maestro, arquitectura, fases y estado.
3. `TODO.md` — tareas por fase.
4. `docs/INDICE.md` y el doc del área (`PLAN`, `ARQUITECTURA`, `PORTING`,
   `VERIFICACION`, `BUILD`, `adr/`).

**Estado:** fase **M0**. La documentación está hecha; **no hay código aún**.

**Tarea inmediata:**
- **T-001** — importar el core desde `/app/kult-atari-st-c`: copiar `src/`,
  `include/`, `test/`, `tools/`.
- **T-002** — dejar `make test` en **499 verde** en este repo (kit diferencial;
  `project.toml` y `work/disk_a/KULT.PRG` para el oráculo).
- Después, M1 (HAL mínima headless) según `docs/PLAN.md`.

**Reglas:**
- **`src/` (el juego) no se modifica**; lo específico de máquina va en
  `platform/`, `boot/` y `app/`.
- **Decisiones cerradas como ADR** (no re-litigar; ver `docs/adr/`): SDL3;
  `SDL_Renderer` base y `SDL_GPU` para CRT; audio en hito propio; menú de idioma
  fiel; `boot/` fuera de `src/`; headless primero.
- Reproducir/medir antes de tocar; una hipótesis un cambio; toda corrección deja
  prueba.
- **No tocar** `/app/kult-port-legacy` ni `/app/kult-atari-st-decomp`.
- Todo en **español**. Mantén `TODO.md` y `RETOMAR.md` al día al cerrar.

**Comandos** (tras importar el core): `make test` (499), `make test
PYTEST_ARGS=-n0`, `make regscan` (`ids_reg=0`), `make core`. El oráculo necesita
`work/disk_a/KULT.PRG` (gitignored; si falta, cópialo de
`/app/kult-port-legacy/work/disk_a/KULT.PRG`).
