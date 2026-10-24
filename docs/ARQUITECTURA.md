# ARQUITECTURA

## Capas

```
app/        main, driver, presentación, perfiles         [nuevo]
   │  (llama)
   ▼
boot/       _start (0x10000), tick VBL (0x160c2)         [nuevo]
   │  (llama)
   ▼
lib/kult-core/src/  juego en C (lógica), 415 funciones           [subtree]
   │  (llama a los 19 servicios)
   ▼
platform/   HAL PC: os_*/hw_*/psg_*/sched_* + render + audio  [nuevo]
   │
   ▼
SDL3 + sistema operativo (Windows / Linux)
```

- **`lib/kult-core/src/`** es el juego, importado del repo C mediante subtree. No conoce ni SDL ni
  el SO: solo llama a los servicios de `platform/`.
- **`platform/`** es "nuestro TOS + hardware": implementa los 19 servicios y
  además pinta (render) y suena (audio).
- **`boot/`** contiene las piezas del original que faltaban en el core.
- **`app/`** es el programa: inicializa, arranca y presenta.

## El contrato (19 servicios)
El juego y la HAL se comunican **solo** por estos nombres (detalle y equivalente
PC en `docs/PORTING.md`):
`os_fopen os_fclose os_fcreate os_fread os_fwrite os_fsfirst os_fsetdta
os_crawio os_cnecin os_setscreen os_setpalette os_setcolor os_super
hw_write8 hw_sub8 hw_bclr8 psg_port_read psg_port_write sched_poll8`.

## Estado del juego (imagen plana)
El juego mantiene su estado en una **imagen plana** `uint8_t *img` (base de carga
`0x10000`) con accesos big-endian. Ahí viven los framebuffers A/B, la paleta, los
contadores (VBL `A_DAT_1ACD4`), etc. La HAL **lee y escribe esa imagen** (p. ej.
alimenta el paquete IKBD en `A_IKBD_PACKET` para que se actualicen
`A_CURSOR_X/Y`).

## Vídeo
- El juego escribe en **framebuffer planar** 320×200, 16 colores, 160 B/fila.
- El **renderizador** (`platform/`) lee ese framebuffer + la paleta de 16 words,
  convierte a RGBA, escala y presenta con SDL3.
- Base: `SDL_Renderer`; CRT: `SDL_GPU` + shaders (ver ADR 0002).

## Tiempo y ritmo
- El juego avanza **1 paso por frame** (`game_loop` llama a `wait_frame` una vez
  por vuelta).
- La HAL aporta el **reloj** que avanza el contador de VBL y ejecuta el tick.
- Para más fluidez: **simulación** a la cadencia del original y **presentación**
  al refresco; el cursor se repinta aparte (ver `docs/PORTING.md` §Cursor).
- El **trabajo de CPU** se modela por función (`platform/cpuwork.c`) y los
  bucles que solo esperan se anotan en el core con `cpu_delay`/
  `cpu_delay_cycles` (no-op en el diferencial). Ver `notes/0015` y `notes/0019`.
- La **presentación es por VBL** (`platform_tick`), no por iteración del bucle:
  así se ven las animaciones que corren en `wait_vbl` dentro de un comando
  (puertas, wipes). Ver `notes/0020`.

## Audio
- El juego programa el **MFP Timer A** y escribe el **PSG** por
  `hw_write8`/`psg_port_write`.
- La HAL **emula el MFP Timer A** (dispara las ISR ya reconstruidas en C) y el
  **PSG/YM2149**, y saca audio por SDL.
- Implementado (M4): `platform/mfp.c` (Timer A → vector `$134`),
  `platform/psg.c` (registros + ledger), `platform/audio.c` (YM + RMS),
  `platform/audio_sdl.c` (SDL3) y `boot/music_player.c` (reproductor `0x3a872`).

## Separación de builds
- El **core** (juego) puede compilarse aparte (ya lo hace `kult-atari-st-c` como
  librería) para las **pruebas diferenciales** sin SDL.
- El **ejecutable** enlaza core + boot + platform + app.
- **Dos rutas de build equivalentes (M7)**: el **Makefile** (`make port`,
  `make port-sdl`, `make test-port`) y **CMake** (`CMakeLists.txt`; targets
  `kult` headless y `kult-sdl`; en Windows → `kult.exe` GUI. El headless es
  solo de desarrollo y no se empaqueta).
  Ambas usan las mismas fuentes y el mismo orden de includes (`platform/include`
  el primero, sombreando los headers del kit), y dan el mismo resultado.
