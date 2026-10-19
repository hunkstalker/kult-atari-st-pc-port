# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 8 — M4/T-040+T-041+T-042 (audio)**: **Timer A del MFP**
  (`platform/mfp.c`): periodo de `TACR`/`TADR` (2.4576 MHz), resuelve el vector
  `$134` a la ISR C (PCM `0x1910e`/`0x190f6`) o al **reproductor de música
  `0x3a872`** (reconstruido en `boot/music_player.c` con la envolvente
  `0x3a936`). **PSG/YM2149** (`platform/psg.c` con ledger + seed; `platform/audio.c`
  con ondas/ruido/envolvente/filtro DC). **Salida SDL3** (`platform/audio_sdl.c`).
  El audio avanza con el reloj de pared en ventana y se bombea en `idle` para la
  música de los menús. Verificación en `make test-port`: **ledger PSG del
  reproductor contra el oráculo** (23 eventos), periodo/ISR del Timer A, RMS del
  YM e integración con `init_hardware`. **M4 completa.**
- **Sesión 7 — M3/T-031+T-032 (entrada real y ritmo)**: SDL teclado/ratón →
  cola GEMDOS e IKBD (`platform_key_push`/`platform_mouse_set`); `Cnecin`
  bloqueante con bombeo (`idle`, presenta y sale al cerrar la ventana); conmutador
  auto/real (`KULT_AUTOINPUT`). Ritmo `platform_set_realtime` (1 VBL = 20 ms,
  50 Hz) con `KULT_REALTIME=0` de escape. Pruebas en `make test-port`.
  **M3 completa.**
- **Sesión 6 — M3/T-030 (ventana SDL3)**: `platform/video_sdl.c` (ventana,
  `SDL_Renderer`, textura RGBA del framebuffer planar, `SDL_Renderer` con
  escalado entero y vsync, ESC para salir), `platform_fb_to_rgba`,
  `kult_set_frame_cb`/`kult_request_stop` y `make port-sdl` → `build/kult-sdl`.
  SDL3 (`sdl3-dev`) instalado. Probado con `SDL_VIDEODRIVER=dummy`.
- **Sesión 5 — T-024 (causa raíz)**: **el loader C `boot/prg.c` aplicaba una
  relocación de más tras cada `RELOC_SKIP`** (byte 1 del stream DRI), corrompiendo
  544 bytes de la imagen (p. ej. un largo de la tabla ICONE). Eso desviaba el
  guion de `_start` (`0xc001`) y reventaba. **Corregido**: la imagen del port
  coincide byte a byte con el loader del oráculo (`loader.py`). El port **ejecuta
  el arranque y `game_loop`** (probado 1200+ frames). Prueba de regresión del
  loader en `make test-port`.
- **Sesión 4 — M2/T-021**: tick de VBL `0x160c2` en `boot/vbl.c`.
- **Sesión 3 — M1**: `platform/` + `boot/` + `app/`; HAL headless.
- **Sesión 2 — M0**: core importado; `make test` **499 verde**.

## Estado actual
- **M0–M4 hechas**: el juego arranca y avanza frames, **headless**, con **ventana
  SDL3 jugable** (render + entrada real + ritmo 50 Hz) y **audio** (Timer A del
  MFP + PSG/YM2149 + SDL).
- `make test` 499 verde; `make test-port` verde.
- Sin bloqueantes conocidos.

## Siguiente tarea
**T-020** (validar/cerrar `_start`, en particular el menú de idioma fiel) y luego
**M6 — pulido y validación** y **M7 — empaquetado**. Ver `docs/PLAN.md`.
**M5 (filtros CRT) queda como extra opcional, no prioritario** (se anota para
desarrollarlo en algún momento, tras validar). La **escucha final del audio** la
hace el usuario (`KULT_AUDIO=0` lo apaga; `KULT_MUSIC_GAIN` ajusta la música).

## Comandos
- Core/diferencial: `make test` (499), `make test PYTEST_ARGS=-n0`,
  `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (→ `build/kult`, headless), `make port-sdl` (→ `build/kult-sdl`,
  ventana SDL3), `make test-port` (HAL, sin oráculo).
- Ejecutar headless: `KULT_DATA_DIR=/tmp/kult_data ./build/kult`
  (junta `disk_a`+`disk_b` del usuario en `/tmp/kult_data`).
- Con ventana: `KULT_WINDOW=1 KULT_DATA_DIR=/tmp/kult_data ./build/kult-sdl`
  (`KULT_SCALE=3`, ESC para salir). Sin display: `SDL_VIDEODRIVER=dummy`.

## M1–M4: qué hay y cómo se prueba
- `platform/include/{os,hw,psg,sched}.h` **sombrean** al kit en el build del
  ejecutable: el core incluye los mismos nombres, pero responden la HAL de PC.
  `platform/{fs,video,input,time,hw,psg,os,audio,mfp,audio_sdl}.c` los implementan.
- **Audio (M4)**: `platform/mfp.c` (Timer A: periodo de `TACR`/`TADR`, vector
  `$134`), `platform/psg.c` (registros + ledger + seed), `platform/audio.c` (YM:
  ondas/ruido/envolvente, RMS), `platform/audio_sdl.c` (solo `make port-sdl`) y
  `boot/music_player.c` (reproductor `0x3a872` + envolvente `0x3a936`).
  `KULT_AUDIO=0` apaga; `KULT_MUSIC_GAIN` (25 % por defecto) escala la música.
- `boot/prg.c` (loader GEMDOS: cabecera + relocación DRI, espejo de `loader.py`) +
  `boot/boot.c` (`_start` provisional, corte por `frames` con `hook_loop_top` +
  `longjmp`) + `boot/vbl.c` (tick 0x160c2).
- `app/main.c`: `KULT_PRG`, `KULT_DATA_DIR` (admite varios con `:`),
  `KULT_FRAMES` (0 = sin límite), `KULT_DUMP`, `KULT_PLANAR`,
  `KULT_KEYS`/`KULT_KEY`, `KULT_SKIP_DRAP`, `KULT_DEBUG`; en ventana:
  `KULT_WINDOW=1`, `KULT_SCALE`, `KULT_REALTIME=0`, `KULT_AUTOINPUT=1`,
  `KULT_AUDIO=0`.
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
