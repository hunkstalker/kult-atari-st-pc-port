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
~7,6 VBL/frame; A/B no usan `16dec`. Detalle: `notes/0019-fix-timing-c.md`.

**T-074 ✅ (input HAL + presentación)**: izquierdo/teclado = acción sin skip;
derecho = acción + skip; la HAL pisa `$1a570` al salir de `wait_frame`, sin tocar
`src/input.c`. `platform_tick` presenta en cada VBL, así se ven las animaciones
de puerta/A-B-C. Detalle: `notes/0020-input-hal-y-presentacion.md`.

**T-075 ✅ (cursor 16×16 / IKBD)**: el juego configura el ratón absoluto a
304×184 (`Ikbdws` `09 01 30 00 b8`); la HAL lo emula y el cursor ya no
reaparece por el lado contrario. Detalle: `notes/0021-cursor-e-ikbd.md`.

**T-076 ✅ (reinicio tras morir)**: `reset_clear` termina en el original con
`bra 0x10158`; el C nativo solo hacía el estado. `boot/` emula el salto con
`longjmp` y vuelve al Maestro/objeto. Detalle:
`notes/0022-reinicio-tras-muerte.md`.

**T-077 ✅ (ritmo de muerte, punto 1)**: la sangre se cobra por línea en
`reveal_line` (`13d0e`, fórmula medida 1063 + 198 × pasos) y el viaje espacial
en `icon_scale_loop` (`1834a`, 22.790/pasada); `181f0` no se cobra al entrar.
Sangre y viaje espacial ya no van instantáneos. Detalle:
`notes/0023-costes-sangre-espacio.md`.

**T-079 ✅ (estrella final + esperas de disco)**: la espera posterior a la
animación es la carga de `FOND.PI1` (`0x13568`, 0x7d22 B); el original bloquea
en Fread ~2 s con la estrella en pantalla. La HAL modela el tiempo de disquete
en `platform_disk_delay` (`notes/0025`), sin tocar `src/`. Como se aplica a
**cada Fread**, hay pausas al cargar salas/reiniciar y en la estrella
(`notes/0026-esperas-de-disco.md`). **Excepción**: la carga inicial completa
(PRES/DRAP, listas A/B, FOND, ECHE) no espera (`platform_disk_timing(0)` hasta
después de ECHE), para no perder el inicio de la canción ni esperar ~20 s tras
el idioma.

**Tarea inmediata — T-061 — validación del usuario en Windows** con
`build-win/kult-0.1.0-win64.zip` (exe `ed175889…`): descomprimir sobre la
carpeta ya usada (los `data/` extraídos se conservan), ejecutar `kult.exe` y
comprobar arranque/menú, partida (**colores**), **guardado**, **audio** y
**morir → reiniciar (Maestro/objeto)** (checklist en `docs/VERIFICACION.md`). **Controles**:
izquierdo/teclado = acción y animación visible; derecho = acción + skip. Si algo
falla: **`kult.log`** junto al exe y/o `kult-headless.exe` en consola. **Ritmo**:
C ya está modelado (`notes/0019`); si otra escena va acelerada, calibrar con
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
