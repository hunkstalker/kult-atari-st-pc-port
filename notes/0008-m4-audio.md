# Nota 0008 — M4: audio (MFP Timer A + PSG/YM2149 + SDL)

- **Sesión 8.** Cierra M4 (T-040/T-041/T-042): el juego suena. La **escucha
  final** la valida el usuario; aquí queda la verificación objetiva.

## Piezas
- **T-040 — Timer A del MFP** (`platform/mfp.c`): lee `TACR` (`$fffa19`) e
  `IERA`/`IMRA` (`$fffa07/$fffa13`) del modelo de `platform/hw.c` y calcula el
  periodo `2.4576 MHz / (prescaler(TACR&7) * TADR)`, con `prescaler =
  {parado,4,10,16,50,64,100,200}` y `TADR=0` → 256. Al expirar **resuelve el
  vector `$134` y llama a la reconstrucción C**: `0x1910e`/`0x190f6` (ISR PCM del
  core) o `0x3a872` (música). Marca ISRA bit 5 antes y la ISR lo limpia, como el
  original.
- **Reproductor de música `0x3a872` + envolvente `0x3a936`**
  (`boot/music_player.c`): NO estaba en el índice de 406 funciones (Ghidra solo
  ve el puntero del vector); es una pieza del original que falta, como `_start`,
  así que vive en `boot/` (ADR 0005). Es un tick del tracker (3 canales + canales
  enlazados por `trap #4` = `music_tick`), vuelca la tabla sombra al PSG por
  `trap #3` (`psg_trap3`) y avanza la envolvente de ruido (R6/R7).
- **T-041 — PSG/YM2149** (`platform/psg.c` + `platform/audio.c`): `psg.c` guarda
  el fichero de 16 registros y un **ledger ordenado** de accesos (escritura Y
  lectura) + `seed`. `audio.c` lo convierte en muestras: 3 ondas cuadradas,
  ruido 17 bits, mixer, volúmenes logarítmicos y envolvente aproximada, con filtro
  de continua (el PCM del juego es unipolar por niveles de volumen).
- **T-042 — salida SDL3** (`platform/audio_sdl.c`, solo `make port-sdl`):
  dispositivo mono S16 a 48 kHz con `SDL_OpenAudioDeviceStream`; el callback drena
  el anillo single-producer/single-consumer. `KULT_AUDIO=0` apaga todo.
- **Tiempo**: `platform_audio_tick` es parte de `platform_tick` (1 VBL = 20 ms
  emulados). En modo ventana/tiempo real toma el intervalo del **reloj de pared**,
  y `app/` lo bombea también en `idle` (`platform_audio_pump`) para que la música
  siga sonando mientras `Cnecin` espera una tecla en los menús. Ganancia de la
  música `0x3a872` con `KULT_MUSIC_GAIN` (por defecto 25 %).

## Verificación objetiva
- **Stream PSG contra el oráculo** (`test_music_player_golden`): se ejecuta el
  reproductor sobre una escena controlada (canales mudos + tablas conocidas) y se
  compara el ledger del port con **23 eventos capturados del oráculo Musashi**
  (`emu.run` en `0x3A872`, `stop_pc=0x3A934`, `psg_seed={7:0x3f}`). Coincide
  exacto (escrituras *y* lecturas; la lectura del RMW del mixer incluida).
- **Timer A** (`test_mfp_timer_a`): `TACR=4`/`TADR=0xaa` → **3458 µs** (~289 Hz);
  20 ms → **5 ISR**; el vector `0x3a872` produce energía.
- **Bombeo en espera** (`test_audio_pump`): 30 ms de reloj de pared con el
  reproductor armado → ISR y muestras.
- **Integración** (`test_integration_music`, si está `work/disk_a/KULT.PRG`):
  `init_hardware` deja `$134 = 0x3a872` y el Timer A armado; el bombeo de espera
  hace sonar la música.
- **RMS/energía del PCM** (`test_audio_rms`): un tono con volumen da RMS > 100 y
  pico > 200; con el mixer en 0x3f (todo desactivado) el RMS cae.
- `make port` (headless) imprime `audio: rms=… peak=… samples=… isr=… music_isr=…`.

## Notas
- La música de intro `0x3a872` solo dispone de tiempo en las **esperas** (título,
  menú de idioma): en headless el port las salta, así que `music_isr=0` ahí es
  esperado; **en ventana con entrada real suena** (verificado: con autoinput=0 el
  contador `music_isr` sube mientras el título espera una tecla). Los efectos/voz
  usan la vía PCM (`0x1910e`), que en headless sí corre.
- El modelo YM es el mínimo (no exacto); la escucha la cierra el usuario.
