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

**Estado:** **M0–M4 hechas**, **M6 en curso** (T-060 ✅; T-061 = validación del
usuario) y **M7 build + empaquetado interino hechos** (T-070/T-071: CMake
Linux/Windows, `kult.exe` + ZIP sin assets, `kult.log`). El juego arranca con el
**menú de idioma fiel**, se ve y suena (ventana SDL3 + audio); **colores del juego
corregidos** (Physbase del HAL `0x98000`) y **T-073c resuelto** (timing del
retrato de C, `notes/0019`). `make test` **499 verde**, `make test-port` verde.
Sin bloqueantes.

**T-073c ✅ (pantalla C del Maestro)**: resuelto en la sesión 22. La causa era
el `dbf` inline de `object_info_card` (`0x16dec`, ~4,6 VBL/entrada) sin anotar;
ahora va por `cpu_delay_cycles` (no-op en diferencial). C pasa de ~3 a
~7,6 VBL/frame; A/B no usan `16dec`; input intacto. Detalle:
`notes/0019-fix-timing-c.md`. Queda la **validación visual del usuario**.

**Tarea inmediata — T-061 — validación del usuario en Windows** con
`build-win/kult-0.1.0-win64.zip` (exe `fc4108e4…`): descomprimir sobre la
carpeta ya usada (los `data/` extraídos se conservan), ejecutar `kult.exe` y
comprobar arranque/menú, partida (**colores**), **guardado**, **audio** y
**morir → reiniciar sala** (checklist en `docs/VERIFICACION.md`). Si algo falla:
**`kult.log`** junto al exe y/o `kult-headless.exe` en consola. **Ritmo**: C ya
está modelado (`notes/0019`); si otra escena va acelerada, calibrar con
`KULT_CPU_HZ`/`KULT_ST_CONTEND`, no parchear animaciones.

**Orden de hitos** (detalle en `notes/0011`–`notes/0014`):
`arreglos → M7 build ✅ → T-061 → T-072 (ingesta .STX) / T-073 (ritmo) →
M7-c (empaquetado) → A1 (investigación Amiga, prerrequisito duro de N3) →
N3 (motor nativo + assets nativos, ADR + rama) → A2 (Amiga) → M5 (opcional)`.
**T-022/`0x106f2`** queda **abierto**.

**Reglas:**
- **`src/` (el juego) no cambia de lógica**; lo específico de máquina va en
  `platform/`, `boot/` y `app/`. El **timing** se anota con
  `cpu_delay`/`cpu_delay_cycles` (no-op en diferencial), como en `anim.c`,
  `reveal.c` y `loop.c`; cualquier otro cambio en `src/` requiere permiso.
- **Decisiones cerradas como ADR** (no re-litigar; ver `docs/adr/`): SDL3;
  `SDL_Renderer` base y `SDL_GPU` para CRT; **audio en hito propio**; menú de
  idioma fiel; `boot/` fuera de `src/`; headless primero.
- Reproducir/medir antes de tocar; una hipótesis un cambio; toda corrección deja
  prueba.
- **No tocar** los repos de referencia (solo lectura).
- Todo en **español**. Mantén `TODO.md` y `RETOMAR.md` al día al cerrar.

**Comandos:**
- Core/diferencial: `make test` (499), `make test PYTEST_ARGS=-n0`,
  `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (headless → `build/kult`), `make port-sdl` (ventana →
  `build/kult-sdl`), `make test-port` (HAL sin oráculo).
- Build CMake (M7): `make cmake-native` → `build-cmake/{kult,kult-sdl}`;
  `make cmake-win` → `build-win/{kult.exe,kult-headless.exe}`;
  `make package-win` → ZIP; `make package-native` → TGZ. El cross usa SDL3
  MinGW en `third_party/` (gitignored; URLs en `docs/BUILD.md`).
- Ejecutar: `KULT_DATA_DIR=/tmp/kult_data ./build/kult` (headless) o
  `KULT_DATA_DIR=/tmp/kult_data ./build/kult-sdl` (ventana por defecto;
  `KULT_WINDOW=0` fuerza headless).
  El directorio de datos junta `disk_a`+`disk_b` del usuario (p. ej.
  `/tmp/kult_data`); el oráculo necesita `work/disk_a/KULT.PRG` (gitignored).
- SDL3 ya instalado (`apk add sdl3-dev`; pkg-config `sdl3` 3.4.10).
