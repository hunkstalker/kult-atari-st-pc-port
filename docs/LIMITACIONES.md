# LIMITACIONES — estado de M6

Lista de limitaciones conocidas de la compilación PC, para el cierre de **M6**
(pulido y validación). El objetivo de M6 es **jugable de principio a fin**; la
**validación final** (jugar y escuchar) la hace el usuario con su copia de datos.

## Intencionales (no son fallos)
- **El FDC no se emula.** La comprobación de disco `0x19316` se **omite**; se
  siembra el patrón FDC en `0xd8e8` para que el modo de escena (`0x14496`) no
  espere (T-023).
- **`os_super` es un no-op** (en PC no hay modo supervisor).
- **Menú de idioma fiel** (ADR 0004): se mantiene el del original (clic de título
  y tecla D/E/F), no un menú nativo.
- **Audio aproximado.** El YM2149 es un modelo mínimo (no ciclo-exacto): 3 ondas
  cuadradas + ruido + envolvente + volúmenes logarítmicos. La **escucha final** la
  cierra el usuario. Ajustes: `KULT_AUDIO=0` (apaga), `KULT_MUSIC_GAIN` (nivel de
  la música, 25 % por defecto).
- **Música de intro solo en las esperas.** En `headless` el port salta el título y
  el menú de idioma, así que `music_isr=0`; en ventana con entrada real la música
  suena mientras el título espera una tecla.
- **Escucha/ventana requieren SDL3 y un equipo con display+audio.** En un
  contenedor sin `/dev/snd`/`DISPLAY` no suena (y el ejecutable cae a headless).

## Pendientes / futuros
- **M5 — filtros CRT** (`SDL_GPU` + shaders): **extra opcional, no prioritario**
  (ver `notes/0009`). Sin filtros hoy (se usa `SDL_Renderer` + escalado entero).
- **Reinicio en caliente (`0x106f2`/`l_0101a8`)**: analizado y **diferido**. Solo
  se alcanza por un puntero relocalizado (`+0x101a8`) y no es observable en el
  flujo actual (ver `notes/0010`). Pendiente de conectar si la partida lo ejerce.
- **M7 — empaquetado**: CMake y builds Windows pendientes; hoy solo `make` en
  Linux (`build/kult`, `build/kult-sdl`).
- **Workstream Amiga (A)**: no iniciado.
- **Guardado/partida completa**: los servicios de fichero son reales (Fcreate/
  Fwrite); falta la validación de una partida completa por el usuario.

## Cómo validar (resumen; detalle en `docs/VERIFICACION.md`)
1. `make test` (499) y `make test-port` en verde.
2. `make port-sdl` y `KULT_WINDOW=1 KULT_DATA_DIR=<datos> ./build/kult-sdl`:
   comprobar título (clic), menú D/E/F, intro/partida, efectos y música.
3. `make port` headless para arranque/estabilidad (`frames`, `fb_nonzero`,
   `audio: rms/peak/...`).
