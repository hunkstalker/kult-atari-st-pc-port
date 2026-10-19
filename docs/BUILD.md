# BUILD

## Estado (M0–M7 build hecho)
- **Core + diferencial**: operativo con el **Makefile + kit** heredado de
  `kult-atari-st-c`. `make test` compila `build/libkult.so` (candidato) y
  `tools/recreate_kit/oracle/build/liboracle.so` (oráculo) y lanza la suite.
- Requisitos: `project.toml`, `work/names.txt`, `work/disk_a/KULT.PRG`
  (gitignored) y `.venv` con `requirements.txt`. `CC := gcc`.
- **Ejecutable headless**: `make port` compila
  `core(src/) + platform/ + boot/ + app/` con `-Iplatform/include` (que sombrea
  `os.h`/`hw.h`/`psg.h`/`sched.h` con la HAL de PC) → `build/kult`.
  `make test-port` compila y corre las pruebas funcionales de la HAL (sin
  oráculo).
- **Ventana + audio SDL3 (M3/M4)**: `make port-sdl` → `build/kult-sdl`
  (`platform/video_sdl.c` y `platform/audio_sdl.c`, `-DHAVE_SDL3`). Requiere
  `sdl3`/`sdl3-dev` (pkg-config `sdl3`). **La ventana se abre por defecto**
  (`KULT_WINDOW=0` fuerza headless); `KULT_SCALE=3`; ESC sale. Entrada real y ritmo 50 Hz por defecto
  (`KULT_AUTOINPUT=1` y `KULT_REALTIME=0` los desactivan, p. ej. sin display).
  `KULT_AUDIO=0` apaga el sonido. Sin display: `SDL_VIDEODRIVER=dummy`
  (y `SDL_AUDIODRIVER=dummy`).
- **CMake (M7/T-070)**: build alternativo del ejecutable para **Linux y
  Windows** (cross MinGW). Ver abajo; el **Makefile se mantiene** para
  core/diferencial y para `make port`/`port-sdl`/`test-port`.
- **Empaquetado interino (T-071)**: ZIP Windows y TGZ Linux con **CPack**, sin
  assets (el usuario aporta `data/`). El empaquetado completo con **ingesta
  (`kult.data`)** es M7-c (va con N3).

## Objetivo
- **Core** compilable aparte para las pruebas diferenciales (sin SDL).
- **Ejecutable** = core + `boot/` + `platform/` + `app/`, con **SDL3**.

## Herramientas
- **CMake** (≥3.16) para el ejecutable; el core/tests siguen con el Makefile.
- **SDL3**: en Linux el paquete del sistema (`apk add sdl3-dev`, pkg-config
  `sdl3` 3.4.10); para Windows, SDL3 3.4.10 MinGW en `third_party/`.
- **Cross a Windows**: `x86_64-w64-mingw32-gcc` + `windres` (paquete
  `mingw-w64-gcc`).

## CMake (Linux nativo)
```sh
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake -j          # -> build-cmake/kult (headless)
                                      #    build-cmake/kult-sdl (SDL3)
(cd build-cmake && cpack -G TGZ)      # -> kult-0.1.0-linux-x86_64.tar.gz
```
Envoltorios equivalentes: `make cmake-native` y `make package-native`.

## CMake + MinGW (Windows x64, cross)
1. Descargar SDL3 **3.4.10** a `third_party/` (gitignored):
   - build: `SDL3-devel-3.4.10-mingw.zip` (GitHub: libsdl-org/SDL, release
     `release-3.4.10`) — headers, `libSDL3.dll.a` y config de CMake;
   - paquete: `SDL3-3.4.10-win32-x64.zip` (de ahí sale el `SDL3.dll`).
   ```sh
   mkdir -p third_party && cd third_party
   wget https://github.com/libsdl-org/SDL/releases/download/release-3.4.10/SDL3-devel-3.4.10-mingw.zip
   wget https://github.com/libsdl-org/SDL/releases/download/release-3.4.10/SDL3-3.4.10-win32-x64.zip
   unzip SDL3-devel-3.4.10-mingw.zip
   unzip -d SDL3-3.4.10-win32-x64 SDL3-3.4.10-win32-x64.zip
   ```
2. Configurar y compilar (o `make cmake-win`):
   ```sh
   cmake -S . -B build-win -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
     -DCMAKE_PREFIX_PATH=$PWD/third_party/SDL3-3.4.10/x86_64-w64-mingw32
   cmake --build build-win -j
   ```
   Salidas: **`build-win/kult.exe`** (ventana/audio SDL3, subsistema GUI con
   icono) y `build-win/kult-headless.exe` (consola, diagnóstico).
3. Empaquetar (o `make package-win`):
   ```sh
   (cd build-win && cpack -G ZIP)      # -> kult-0.1.0-win64.zip
   ```
   Contenido: `kult.exe` + `kult-headless.exe` + `SDL3.dll` + `data/` (vacía) +
   `LEEME.txt` + `DIAGNOSTICO.txt`. **Sin assets.** Requiere Windows 10+ (UCRT:
   `api-ms-win-crt-*`).

## Diagnóstico en ejecución (M7)
- **`kult.log`** se escribe **junto al `.exe`** (fallback `%LOCALAPPDATA%\Kult`
  en Windows y `/tmp` en POSIX) y también va a stderr. Contiene: versión,
  directorios de datos, ruta de `KULT.PRG`, resultado de SDL/ventana/audio,
  frames, volcados y **exit code**.
- `KULT_LOG=<ruta>` redirige el log; `KULT_LOG=0` lo apaga.
- `platform_fatal` muestra un **MessageBox** si el proceso no tiene consola
  (build GUI); en `kult-headless.exe` (consola) no interpone diálogo.
- Si en modo ventana falla SDL/ventana se sale con **error visible** (ya no se
  continúa en un bucle headless invisible).
- **Preflight**: antes de arrancar se comprueban los 26 ficheros del juego
  (`PRES/DRAP/FOND.PI1`, `ECHE0-7`, `PUZZL`, `DIAL*`, `MOTS*`, `VERB*`, `DESC*`,
  `BRUIT`, `CLEAR`). Si falta alguno, se lista en el error y en el log.
- **Exit codes**: `0` ok, `2` PRG/datos ausentes, `3` fallo de arranque, `4`
  fallo de ventana SDL.
- `KULT_DUMP_TITLE=<ruta.ppm>` vuelca el **frame del título** (PRES) antes de la
  espera de tecla/clic; útil para validar su paleta sin ventana.

## Icono (Windows)
- `assets/kult_icon.ico` (16..256) + `assets/kult_icon.rc`
  (`101 ICON "kult_icon.ico"`); windres lo incrusta en el `.exe`.
- `assets/kult_icon_rgba.h` (48x48 RGBA) para `SDL_SetWindowIcon`; se regenera
  con `python3 tools/make_icon_rgba.py` (el `.h` está versionado; el build no
  necesita Python).

## Ritmo / CPU (T-073)
- El build del port compila `src/*.c` con **`-finstrument-functions`** (solo ahí;
  el diferencial `make test` y `test-port` no) y enlaza `platform/cpuwork.c`,
  que cobra a cada llamada el **coste medido** del original (ciclos/llamada) y lo
  duerme contra el reloj de CPU, cediendo VBLs.
- Tabla: `platform/cpuwork_costs.inc` (**generada** por
  `tools/gen_cpuwork_costs.py` desde `tools/cpuwork/funcost_st_reference.txt`
  (medida del oráculo), `docs/mapa` y los símbolos de `nm`). Regenerar:
  ```sh
  make cmake-native
  python3 tools/gen_cpuwork_costs.py
  ```
- `cpu_delay` (bucles `dbf` transcritos) usa la misma puerta
  (`platform/include/kult_port.h` sombra + `platform_cpu_delay`).
- Ajustes: `KULT_CPU=0` desactiva todo el modelo; `KULT_CPU_HZ` (def. 8 MHz),
  `KULT_ST_CONTEND` (% de CPU efectiva por contención de bus; def. 100),
  `KULT_15DA6_BLIT` (ciclos extra del blit de elementos con delay 0; def. **0 =
  desactivado**; se probó y afectaba al objeto del inventario, no al Maestro),
  `KULT_ANIM_GATE` y `KULT_REVEAL_GATE` (%) para calibrar. En headless/diferencial
  es no-op (la imagen no cambia).
- **Marcas (animaciones cortas)**: `KULT_MARKS=<ruta>` escribe `F <addr> <vbl>`
  al entrar en funciones clave, `S <vbl> <delay> <w> <h> <reveal>` en `15da6` y
  `M f12 <vbl>` al pulsar **F12**: vuelca los ~3 s anteriores (buffer circular)
  y sigue trazando todo el core instrumentado (~1,5 s, una vez por función/VBL).
  `kult-marcas.bat` lo activa.
  Los logs van en **append** con separador `# RUN`; F12 también deja
  `F12: marca manual` en `kult.log`.
- **Medida objetiva**: `KULT_FPSLOG=1` imprime cada ~3 s el **FPS del bucle del
  juego** (una iteración = un `wait_frame`, igual que el contador `Ikbdws` del
  arnés Hatari del predecesor: `tools/hatari_fps/`). Comparar la **misma escena**
  en Hatari (CPU cycle-exact) y en el port; si el port va más rápido, bajar
  `KULT_ST_CONTEND` (o `KULT_CPU_HZ`) hasta igualar los FPS.

## Datos y rutas
- El ejecutable busca los datos del usuario en **`data/` junto al ejecutable**
  (o en `KULT_DATA_DIR`; en Windows varios directorios separados por `;`, en
  POSIX por `:`). **No** se versionan assets.
- El **`KULT.PRG`** se busca también en el directorio de datos (`data/KULT.PRG`)
  si no se define `KULT_PRG`.
- **Estado interino (T-071)**: `data/` debe contener los ficheros **extraídos**
  de `disk_a`+`disk_b`. Las imágenes `.STX/.MSA/.ST` **aún no se leen**; la
  ingesta automática es **T-072** (siguiente tras T-061).

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
ejecutable (o en una ruta configurable con `KULT_DATA_DIR`; en Windows el
separador de varios directorios será `;`). **No** se versionan assets.

