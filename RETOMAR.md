# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 22 — T-073c resuelto (timing de C)**: la causa era el `dbf` inline de
  `object_info_card` (`0x16dec`): el guion de C deja `wait=0x0a`, que produce
  **40960 iteraciones × 18 ciclos ≈ 92 ms ≈ 4,6 VBL por entrada**; el core lo
  transcribía como bucle C normal y no lo anotaba como espera. Fix **con permiso
  del usuario para tocar `src/`**: `src/loop.c` usa `cpu_delay_cycles()`; el
  diferencial sigue no-op y el port cobra ciclos por el mismo reloj de T-073b.
  Prueba nueva `test_cpu_charge_cycles`. `make test` **499**, `make test-port`
  OK, builds nativo/win y ZIP. Exe nuevo
  `fc4108e473a9d44c722245345a44fa32` (headless `e37f65…`, ZIP `8c9266…`).
  C pasa de ~3 a ~7,6 VBL/frame; A/B no usan `16dec`; input intacto. Falta la
  validación visual del usuario. Detalle: `notes/0019-fix-timing-c.md`
  (resumen en `notes/0017` §8).
- **Sesión 21 — guardrails T-073c**: reglas duras en `AGENTS.md` (§ T-073c),
  prueba de guardia **`test_left_click_skips`** en `make test-port` (el clic
  izquierdo salta; el derecho no) y plan detallado
  **`notes/0018-plan-c.md`**. La próxima sesión debe leer
  `AGENTS.md`/`notes/0017`/`notes/0018`, **no cambiar el input** y **medir antes
  de tocar** (una hipótesis, un cambio; si no mejora, revertir). `make test`
  499 y `make test-port` verdes.
- **Sesión 20 — documentación corregida (T-073c, pantallas A/B/C)**: se
  reescribieron el handoff (`notes/0017-retomar-maestro.md`) y el histórico
  (`notes/0016-maestro-handoff.md`), y se corrigieron `TODO`, `PROYECTO`,
  `notes/0015`, `README`, `AGENTS`, `docs/PROMPT_SESION`, `assets/MEDIR_FPS` y
  `assets/kult-marcas.bat`.
  - **A**: 3 retratos + descripciones (pixel-reveal), quietos, **sin diálogo**.
  - **B**: 4 retratos + descripciones (lift-reveal), quietos, **sin diálogo**;
    aquí están los 4 bancos `15ac8` y el `0x36` (~1095/1352).
  - **C**: 1 retrato del Maestro + lift-reveal + **diálogo**; único diálogo y
    único retrato que se mueve; después de B. Logs: 1219–1249 / 1513–1543;
    ~8 frames/16 blits a 1–3 VBL/frame; **sin** `15ac8`/`15da6`.
  - Las capturas de las pantallas A/B/C quedan en `temp/screenshots/` (local,
    gitignored; **no se versionan**).
  - Eliminadas referencias externas obsoletas y `assets/kult-frame10.bat`.
  - Estado: commit `47e1028`; exe `900fd…`; ZIP; 499/test-port; input izquierdo;
    traza fuera del estándar. **T-073c sigue abierto**: analizar el bucle de
    frames de C y la espera/reveal que falta; ver `notes/0017`.
- **Sesiones 16–19 (histórico corregido)**: 16: trazas Y/V y click pegado;
  17: intento de cambiar el input, **rechazado**; 18: revertido y build estándar;
  19: corrección del handoff. La asociación inicial “4 bancos = Maestro” era
  errónea: esos 4 bancos son **B**; el Maestro es **C**.
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
  proyecto: se extrajeron (`stx.py`+`fat12.py`, sin tocar repos de referencia) y se copiaron
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
- **T-073c (Maestro C) resuelto, pendiente validación visual**: A/B/C
  corregidas. A: 3 retratos pixel + descripciones; B: 4 retratos lift +
  descripciones; C: 1 retrato del Maestro + lift + diálogo (único diálogo y
  único retrato que se mueve), después de B. La causa de la aceleración era el
  `dbf` inline de `0x16dec` (40960 iteraciones ≈ 4,6 VBL/entrada) sin anotar;
  ahora va por `cpu_delay_cycles()` en `src/loop.c` (no-op en diferencial).
  C queda en ~7,6 VBL/frame; A/B no usan `16dec`; input intacto. Detalle:
  **`notes/0019-fix-timing-c.md`** (resumen en `notes/0017` §8).
- **M7 build hecho** (T-070) y **empaquetado interino hecho** (T-071): ZIP
  Windows **`build-win/kult-0.1.0-win64.zip`** (`kult.exe` +
  `kult-headless.exe` + `SDL3.dll` + `data/` + `LEEME.txt` +
  `DIAGNOSTICO.txt`) y TGZ Linux; **sin assets**. **Log**: `kult.log` junto al
  exe (ver `notes/0013`).
- **Build estándar actual**: exe `fc4108…`, sin traza `Y`/`V`; la traza
  (`platform/dispatch_trace.c`) es diagnóstica y va aparte. `src/` sin cambios de
  lógica: solo la anotación de timing `cpu_delay_cycles()` en `src/loop.c`
  (autorizada). `make test` 499 y `make test-port` verdes. La documentación está
  corregida; las capturas de A/B/C quedan en `temp/screenshots/` (no
  versionadas); no se requiere capturar más.
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
**T-073c — validar visualmente el fix de C y seguir con T-061.**

0. Leer `AGENTS.md` (§ T-073c), `notes/0019` (y `notes/0017` §8/`notes/0018`).
   **No cambiar el input**; la guardia `test_left_click_skips` debe seguir verde.
1. Probar el exe nuevo `fc4108…` en Windows: la animación del retrato del
   Maestro (C) debe ir a ~7,6 VBL/frame (antes ~3); A/B deben quedar igual.
2. Si el ritmo no cuadra visualmente, calibrar con `KULT_CPU_HZ`/
   `KULT_ST_CONTEND` **sin** volver a parchear animaciones; el modelo ya cobra
   el `dbf` de `0x16dec` (18 ciclos/iteración).
3. Después **T-061** (validación Windows: título/idioma, partida/colores,
   guardado, audio, morir → reiniciar) → **T-072** (ingesta `.STX`) → **M7-c**
   → **A1** → **N3** → **A2** → **M5**.

Detalle autoritativo: **`notes/0017-retomar-maestro.md`** (§8, fix aplicado).
No cambiar el input; no usar `KULT_FRAME_GATE`/`15da6` como fix de C.

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
- No tocar los repos de referencia (solo lectura).
