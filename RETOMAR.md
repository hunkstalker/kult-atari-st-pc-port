# RETOMAR — handoff de sesión

Punto de entrada al retomar. **Mantener al día al cerrar cada sesión**: estado
exacto, siguiente tarea, comandos y trampas. Contexto maestro: `PROYECTO.md`.
Tareas: `TODO.md`.

## Última sesión
- **Sesión 54 — core importado como subtree (2026-10-24)**: el core C vive en
  `lib/kult-core/`, importado con `git subtree` desde `kult-atari-st-c`. El
  build (`Makefile`, CMake, tests, tools) compila desde el subtree; se retiraron
  las copias locales de `src/`, `include/`, `tools/recreate_kit` y
  `work/names.txt`. El delta propio del port (diagnóstico y shim de puerta) está
  en 7 ficheros del core y documentado en `docs/CORE_DELTA.md`. `make test` 519,
  `make test-port` OK, `make port`, `make port-sdl` y CMake OK.
- **Sesión 53 — T-061 cerrada y validada en Windows (2026-10-23)**: el usuario
  da por validada la versión de Windows. Criterio de cierre: el C sigue los
  mismos caminos/comportamiento que el ASM **a nivel de experiencia del
  jugador**; no se exige timing ciclo-exacto. Checklist M6 completo: título,
  idioma, colores, partida/controles, guardado/carga, audio y puertas,
  muerte→reinicio, combate/PSI, final (`attract`) y SOLAR EYES. El hueco de audio
  de SOLAR EYES queda como artefacto de timing del original, fuera de alcance;
  T-073 sigue como feature opcional. **M6 cerrada.** Siguiente: **T-072**
  (ingesta automática `.STX`/`.ST`/`.MSA`). Sin cambios de código en esta sesión.
- **Sesión 52 — T-090: SOLAR EYES cerrada y documentada (sin commit)**:
  la habilidad es el poder PSI **SOLAR EYES** (`0xa00a` → `f_014f5e` →
  `palette_cycle`, `0x16310`). El ASM hace por pasada `wait_vbl` + `dbra d0`
  con `d0=0xffff` + `set_palette`; el C omitía los dos `dbra`. Es una
  divergencia ASM↔C real y se corrigió solo el timing en `lib/kult-core/src/video.c` con
  `cpu_delay(0x10000u)` en fase 1 y fase 2. Replay nuevo/generico desde un save
  fuera de combate y dos reproducciones deterministas. Medición: original
  (oráculo, misma paleta) **7.879.396 ciclos ≈985 ms**, 12 pasadas; port
  **12 VBL (240 ms) → 60 VBL (1,2 s)** con el mismo frame de entrada (4983).
  Nueva `test_palette_cycle_solar_eyes_timing`; `make test` **519**,
  `test-port` OK; builds nativo/Windows/ZIP. Evidencia en
  `build-win/kult-0.1.0-win64/trazas-t090-20261023/` (replay, save, logs,
  vídeos, audio de Hatari y del port, gdb, oráculo, `SHA256SUMS`). Detalle:
  **`notes/0041`**. El hueco de audio de Hatari (~21 ms) se investigó y queda
  documentado como artefacto de timing del original (sample backward + retrigger
  forward), **fuera del alcance de T-090**: no es una divergencia ASM↔C. A 8 MHz
  el port es más rápido y no lo reproduce; a `KULT_CPU_HZ=5000000` dura 84 VBL
  (1,68 s) y aparece. Eso queda como **T-073 (feature opcional de
  calibración/fidelidad)**, no como tarea pendiente. T-061 se cerró en la
  sesión 53 (validación de Windows).
- **Sesión 51 — handoff para T-090: animación de habilidad del jugador acelerada**:
  T-089 está cerrado y validado (`32730fc`); T-061 estaba abierta en ese
  momento (se cerró en la sesión 53). La próxima sesión debe abordar una
  **animación acelerada que forma parte de las habilidades del personaje del
  jugador**. Hay que identificarla, capturarla con un replay nuevo y genérico,
  medirla contra el original y corregirla solo con evidencia. Herramienta ya
  preparada: `assets/kult-replay-generic.bat` (y copia en el paquete), con
  `record|play <carpeta-save> [replay.bin]`. Detalle operativo completo en
  **`T-090 — SOLAR EYES`**, más abajo.
- **Sesión 50 — T-089: final del juego (bucle de `attract`) implementado y validado**:
  el original **no vuelve al título**: tras la última secuencia entra en
  `attract` y se queda en `l_018c80` (`0x18c80`) llamando indefinidamente a
  `f_01834a` con `D5=0x64` (estrellas). El C retornaba de `attract` tras la
  primera pasada y el guion seguía, dejando escena/paleta corruptas. Evidencia
  en Hatari: el juego ejecuta una copia relocalizada `+0xB018`; la pila da
  `0x23CA2 = 0x18C8A + 0xB018`, retorno del `bsr f_01834a` dentro de
  `l_018c80`. Fix en `lib/kult-core/src/screens.c`: `attract_once` + `attract_loop` +
  `attract` (no retorna). `make test` **518** y `test-port` OK; builds
  nativo/Windows/ZIP. **Validado visualmente por el usuario en Windows
  (2026-10-22)**: se queda en las estrellas, sin escena/paleta corruptas.
  Detalle: **`notes/0040-t089-final-attract-loop.md`**.
- **Sesión 49 — handoff para analizar el final del juego**:
  incidencia de T-061: con `save-endgame` + `kult-replay-endgame.bin`, tras la
  escena final el port hace `reset_clear -> 0x10158` y muestra una escena
  corrupta con la paleta rota; el original debería volver al título con su
  música principal. Evidencia preservada en
  `trazas-endgame-20261022-125714/` (replay, log, save y `SHA256SUMS`). Próxima
  sesión: **solo análisis de divergencia** (Hatari/oráculo + traza del port),
  sin fix. No crear T-089 hasta confirmar. Detalle:
  **`notes/0039-handoff-endgame-titulo.md`**.
- **Sesión 48 — T-061 reabierta: falta la partida completa**:
  el usuario confirma que la pasada visual actual no tiene glitches y que los
  bugs corregidos (T-073c…T-088) formaban parte de la validación de Windows. El
  checklist base queda validado; T-061 quedó abierta en ese momento (se cerró en
  la sesión 53). Faltaba jugar de
  principio a fin, reproducir y corregir los bugs que aparezcan. El antiguo
  “glitch gráfico aislado” se retira como resto del punto 4 (H-el1), ya cubierto
  por T-081; no hay un segundo caso. Siguiente: **T-061** (partida completa) →
  **T-072**. T-073 sigue abierta como calibración fina no bloqueante. Detalle:
  **`notes/0028`** §14.
- **Sesión 47 — T-088 validado en Windows y cerrado**:
  el usuario confirmó que el crash de la ficha al atacar a un personaje está
  resuelto en Windows usando el replay. Con esto T-088 queda cerrado. Causa:
  `object_info_card` no avanzaba `p` tras `text_block_build`; el ASM `0x129d6`
  deja A5 al final del bloque de 3 bytes. Fix `p += 3` en `lib/kult-core/src/loop.c`; test
  `test_fun_16dec_text_block`; `make test` **517** y `test-port` OK. Detalle:
  **`notes/0038`** §9–§11. Siguiente: T-061 (validación jugada) → T-072.
- **Sesión 46 — T-088: causa raíz corregida y replay validado**:
  `object_info_card` no avanzaba `p` tras `text_block_build`; el ASM `0x129d6`
  deja A5 al final del bloque de 3 bytes. El bucle releía el bloque y llegaba al
  patrón inválido `idx=9`. Fix en `lib/kult-core/src/loop.c` (`p += 3`), test
  `test_fun_16dec_text_block`; `make test` **517** y `test-port` OK. El replay
  nativo ya no hace SIGSEGV: consume todo el fichero y sigue esperando entrada.
  Logs: `trazas-t088-crash-20261021-224446/` (`replay-fixed.log`). Build:
  `kult.exe` `154122ef…`, `kult-diag.exe` `da7afa3b…`, ZIP `76c5ee01…`.
  Pendiente: validación visual en Windows con el replay. Detalle:
  **`notes/0038`** §9–§10.
- **Sesión 45 — T-088: replay grabado, reproducido y crash localizado**:
  el usuario grabó la run con `kult-replay-record.bat`; `kult-replay.bin` se
  reproduce en Linux nativo y termina en SIGSEGV, igual que en Windows. GDB:
  `elem_bitmap_build` (`lib/kult-core/src/element.c:186`) -> `elem_list_build` ->
  `object_info_card`; patrón `idx=9` (`0x1a2ac`) con `width=1` y scratch
  `height=210` dispara `dest` por encima de 1 MiB. Replay y save preservados en
  `build-win/kult-0.1.0-win64/trazas-t088-crash-20261021-224446/`.
  SHA256 del replay `d9b86974…`. Pendiente: corregir la causa raíz y validar con
  el replay. Detalle: **`notes/0038`** §8.
- **Sesión 44 — T-088: replay de entrada implementado; pendiente grabar el crash**:
  el usuario confirmó que el fallo al atacar a un PJ es un **crash** (la ventana
  desaparece). Añadido `KULT_REPLAY_REC` / `KULT_REPLAY_PLAY` en `platform/input.c`:
  graba las respuestas de cada sondeo (IKBD con cursor x/y, `Crawio` y `Cnecin`)
  y las reproduce en orden. Bats `kult-replay-record.bat` / `kult-replay-play.bat`.
  Test `test_replay_input` en `make test-port`. Save preservado en
  `build-win/kult-0.1.0-win64/save-t088/SCENA.BIN`; logs viejos archivados en
  `trazas-antes-t088-20261021-222057/`. Siguiente: el usuario graba el crash con
  el save y se analiza el replay contra ASM. Detalle: **`notes/0038`**.
- **Sesión 43 — T-087 validado y cerrado (dos caminos A7/unwind)**:
  la run de validación del usuario (2026-10-21 22:21 local) muestra
  `RUN_UNWIND4D` en VBL 2939 tras `e1fc -> 0x4d/f1c2`, sin `loop_reopen`, y
  `c357` en 3184 con clase `01ab30` en 3448. Después, `01aa86` muere con
  `c1d2`/`c1d7`/`c357` (5700/5772/5930). Logs archivados en
  `build-win/kult-0.1.0-win64/trazas-post-fix-20261021/validacion/` con `README.txt` y
  `SHA256SUMS`. Documentación de cierre: **`notes/0037-dos-caminos-unwind-a7.md`**;
  detalle técnico en **`notes/0036`** §11–§13. `make test` **516** y
  `test-port` OK. Salvedad: la run manual valida el camino indirecto; el
  directo `b002/f2f8` queda cubierto por test automático, sin reejecución
  manual en esta captura. Siguiente: **T-061**/T-072.
- **Sesión 42 — T-087: segundo camino A7/unwind (`dispatch_a6`) implementado**:
  el fix de la sesión 41 cubría `fxxx` directo (`run_command_keep_sp_a6`), pero
  el último combate usa `e1fc -> byte 0x4d -> dispatch_a6 -> f1c2`. Ahora
  `dispatch_a6` dispara el mismo unwind cuando hay boundary de `run_command`;
  sin boundary conserva el retorno anidado. Traza `RUN_UNWIND4D`. Nuevo
  `test/test_a7_unwind.py` (helper C con boundary real): **516 tests** y
  `test-port` OK. Exes recompilados y copiados al paquete. **Validado en la
  sesión 43.** Detalle: **`notes/0036`** §12.
- **Sesión 41 — T-087: fix central A7/unwind implementado (validado en sesión 43)**:
  boundary `setjmp`/`longjmp` en `run_command` (`lib/kult-core/src/handlers.c`) y unwind
  central para `nibble == 0xf000` en `run_command_keep_sp_a6`
  (`lib/kult-core/src/dispatch.c`), con fallback normal si no hay boundary. Trazas
  `run_unwind`/`run_unwind_out`; `dispatch_a6` y el tail-call `c/a`/`$1abf8`
  intactos. Nuevo test diferencial `test_run_command_f000_unwind`: **515
  tests** y `test-port` OK. `kult-diag.exe` y `kult.exe` recompilados y
  copiados al paquete. **Validado en la sesión 43** (el camino indirecto en la
  run final; el directo por test automático). Detalle: **`notes/0036`** §11.
- **Sesión 40 — T-087: causa raíz A7/unwind localizada (abierto, sin fix)**:
  captura limpia con dos combates. Enemigo 1 (`01aa77`, `Attack` normal):
  muere correctamente con `c2f8 -> c1d2 -> c1d7 -> c357`. Enemigo 2
  (`01aa86`, `PSI Attack`): `b002 -> a010/f2f8` marca la clase
  `00 44 31 d9 0b`, pero en vez de la secuencia de muerte aparece
  `loop_reopen` al anillo; después `c2f8/c357` deja el recuadro negro y muere el
  jugador. **Causa raíz:** falta el retorno no local (A7) de `0x103ae` para
  comandos `0xfxxx`; `A_RUN_STACK` se guarda pero no se usa. Solución
  recomendada: unwind central en el boundary de `run_command` con
  `setjmp`/`longjmp`, no un parche en `palette_cmd`. Logs:
  `build-win/kult-0.1.0-win64/trazas-run-combates-20261021-2055/` (con
  `README.txt` y `SHA256SUMS`). Detalle completo: **`notes/0036-a7-unwind-combate.md`**.
- **Sesión 39 — handoff inicial de T-087**: sin fix y sin commit; log contaminado
  por varias runs. El protocolo antiguo quedó en **`notes/0035`**; la captura
  limpia y la causa raíz están en la sesión 40 y **`notes/0036`**.
- **Sesión 38 — T-086 resuelto y validado (selección de acción en el anillo)**:
  `icon_ring` (`0x1566a`) tenía tres `dbra d0` de `0xffff` entre los tres blits
  del icono seleccionado y el C los omitía; el cierre duraba ~1 VBL. Ahora usan
  `cpu_delay(0x10000u)` (no-op en diferencial): se ven los tres destellos
  (~12-15 VBL). Solo cambia ese timing. **Validado por el usuario**. Detalle:
  **§ T-086** y **`notes/0034`**.
- **Sesión 37 — T-085 resuelto y validado (retrato/intro acelerado)**: la
  animación es `portrait_intro` (`0x16a20`, comando `a013`). El ASM repite 11
  veces un bloque con dos `dbra d0` de `0xffff`; el C los tenía como
  `while (spin != 0xffff)`, que no ejecuta nunca el retardo. Se sustituyen por
  `cpu_delay(0x10000u)` (no-op en diferencial). La animación pasa de ~0,4 s a
  **~2,2 s** (110 VBL). Solo afecta a `portrait_intro`; **validado por el
  usuario**. Detalle: **§ T-085** y **`notes/0033`**.
- **Sesión 36 — T-084 resuelto y validado (input mantenido / clics repetidos)**:
  el IKBD original se interroga con `0x0D` y responde con **eventos desde la
  última interrogación**, no con el nivel del botón. La HAL entregaba nivel en
  cada `wait_frame`; ahora entrega un flanco de bajada una sola vez. Clic
  mantenido = un clic, soltar y pulsar = otro, derecho = acción + skip sólo en
  su bajada. `lib/kult-core/src/` intacto; tests `test_click_held`, `test_click_skip`,
  `test_crawio_queue`. **Validado por el usuario en Windows**: el anillo se abre
  y se queda abierto para elegir. Detalle: **§ T-084** y **`notes/0032`**.
- **Sesión 34 — T-083 resuelto y validado (fix + refactor)**: causa raíz en
  `icon_ring_loop` (0x15888): el tail-call/retorno doble de `0x1592e` se
  modelaba como llamada normal y el VM exterior seguía ejecutando la cabecera
  de TEMPL siguiente. Fix inicial con `run_command_keep_sp_a6` y refactor de
  tail-call explícito (`VM_TAIL_FLAG` / `icon_ring_loop_ex`); **513 tests** +
  `test-port`. Validado con `kult.exe` (dado/2.ª y mosca/1.ª) y con
  `kult-diag.exe`: el log
  `diag-logs/post-refactor-2026-10-21.log` no contiene `0x1fc2e`, `0x206b1`,
  `0x2563e` ni `0x206b5`, y muestra `TAIL_3D`/`VMTAIL a6=20726`. Ejecutables:
  `kult.exe` `41be915a…`, `kult-diag.exe` `b5e94d06…`. Pre-fix en
  `diag-logs/pre-fix-2026-10-21.log`.
### T-083 — bugs al usar objetos en las serpientes (resuelto y validado)

> **Fix + refactor validados en sesión 34.** Detalle completo: **`notes/0031`**.

**Comportamiento esperado:** primera serpiente, un objeto incorrecto no debe
hacer nada y se consume; el agua llena la sala y hace flotar un dado; el dado
abre la puerta de la segunda serpiente.

**Causa raíz:** `0x3d` (`icon_ring_loop`, `0x15888`) hace un retorno doble /
tail-call en `0x1592e` (`addq.l #4,a7` + `bra 0x103ae`). El C volvía al VM
exterior con `return script`; como el despacho anidado ya había pisado
`A_TEMPL_PTR`, el VM exterior no reconocía su antiguo fin y ejecutaba la
**cabecera** de la entrada TEMPL siguiente (longitud como opcode):

- cuchillo/1.ª: cabecera `0x1fc2e` (`0xd3`, longitud `0x34` como `call`)
  → `0x2563e` (`scale_prep`, retrato 73×182).
- dado/2.ª: cabecera `0x206b1` (`0x132`, longitud `0x11`) → `0x206b5` con
  `py=0` → índice `0xffff` → bucle `12b40`.

Los blits y `scale_prep` eran víctimas, no causa.

**Fix:** `run_command_keep_sp_a6()` devuelve el `A6` final del despacho y
`icon_ring_loop_ex()` marca el tail-call; el byte handler `0x3d` devuelve
`VM_TAIL_FLAG | A6` y `vm_run` abandona el VM exterior sin ejecutar más bytes.
`icon_ring_loop()` queda como wrapper compatible.

**Diagnóstico:** build `KULT_DIAG_TRACE=ON` (`make cmake-win-diag`) con líneas
`Y/V/U/P/B` y `D` (`VMIN/VMOUT`, `RUN`, `WALK_*`, `JMP/CALL/RET`,
`TAIL_3D/VMTAIL`).

- `kult-diag.exe`: `b5e94d0645dc3d86fee3909eb9180a47`
- `kult-diag-headless.exe`: `b178d8413167acd2b29c81146bb9c4d3`
- `kult-diag-marcas.bat` → `kult-marcas-diag.log`
- Logs en `build-win/kult-0.1.0-win64/diag-logs/`: `pre-fix-2026-10-21.log`,
  `post-fix-dado-mosca-2026-10-21.log`, `post-refactor-2026-10-21.log`.

**Validación:** `kult.exe` (dado/2.ª y mosca/1.ª) sin errores; el log
`post-refactor-2026-10-21.log` no contiene `0x1fc2e`, `0x206b1`, `0x2563e` ni
`0x206b5`, y muestra `TAIL_3D`/`VMTAIL a6=20726`. `make test` **513** y
`make test-port` OK.

**Ejecutables:** `kult.exe` `41be915acfd6eedccae459e5be014a84`,
`kult-headless.exe` `a7b180cdfdec055f70165ce2b17bec4a`.

### T-084 — input mantenido / clics repetidos (resuelto y validado)

> **Resuelto en sesión 36 y validado por el usuario. Detalle:
> `notes/0032-input-clic-mantenido.md`.**

**Síntoma:** mantener el clic izquierdo producía clics continuos; el anillo de
acciones se abría/cerraba y no se podía elegir.

**Causa confirmada (ASM + Hatari):** `wait_frame` (`0x13698`) hace
`Ikbdws(0x19, 0x1a44e, 0)`, es decir envía el comando IKBD **0x0D
(INTERROGATE MOUSE POSITION)**. El byte en `0x1a44e` es `0x0d`; la respuesta
`F7 + 5 bytes` la entrega TOS al handler `0x13686`, que copia botones/x/y a
`0x1a44f`. El byte de botones son **eventos "desde la última interrogación"**
(bits de bajada/subida), no un nivel: un botón mantenido sólo produce el bit de
bajada una vez. Hatari (`IKBD_Cmd_ReadAbsMousePos`) implementa ese mismo
diferencial contra `PrevReadAbsMouseButtons`. El pestillo legacy
(`g_click_hold=12`, `g_click_latch`) es una aproximación empírica, pero su
semántica de "una pulsación = un clic" sí es fiel; el timeout de 12 VBL no sale
del ASM.

**Fix (sólo `platform/`):** `platform_mouse_set()` detecta flancos de bajada de
izquierdo/derecho; `platform_ikbd_deliver()` entrega la acción (bit0) una sola
vez y consume el flanco (0 si sigue pulsado). El derecho además arma el skip de
esa bajada; `platform_input_apply_skip()` lo pisa en `$1a570`. Mantener
izquierdo/derecho ya no repite acción ni skip. `lib/kult-core/src/` intacto.

**Teclado:** `pc_os_crawio()` avanza `g_head` **una** vez (la sospecha de doble
pop era de un estado anterior); prueba nueva de dos teclas = dos `Crawio`. Se
mantiene `e.key.repeat`: TOS también auto-repite (`kb_initial`/`kb_repeat`), así
que filtrarlo sería menos fiel; no es la causa del anillo.

**Validación:** `make test` **513** y `make test-port` OK. En el diag nativo SDL
bajo Xvfb+xdotool, 5 s con el botón mantenido producen **un solo flanco de
entrada**; el bucle de 32 frames de `test_click_held` no ve clics nuevos.
**Validado visualmente por el usuario en Windows**: el anillo de acciones se
abre una vez con el clic mantenido y queda abierto para elegir. Ejecutables:
`kult-diag.exe` `64505805…`, `kult.exe` `cc4d5cee…`, headless `414a82c8…`.

### T-085 — retrato/intro acelerado (resuelto y validado)

> **Resuelto en sesión 37 y validado por el usuario. Detalle: `notes/0033`.**

**Síntoma:** una animación de retrato/intro duraba <1 s cuando debía durar >2 s.

**Causa:** `portrait_intro` (`0x16a20`, comando `a013`) repite 11 veces un bloque
con **dos `dbra d0` de `0xffff`**. El C los transcribió como
`uint32_t spin = 0xffff; while (spin != 0xffff) ...`, una condición falsa desde
el inicio: nunca se ejecutaba el retardo y se perdían ~14,4 M ciclos (~1,8 s).
El diferencial no podía verlo porque solo compara imagen/estado.

**Fix (solo timing en `lib/kult-core/src/loop.c`):** los dos bloques pasan a
`cpu_delay(0x10000u)` (65536 iteraciones), no-op en diferencial y con coste real
en el port. No se toca `15da6`, `15ac8`, input ni otras animaciones.

**Pruebas:** `test_fun_16a20` sigue verde; guardia en `test_cpu_delay` para
`0x10000` unidades (>=4 VBL); medición funcional **110 VBL / 2202 ms**.
`make test` **513** y `make test-port` OK.

**Ejecutables:** `kult.exe` `5342b77c…`, `kult-headless.exe` `ccf5f7a9…`,
`kult-diag.exe` `62710162…`.

### T-086 — selección de acción en el anillo (resuelto y validado)

> **Resuelto en sesión 38 y validado por el usuario. Detalle: `notes/0034`.**

**Síntoma:** al seleccionar una acción en el anillo, el icono elegido apenas se
veía; el cierre pasaba al comando siguiente casi en el mismo VBL.

**Causa:** `icon_ring` (`0x1566a`) tiene tres blits del icono seleccionado
separados por **tres `dbra d0` de `0xffff`**; el C tenía los blits pero no los
retardos (~82 ms cada uno).

**Fix (solo timing en `lib/kult-core/src/commands.c`):** añadir `cpu_delay(0x10000u)` tras
cada blit. No se toca el cuadro de texto, el anillo, `15da6`, `15ac8` ni el
input.

**Auditoría:** los únicos `dbra 0xffff` de espera son `portrait_intro` (2, T-085),
`icon_ring` (3, este fix) y `palette_cycle` (2, documentados y no tocados).

**Pruebas:** diferencial **513** y `test-port` OK; guardia de
`cpu_delay(0x10000u)` en `test-port`. **Validado visualmente por el usuario**:
se ven los tres destellos al seleccionar acciones.

**Ejecutables:** `kult.exe` `4fc2cfa2…`, `kult-headless.exe` `8c222aff…`,
`kult-diag.exe` `6628f205…`.

### T-087 — A7/unwind de comandos `0xfxxx` (resuelto y validado)

> **Sesión 43: cerrado.** Los dos caminos que llegan a `0x103ae` están
> corregidos y el usuario validó la run final. Resumen:
> **`notes/0037-dos-caminos-unwind-a7.md`**; técnico: **`notes/0036`** §11–§13.**

**Síntoma confirmado con captura limpia:** el `PSI Attack` ejecuta la animación
de muerte y marca la clase (`byte0=0`, bit6), pero no dispara la secuencia de
muerte; el anillo se reabre. Un `Attack` posterior deja el recuadro negro del
enemigo vivo y el jugador muere.

**Traza:** enemigo normal `01aa77`: `c2f8` (nibble `c`) -> `loop_tail_ac` ->
`c1d2`/`c1d7`/`c357`; muerte correcta. Enemigo `01aa86`: `b002` ->
`a010`/`f2f8` (nibble `f`) -> clase `00 44 31 d9 0b` en VBL 5926 ->
`loop_reopen` al anillo `20c88`; no hay `c1d2`/`c1d7`/`c357`.

**Causa:** en el ASM, `0x103a8` guarda A7 en `$1a826` y `0x103ae`, para
`0xfxxx`, restaura A7 y hace tail-jump al VM; el `rts` del VM vuelve al llamante
de `run_command`, saltándose `palette_cmd`/`b002`/anillo. El port guardaba
`A_RUN_STACK` pero no lo leía, así que el anillo se reabría.

**Fix (sesiones 41–42):** boundary con `setjmp` en `run_command` y unwind
central en dos entradas: `run_command_keep_sp_a6` para `nibble == 0xf000`
(sesión 41) y `dispatch_a6` para el `fxxx` que forma el byte `0x4d` (sesión 42).
Fuera de boundary se conserva el retorno normal/anidado. Trazas
`run_unwind`/`run_unwind_out`/`RUN_UNWIND4D`. No se toca el tail-call `c/a` ni
`$1abf8`. Tests: `test_run_command_f000_unwind` y `test_a7_unwind`; `make test`
**516** y `test-port` OK. Ejecutables en el paquete: `kult.exe` `be2cd45a…`,
headless `3e44ba7d…`, `kult-diag.exe` `16605768…`,
`kult-diag-headless.exe` `5065fcc1…`.

**Validación (sesión 43):** la run del usuario confirmó el fix indirecto:
`e1fc -> 0x4d/f1c2` -> `RUN_UNWIND4D` en VBL 2939, sin `loop_reopen`; `c357` en
3184 y clase `01ab30` en 3448. Después, `01aa86` muere con
`c1d2`/`c1d7`/`c357` (5700/5772/5930). Logs en
`build-win/kult-0.1.0-win64/trazas-post-fix-20261021/validacion/`. El camino directo
`b002/f2f8` no se reejecutó manualmente en esta captura; queda cubierto por
`test_run_command_f000_unwind`.

**Seguimiento:** auditar los demás handlers con restauración de A7
(`0x14ee8`, `0x14f74`, `0x15052`, `0x151b0`, `0x152ee`, `0x15382`, `0x15530`,
`0x1699c/a4/b4`). No parchear el anillo por clase ni solo `palette_cmd`.

**Logs de referencia:** `build-win/kult-0.1.0-win64/trazas-run-combates-20261021-2055/`
(`dano-todo-*`, `kult.log`, `README.txt`, `SHA256SUMS`). Logs antiguos:
`.../trazas-antes-run-combates-20261021-2035/`.

### T-088 — Crash al atacar a un personaje (resuelto y validado en Windows)

> **Sesión 47: cerrado y validado en Windows.** El crash era independiente de
> T-087 y ocurría en la ficha del personaje (`object_info_card`, `0x16dec`).
> Detalle: **`notes/0038-replay-input-t088.md`** §9–§11; evidencia:
> **`notes/0037`** §8.

- **Replay:** `KULT_REPLAY_REC=<ruta>` graba cada respuesta de entrada
  (paquete IKBD con botón/skip/cursor x/y, `Crawio` y `Cnecin`);
  `KULT_REPLAY_PLAY=<ruta>` la reproduce e ignora los eventos reales de SDL.
  Cada evento se vuelca con `fflush`, así que el final se conserva aunque el
  proceso muera.
- **Bats:** `kult-replay-record.bat` y `kult-replay-play.bat` (en `assets/` y en
  el paquete).
- **Save:** `save-t088/SCENA.BIN` (74.524 B = `0x291c + 2×0x7d00`). `CLEAR.BIN`
  no vale como save: el arranque lo reescribe.
- **Prueba:** `test_replay_input` en `make test-port`.
- **Estado:** **resuelto y validado en Windows**. `object_info_card` no avanzaba
  `p` tras `text_block_build`; el ASM `0x129d6` deja A5 al final del bloque de
  3 bytes. El bucle releía el bloque y llegaba al patrón inválido `idx=9`. Fix
  en `lib/kult-core/src/loop.c` (`p += 3`), test `test_fun_16dec_text_block`; `make test` 517
  y `test-port` OK. El replay nativo consume todo el fichero sin SIGSEGV y el
  usuario confirmó el fix en Windows. Artefactos en
  `trazas-t088-crash-20261021-224446/` (`replay-fixed.log`).

- **Sesión 32 — audio de puertas (T-082)**: log 88310: apertura 1192, sample
  del tope `0x1f` en 1321 y `sound_stop` en 1322; faltaba el trabajo de CPU
  previo a `sound_stop` (~242.030 ciclos medidos con el oráculo). Fix en
  `lib/kult-core/src/mode.c`: la espera se movió **al final de `textform_fwd`**, antes de que
  la transición pinte/cargue la sala siguiente, hasta que el puntero de sample
  (`0x192c6`) alcanza su fin (`0x192ca`). Así la sala actual sigue visible
  mientras suena el tope. **Validado por el usuario.** `lib/kult-core/src/` solo cambia como
  espera de audio; tests 511, build/ZIP OK. Exe `bd3764b9…`, headless
  `0fdda9c8…`, ZIP `1396dd00…`.
  **Validado por el usuario.** Detalle: `notes/0029-audio-puertas.md`.
- **Sesión 31 — documentación: retirado `PROMPT_SESION.md`**: era un duplicado
  de `RETOMAR`/`AGENTS`/`PROYECTO`/`TODO` y había derivado. El punto de entrada
  único es **`RETOMAR.md`**; reglas en `AGENTS.md`, contexto en `PROYECTO.md` y
  tareas en `TODO.md`. Se actualizan las referencias.
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
  `787bd41d600b3d0ec19046d296d4df08`; Linux `f0f560b6…`). **Validado
  visualmente por el usuario** (el glitch del punto 4 desaparece). Detalle:
  **`notes/0028`**.
- **Sesión 29 — deuda H-el1 documentada (T-081)**: el glitch gráfico del punto 4
  apunta a `reveal_pixel` (`0x12f28`) ejecutándose tras una salida anticipada de
  `elem_command` (`0x128a4`, `addq.l #4,a7; rts`). El C actual no propaga el
  aborto y sigue revelando con `A_ELEM_*` viejo (log: `12f28` 4471, `12fb6`
  hasta 4919). **No se ha tocado `lib/kult-core/src/`**: queda documentado el alcance de los
  12 llamantes, el diseño del arreglo y el plan de pruebas/auditoría en
  `notes/0028-retornos-dobles-128a4.md` (T-081). Detalle: **`notes/0028`**.
- **Sesión 28 — cursor del sistema (T-080)**: se veía el cursor de Windows
  encima del cursor del juego. `platform/video_sdl.c` oculta el cursor del
  sistema con `SDL_HideCursor()` al abrir/recuperar foco y lo muestra al perder
  foco/cerrar; dentro de la ventana solo queda el cursor de `lib/kult-core/src/cursor.c`.
  `lib/kult-core/src/` intacto; `make port-sdl` y `package-win` OK. Exe
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
  `test_disk_delay`. `lib/kult-core/src/` intacto; 499 + `test-port`; nativo/win/ZIP OK.
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
  `lib/kult-core/src/` intacto; 499 + `test-port` verdes. Exe nuevo
  `b7edc93660ea7caadd87191f8ffeee7a` (headless `7faac3d3…`, ZIP
  `97d56789…`). **Validado en Windows**: sangre perfecta; viaje espacial
  `1834a` 2373–2473 = 101 VBL (101 pasadas del ASM, ~2,02 s). Detalle:
  `notes/0023-costes-sangre-espacio.md`.
- **Sesión 25 — reinicio tras morir (T-076)**: al morir se repetía la secuencia
  de muerte. El original termina `reset_clear` (`0x1682a`) con `bra.w 0x10158`
  (inicio del bucle); el C nativo solo hacía la parte de estado y **retornaba**.
  Fix **solo `boot/` + hook**: al salir de `reset_clear`, `kult_request_restart()`
  hace `longjmp` a un `setjmp` armado y `boot_enter_game()` reproduce `0x10158`
  (espera clic, modo 7, `0xc001`, Maestro/objeto, `game_loop`). `lib/kult-core/src/` intacto;
  diagnóstico `KULT_RESTART_AT`; 499 + `test-port` verdes. Exe nuevo
  `b7edc93660ea7caadd87191f8ffeee7a` (headless `7faac3d3…`, ZIP `97d56789…`).
  Detalle: `notes/0022-reinicio-tras-muerte.md`.
- **Sesión 24 — cursor 16×16 y máximo IKBD**: el cursor reaparecía por el lado
  contrario al llegar al borde. Causa: el juego configura el ratón absoluto del
  IKBD con `Ikbdws(0x19, 0x1a459, 4)` → 5 bytes `09 01 30 00 b8`, es decir
  máximo **304×184** (320−16, 200−16); el port lo ignoraba y recortaba a
  319×199. Fix **solo HAL**: `platform_ikbdws()` parsea el comando 0x09,
  `platform_mouse_set()` recorta a ese máximo y `boot/boot.c` lo llama tras
  `crt0_init`. `lib/kult-core/src/` intacto; nueva prueba `test_ikbd_mouse_max`; 499 +
  `test-port` verdes. Exe nuevo `b7edc93660ea7caadd87191f8ffeee7a`
  (headless `7faac3d3…`, ZIP `97d56789…`). Detalle:
  `notes/0021-cursor-e-ikbd.md`.
- **Sesión 23 — input y presentación (HAL)**: el bug de puertas/A-B-C no era de
  `lib/kult-core/src/`: el core deriva acción y skip del bit0 del IKBD y nuestra HAL lo tenía
  invertido (izquierdo = acción+skip; derecho ignorado). Se corrige en la HAL,
  como el legacy: acción = izquierdo **o** derecho; skip = solo derecho;
  `platform_input_apply_skip()` pisa `$1a570` al salir de `wait_frame` (hook de
  instrumentación). Además, `platform_tick` ahora presenta en **cada VBL** (antes
  solo en arranque/CPU-delay), por lo que las animaciones de puerta que corren en
  `wait_vbl` ya se ven. **`lib/kult-core/src/` intacto**; 499 + `test-port` verdes; nueva
  guardia `test_click_skip`. Exe nuevo `b7edc93660ea7caadd87191f8ffeee7a`
  (headless `7faac3d3…`, ZIP `97d56789…`). Detalle:
  `notes/0020-input-hal-y-presentacion.md`.
- **Sesión 22 — T-073c resuelto (timing de C)**: la causa era el `dbf` inline de
  `object_info_card` (`0x16dec`): el guion de C deja `wait=0x0a`, que produce
  **40960 iteraciones × 18 ciclos ≈ 92 ms ≈ 4,6 VBL por entrada**; el core lo
  transcribía como bucle C normal y no lo anotaba como espera. Fix **con permiso
  del usuario para tocar `lib/kult-core/src/`**: `lib/kult-core/src/loop.c` usa `cpu_delay_cycles()`; el
  diferencial sigue no-op y el port cobra ciclos por el mismo reloj de T-073b.
  Prueba nueva `test_cpu_charge_cycles`. `make test` **499**, `make test-port`
  OK, builds nativo/win y ZIP. Exe nuevo
  `fc4108e473a9d44c722245345a44fa32` (headless `e37f65…`, ZIP `8c9266…`).
  C pasa de ~3 a ~7,6 VBL/frame; A/B no usan `16dec`. **Validación visual
  posterior: confirmada; commit `3235aa6`.** Detalle: `notes/0019-fix-timing-c.md`
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
  `notes/0015`, `README`, `AGENTS`, `assets/MEDIR_FPS` y
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
  headless. **T-073b**: modelo **medido** por función: `lib/kult-core/src/*.c` con
  `-finstrument-functions` solo en el port, `platform/cpuwork.c` cobra ciclos/
  llamada y `platform/cpuwork_costs.inc` (79 funciones; generado por
  `tools/gen_cpuwork_costs.py` desde la referencia del oráculo + `lib/kult-core/docs/mapa` +
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
  `FOND.PI1` y frame correcto. `lib/kult-core/src/` intacto; 499 + `test-port` verdes. También
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
  Se corrige la deriva de docs (`AGENTS.md`, `PROYECTO.md`, `BUILD.md`,
  `LIMITACIONES.md`).
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
- **M6 cerrada**: T-060 ✅ (menú fiel); **T-061 ✅ validada en Windows
  (2026-10-23)** con el checklist M6 completo (partida de principio a fin,
  guardado/carga, audio, muerte→reinicio, combate/PSI, final T-089 y T-090).
  Criterio: mismos caminos/comportamiento ASM↔C a nivel de experiencia del
  jugador; no se exige timing ciclo-exacto. **T-089 ✅** (final en `attract`) y
  **T-090 ✅** (SOLAR EYES) cerrados. **T-073** queda como feature opcional de
  calibración fina (CPU/ritmo), no bloqueante. Incluye **morir → reiniciar sala**
  (`reset_clear`, core). Siguiente: **T-072** (ingesta `.STX`/`.ST`/`.MSA`).
- **T-073c (Maestro C) resuelto y validado (commit `3235aa6`)**: A/B/C
  corregidas. A: 3 retratos pixel + descripciones; B: 4 retratos lift +
  descripciones; C: 1 retrato del Maestro + lift + diálogo (único diálogo y
  único retrato que se mueve), después de B. La causa de la aceleración era el
  `dbf` inline de `0x16dec` (40960 iteraciones ≈ 4,6 VBL/entrada) sin anotar;
  ahora va por `cpu_delay_cycles()` en `lib/kult-core/src/loop.c` (no-op en diferencial).
  C queda en ~7,6 VBL/frame; A/B no usan `16dec`. Detalle:
  **`notes/0019-fix-timing-c.md`** (resumen en `notes/0017` §8).
- **Input/presentación (sesión 23, HAL)**: izquierdo/teclado = acción sin skip;
  derecho = acción + skip. La HAL pisa `$1a570` al salir de `wait_frame` (hook),
  sin tocar `lib/kult-core/src/input.c`. `platform_tick` presenta en cada VBL, así se ven las
  animaciones de puerta que corren en `wait_vbl`. Detalle:
  **`notes/0020-input-hal-y-presentacion.md`**.
- **Cursor (sesión 24, HAL)**: la HAL emula `Ikbdws` 0x09 y recorta el ratón al
  máximo que configura el juego (**304×184**), no a 319×199. El sprite 16×16 ya
  no reaparece por el lado contrario. `lib/kult-core/src/` intacto; `test_ikbd_mouse_max`.
  Detalle: **`notes/0021-cursor-e-ikbd.md`**.
- **Reinicio tras morir (sesión 25, `boot/`)**: el original salta de
  `reset_clear` a `0x10158`; el port lo emula con `longjmp` y
  `boot_enter_game()`, así vuelve el Maestro/objeto en vez de repetir la muerte.
  `lib/kult-core/src/` intacto; `KULT_RESTART_AT` para diagnóstico. Detalle:
  **`notes/0022-reinicio-tras-muerte.md`**.
- **Ritmo de muerte (punto 1, sesión 26)**: `reveal_line` (`13d0e`) cobra por
  línea (1063 + 198 × pasos) e `icon_scale_loop` (`1834a`) 22.790/pasada;
  `room_reset` (`181f0`) se deja a 0 para no meter un retardo previo. **Validado
  en Windows**: sangre perfecta y espacio 101 VBL (2373–2473). Detalle:
  **`notes/0023-costes-sangre-espacio.md`**. Detalle de la espera de la
  estrella y del alcance global de las esperas de disco:
  **`notes/0025-espera-carga-fond.md`** y **`notes/0026-esperas-de-disco.md`**.
  La estrella final (punto 2) está **validada por el usuario** (commit
  `b599cf9`). El glitch gráfico del punto 4 está **resuelto y validado**
  (T-081); no hay un punto 3 independiente: 4905 era el mismo reveal de H-el1.
- **H-el1 resuelta (T-081, sesión 30)**: las salidas anticipadas de
  `elem_command` (`0x128a4`) se propagan a los 12 llamantes; `reveal_pixel` ya
  no ejecuta reveal ni usa `A_ELEM_*` viejo tras el aborto. Tests por llamante,
  auditoría y validación visual en **`notes/0028-retornos-dobles-128a4.md`**.
- **M7 build hecho** (T-070) y **empaquetado interino hecho** (T-071): ZIP
  Windows **`build-win/kult-0.1.0-win64.zip`** (`kult.exe` +
  `kult-headless.exe` + `SDL3.dll` + `data/` + `LEEME.txt` +
  `DIAGNOSTICO.txt`) y TGZ Linux; **sin assets**. **Log**: `kult.log` junto al
  exe (ver `notes/0013`).
- **Build actual (sesión 52)**: `kult.exe` `35b18ae0…`, `kult-headless.exe`
  `71a0e96c…`, `kult-diag.exe` `938f0268…`, `kult-diag-headless.exe`
  `e246c2ce…` (en `build-win/kult-0.1.0-win64/`); ZIP `02a571d5…`. La traza
  `Y`/`V` sigue siendo diagnóstica y va aparte. `make test` **519** y
  `make test-port` OK. El working tree conserva la instrumentación de
  diagnóstico de sesiones anteriores **sin commit**. Las capturas de A/B/C
  quedan en `temp/screenshots/` (no versionadas).
- **Colores del juego corregidos** (sesión 14): `OS_SCREEN_BASE = 0x98000` en
  `platform/include/os.h` (Physbase es HAL); con `0x80000` las pantallas y su
  paleta caían dentro de ECHE/heap del juego. **Ritmo en observación**: el port
  va a 50 VBL/s correctos; la aceleración es trabajo de CPU no modelado → T-073.
  Detalle: `notes/0014`.
- **M2 con `0x106f2` abierto** (no cerrado): ver `notes/0011`.
- `make test` **519** verde; `make test-port` verde. Sin bloqueantes.
- **Plan de hitos fijado** (sesión 11): `notes/0011`; build (sesión 12):
  `notes/0012`; datos/log (13): `notes/0013`; Physbase/ritmo (14): `notes/0014`.

## T-090 — SOLAR EYES (flash PSI acelerado): cerrado

**Resultado (sesión 52):** la habilidad es **SOLAR EYES** (`0xa00a` →
`f_014f5e` → `palette_cycle`, `0x16310`). El ASM hace por pasada `wait_vbl` +
`dbra d0` con `d0=0xffff` + `set_palette`; el C omitía los dos `dbra`. Fix solo
de timing en `lib/kult-core/src/video.c` con `cpu_delay(0x10000u)` en las dos fases. El
oráculo mide **7.879.396 ciclos ≈985 ms** (12 pasadas); el port pasa de
**12 VBL (240 ms)** a **60 VBL (1,2 s)** con el mismo frame de entrada (4983).
Evidencia en `build-win/kult-0.1.0-win64/trazas-t090-20261023/`; detalle en
**`notes/0041`**. `make test` **519** y `test-port` OK; builds nativo/Windows/ZIP.
**Cerrado.** El hueco de audio de Hatari es un artefacto de timing del original
y queda fuera de T-090; T-061 se cerró en la sesión 53.

**Objetivo original:** reproducir, medir y corregir una animación que forma
parte de las **habilidades del personaje del jugador** y que en el port iba
acelerada.

**Estado de partida**
- T-089 ✅ cerrado y validado en Windows (`32730fc`): el final se queda en el
  bucle infinito de `attract` (`0x18c80`), no vuelve al título. Ver
  `notes/0040-t089-final-attract-loop.md`.
- T-061 ✅ **cerrada y validada en Windows (2026-10-23)**: checklist M6 completo;
  el C sigue los mismos caminos/comportamiento que el ASM a nivel de
  experiencia del jugador.
- T-073 🧩 **feature opcional** de calibración de CPU/ritmo/fidelidad
  (FPS/`KULT_CPU_HZ`/`KULT_ST_CONTEND`, ISR/VBL); no bloqueante y no es una
  tarea pendiente del core ASM↔C.
- T-072 (**Ingesta automática de `.STX`/`.ST`/`.MSA` a `data/`**) es la
  siguiente tarea.

**Datos ya resueltos**
- Habilidad: **SOLAR EYES**, poder PSI del jugador, fuera de combate; consume
  2 de energía (`$1ad20`: 0x3f → 0x3d).
- Entrada: replay nuevo/generico desde un save fuera de combate
  (`save-t090`, copia de save-t088); dos reproducciones deterministas.
- Duración: la del ASM es 12 pasadas de `dbra 0xffff`; el oráculo da 984,9 ms
  de ciclos para esa paleta. El port actual queda ~1,2 s por el ajuste de VBL.
  No se fuerza el ~2 s percibido por el usuario: manda el ASM.

**Replay genérico (ya preparado)**
- `assets/kult-replay-generic.bat` (copia en `build-win/kult-0.1.0-win64/`).
- Uso:
  ```bat
  kult-replay-generic.bat record <carpeta-save> [replay.bin]
  kult-replay-generic.bat play   <carpeta-save> [replay.bin]
  ```
- Copia `SCENA.BIN`/`CLEAR.BIN` de `<carpeta-save>` a `data\` si existen,
  define `KULT_REPLAY_REC`/`KULT_REPLAY_PLAY` y `KULT_FPSLOG=0`, y lanza
  `kult.exe` del mismo directorio. No es exclusivo de T-088 ni de endgame.
- Formato/semántica del replay: `notes/0038-replay-input-t088.md` (cabecera
  `KULTREP1`, eventos IKBD/Crawio/Cnecin; graba respuestas a cada sondeo, no
  eventos crudos de SDL).
- Para trazas, usar `kult-diag.exe` con `KULT_MARKS`, `KULT_RINGLOG`,
  `KULT_INPUTLOG`, `KULT_DBG_RUN`, `KULT_DBG_CMDS` y `KULT_LOG` (ver
  `docs/BUILD.md`).

**Plan de captura**
1. Guardar la partida justo antes de usar la habilidad.
2. Grabar con el bat genérico y usar la habilidad; cerrar.
3. Reproducir dos veces y comprobar que llega al mismo punto (determinismo).
4. Repetir con `kult-diag.exe` y trazas; archivar en
   `trazas-t090-<fecha>/`: replay, copia de `SCENA.BIN`/`CLEAR.BIN`, `kult.log`,
   `kult-marcas.log`, `kult-ring.log`, `kult-input.log`, `README.txt` y
   `SHA256SUMS`.

**Plan de análisis**
- Medir VBLs/frames de la animación en original y port (`KULT_FPSLOG`, marcas de
  entrada/salida de la rutina).
- Identificar la rutina/PC en ASM
  (`/app/kult-atari-st-decomp/matching/src`) y en el port (`KULT_DBG_RUN`,
  `KULT_DBG_CMDS`, `KULT_MARKS`/`KULT_RINGLOG`).
- Revisar `platform/cpuwork_costs.inc` y
  `tools/cpuwork/funcost_st_reference.txt`: si la función no está o tiene coste
  0, es candidata (patrón T-073).
- Buscar esperas `dbra d0` de `0xffff` transcritas como bucle normal (patrón
  T-085/T-086); medir antes de tocar.
- Comparar estado/paleta en RAM (`A_SAVE_DESC`, screen A/B, `A_PALETTE_PTR`), no
  solo capturas.
- Ojo con la relocalización: en T-089 el original ejecutaba copia `+0xB018`;
  las direcciones estáticas del ASM pueden no ser las de ejecución.

**Criterios de cierre**
- ✅ Replay reproducible (dos pasadas al mismo comando y misma bajada de PSI).
- ✅ Causa raíz y fix medidos (oráculo 984,9 ms; port 12 → 60 VBL); sin gates.
- ✅ `make test` **519** y `make test-port` verdes.
- ✅ Builds nativo/Windows/ZIP actualizados.
- ✅ Documentado (`notes/0041`) y actualizado TODO/RETOMAR/VERIFICACION.
- ✅ T-090 cerrado; el hueco de audio queda documentado y fuera de alcance.
- ✅ La validación visual global se cerró con **T-061** (sesión 53).

**Trampas**
- No usar `KULT_FRAME_GATE` ni parches por animación sin medición.
- No tocar `lib/kult-core/src/` sin permiso explícito; una hipótesis, un cambio; revertir si
  no mejora.
- No confundir esta tarea con T-073: si es falta de coste de CPU, se mide y se
  modela, no se “acelera/decelera a ojo”.

**Después:** **T-072** (ingesta automática `.STX`/`.ST`/`.MSA`) → **M7-c** →
**A1** →
**N3** → **A2** → **M5**. **T-073** (calibración opcional de CPU/ritmo) queda
como feature opcional, no bloqueante.

**Seguimiento técnico de T-087:** auditar los demás handlers con restauración
de A7 (`0x14ee8`, `0x14f74`, `0x15052`, `0x151b0`, `0x152ee`, `0x15382`,
`0x15530`, `0x1699c/a4/b4`); no parchear el anillo por clase ni solo
`palette_cmd`.

T-084 (input mantenido), T-085 (retrato/intro) y T-086 (selección de acción)
están **cerrados y validados**: detalle en **`notes/0032`**, **`notes/0033`** y
**`notes/0034`**.
T-083 (objetos en serpientes) está **cerrado y validado**; detalle en
**`notes/0031`**.

## Comandos
- Core/diferencial: `make test` (518 core + HAL + port), `make test-core`,
  `make test PYTEST_ARGS=-n0`, `make regscan` (`ids_reg=0`), `make core`.
- Port: `make port` (→ `build/kult`, headless de desarrollo), `make port-sdl`
  (→ `build/kult-sdl`, ventana SDL3), `make test-port` (HAL, sin oráculo).
- Ejecutar headless: `./build/kult` (usa `build/data` y escribe en
  `build/saves`).
- Con ventana: `./build/kult-sdl` (la ventana se abre por defecto;
  `KULT_SCALE=3`, ESC para salir; `KULT_WINDOW=0` fuerza headless). Sin display:
  `SDL_VIDEODRIVER=dummy`.
- **Ritmo (T-073)**: `KULT_CPU=0` desactiva el modelo; `KULT_CPU_HZ` (def.
  8 MHz), `KULT_ST_CONTEND` (%) y `KULT_ANIM_GATE`/`KULT_REVEAL_GATE` calibran.
  `KULT_FPSLOG=1` imprime el FPS del bucle cada ~3 s (comparar con Hatari en la
  misma escena). Regenerar la tabla:
  `make cmake-native && python3 tools/gen_cpuwork_costs.py`.
- **CMake (M7)**: `make cmake-native` (→ `build-cmake/kult`, `kult-sdl`),
  `make cmake-win` (→ `build-win/kult.exe` GUI + `kult-headless.exe` de
  desarrollo), `make package-win` (ZIP solo con `kult.exe`) y
  `make package-native` (TGZ). El cross necesita SDL3 MinGW en `third_party/`
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
- `app/main.c`: `KULT_PRG` (si no, se busca `KULT.PRG` en `data/`),
  `KULT_FRAMES` (0 = sin límite), `KULT_DUMP` (en ventana ya no se vuelca PPM
  por defecto), `KULT_PLANAR`, `KULT_KEYS`/`KULT_KEY`, `KULT_SKIP_DRAP`,
  `KULT_DEBUG`; en ventana: `KULT_SCALE`, `KULT_REALTIME=0`,
  `KULT_AUTOINPUT=1`, `KULT_AUDIO=0`; `KULT_WINDOW=0` fuerza headless (en la
  build SDL la ventana es el modo por defecto). `data/`, `saves/` y `kult.log`
  viven junto al ejecutable.
- **M7 (build/empaquetado)**: `CMakeLists.txt` + `cmake/mingw-w64.cmake`,
  icono `assets/` (`.rc`, RGBA con `SDL_SetWindowIcon`) y CPack (ZIP/TGZ).
  Detalle: `notes/0012`, `docs/BUILD.md`.
- **Diagnóstico (M7)**: `platform/log.c` escribe **`kult.log` junto al exe** (y
  stderr) con timestamp. Política B: sin MessageBox normal; solo diálogo de
  emergencia si no se puede escribir el log. El ZIP no lleva headless ni
  `DIAGNOSTICO.txt`. Sin `KULT_LOG`. Fallo de ventana y datos incompletos van al
  log. Detalle: `notes/0013`.
- Datos de prueba: **hay que juntar** `disk_a`+`disk_b` (el juego necesita
  además `FOND.PI1`, `PUZZL.BIN`, `ECHE*.BIN`… que están en `disk_b`). Los
  recursos "empotrados" (CARAC/TEMPL/ICONE/ARPLA…) van **dentro del PRG**.

## Decisiones cerradas (no re-litigar; ver `docs/adr/`)
- **SDL3** para ventana/entrada/audio/GPU.
- **`SDL_Renderer`** para lo básico; **`SDL_GPU` + shaders** solo para CRT.
- **Audio** en hito propio (no al principio).
- **Menú de idioma**: mantener el fiel (espera de tecla) al inicio.
- **`boot/`** fuera de `lib/kult-core/src/` (no se toca el core verificado).
- **Headless primero**, luego ventana SDL.

## Trampas / notas
- `lib/kult-core/src/` (el juego) **no se modifica**; todo lo de la máquina va en `platform/`.
- El arranque `_start` (0x10000) y el tick `0x160c2` **no existen en C** en el
  core; se reconstruyen aquí.
- El juego avanza **1 paso por frame**; separar simulación (ritmo del original)
  de presentación (refresh) para no acelerar animaciones.
- No tocar los repos de referencia (solo lectura).
