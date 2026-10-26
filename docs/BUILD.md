# BUILD

## Estado (M0–M7 build + macOS 1.1.0)
- **Core + diferencial**: operativo con el **Makefile + kit** heredado de
  `kult-atari-st-c`. `make test` compila `build/libkult.so` (candidato) y
  `lib/kult-core/tools/recreate_kit/oracle/build/liboracle.so` (oráculo) y lanza la suite.
- Requisitos: `project.toml`, `lib/kult-core/symbols/names.txt`, `work/disk_a/KULT.PRG`
  (gitignored) y `.venv` con `requirements.txt`. `CC := gcc`.
- **Ejecutable headless**: `make port` compila
  `core(lib/kult-core/src/) + platform/ + boot/ + app/` con `-Iplatform/include` (que sombrea
  `os.h`/`hw.h`/`psg.h`/`sched.h` con la HAL de PC) → `build/kult`.
  `make test-port` compila y corre las pruebas funcionales de la HAL (sin
  oráculo).
- **Ventana + audio SDL3 (M3/M4)**: `make port-sdl` → `build/kult-sdl`
  (`platform/video_sdl.c` y `platform/audio_sdl.c`, `-DHAVE_SDL3`). Requiere
  `sdl3`/`sdl3-dev` (pkg-config `sdl3`). **La ventana se abre por defecto en
  fullscreen sin bordes a la resolución nativa** (`KULT_WINDOW=0` fuerza
  headless; `KULT_FULLSCREEN=0` arranca en ventana). La escala inicial sale de
  `kult.ini` (`scale`); si no existe, **Estirar a pantalla**. `KULT_SCALE`
  (`0` Auto, `1..6` fijo, `7` Estirar) y `KULT_FULLSCREEN` fuerzan el arranque.
  **F1** alterna fullscreen/ventana; **F2**
  cicla Estirar a pantalla/×1–×6/Auto; **F5** muestra la ranura actual y, si se repite con el aviso visible, la cambia; **F6** guarda y
  **F7** carga (`saves/slot-N.save`, ranura en `kult.ini`); **F9/F11** bajan/suben
  el volumen 10%; **F10** silencia/restaura; ESC sale. Entrada real y ritmo 50 Hz
  por defecto
  (`KULT_AUTOINPUT=1` y `KULT_REALTIME=0` los desactivan, p. ej. sin display).
  `KULT_AUDIO=0` apaga el sonido. Sin display: `SDL_VIDEODRIVER=dummy`
  (y `SDL_AUDIODRIVER=dummy`).
- **CMake (M7/T-070)**: build alternativo del ejecutable para **Linux**,
  **Windows** (cross MinGW) y **macOS Universal 2**. Ver abajo; el **Makefile
  se mantiene** para core/diferencial y para `make port`/`port-sdl`/`test-port`.
- **Empaquetado de release**: ZIP Windows, TGZ Linux/AppImage y ZIP/DMG macOS
  Universal 2, sin assets (el usuario aporta sus STX). `kult-headless` y
  `DIAGNOSTICO.txt` no forman parte del paquete. En el primer arranque se
  ingieren los STX y se genera `data/kult.data`.

## Objetivo
- **Core** compilable aparte para las pruebas diferenciales (sin SDL).
- **Ejecutable** = core + `boot/` + `platform/` + `app/`, con **SDL3**.

## Herramientas
- **CMake** (≥3.16) para el ejecutable; el core/tests siguen con el Makefile.
- **SDL3**: en Linux el paquete del sistema (`apk add sdl3-dev`, pkg-config
  `sdl3` 3.4.10); para Windows, SDL3 3.4.10 MinGW en `third_party/`; para macOS,
  SDL3 universal desde fuente con `make deps-macos`.
- **Cross a Windows**: `x86_64-w64-mingw32-gcc` + `windres` (paquete
  `mingw-w64-gcc`).
- **macOS**: macOS 11+, Xcode Command Line Tools, CMake y Python. Para
  Universal 2 se compila con `arm64` y `x86_64` en el mismo build.

## CMake (Linux nativo)
Con SDL3 del sistema:

```sh
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake -j
(cd build-cmake && cpack -G TGZ)
```

Para un paquete Linux autosuficiente y AppImage, usar SDL3 3.4.10 local:
`fetch_sdl3_linux.sh` compila SDL3 con `SDL_X11_XSCRNSAVER=OFF` y
`SDL_X11_XTEST=OFF`, de modo que no hacen falta las cabeceras de XScrnSaver
(`libxss-dev`) ni XTest (`libxtst-dev`).

```sh
make deps-linux
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release   -DCMAKE_PREFIX_PATH="$PWD/third_party/sdl3-linux"   -DKULT_SDL3_LIB="$PWD/third_party/sdl3-linux/lib/libSDL3.so.0"
cmake --build build-linux -j
(cd build-linux && cpack -G TGZ)
cmake --install build-linux --prefix build/linux-stage
tools/build_appimage.sh build/linux-stage dist/kult-1.1.0-x86_64.AppImage
```

Envoltorios equivalentes: `make cmake-native` y `make package-native` para el
build con SDL3 del sistema.

## CMake + MinGW (Windows x64, cross)
1. Obtener SDL3 **3.4.10**:
   ```sh
   make deps-win
   ```
   El script descarga los dos ZIP de libsdl-org/SDL, verifica sus hashes
   SHA-256 y los descomprime en `third_party/`. Detalle:
   `third_party/README.md`.
2. Configurar y compilar (o `make cmake-win`):
   ```sh
   cmake -S . -B build-win -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
     -DCMAKE_PREFIX_PATH=$PWD/third_party/SDL3-3.4.10/x86_64-w64-mingw32
   cmake --build build-win -j
   ```
   Salidas de build: **`build-win/kult.exe`** (ventana/audio SDL3, GUI con
   icono) y `build-win/kult-headless.exe` (solo desarrollo). El paquete solo
   instala `kult.exe`.
3. Empaquetar (o `make package-win`):
   ```sh
   (cd build-win && cpack -G ZIP)      # -> kult-1.1.0-win64.zip
   ```
   Contenido: `kult.exe` + `SDL3.dll` + `data/` + `saves/` + `LEEME.txt` + `Kult Settings.txt` + `Kult Settings.png`.
   **Sin assets, sin headless y sin DIAGNOSTICO.txt.** Requiere Windows 10+
   (UCRT: `api-ms-win-crt-*`).

## CMake + macOS Universal 2

1. Obtener SDL3 universal (mismo SHA que Linux):
   ```sh
   make deps-macos
   ```
   El script descarga SDL3 3.4.10 y lo compila para `arm64;x86_64` con
   deployment target macOS 11.0.
2. Configurar y compilar (o `make cmake-macos`):
   ```sh
   cmake -S . -B build-macos -DCMAKE_BUILD_TYPE=Release      -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"      -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0      -DCMAKE_PREFIX_PATH="$PWD/third_party/sdl3-macos"      -DKULT_SDL3_LIB="$PWD/third_party/sdl3-macos/lib/libSDL3.0.dylib"
   cmake --build build-macos -j
   ```
   Salida: `build-macos/Kult.app`. El bundle incluye `language.data`, icono,
   `LEEME.txt` y SDL3 en `Contents/Frameworks`.
3. Empaquetar (o `make package-macos`):
   ```sh
   cmake --install build-macos --prefix build/macos-stage
   KULT_VERSION=1.1.0 tools/build_macos.sh build/macos-stage dist
   ```
   Genera `kult-1.1.0-macos-universal.zip` y `.dmg`, firma ad-hoc (o con
   `MACOS_SIGN_IDENTITY`) y crea los `.sha256`. Si existen `APPLE_ID`,
   `APPLE_TEAM_ID` y `APPLE_APP_PASSWORD`, notariza el DMG; sin esos secretos
   queda una build ad-hoc no notarizada.
4. Tests: `make test-port CC=clang` y `make test-tools PY=python`. El
   diferencial completo no se ejecuta en macOS porque depende de `.so`,
   `KULT.PRG` y el oráculo Linux.

## Diagnóstico en ejecución (M7)

Arquitectura, variables y pruebas: [`DIAGNOSTICO.md`](DIAGNOSTICO.md).

- **`kult.log`** se escribe junto al ejecutable/AppImage (en macOS, en
  `~/Library/Application Support/Kult/`) y también va a stderr. Contiene:
  versión, directorios de datos, ruta de `KULT.PRG`, resultado de
  SDL/ventana/audio, frames, volcados y **exit code**. Lleva timestamp.
- Política B: los errores no abren ventanas; solo si no se puede escribir el log
  se muestra un diálogo de emergencia en la build GUI.
- Si en modo ventana falla SDL/ventana se sale con **error visible** (ya no se
  continúa en un bucle headless invisible).
- **Preflight**: antes de arrancar se comprueban los 26 ficheros del juego
  (`PRES/DRAP/FOND.PI1`, `ECHE0-7`, `PUZZL`, `DIAL*`, `MOTS*`, `VERB*`, `DESC*`,
  `BRUIT`, `CLEAR`). Si falta alguno, se lista en el error y en el log.
- **Exit codes**: `0` ok, `2` PRG/datos ausentes, `3` fallo de arranque, `4`
  fallo de ventana SDL.
- `KULT_DUMP_TITLE=<ruta.ppm>` vuelca el **frame del título** (PRES) antes de la
  espera de tecla/clic; útil para validar su paleta sin ventana.

## Iconos
- **Windows**: `assets/kult_icon.ico` (16..256) + `assets/kult_icon.rc`
  (`101 ICON "kult_icon.ico"`); windres lo incrusta en el `.exe`.
- **macOS**: `assets/kult.icns`, generado desde `assets/kult_icon.ico` con
  `python3 tools/make_icns.py` (el `.icns` está versionado).
- **Ventana**: `assets/kult_icon_rgba.h` (48x48 RGBA) para `SDL_SetWindowIcon`;
  se regenera con `python3 tools/make_icon_rgba.py` (el `.h` está versionado; el
  build no necesita Python).

## Ritmo / CPU (T-073)
- El build del port compila `lib/kult-core/src/*.c` con **`-finstrument-functions`** (solo ahí;
  el diferencial `make test` y `test-port` no) y enlaza `platform/cpuwork.c`,
  que cobra a cada llamada el **coste medido** del original (ciclos/llamada) y lo
  duerme contra el reloj de CPU, cediendo VBLs.
- Tabla: `platform/cpuwork_costs.inc` (**generada** por
  `tools/gen_cpuwork_costs.py` desde `tools/cpuwork/funcost_st_reference.txt`
  (medida del oráculo), `lib/kult-core/docs/mapa` y los símbolos de `nm`). Regenerar:
  ```sh
  make cmake-native
  python3 tools/gen_cpuwork_costs.py
  ```
- `cpu_delay` (bucles `dbf` transcritos) usa la misma puerta
  (`platform/include/kult_port.h` sombra + `platform_cpu_delay`).
- Ajustes: `KULT_CPU=0` desactiva todo el modelo; `KULT_CPU_HZ` (def. 8 MHz),
  `KULT_ST_CONTEND` (% de CPU efectiva por contención de bus; def. 100),
  `KULT_15DA6_BLIT` (ciclos extra del blit de elementos con delay 0; def. **0 =
  desactivado**; se probó y afectaba al objeto del inventario, no a la pantalla C del Maestro),
  `KULT_ANIM_GATE` y `KULT_REVEAL_GATE` (%) para calibrar. En headless/diferencial
  es no-op (la imagen no cambia).
- **Marcas (animaciones cortas)**: `KULT_MARKS=<ruta>` escribe `F <addr> <vbl>`
  al entrar en funciones clave, `S <vbl> <delay> <w> <h> <reveal>` en `15da6` y
  `M f12 <vbl>` al pulsar **F12**: vuelca los ~3 s anteriores (buffer circular)
  y sigue trazando todo el core instrumentado (~1,5 s, una vez por función/VBL).
  `kult-marcas.bat` lo activa.
  Los logs van en **append** con separador `# RUN`; F12 (solo con
  `KULT_MARKS` activo) también deja `F12: marca manual` en `kult.log`.
- **Texto y F12**: con `KULT_MARKS` activo se registran líneas
  `T <vbl> <tag> idx=.. w=.. str=.. end=.. cx=.. cy=.. raw=..` al dibujar
  líneas de texto. F12 (con `KULT_MARKS`) deja una línea `X` con la última línea de texto (índice,
  ancho, posición y bytes). Con `KULT_AUTOLOG=1` se vuelca el anillo cada 150
  VBL, de modo que un bloqueo sin F12 deja los últimos ~3 s en el log.
- **Traza del anillo (`R`)**: con `KULT_RINGLOG=<ruta>` (solo líneas `R`) o
  `KULT_MARKS` activo, cada anillo de acciones escribe en vivo (no depende de
  F12) líneas
  `R <vbl> <evento> a=.. b=.. c=.. cr2=.. abf8=.. ace7=.. acf2=.. ace8=..
  ad09=.. w19316=.. a826=.. sel=.. cls=..`.
  Eventos: `ring_open` (A6 y máscara), `ring_entry` (`icono`, comando y bit),
  `ring_pick`, `ring_return`, `loop_in/sel/cmd/after_cmd/threshold/pending/
  pending_tail/reopen/clear_flags/exit_ffff/tail_ac`, `run_cmd`, `run_f`,
  `run_f_ret`, `walk4d`, `walk4d_ret`, `vm_tail`, `run_boundary_in/out`,
  `run_unwind`, `run_unwind_out` y `RUN_UNWIND4D` (esta última en el log `D`
  de `kult-diag.exe`).
  `cr2` es `$1a4c2`, `abf8` el comando pendiente, `a826` el A7 guardado por
  `run_command`, `w19316` la palabra comparada con `0x2f3c` y `sel/cls` el
  objeto y clase seleccionados (con sus primeros 8 bytes). El `icono` de cada
  `ring_entry` se traduce a verbo con `work/texts/VERBE.BIN.json` (el índice
  JSON es el icono; p. ej. `0x94` = *LIE DOWN*).
  `kult-anillo.bat` activa solo esta traza (`kult-anillo.log`); el resumen se
  obtiene con `python3 tools/ringreport.py kult-anillo.log`.
  La misma linea `R` incluye `hp` (`$1ad20`), `ad03`, `ad02`, `rnd_idx`
  (`$1a579`) y `rnd_byte` (`$1acd7`). Con **`KULT_RANDLOG=<ruta>`** se registra
  cada lectura de la tabla ALEAT (`0x2741e`) desde `pal_next_byte`/`pal_next_word`
  con `D <vbl> rnd_b|rnd_w ... idx=.. val=.. hp=.. ad03=.. ad02=..`.
  Para capturas de dano hay dos batches: `kult-dano-normal.bat` y
  `kult-dano-psi.bat` (usan `kult-diag.exe`, `KULT_MARKS`, `KULT_RINGLOG`,
  `KULT_RANDLOG` y `KULT_INPUTLOG` con ficheros separados). Para una sola run
  con todos los ataques (normal + PSI) esta `kult-dano-todo.bat`.
  **`KULT_INPUTLOG=<ruta>`** registra la entrada con lineas `I <vbl> <evento>
  x=.. y=.. btn=.. [hp/ad03/ad02/ad09/cmd/sel/cls]`: `mouse_down/up`,
  `mouse_down_r/up_r` (clic crudo de SDL) e `ikbd_click`/`click` (clic
  entregado/procesado por `wait_frame`). En la traza `R` tambien aparecen
  `click_cmd`, `queue_abf8/ac06/acaa/acae`, `dispatch_queued`, `set_ac06`,
  `set_abf8`, `set_acaa`, `cmd_handler` e `inline_cmd`, para no perder
  comandos que no pasan por el anillo.
- **Replay de entrada (T-088)**: `KULT_REPLAY_REC=<ruta>` graba un replay
  binario con las respuestas que ve el juego en cada sondeo: paquete IKBD
  (botón, skip y **cursor x/y**), `Crawio` y `Cnecin`. `KULT_REPLAY_PLAY=<ruta>`
  lo reproduce e ignora la entrada real de SDL. Cada evento se vuelca con
  `fflush`, así que el final se conserva aunque el proceso muera. Bats
  específicos: `kult-replay-record.bat` y `kult-replay-play.bat`. Genérico
  (cualquier save): `kult-replay-generic.bat record|play <carpeta-save>
  [replay.bin]`. Formato y uso:
  **`notes/0038-replay-input-t088.md`**. Flujo **validado en Windows**: sirvió
  para reproducir y cerrar el crash de la ficha al atacar a un personaje
  (T-088).
- **Presentación (T-138)**: `KULT_PRESENT_LOG=1` registra cada segundo la
  frecuencia real de `SDL_RenderPresent` (`present: N Hz`). Está limitada por
  VSync y por la frecuencia del monitor.
- **Medida objetiva**: `KULT_FPSLOG=1` imprime cada ~3 s el **FPS del bucle del
  juego** (una iteración = un `wait_frame`, igual que el contador `Ikbdws` del
  arnés Hatari del predecesor: `tools/hatari_fps/`). Comparar la **misma escena**
  en Hatari (CPU cycle-exact) y en el port; si el port va más rápido, bajar
  `KULT_ST_CONTEND` (o `KULT_CPU_HZ`) hasta igualar los FPS.

## Datos y rutas
- Release portable: `data/`, `saves/` y `kult.log` viven junto al ejecutable.
  No hay `KULT_DATA_DIR`, appdata ni `/tmp`. **No** se versionan assets.
- El **`KULT.PRG`** se busca en `data/KULT.PRG` si no se define `KULT_PRG`.
- **Ingesta**: copia `Kult.STX` y `Kult (B).STX` en `data/` o junto al `.exe`.
  Se identifican por SHA-256, se extraen y se genera `data/kult.data`; después
  los `.STX` se pueden borrar. También se aceptan ficheros sueltos en `data/`
  para desarrollo. Los packs de idioma se generan con
  `tools/make_language.py` (ver `docs/DATA_FORMAT.md`).

## Comandos del core (heredados, ya verdes)
```
make test                      # core + HAL + tests propios del port
make test PYTEST_ARGS=-n0      # serial
make regscan                   # ids_reg=0
make core                      # build/libkultcore.so (API restringida)
make mapa                      # regenera lib/kult-core/docs/mapa/
```

## Datos
El ejecutable usa `data/` y `saves/` junto al ejecutable. **No** se versionan
assets.

