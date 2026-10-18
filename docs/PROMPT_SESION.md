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

**Estado:** **M0–M3 hechas.** El juego arranca y **es jugable en ventana SDL3**
(render + entrada real + ritmo 50 Hz) y corre headless. `make test` **499 verde**,
`make test-port` verde. Sin bloqueantes.

**Tarea inmediata: M4 — audio** (hito propio, ADR 0003):
- **T-040** — modelo del **MFP Timer A** (`$fffa13/07/19/1f`) que dispare las ISR
  C ya reconstruidas (`0x1910e` PCM; vector `$134`) y los traps `#3/#4` de música;
- **T-041** — emulación **PSG/YM2149** desde `psg_port_write/read` (hoy guarda los
  16 registros en `platform/psg.c`; falta el chip y el sonido);
- **T-042** — salida por **SDL audio** (`platform/audio.c`).
- **Verificación objetiva** (docs/VERIFICACION.md): comparar el **stream de
  registros PSG con el oráculo** (ledger del kit) y comprobar **RMS/energía** del
  PCM (que no sea silencio). La **escucha final** ("suena bien / sin petardeo") la
  hace el usuario.

**Reglas:**
- **`src/` (el juego) no se modifica**; lo específico de máquina va en
  `platform/`, `boot/` y `app/`.
- **Decisiones cerradas como ADR** (no re-litigar; ver `docs/adr/`): SDL3;
  `SDL_Renderer` base y `SDL_GPU` para CRT; **audio en hito propio**; menú de
  idioma fiel; `boot/` fuera de `src/`; headless primero.
- Reproducir/medir antes de tocar; una hipótesis un cambio; toda corrección deja
  prueba.
- **No tocar** `/app/kult-port-legacy` ni `/app/kult-atari-st-decomp`.
- Todo en **español**. Mantén `TODO.md` y `RETOMAR.md` al día al cerrar.

**Comandos:**
- Core/diferencial: `make test` (499), `make test PYTEST_ARGS=-n0`,
  `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (headless → `build/kult`), `make port-sdl` (ventana →
  `build/kult-sdl`), `make test-port` (HAL sin oráculo).
- Ejecutar: `KULT_DATA_DIR=/tmp/kult_data ./build/kult` (headless) o
  `KULT_WINDOW=1 KULT_DATA_DIR=/tmp/kult_data ./build/kult-sdl`.
  El directorio de datos junta `disk_a`+`disk_b` del usuario (p. ej.
  `/tmp/kult_data`); el oráculo necesita `work/disk_a/KULT.PRG` (gitignored).
- SDL3 ya instalado (`apk add sdl3-dev`; pkg-config `sdl3` 3.4.10).
