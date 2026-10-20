# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 30 — H-el1 resuelta (T-081)**: las salidas anticipadas de
  `elem_command` (`0x128a4`, `$1ad0a` o clase no encontrada) ya se propagan a
  los **12 llamantes** con `elem_command_ex`/`elem_command_wrap_ex`; ningún
  llamante hace blit/reveal ni toca `A_ELEM_*` tras el aborto. El diagnóstico
  diferencial confirmó A6=`script+2` y pantalla intacta en el original, frente a
  32.640 bytes alterados por el C anterior. Tests nuevos por llamante; auditoría
  de otros `addq.l #N,a7`: solo `0x1294c`, `0x14f4a` y `0x1592e`, los tres ya
  modelados. `make test` **511**, `test-port`, nativo/Windows/ZIP OK. Exe
  Windows `c7b8dac1531acd828cf037ac8f309d2d` (headless
  `76e7974e1f52ce5eb65f9c542dec4327`, ZIP
  `787bd41d600b3d0ec19046d296d4df08`; Linux `f0f560b6…`). **Pendiente:** validar
  visualmente el punto 4 en Windows. Detalle: **`notes/0028`**.
- **Sesión 29 — deuda H-el1 documentada (T-081)**: el glitch gráfico del punto 4
  apunta a `reveal_pixel` (`0x12f28`) ejecutándose tras una salida anticipada de
  `elem_command` (`0x128a4`, `addq.l #4,a7; rts`). El C actual no propaga el
  aborto y sigue revelando con `A_ELEM_*` viejo (log: `12f28` 4471, `12fb6`
  hasta 4919). **No se ha tocado `src/`**: queda documentado el alcance de los
  12 llamantes, el diseño del arreglo y el plan de pruebas/auditoría en
  `notes/0028-retornos-dobles-128a4.md` (T-081). Detalle: **`notes/0028`**.
- **Sesión 28 — cursor del sistema (T-080)**: se veía el cursor de Windows
  encima del cursor del juego. `platform/video_sdl.c` oculta el cursor del
  sistema con `SDL_HideCursor()` al abrir/recuperar foco y lo muestra al perder
  foco/cerrar; dentro de la ventana solo queda el cursor de `src/cursor.c`.
  `src/` intacto; `make port-sdl` y `package-win` OK. Exe
  `3237a4d7cc6d1a82423a2b95e017f814` (headless
  `68ae05eaddcb6c33f4b66da437576aa3`, ZIP
  `ad7d30dded0c1afe2fee2d4417fb5258`). Detalle:
  `notes/0027-cursor-sistema-sdl.md`.
- **Sesión 27 — estrella final (T-079, punto 2)**: la animación ya duraba bien
  (3 elementos, `15da6` delay 20 = 5 VBL cada uno = 15 VBL), pero faltaba la
  espera posterior. En el ASM no hay espera programada: tras `159e2` se llama
  `load_fond` (`0x13568`), que lee `0x7d22` B (32.034) de `FOND.PI1`; el ST
  bloquea en Fread y la estrella queda en pantalla. El port leía al instante y
  el `room_reset` la tapaba. Fix **solo HAL**: `platform_disk_delay(bytes)` en
  `pc_os_fread`, estimando disquete ST DD (~1,4 s por 7 pistas + búsqueda/motor
  ≈ **2,0 s**, 62,5 µs/B; usuario: >1,4 s). No-op en headless/diferencial;
  `test_disk_delay`. `src/` intacto; 499 + `test-port`; nativo/win/ZIP OK.
  Exe `ed17588978ec5560e2024f1fa995624b` (headless
  `68ae05eaddcb6c33f4b66da437576aa3`, ZIP
  `c27a193581bec5e613af547d76b7c209`). El modelo es **global a cada Fread**:
  salas (`load_scena`), reinicio (`CLEAR.BIN`) y estrella. **Excepción**: el
  flag está a 0 desde PRES/DRAP hasta después de ECHE (incluye listas A/B y
  FOND); así el título sale sin pantalla negra, la canción empieza desde el
  principio y no hay ~20 s de pausa tras el idioma (`notes/0026`). Detalle:
  `notes/0025-espera-carga-fond.md` y `notes/0026-esperas-de-disco.md`.
- **Sesión 26 — ritmo de la muerte, punto 1 (T-077)**: sangre y viaje espacial
  iban acelerados porque `room_reset` (`0x181f0`) e `icon_scale_loop`
  (`0x1834a`) tenían coste 0 en `cpuwork_costs.inc`. La sangre se cobra **por
  línea** en `reveal_line` (`0x13d0e`) con la fórmula medida del oráculo
  **1063 + 198 × pasos** (pasos = max(|dx|,|dy|)+1); `181f0` se deja a 0 porque
  cobrarlo al entrar era un retardo previo, no repartido. El viaje espacial
  cobra `1834a` **2.301.836/101 ≈ 22.790 ciclos/pasada** (CPU del `icon_scale`
  menos las 101 esperas de VBL). Tabla regenerada (80 funciones con coste).
  `src/` intacto; 499 + `test-port` verdes. Exe nuevo
  `b7edc93660ea7caadd87191f8ffeee7a` (headless `7faac3d3…`, ZIP
  `97d56789…`). **Validado en Windows**: sangre perfecta; viaje espacial
  `1834a` 2373–2473 = 101 VBL (101 pasadas del ASM, ~2,02 s). Detalle:
  `notes/0023-costes-sangre-espacio.md`.
- **Sesión 25 — reinicio tras morir (T-076)**: al morir se repetía la secuencia
  de muerte. El original termina `reset_clear` (`0x1682a`) con `bra.w 0x10158`
  (inicio del bucle); el C nativo solo hacía la parte de estado y **retornaba**.
  Fix **solo `boot/` + hook**: al salir de `reset_clear`, `kult_request_restart()`
  hace `longjmp` a un `setjmp` armado y `boot_enter_game()` reproduce `0x10158`
  (espera clic, modo 7, `0xc001`, Maestro/objeto, `game_loop`). `src/` intacto;
  diagnóstico `KULT_RESTART_AT`; 499 + `test-port` verdes. Exe nuevo
  `b7edc93660ea7caadd87191f8ffeee7a` (headless `7faac3d3…`, ZIP `97d56789…`).
  Detalle: `notes/0022-reinicio-tras-muerte.md`.
- **Sesión 24 — cursor 16×16 y máximo IKBD**: el cursor reaparecía por el lado
  contrario al llegar al borde. Causa: el juego configura el ratón absoluto del
  IKBD con `Ikbdws(0x19, 0x1a459, 4)` → 5 bytes `09 01 30 00 b8`, es decir
  máximo **304×184** (320−16, 200−16); el port lo ignoraba y recortaba a
  319×199. Fix **solo HAL**: `platform_ikbdws()` parsea el comando 0x09,
  `platform_mouse_set()` recorta a ese máximo y `boot/boot.c` lo llama tras
  `crt0_init`. `src/` intacto; nueva prueba `test_ikbd_mouse_max`; 499 +
  `test-port` verdes. Exe nuevo `b7edc93660ea7caadd87191f8ffeee7a`
  (headless `7faac3d3…`, ZIP `97d56789…`). Detalle:
  `notes/0021-cursor-e-ikbd.md`.
- **Sesión 23 — input y presentación (HAL)**: el bug de puertas/A-B-C no era de
  `src/`: el core deriva acción y skip del bit0 del IKBD y nuestra HAL lo tenía
  invertido (izquierdo = acción+skip; derecho ignorado). Se corrige en la HAL,
  como el legacy: acción = izquierdo **o** derecho; skip = solo derecho;
  `platform_input_apply_skip()` pisa `$1a570` al salir de `wait_frame` (hook de
  instrumentación). Además, `platform_tick` ahora presenta en **cada VBL** (antes
  solo en arranque/CPU-delay), por lo que las animaciones de puerta que corren en
  `wait_vbl` ya se ven. **`src/` intacto**; 499 + `test-port` verdes; nueva
  guardia `test_click_skip`. Exe nuevo `b7edc93660ea7caadd87191f8ffeee7a`
  (headless `7faac3d3…`, ZIP `97d56789…`). Detalle:
  `notes/0020-input-hal-y-presentacion.md`.
- **Sesión 22 — T-073c resuelto (timing de C)**: la causa era el `dbf` inline de
  `object_info_card` (`0x16dec`): el guion de C deja `wait=0x0a`, que produce
  **40960 iteraciones × 18 ciclos ≈ 92 ms ≈ 4,6 VBL por entrada**; el core lo
  transcribía como bucle C normal y no lo anotaba como espera. Fix **con permiso
  del usuario para tocar `src/`**: `src/loop.c` usa `cpu_delay_cycles()`; el
  diferencial sigue no-op y el port cobra ciclos por el mismo reloj de T-073b.
  Prueba nueva `test_cpu_charge_cycles`. `make test` **499**, `make test-port`
  OK, builds nativo/win y ZIP. Exe nuevo
  `fc4108e473a9d44c722245345a44fa32` (headless `e37f65…`, ZIP `8c9266…`).
  C pasa de ~3 a ~7,6 VBL/frame; A/B no usan `16dec`. Falta la validación
  visual del usuario. Detalle: `notes/0019-fix-timing-c.md`
  (resumen en `notes/0017` §8). *(El input se corrigió en la sesión 23; ver
  `notes/0020`.)*
- **Sesión 21 — guardrails T-073c**: reglas duras en `AGENTS.md` (§ T-073c),
  prueba de guardia **`test_left_click_skips`** en `make test-port` (el clic
  izquierdo salta; el derecho no) y plan detallado
  **`notes/0018-plan-c.md`**. La próxima sesión debe leer
  `AGENTS.md`/`notes/0017`/`notes/0018`, **no cambiar el input** y **medir antes
  de tocar** (una hipótesis, un cambio; si no mejora, revertir). `make test`
  499 y `make test-port` verdes. *(La regla de input se corrigió en la sesión
  23: `test_click_skip`; ver `notes/0020`.)*
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
  C queda en ~7,6 VBL/frame; A/B no usan `16dec`. Detalle:
  **`notes/0019-fix-timing-c.md`** (resumen en `notes/0017` §8).
- **Input/presentación (sesión 23, HAL)**: izquierdo/teclado = acción sin skip;
  derecho = acción + skip. La HAL pisa `$1a570` al salir de `wait_frame` (hook),
  sin tocar `src/input.c`. `platform_tick` presenta en cada VBL, así se ven las
  animaciones de puerta que corren en `wait_vbl`. Detalle:
  **`notes/0020-input-hal-y-presentacion.md`**.
- **Cursor (sesión 24, HAL)**: la HAL emula `Ikbdws` 0x09 y recorta el ratón al
  máximo que configura el juego (**304×184**), no a 319×199. El sprite 16×16 ya
  no reaparece por el lado contrario. `src/` intacto; `test_ikbd_mouse_max`.
  Detalle: **`notes/0021-cursor-e-ikbd.md`**.
- **Reinicio tras morir (sesión 25, `boot/`)**: el original salta de
  `reset_clear` a `0x10158`; el port lo emula con `longjmp` y
  `boot_enter_game()`, así vuelve el Maestro/objeto en vez de repetir la muerte.
  `src/` intacto; `KULT_RESTART_AT` para diagnóstico. Detalle:
  **`notes/0022-reinicio-tras-muerte.md`**.
- **Ritmo de muerte (punto 1, sesión 26)**: `reveal_line` (`13d0e`) cobra por
  línea (1063 + 198 × pasos) e `icon_scale_loop` (`1834a`) 22.790/pasada;
  `room_reset` (`181f0`) se deja a 0 para no meter un retardo previo. **Validado
  en Windows**: sangre perfecta y espacio 101 VBL (2373–2473). Detalle:
  **`notes/0023-costes-sangre-espacio.md`**. Detalle de la espera de la
  estrella y del alcance global de las esperas de disco:
  **`notes/0025-espera-carga-fond.md`** y **`notes/0026-esperas-de-disco.md`**.
  Pendientes: animaciones de `object_info_card` (punto 3) y glitches gráficos
  (punto 4).
- **H-el1 resuelta (T-081, sesión 30)**: las salidas anticipadas de
  `elem_command` (`0x128a4`) se propagan a los 12 llamantes; `reveal_pixel` ya
  no ejecuta reveal ni usa `A_ELEM_*` viejo tras el aborto. Tests por llamante y
  auditoría en **`notes/0028-retornos-dobles-128a4.md`**. Queda la validación
  visual del punto 4 en Windows.
- **M7 build hecho** (T-070) y **empaquetado interino hecho** (T-071): ZIP
  Windows **`build-win/kult-0.1.0-win64.zip`** (`kult.exe` +
  `kult-headless.exe` + `SDL3.dll` + `data/` + `LEEME.txt` +
  `DIAGNOSTICO.txt`) y TGZ Linux; **sin assets**. **Log**: `kult.log` junto al
  exe (ver `notes/0013`).
- **Build estándar actual**: exe Windows `c7b8dac1…`, sin traza `Y`/`V`; la
  traza (`platform/dispatch_trace.c`) es diagnóstica y va aparte. `src/` solo
  cambia en `elem_command`/`reveal`/`scale`/`handlers`/`screens` (T-081) y la
  anotación de timing de `src/loop.c`; ambas autorizadas. `make test` **511** y
  `make test-port` verdes. Las capturas de A/B/C quedan en
  `temp/screenshots/` (no versionadas); no se requiere capturar más.
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
**Validar visualmente el punto 4** (glitches 4516/4640/4767) con el ZIP nuevo
y, después, el **punto 3** (`object_info_card`/`16dec` en 4905–5205).

1. En Windows, descomprimir `build-win/kult-0.1.0-win64.zip` sobre la carpeta
   de datos y ejecutar `kult.exe`; comprobar que el reveal de la estrella /
   elemento ya no deja artefactos tras una salida anticipada de `elem_command`.
2. Si hay glitches, capturar `kult.log` y una traza Y/V; no usar
   `KULT_FRAME_GATE` ni parches en la HAL.
3. Pasar al **punto 3** (`object_info_card`/`16dec`).
4. Cerrar **T-061** (validación Windows: título/idioma, partida/colores,
   guardado, audio, morir → reiniciar) → **T-072** (ingesta `.STX`) →
   **M7-c** → **A1** → **N3** → **A2** → **M5**.

Detalle autoritativo: **`notes/0028-retornos-dobles-128a4.md`** (H-el1),
**`notes/0017-retomar-maestro.md`** (§8), **`notes/0023`**, **`notes/0025`** y
**`notes/0026`**. No usar `KULT_FRAME_GATE`/`15da6` como fix de C.

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
