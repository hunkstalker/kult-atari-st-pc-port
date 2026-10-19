# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 18 — corrección: se rechaza el cambio de botón de la sesión 17**: se
  elimina `platform/input_core.c`; el port vuelve a compilar `src/input.c` y el
  **clic izquierdo** sigue saltando transiciones/reveal; el **botón derecho
  queda sin función** (decisión del usuario). Se conserva el fix de click pegado
  (bit2 de `A_INPUT_BUTTONS` sincronizado con el botón real) y el hallazgo útil
  de la sesión 17: los brazos del Maestro aparecen **después del primer prompt**
  (el primer `0x36 1d41b` es el scene draw inicial). El exe `296ce6…` y los docs
  que daban el botón derecho por bueno quedan **invalidados**; se reconstruye el
  exe estándar y el ZIP. T-073c sigue abierto: `15da6`/segundo scene draw.
- **Sesión 17 — T-073c (Maestro): causa raíz y fix del skip del reveal (RECHAZADA; ver sesión 18)**: el
  reveal/lift del retrato comprueba `$1a570`; el `wait_frame` del core lo fijaba
  con el **clic izquierdo**, así que cada clic para avanzar saltaba el reveal.
  En el original `$1a570` sigue al **botón derecho** y el izquierdo solo
  interactúa (referencia: `kult-port-legacy/docs/RUNTIME.md` §"flag de saltar
  transición"). **Fix port-side** (`src/` intacto):
  - `platform/input_core.c` (copia justificada de `src/input.c`): izquierdo →
    bit2/interacción; `$1a570` → `platform_input_skip_requested()` (derecho).
  - `platform/input.c` + `platform/video_sdl.c`: estado del botón derecho de SDL
    (`SDL_BUTTON_RMASK`), refresco al final del bombeo.
  - `Makefile`/`CMakeLists.txt`: `src/input.c` excluido **solo del port**;
    entra `platform/input_core.c`. `make test` sigue con `src/input.c`.
  - Prueba nueva en `make test-port`: izquierdo → `skip=0`; derecho → `skip=0xff`.
  - Verificado: **499**, `test-port`, `port`, `port-sdl`, `cmake-native`,
    `cmake-win`. En headless con clic izquierdo el Maestro entra con `skip=00`
    y hace scene draw **53 VBL** + lift `0x06` **61 VBL**; con el derecho
    mantenido en SDL (Xvfb) el `skip` es `ff`.
  - **Build estándar sin traza** regenerado en `build-win/`: `kult.exe` md5
    **`296ce6c689c9237097cfb5f454eaf787`**. El ZIP aún no se ha regenerado.
    **Pendiente: validación visual del usuario** (izquierdo no salta, derecho
    sí); después borrar `platform/dispatch_trace.c`/marcas y empaquetar.
- **Sesión 16 — T-073c (Maestro): trazas Y/V, puerta descartada, estado y click fantasma**: se hizo una **copia diagnóstica** `platform/dispatch_trace.c` (sustituye a `src/dispatch.c` solo en el port) que escribe `Y <vbl> <byte> <script>` y `V <vbl> <byte> <script> <acc9> <acd2> <aaf9> <ad1d> <acc6> <skip> <a572> <a0f> <cx> <cy> <btn> <cmd> <obj>`.
  - Los **3 primeros retratos** usan `0x0d` → `12f28`: VBL **22/119/211** (deltas 97/92) en la run nueva frente a **29/127/219** (98/92) en la anterior; `S 15da6` del intro = **63** en ambas. **No se ha cambiado ningún otro reveal.**
  - El **Maestro** entra por `0x36 scene_mode_draw`; el operando del guion en `0x1d41c` es **0x07** (base PRG 0x10000). No llama a los reveals de tipo 4 (`12e2e/12e08/12ea2/12fb6`); los bancos `15ac8` son 4 objetos/partes (ASM `0x16582`/`0x16636`, el C coincide) y los blits `12ad6/12b04` son el dibujo del retrato, no el reveal.
  - **`KULT_FRAME_GATE` descartado**: con 10 los 4 bancos salen a 10 VBL exactos (prueba nativa), pero el frame visible sigue acelerado (run `kult-frame10.log`: 1365/1375/1385/1395; scene draw 96 VBL). La puerta solo espacia la construcción en el back buffer. El bat normal vuelve a `KULT_FRAME_GATE=0`.
  - **Estado V del Maestro**: `$1acc9=0x0a` (tras `0x52`), `$1acd2=0x02`, `$1aaf9=0x63`, `$1ad1d=0`, modo `0x07`, skip 0. `anim_mode` (`0x188ce`) exige `0x3b..0x3e` y `aaf9==acd2`; no entra. `icon_repeat` (`0x18944`) retorna si `ad1d==0`; por eso **no hay `15da6`** en el Maestro. El `$1acc9` se fija **después** del scene draw y `aaf9` no se actualiza: falta el **segundo scene draw/camino** del original tras la selección (hipótesis principal; ver `notes/0016`).
  - **Bug extra**: `A_INPUT_BUTTONS` bit2 = `0x04` **pegado** desde VBL 2 (click fantasma). Corregido en `platform/input.c` (sincroniza con el IKBD) y `platform/video_sdl.c` (refresca con `SDL_GetMouseState`); exe de traza nuevo **`3fc2f940e11db072f1d0ce65785158ee`**. **Pendiente de probar por el usuario** (run con `kult-marcas.bat`, sin mantener el botón; ver si `btn=00` y si el retrato mejora).
  - **Estado del build**: la traza sigue activa (`platform/dispatch_trace.c` + filtros Makefile/CMake); **hay que revertirla** antes del ZIP final. El ZIP estándar `build-win/kult-0.1.0-win64.zip` **no se ha regenerado**; la carpeta del usuario tiene el exe de traza. `make test` **499** y `make test-port` verdes. **Handoff completo: `notes/0016-maestro-handoff.md`.**
- **Sesión 15 — T-073 ritmo: `cpu_delay` + coste medido por función**: se
  verificó en el ASM que las animaciones aceleradas son **CPU-bound** (`dbra`
  en `0x15da6` motor de elementos/retrato, `0x12e2e`/`0x12f28` reveal, y trabajo
  de blits). **T-073a**: `cpu_delay` deja de ser no-op (`platform/include/
  kult_port.h` sombra + `platform_cpu_delay`), coste ~10 ciclos/unidad a
  `KULT_CPU_HZ` (def. 8 MHz), durmiendo y cediendo VBLs; no-op en diferencial/
  headless. **T-073b**: modelo **medido** por función: `src/*.c` con
  `-finstrument-functions` solo en el port, `platform/cpuwork.c` cobra ciclos/
  llamada y `platform/cpuwork_costs.inc` (79 funciones; generado por
  `tools/gen_cpuwork_costs.py` desde la referencia del oráculo + `docs/mapa` +
  `nm`). Excluidas esperas (`13594/13698`), `dbf` ya cobrados por `cpu_delay`
  (`15da6/12e2e/12f28`) y audio. Retardos reales de `15da6` medidos: 0/10/20/60
  → 1/3/5/13 VBLs. Efecto medido: 600 frames 26,7 s → 34,6 s. Verificado:
  **499** + `test-port` + nativo/cross + paquetes. Añadidos **`KULT_FPSLOG=1`**
  (FPS del bucle, comparable al `Ikbdws` del arnés Hatari) y
  **`KULT_ST_CONTEND`** (CPU efectiva por contención de bus; def. 100 %) para
  calibrar contra Hatari/ST real. **Corrección**: el tramo de `15da6` a 1 VBL
  era el **objeto al inventario**, no el Maestro; su cobro extra se ha
  **desactivado** (`KULT_15DA6_BLIT=0`, el objeto recupera su velocidad).
  Para identificar el Maestro sin romper nada, `KULT_MARKS` admite la marca
  manual **F12** (`M f12 <vbl>`) y líneas `S <vbl> <delay> <w> <h> <reveal>`.
  Logs ahora en **append** (`# RUN`), F12 también en `kult.log` y **abre una
  buffer de ~3 s anteriores + ventana de ~1,5 s** que traza todo el core instrumentado (una vez por función/VBL), y marcas añadidas para
  `15d7e/159e2/159d2/15ac8/138a6/13740/12ce4/12e08/12ea2`. Hay que esperar 4-5 s
  tras F12; se puede pulsar después de la animación.
  Detalle: `notes/0015`. **Siguiente: el usuario pulsa F12 al mover los brazos
  el Maestro y manda `kult-marcas.log`; localizar/medir su función (T-073c).**
- **Sesión 14 — colores del juego (Physbase) + ritmo**: el juego salía con
  colores distorsionados porque la base de pantalla del kit (`0x80000`) hacía que
  pantalla B (`0x78000`) y su paleta (`0x77fe0`) cayeran dentro de los datos del
  juego (ECHE `0x64c8e..~0x88b84` + heap). **Fix HAL**: `OS_SCREEN_BASE =
  0x98000` en `platform/include/os.h` (como el predecesor); paleta = la de
  `FOND.PI1` y frame correcto. `src/` intacto; 499 + `test-port` verdes. También
  `make port-sdl` ahora depende de `platform/include/*.h`. **Ritmo**: medido a
  50 VBL/s (20,1 ms/tick); la aceleración es el **trabajo de CPU no modelado**
  (busy-waits descartados) → **T-073** (medir ciclos con el oráculo + reloj
  virtual; no hacks por animación). **Además**, corregida la **primera pantalla**:
  no era negra intencional; es el **título (PRES.PI1)**. Faltaba aplicar la paleta
  del PI1 antes de la espera (el ST hereda la del escritorio; el HAL arranca a
  cero). `boot/boot.c` aplica `set_palette` tras copiar PRES; verificado con
  `KULT_DUMP_TITLE` (**0 diferencias** vs `PRES.PI1`). **El usuario confirma
  que el título ya se ve bien (sin negro).** Detalle: `notes/0014`.
- **Sesión 13 — desbloqueo de T-061 (datos) + log de diagnóstico**: el `.exe`
  "no hacía nada" porque en `data/` solo había **imágenes `.STX`**; el paquete
  interino necesita los ficheros **extraídos** y el `.exe` GUI no mostraba el
  error. Los `.STX` del usuario son **idénticos** (`md5`) a los discos del
  proyecto: se extrajeron (`stx.py`+`fat12.py`, sin tocar legacy) y se copiaron
  los **28 ficheros aplanados** a `build-win/kult-0.1.0-win64/data/`
  (`KULT.PRG` md5 `e4fdc6fa…`; `CLEAR.BIN` restaurado del disco). Nuevo
  **`platform/log.c`** (+`paths.c`): **`kult.log` junto al exe** y stderr,
  `platform_fatal` con **MessageBox** si no hay consola, **fallo de ventana =
  fatal** (antes bucle headless invisible) y **preflight de los 26 ficheros** del
  juego con lista clara de los que faltan. ZIP con **`kult-headless.exe`** +
  `DIAGNOSTICO.txt`; `LEEME` actualizado; prueba `test_log`. **Además**, el log
  del usuario reveló que faltaba `KULT_WINDOW`: ahora la build SDL **abre
  ventana por defecto** (`KULT_WINDOW=0` fuerza headless). Y un tercer arreglo:
  el `_start` original pide **"INSERT DISK 2"** (`0x918e`) antes de cargar los
  recursos del disco B; en el port (discos juntos en `data/`) **se omite** el
  aviso (antes colgaba esperando un clic). Y cuarto: tras elegir idioma se
  **colgaba/negro** porque SDL solo se bombeaba en `Cnecin` y en el bucle, no en
  las esperas de `wait_frame`; ahora `platform_tick` **presenta y bombea en cada
  VBL del arranque** (`platform_set_present`, apagado al entrar en `game_loop` con
  `platform_set_loop`). Verificado: **499** + `test-port` (con
  `test_present_hook`) + nativo/cross + smoke (ventana por defecto, headless
  forzado, arranque sin `0x918e`). Detalle: `notes/0013`. **Siguiente: T-061
  (usuario) y después T-072 (ingesta `.STX`).**
- **Sesión 12 — M7 build (T-070) + empaquetado interino (T-071)**: build
  **Windows/Linux con CMake** (raíz) + `cmake/mingw-w64.cmake` + SDL3 **3.4.10**
  MinGW en `third_party/` (gitignored). **Portabilidad**: `platform/time.c`
  (`_WIN32`: `QueryPerformanceCounter`+`Sleep`), `platform/fs.c` (separador `;`
  en Windows, `data/` **junto al exe**, `platform_data_resolve`), `app/main.c`
  (`KULT_PRG` se busca en el dir de datos; en ventana no vuelca PPM por defecto;
  firma `main(argc,argv)` para SDL3). Targets CMake `kult` (headless) y
  `kult-sdl`; en **Windows** → **`kult.exe`** (GUI, con icono) +
  `kult-headless.exe`. **Icono**: `assets/kult_icon.rc` +
  `assets/kult_icon_rgba.h` (generado con `tools/make_icon_rgba.py`,
  byte-idéntico al del predecesor) + `SDL_SetWindowIcon`. **Empaquetado CPack**:
  ZIP Windows (`kult.exe`+`SDL3.dll`+`data/`+`LEEME.txt`) y TGZ Linux, **sin
  assets**. Verificado: `make test` 499, `make test-port`, smoke Linux
  (Makefile y CMake coinciden) y cross OK (**sin wine**: el `.exe` lo valida el
  usuario en T-061). Detalle: `notes/0012`. **Sin bloqueantes.**
- **Sesión 11 — plan fijado y documentado (sin código)**: se acuerda y documenta
  el plan de hitos (`notes/0011`): `arreglos → M7 build → T-061 validación →
  M7-c (empaquetado+ingesta) → A1 (investigación Amiga, **prerrequisito duro** de
  N3) → N3 (motor nativo + assets nativos, ADR + rama) → A2 (Amiga) → M5
  (opcional)`. **T-022/`0x106f2`** se deja **abierto** (parece inobservable, pero
  puede necesitarse; **no** es el reinicio tras morir, que es `0x1842c`→
  `reset_clear`, ya en el core). Se copia el icono a **`assets/kult_icon.ico`**.
  Se corrige la deriva de docs (`AGENTS.md`, `PROMPT_SESION.md`, `PROYECTO.md`,
  `BUILD.md`, `LIMITACIONES.md`).
- **Sesión 10 — T-020/T-022/T-023 + M6**: **menú de idioma fiel** de `_start`
  (`boot/language.c`: drena teclas pendientes con Crawio y espera D/E/F;
  `kult_boot_apply_language` escribe las 4 entradas de la lista A). Se quita el
  atajo `MODE_FLAG=0x87`: `prompt_wait` espera el **clic real** del título y
  `l_010158` espera a **soltarlo**. `0x135ba` → `os_setscreen(-1,-1,0)`.
  `0x106f2` (reinicio) **abierto/pendiente** (solo por puntero relocalizado; ver
  notas). **M6**: T-060 (menú fiel) ✅; T-061 (validación) checklist en
  `docs/VERIFICACION.md` y `docs/LIMITACIONES.md`, **la ejecuta el usuario**.
  Prueba nueva: `test_boot_language` en `make test-port`.
- **Sesión 9 (docs)**: M5 (CRT) pasa a **extra opcional** (ver `notes/0009`).
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
- **M0–M4 hechas**: el juego arranca con el **menú de idioma fiel**, avanza frames,
  **headless**, con **ventana SDL3 jugable** (render + entrada real + ritmo 50 Hz)
  y **audio** (Timer A del MFP + PSG/YM2149 + SDL).
- **M6 en curso**: T-060 ✅ (menú fiel); **T-061 pendiente de la validación del
  usuario** (checklist en `docs/VERIFICACION.md`, limitaciones en
  `docs/LIMITACIONES.md`). Incluye **morir → reiniciar sala** (`reset_clear`, core).
- **T-073c (Maestro) EN DIAGNÓSTICO** (sesión 16): los 3 primeros reveals siguen
  igual (`0x0d`/`12f28`); el Maestro entra por `scene_mode_draw` (`0x36`, modo
  `0x07`) y **no** llama a los reveals de tipo 4. Los 4 bancos son 4
  objetos/partes (ASM `0x16582`/`0x16636`); `KULT_FRAME_GATE` **descartado**
  (solo retrasa el back buffer). Estado `V`: `$1acc9=0x0a`, `$1acd2=0x02`,
  `$1aaf9=0x63`, `$1ad1d=0`, modo 0x07; `anim_mode`/`icon_repeat` no disparan
  `15da6`. Hipótesis principal: falta el **segundo scene draw/camino** tras la
  selección. Bug extra: click fantasma `A_INPUT_BUTTONS` bit2=0x04; **fix
  aplicado** (`platform/input.c`, `platform/video_sdl.c`), exe de traza
  `3fc2f940e11db072f1d0ce65785158ee`, **pendiente de probar**. Handoff completo:
  `notes/0016-maestro-handoff.md`.
- **M7 build hecho** (T-070) y **empaquetado interino hecho** (T-071): ZIP
  Windows **`build-win/kult-0.1.0-win64.zip`** (`kult.exe` +
  `kult-headless.exe` + `SDL3.dll` + `data/` + `LEEME.txt` +
  `DIAGNOSTICO.txt`) y TGZ Linux; **sin assets**. **Log**: `kult.log` junto al
  exe (ver `notes/0013`).
- **Build de diagnóstico T-073c activo** (sesión 16): `platform/dispatch_trace.c`
  sustituye a `src/dispatch.c` en el port (Makefile/CMake) y escribe `Y`/`V`;
  **hay que revertirlo** antes del ZIP final. El ZIP estándar **no se ha
  regenerado**; la carpeta del usuario tiene el exe de traza. `src/` intacto;
  `make test` 499 y `make test-port` verdes.
- **Colores del juego corregidos** (sesión 14): `OS_SCREEN_BASE = 0x98000` en
  `platform/include/os.h` (Physbase es HAL); con `0x80000` las pantallas y su
  paleta caían dentro de ECHE/heap del juego. **Ritmo en observación**: el port
  va a 50 VBL/s correctos; la aceleración es trabajo de CPU no modelado → T-073.
  Detalle: `notes/0014`.
- **M2 con `0x106f2` abierto** (no cerrado): ver `notes/0011`.
- `make test` 499 verde; `make test-port` verde. Sin bloqueantes.
- **Plan de hitos fijado** (sesión 11): `notes/0011`; build (sesión 12):
  `notes/0012`; datos/log (13): `notes/0013`; Physbase/ritmo (14): `notes/0014`.

## Siguiente tarea
**T-073c — corrección hecha; Maestro aún abierto.** Se revirtió el cambio de la
sesión 17: el **clic izquierdo** interactúa y **salta** las transiciones/reveal;
el **botón derecho queda sin función**. Se conserva el fix de click pegado. El
exe `296ce6…` de la sesión 17 queda **invalidado**; reconstruir el estándar
izquierdo (`build-win/`) y regenerar el ZIP. El primer `0x36 1d41b` es el scene
draw inicial; los brazos aparecen **después del primer prompt**: la próxima run
debe avanzar el diálogo y pulsar F12 después. Si se necesita la traza,
reactivar `Y`/`V` en una build de diagnóstico. Después: **T-061** (validación
del usuario en Windows: título/idioma, partida/colores, guardado, audio, morir →
reiniciar). Si el Maestro sigue: atacar `15da6`/segundo scene draw (ver
`notes/0016`). No cambiar el mapeo de botones.
la traza y comparar `skip`/funciones.
Después, **T-061 — validación del usuario en Windows** con el ZIP final
(primera pantalla/título, idioma D/E/F, partida/colores, guardado, audio,
morir → reiniciar; checklist en `docs/VERIFICACION.md`). Si algo falla, adjuntar
**`kult.log`** (o `kult-headless.exe` en consola). Después **T-072 — ingesta
`.STX`** (no pedir nunca un `.PRG`) → **M7-c** → **A1** (investigación Amiga) →
**N3** (motor nativo + assets nativos) → **A2** (Amiga) → **M5** (opcional). La
**escucha del audio** se hace en ventana con entrada real (`KULT_AUDIO=0` lo
apaga; `KULT_MUSIC_GAIN` ajusta la música). Detalle:
`notes/0011`–`notes/0016`.

## Comandos
- Core/diferencial: `make test` (499), `make test PYTEST_ARGS=-n0`,
  `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (→ `build/kult`, headless), `make port-sdl` (→ `build/kult-sdl`,
  ventana SDL3), `make test-port` (HAL, sin oráculo).
- Ejecutar headless: `KULT_DATA_DIR=/tmp/kult_data ./build/kult`
  (junta `disk_a`+`disk_b` del usuario en `/tmp/kult_data`).
- Con ventana: `KULT_DATA_DIR=/tmp/kult_data ./build/kult-sdl` (la ventana se
  abre por defecto; `KULT_SCALE=3`, ESC para salir; `KULT_WINDOW=0` fuerza
  headless). Sin display: `SDL_VIDEODRIVER=dummy`.
- **Ritmo (T-073)**: `KULT_CPU=0` desactiva el modelo; `KULT_CPU_HZ` (def.
  8 MHz), `KULT_ST_CONTEND` (%) y `KULT_ANIM_GATE`/`KULT_REVEAL_GATE` calibran.
  `KULT_FPSLOG=1` imprime el FPS del bucle cada ~3 s (comparar con Hatari en la
  misma escena). Regenerar la tabla:
  `make cmake-native && python3 tools/gen_cpuwork_costs.py`.
- **CMake (M7)**: `make cmake-native` (→ `build-cmake/kult`, `kult-sdl`),
  `make cmake-win` (→ `build-win/kult.exe` GUI + `kult-headless.exe`),
  `make package-win` (→ `build-win/kult-0.1.0-win64.zip`) y
  `make package-native` (→ TGZ). El cross necesita SDL3 MinGW en `third_party/`
  (gitignored; URLs en `docs/BUILD.md`).

## M1–M7: qué hay y cómo se prueba
- `platform/include/{os,hw,psg,sched}.h` **sombrean** al kit en el build del
  ejecutable: el core incluye los mismos nombres, pero responden la HAL de PC.
  `platform/{fs,video,input,time,hw,psg,os,audio,mfp,audio_sdl}.c` los implementan.
- **Audio (M4)**: `platform/mfp.c` (Timer A: periodo de `TACR`/`TADR`, vector
  `$134`), `platform/psg.c` (registros + ledger + seed), `platform/audio.c` (YM:
  ondas/ruido/envolvente, RMS), `platform/audio_sdl.c` (solo `make port-sdl`) y
  `boot/music_player.c` (reproductor `0x3a872` + envolvente `0x3a936`).
  `KULT_AUDIO=0` apaga; `KULT_MUSIC_GAIN` (25 % por defecto) escala la música.
- `boot/prg.c` (loader GEMDOS: cabecera + relocación DRI, espejo de `loader.py`) +
  `boot/boot.c` (`_start` **fiel**: corte por `frames` con `hook_loop_top` +
  `longjmp`), `boot/language.c` (menú de idioma fiel) y `boot/vbl.c` (tick 0x160c2).
- `app/main.c`: `KULT_PRG` (si no, se busca `KULT.PRG` en el dir de datos),
  `KULT_DATA_DIR` (varios con `:` en POSIX y `;` en Windows; por defecto `data/`
  **junto al exe**), `KULT_FRAMES` (0 = sin límite), `KULT_DUMP` (en ventana ya
  no se vuelca PPM por defecto), `KULT_PLANAR`, `KULT_KEYS`/`KULT_KEY`,
  `KULT_SKIP_DRAP`, `KULT_DEBUG`; en ventana: `KULT_SCALE`,
  `KULT_REALTIME=0`, `KULT_AUTOINPUT=1`, `KULT_AUDIO=0`; `KULT_WINDOW=0` fuerza
  headless (en la build SDL la ventana es el modo por defecto).
- **M7 (build/empaquetado)**: `CMakeLists.txt` + `cmake/mingw-w64.cmake`,
  icono `assets/` (`.rc`, RGBA con `SDL_SetWindowIcon`) y CPack (ZIP/TGZ).
  Detalle: `notes/0012`, `docs/BUILD.md`.
- **Diagnóstico (M7)**: `platform/log.c` escribe **`kult.log` junto al exe** (y
  stderr); `platform_fatal` muestra MessageBox si no hay consola. En el ZIP van
  `kult-headless.exe` y `DIAGNOSTICO.txt`. `KULT_LOG=<ruta>` / `KULT_LOG=0`.
  Fallo de ventana y datos incompletos dan error visible, no crash/silencio.
  Detalle: `notes/0013`.
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
