# BUILD

## Estado (M0/M1)
- **Core + diferencial**: operativo con el **Makefile + kit** heredado de
  `kult-atari-st-c`. `make test` compila `build/libkult.so` (candidato) y
  `tools/recreate_kit/oracle/build/liboracle.so` (oráculo) y lanza la suite.
- Requisitos: `project.toml`, `work/names.txt`, `work/disk_a/KULT.PRG`
  (gitignored) y `.venv` con `requirements.txt`. `CC := gcc`.
- **Ejecutable headless (M1)**: `make port` compila
  `core(src/) + platform/ + boot/ + app/` con `-Iplatform/include` (que sombrea
  `os.h`/`hw.h`/`psg.h`/`sched.h` con la HAL de PC) → `build/kult`.
  `make test-port` compila y corre las pruebas funcionales de la HAL (sin
  oráculo).
- **Ventana + audio SDL3 (M3/M4)**: `make port-sdl` → `build/kult-sdl`
  (`platform/video_sdl.c` y `platform/audio_sdl.c`, `-DHAVE_SDL3`). Requiere
  `sdl3`/`sdl3-dev` (pkg-config `sdl3`). Ejecutar con `KULT_WINDOW=1`;
  `KULT_SCALE=3`; ESC sale. Entrada real y ritmo 50 Hz por defecto
  (`KULT_AUTOINPUT=1` y `KULT_REALTIME=0` los desactivan, p. ej. sin display).
  `KULT_AUDIO=0` apaga el sonido. Sin display: `SDL_VIDEODRIVER=dummy`
  (y `SDL_AUDIODRIVER=dummy`). (CMake para el ejecutable: pendiente.)

## Objetivo
- **Core** compilable aparte para las pruebas diferenciales (sin SDL).
- **Ejecutable** = core + `boot/` + `platform/` + `app/`, con **SDL3**.

## Herramientas previstas
- **CMake** (para el ejecutable; el core/tests siguen con el Makefile del kit).
- **SDL3** (ventana, entrada, audio, GPU).
- En Linux: paquete SDL3 del sistema o compilado; en Windows: SDL3 + DLLs.

## Flujo previsto (ejecutable)
```
cmake -S . -B build
cmake --build build -j
```
Salidas: librería del core (tests) + ejecutable del juego.

## Comandos del core (heredados, ya verdes)
```
make test                      # candidato + oráculo + 499 pruebas (paralelo)
make test PYTEST_ARGS=-n0      # serial
make regscan                   # ids_reg=0
make core                      # build/libkultcore.so (API restringida)
make mapa                      # regenera docs/mapa/
```

## Datos
El ejecutable busca los datos del usuario en un directorio `data/` junto al
ejecutable (o en una ruta configurable). **No** se versionan assets.

## Cross (Windows) — a valorar
Compilación cruzada desde Linux con MinGW + SDL3, o build nativo en Windows.
