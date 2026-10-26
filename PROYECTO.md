# PROYECTO — contexto maestro y estado

Documento raíz de **`kult-atari-st-pc-port`**. Es el punto de entrada: qué es
el proyecto, cómo se organiza, qué piezas hay y en qué estado están. Las tareas
viven en **`TODO.md`**; el punto de retorno de sesión, en **`RETOMAR.md`**; el
índice de documentación, en **`docs/INDICE.md`**.

## 1. Qué es

Port nativo a **Windows, Linux y macOS** de **Kult: The Temple of Flying Saucers**
(Exxos/ERE, 1989) que:
- parte de la versión **Atari ST**,
- corre como **C nativo sin emulador**,
- incluye **español y catalán**.

**Idea central:** el juego no habla con el Atari, sino con una **capa de
servicios**. Esa capa es lo que se sustituye por un backend de PC.

```
              ┌───────────────────────────────┐
App/driver → │  juego (lib/kult-core/src/, C verificado)   │  ← 424 funciones, 519 tests del core
              └───────────────┬───────────────┘
                              │ 19 servicios (contrato fijo)
              ┌───────────────▼───────────────┐
              │  backend PC = "nuestro TOS"   │  SDL3 + SO (ficheros, entrada,
              └───────────────────────────────┘  vídeo, audio, tiempo)
```

## 2. Repos implicados

| repo | papel | regla |
|---|---|---|
| `kult-atari-st-decomp` | decomp ASM del ST, byte a byte | spec congelada; no se toca |
| `kult-atari-st-c` | C verificado del ST | **base**; importado en `lib/kult-core` (subtree) |
| **este repo** | el port a PC (base ST) | el trabajo |

## 3. Arquitectura de capas (objetivo)

```
lib/kult-core/      core C importado de kult-atari-st-c (subtree)
                    incluye src/, include/, symbols/, test/ y tools/
boot/       _start (0x10000) y tick VBL (0x160c2)  — NUEVO (reconstrucción)
platform/   HAL PC: os_* / hw_* / psg_* / sched_* + render + audio — NUEVO
app/        main + driver + presentación           — NUEVO
assets/     icono de la app (`kult_icon.ico`), recurso del .exe — NUEVO
docs/       documentación · notes/ · adr/
```

`lib/kult-core/` es un subtree del repo C. El port mantiene un delta mínimo
(documentado en `docs/CORE_DELTA.md`); si algo más hay que cambiarlo, sería señal de que la
frontera (los 19 servicios) está mal definida.

## 4. El contrato: 19 servicios que el C pide

El C solo depende de estos servicios; **toda la máquina vive detrás de ellos**:

- **Ficheros:** `os_fopen`, `os_fclose`, `os_fcreate`, `os_fread`, `os_fwrite`,
  `os_fsfirst`, `os_fsetdta`
- **Entrada:** `os_crawio`, `os_cnecin`
- **Vídeo:** `os_setscreen`, `os_setpalette`, `os_setcolor`
- **Sistema:** `os_super`
- **Hardware:** `hw_write8`, `hw_sub8`, `hw_bclr8`
- **Sonido:** `psg_port_read`, `psg_port_write`
- **Tiempo:** `sched_poll8`

Detalle, uso y equivalente PC: **`docs/PORTING.md`**.

## 5. Piezas: reutilizar / reconstruir / implementar

### 5.1 Se reutiliza (no se toca)
- Las 424 funciones de `lib/kult-core/src/` y los 519 tests diferenciales del core.

### 5.2 Se reconstruye en C (lógica del original que falta)
| pieza | qué es |
|---|---|
| `_start` (0x10000) | arranque + **menú de idioma** (único nombre sin C) |
| `0x160c2` | tick de VBL: contador `A_DAT_1ACD4` + trabajo por frame |
| `0x135ba` | XBIOS `Setscreen(-1,-1,0)` → `os_setscreen` no-op (✅) |
| `0x106f2` | `load_resource_list_c` (lista C) + reinicio suave `l_0101a8`, **portado como ruta latente (T-022)**; solo se fuerza con `KULT_SOFT_RESTART_AT`. **No** es el reinicio tras morir (ese es `0x1842c`→`reset_clear`; su `bra 0x10158` se emula en `boot/`, ver T-076) |
| `0x19316` | comprobación FDC → **se omite** en PC |

### 5.3 Se implementa (capa de PC)
- La HAL (los 19 servicios) con **SDL3** + SO.
- **Renderizador**: framebuffer planar 320×200/16 colores → RGBA → escalado.
- **Audio**: emulación **MFP Timer A** + **PSG/YM2149** + salida SDL.
- **Tiempo/tick**: reloj que avanza el contador de VBL y ejecuta el tick.

## 6. Fases

| fase | contenido | estado |
|---|---|---|
| **M0** | repo y documentación; importar el core en `lib/kult-core/` | ✅ hecho |
| **M1** | HAL mínima **headless** + driver (sin ventana) | ✅ hecho |
| **M2** | portar `_start` y `0x160c2` | ✅ `_start` **fiel** (menú validado en T-020; `0x106f2`/`l_0101a8` portados en T-022) + tick `0x160c2` |
| **M3** | render SDL3 + entrada real (primera ventana jugable) | ✅ ventana + entrada real + ritmo 50 Hz |
| **M4** | audio (MFP Timer A + PSG + SDL audio) | ✅ hecho (T-040/041/042) |
| **M6** | pulido y validación | ✅ **cerrada**: T-060 ✅ (menú fiel); **T-061 ✅ validada en Windows (2026-10-23)**; **T-089 ✅** (final en bucle de estrellas); **T-090 ✅** (SOLAR EYES, cerrado; audio gap fuera de alcance). T-073 queda como **feature opcional** de calibración fina |
| **M7** | build/empaquetado (Windows/Linux) + ingesta + idiomas + licencias | ✅ build (T-070), paquete portable (T-071), ingesta `.STX`/`kult.data` (T-072), `language.data` con español y catalán (T-091), licencias (T-D02). T-022, T-101 y T-102–T-135 cerradas (fullscreen/F1/F2, ranuras F5/F6/F7 en `slot-N.save`, OSD con fuente original, terminología catalana T-119/T-132); release 1.0.0 publicada (T-092; commit `8eb079a`, ZIP/TGZ/AppImage); CI de release ejecutado con éxito (T-096); Linux TGZ + AppImage (T-135) |
| **M8** | **macOS Universal 2 + release 1.1.0** | 🚧 T-136 con CI verde (run 12) y artifact generado; falta QA del DMG en Mac Intel y T-137 (tag/release). Sin notarizar (ad-hoc) |
| **M9** | **cursor a refresco nativo + separación render/simulación** | ⬜ T-138 (overlay B) y T-139 (hilos, solo si hace falta) después de 1.1.0; ver `notes/0094` |
| **A1** | **investigación** Amiga (censo + decomp) | ⬜ **prerrequisito duro de N3** |
| **N3** | **motor nativo + assets nativos** (ADR + rama) | ⬜ hito futuro (tras validación) |
| **A2** | **implementación** Amiga (sobre el motor de N3) | ⬜ |
| **M5** | filtros CRT (SDL_GPU + shaders) — **nota: extra opcional, no prioritario** | ⬜ nota (al final) |

Detalle por fase: **`docs/PLAN.md`**; plan de hitos: **`notes/0011`**.

> **Orden acordado:** `arreglos (portabilidad Windows + docs) → M7 build → T-061
> (validación del usuario) ✅ → T-072/M7-c (empaquetado + ingesta + idiomas) ✅ →
> T-022 (`0x106f2` + `l_0101a8`) ✅ → T-102–T-135 (fullscreen/F1/F2/Estirar a pantalla + ranuras
> `slot-N.save` + OSD original + terminología catalana T-119/T-132) ✅ → validación final (T-094) → publicación de
> la release 1.0.0 y CI (T-092/T-096) ✅ → T-136 macOS → T-137 release 1.1.0 → T-138 cursor / T-139 hilos → A1 → N3 → A2 → M5.
> **M5** es un extra de presentación independiente (no se junta con A). **A1** es
> **prerrequisito duro de N3** (su censo/decomp informa el modelo a dos versiones).
> **T-022/`0x106f2`** quedó cerrada en la **sesión 69**; detalle: `notes/0053`.

## 7. Decisiones cerradas
Recogidas como ADR en `docs/adr/`:
- SDL3 (`0001`), `SDL_Renderer` base y `SDL_GPU` para CRT (`0002`),
  audio en hito propio (`0003`), menú de idioma fiel (`0004`), `boot/` fuera de
  `lib/kult-core/src/` (`0005`), headless primero (`0006`).

## 8. Estado (punto de control)
- **Fase M0 cerrada**: core importado y **499 diferenciales verdes** (`make test`).
- **Fase M1 hecha**: HAL PC (`platform/`), loader + `_start` (provisional en M1;
  **fiel** en M2) (`boot/`) y driver headless (`app/`). **`make test-port` verde**;
  `make port` arranca el juego y vuelca la sala cargada.
- **M2/T-021 hecha**: tick de VBL `0x160c2` en `boot/vbl.c` (con prueba).
- **T-024 resuelta**: el loader C (`boot/prg.c`) aplicaba mal la relocación DRI
  (relocalizaba tras un `RELOC_SKIP`); corregido y con prueba. **El port ejecuta
  el arranque y `game_loop`** (avanza frames en headless).
- **M3 hecha**: ventana SDL3 + `SDL_Renderer` (RGBA del framebuffer planar,
  paleta, escalado entero), **entrada real** (teclado/ratón → IKBD) y **ritmo**
  50 Hz. `make port-sdl` → `build/kult-sdl`; `KULT_WINDOW=1`.
- **M4 hecha (T-040/041/042)**: **MFP Timer A** (`platform/mfp.c`, dispara el
  vector `$134`), **PSG/YM2149** (`platform/psg.c` + `platform/audio.c`, con
  ledger), **salida SDL3** (`platform/audio_sdl.c`) y el **reproductor de música
  `0x3a872`/0x936** reconstruido en `boot/music_player.c`. Verificación: ledger
  PSG del reproductor contra el **oráculo** (23 eventos) y RMS/energía del YM en
  `make test-port`; la escucha final la valida el usuario.
- **M6 cerrada (T-060 ✅ / T-061 ✅)**: se cierra el **menú de idioma fiel** de
  `_start` (T-020: clic de título, drenado de teclas, D/E/F; `0x135ba` como
  `os_setscreen` no-op) y la **validación** (checklist en
  `docs/VERIFICACION.md`, limitaciones en `docs/LIMITACIONES.md`). **T-061
  validada en Windows (2026-10-23)**: el C sigue los mismos caminos y
  comportamiento que el ASM a nivel de experiencia del jugador; no se exige
  timing ciclo-exacto. **T-089 ✅** (final del juego: `attract`, validado en
  Windows). **T-090 ✅** (SOLAR EYES, cerrado; audio gap fuera de alcance).
  T-073 queda como **feature opcional** de calibración fina no bloqueante.
  `0x106f2` (reinicio suave) queda **portado como ruta latente (T-022)**;
  detalle `notes/0053`. Ingesta `.STX` / `kult.data` (T-072): ✅ implementada.
- **Plan fijado (sesión 11, sin código)**: orden `arreglos (portabilidad Windows +
  docs) → M7 build → T-061 validación ✅ → T-072/M7-c (empaquetado + ingesta +
  idiomas) ✅ → T-022 ✅ → T-102–T-135 (fullscreen/F1/F2/Estirar a pantalla + ranuras `slot-N.save`
  + OSD + terminología catalana T-119/T-132) ✅ → T-094 ✅ → publicación 1.0.0 y CI (T-092/T-096) ✅ → T-136 macOS → T-137 release 1.1.0 → A1 (investigación Amiga, prerrequisito
  duro de N3) → N3 (motor nativo + assets nativos, ADR + rama) → A2 (Amiga) →
  M5 (opcional)`. Detalle: **`notes/0011`**.
- **M7 build hecho (sesión 12, T-070)**: portabilidad Windows (`platform/time.c`
  reloj/espera, `platform/fs.c` `;` + `data/` junto al exe, `KULT_PRG` del dir de
  datos), **CMake** nativo (`kult`, `kult-sdl`) y **cross MinGW**
  (`kult.exe` GUI con icono + `kult-headless.exe`), `cmake/mingw-w64.cmake`,
  SDL3 **3.4.10** MinGW en `third_party/` (gitignored) e icono (`.rc` + RGBA +
  `SDL_SetWindowIcon`). Detalle: **`notes/0012`**.
- **Paquete portable**: CPack → ZIP Windows
  (`build-win/kult-1.1.0-win64.zip`: `kult.exe`+`SDL3.dll`+`data/`+`saves/`+
  `LEEME.txt`+licencias) y TGZ Linux; sin assets, sin headless y sin
  `DIAGNOSTICO.txt`.
- **Datos + diagnóstico (sesión 13)**: el `.exe` "no hacía nada" porque en
  `data/` solo había imágenes `.STX` (el paquete interino necesita ficheros
  extraídos). Los `.STX` del usuario (idénticos a los discos del proyecto) se
  extrajeron y se dejaron los 28 ficheros en `data/`. Se añadió **`kult.log`**
  junto al `.exe` (+stderr, `MessageBox` sin consola), **fallo de ventana =
  fatal** (antes bucle invisible) y **preflight de los 26 ficheros** del juego
  con lista clara. Detalle:
  **`notes/0013`**.
- **Colores del juego (sesión 14)**: corregidos cambiando la **base de pantalla
  del HAL** (`OS_SCREEN_BASE = 0x98000` en `platform/include/os.h`). Con el valor
  del kit (`0x80000`), pantalla B y su paleta caían dentro de los datos del juego
  (ECHE + heap) → ruido/colores mal. Paleta de `FOND.PI1` correcta y frame
  validado; `lib/kult-core/src/` intacto; **499 + test-port verdes**. Detalle: `notes/0014`.
- **Primera pantalla (sesión 14)**: corregida; es el **título (PRES)** visible
  con música y espera **tecla o clic**. El negro era un fallo del estado inicial
  del HAL (faltaba aplicar la paleta del PI1 antes de la espera); ahora
  `boot/boot.c` la aplica. Verificado: frame del título idéntico a `PRES.PI1`.
- **Ritmo (T-073a/b, sesión 15)**: implementado el modelo de CPU: `cpu_delay`
  real para los `dbf` transcritos + **coste medido por función** del oráculo
  (`-finstrument-functions` solo en el port, `platform/cpuwork_costs.inc`, 79
  funciones). El port cobra ciclos y cede VBLs; `KULT_CPU=0` lo apaga.
  **T-073c** resuelto y validado (commit `3235aa6`); queda calibración por
  escenas/casos no medidos (p.ej. `palette_cycle`). Detalle: `notes/0015`.
- **T-073c — Maestro (pantalla C, resuelto y validado por el usuario)**:
  **A** 3 retratos pixel + descripciones; **B** 4 retratos lift + descripciones
  (4 bancos `15ac8`/`0x36`); **C** 1 retrato del Maestro + lift + **diálogo**
  (único diálogo y único retrato que se mueve), después de B. La aceleración de
  C era el `dbf` inline de `object_info_card` (`0x16dec`): `wait=0x0a` → 40960
  iteraciones × 18 ciclos ≈ 92 ms ≈ 4,6 VBL/entrada, que el core no anotaba.
  Fix autorizado en `lib/kult-core/src/loop.c` con `cpu_delay_cycles()` (no-op en diferencial;
  en el port cobra con el reloj de T-073b). C pasa de ~3 a ~7,6 VBL/frame; A/B
  no usan `16dec`. Exe actual: ver T-074. Detalle:
  **`notes/0019-fix-timing-c.md`** (resumen en `notes/0017` §8); plan
  anti-fallo: **`notes/0018-plan-c.md`**; guardia: **`test_click_skip`**.
- **Input/presentación (T-074, sesión 23)**: bug de puertas/A-B-C. El core
  deriva acción y skip del bit0 del IKBD; la HAL lo tenía invertido (izquierdo =
  acción+skip; derecho ignorado). Se corrige en la HAL: acción = izquierdo
  **o** derecho; skip = solo derecho; `platform_input_apply_skip()`
  pisa `$1a570` al salir de `wait_frame` (hook de instrumentación). Además,
  `platform_tick` presenta en **cada VBL**, así se ven las animaciones que corren
  en `wait_vbl` (puertas, wipes). `lib/kult-core/src/` intacto; `test_click_skip` y
  `test_present_hook`. Exe `b7edc936…`, headless `7faac3d3…`, ZIP
  `97d56789…`. Detalle: **`notes/0020-input-hal-y-presentacion.md`**.
- **Input mantenido (T-084, sesión 36)**: el IKBD original (`wait_frame` envía
  `0x0D`, INTERROGATE MOUSE POSITION) entrega **eventos desde la última
  interrogación**, no el nivel del botón. La HAL ahora detecta flancos de
  bajada: mantener el clic no repite, soltar y pulsar sí, y el derecho sólo pide
  skip en su bajada. `lib/kult-core/src/` intacto; `test_click_held`, `test_click_skip` y
  `test_crawio_queue`. **Validado por el usuario en Windows** (el anillo se abre
  una vez y queda abierto para elegir). Detalle:
  **`notes/0032-input-clic-mantenido.md`**.
- **Retrato/intro acelerado (T-085, sesión 37)**: `portrait_intro` (`0x16a20`,
  comando `a013`) tenía dos `dbra d0` de `0xffff` por iteración transcritos como
  un `while` que nunca se ejecutaba. Ahora son `cpu_delay(0x10000u)` (no-op en
  diferencial): la animación pasa de ~0,4 s a **~2,2 s**. Solo cambia ese
  timing; no se tocan `15da6`, `15ac8` ni el input. **Validado por el usuario**.
  Detalle: **`notes/0033-portrait-intro-dbf.md`**.
- **Selección de acción en el anillo (T-086, sesión 38)**: `icon_ring`
  (`0x1566a`) tenía tres `dbra d0` de `0xffff` entre los tres blits del icono
  seleccionado y el C los omitía. Ahora usan `cpu_delay(0x10000u)` (no-op en
  diferencial): se ven los tres destellos (~12-15 VBL). Solo cambia ese timing;
  no se tocan `15da6`, `15ac8` ni el input. **Validado por el usuario**. Detalle:
  **`notes/0034-seleccion-anillo-dbra.md`**.
- **Cursor 16×16 / máximo IKBD (T-075, sesión 24)**: el cursor reaparecía por el
  lado contrario en el borde. El juego configura el ratón absoluto con
  `Ikbdws` `09 01 30 00 b8` → máximo **304×184**; la HAL lo emula con
  `platform_ikbdws()` y `platform_mouse_set()` recorta a ese máximo. `boot/boot.c`
  lo llama tras `crt0_init`; `lib/kult-core/src/` intacto. Prueba `test_ikbd_mouse_max`.
  Detalle: **`notes/0021-cursor-e-ikbd.md`**.
- **Reinicio tras morir (T-076, sesión 25)**: el original termina `reset_clear`
  (`0x1682a`) con `bra.w 0x10158` (inicio del bucle); el C nativo solo hacía el
  estado y retornaba, por lo que se repetía la muerte. Fix en `boot/` + hook: al
  salir de `reset_clear`, `kult_request_restart()` hace `longjmp` y
  `boot_enter_game()` reproduce `0x10158` (modo 7, `0xc001`, Maestro/objeto,
  `game_loop`). `lib/kult-core/src/` intacto; `KULT_RESTART_AT` diagnóstico. Detalle:
  **`notes/0022-reinicio-tras-muerte.md`**.
- **Ritmo de la muerte (T-077, sesión 26, punto 1)**: sangre (`181f0`/`13d0e`)
  y viaje espacial (`1834a`/`18316`) iban acelerados por coste 0. La sangre se
  cobra **por línea** en `reveal_line` con la fórmula medida
  `1063 + 198 × pasos`; `181f0` se deja a 0 para no meter un retardo previo.
  El viaje espacial cobra `1834a` (2.301.836/101 ≈ 22.790/pasada). Tabla
  regenerada (80 funciones). `lib/kult-core/src/` intacto. **Validado en Windows**: sangre
  perfecta y espacio 101 VBL. Detalle:
  **`notes/0023-costes-sangre-espacio.md`**.
- **Estrella final (T-079, sesión 27, punto 2)**: la animación ya era correcta
  (15 VBL); faltaba la espera posterior. El ASM no tiene un `wait`: tras
  `159e2` se lee `FOND.PI1` (`0x13568`, 0x7d22 B) y el ST bloquea en Fread con
  la estrella en pantalla. La HAL modela la lectura de disquete ST
  (`platform_disk_delay`, ~2 s; usuario: >1,4 s) desde `pc_os_fread`. `lib/kult-core/src/`
  intacto; nativo/win/ZIP OK. Build de la estrella: exe `ed175889…`, headless
  `68ae05ea…`, ZIP `c27a1935…`; build actual: exe `c7b8dac1…`, headless
  `76e7974e…`, ZIP `787bd41d…`. El retardo se aplica a **cada Fread** (salas, reinicio,
  estrella), pero **no** a la carga inicial: `platform_disk_timing(0)` desde
  PRES/DRAP hasta después de ECHE (listas A/B, FOND incluidos) evita la pantalla
  negra inicial, la canción avanzada y los ~20 s tras el idioma. **Validada
  visualmente por el usuario** (commit `b599cf9`). Detalle:
  **`notes/0025-espera-carga-fond.md`** y **`notes/0026-esperas-de-disco.md`**.
- **Cursor del sistema (T-080, sesión 28)**: SDL mostraba el cursor de Windows
  sobre el cursor del juego. `platform/video_sdl.c` lo oculta con
  `SDL_HideCursor()` al abrir/recuperar foco y lo restaura al perder foco/cerrar;
  `lib/kult-core/src/` intacto. Exe `3237a4d7…`, headless `68ae05ea…`, ZIP `ad7d30dd…`.
  Detalle: **`notes/0027-cursor-sistema-sdl.md`**.
- **H-el1 (T-081, sesión 30)**: **resuelto**. Las salidas anticipadas de
  `elem_command` (`0x128a4`, `addq.l #4,a7; rts`) se propagan con
  `elem_command_ex`/`elem_command_wrap_ex` a los 12 llamantes; `reveal_pixel`
  ya no revela ni usa `A_ELEM_*` viejo. Tests por llamante (imagen + A6) y
  auditoría de otros retornos dobles en **`notes/0028-retornos-dobles-128a4.md`**.
  `make test` 511, `test-port`, nativo/win/ZIP OK. Exe `c7b8dac1…`, ZIP
  `787bd41d…`. **Validado visualmente por el usuario** (el glitch del punto 4
  desaparece). No hay un punto 3 independiente: 4905 era el mismo reveal H-el1.
- **Siguiente:** **T-136 — macOS Universal 2** y **T-137 — release 1.1.0**;
  después A1 (investigación Amiga, prerrequisito duro de N3). La release 1.0.0
  y el CI (T-092/T-096) están cerrados; T-094 se validó con el ZIP de T-135.
  T-022 (reinicio suave/ruta latente), T-095 (tests de ingesta/packs),
  T-097/T-098, T-101 («DIE» → DADO/DAU) y T-102–T-135 ya están cerrados.
  Después **N3** → **A2** → **M5**. **T-061 ✅ cerrada y validada en Windows
  (2026-10-23)**: checklist completo (título/idioma/colores, partida/controles,
  retrato C, sangre/espacio, estrella, cursor, glitch del reveal,
  guardado/carga, muerte → reinicio, **audio** T-082, input/anillo T-084,
  combate/PSI T-087, ficha al atacar T-088, **final T-089** y **T-090**). El C
  sigue los mismos caminos/comportamiento que el ASM a nivel de experiencia del
  jugador; no se exige timing ciclo-exacto. T-073 queda como **feature opcional**
  de calibración fina.
  **T-083** (objetos en serpientes) **resuelto y validado**: retorno doble de
  `icon_ring_loop` (`0x1592e`), con tail-call explícito en la VM; ver
  **`notes/0031`**. **T-084** (input mantenido), **T-085** (retrato/intro) y
  **T-086** (selección de acción) **resueltos, validados y cerrados**; ver
  **`notes/0032`**, **`notes/0033`** y **`notes/0034`**. **T-087**
  (incidencias tras matar un enemigo) **resuelto y validado**: el unwind A7 de
  los `fxxx` se implementó en sus dos caminos, el directo de
  `run_command_keep_sp_a6` y el indirecto de `dispatch_a6` desde el byte `0x4d`
  (boundary `setjmp`/`longjmp` en `run_command`). La run final muestra
  `RUN_UNWIND4D` tras `e1fc -> f1c2`, sin `loop_reopen`, y la muerte completa
  del enemigo. Ver **`notes/0037`** (resumen de los dos fixes), **`notes/0036`**
  (detalle técnico) e histórico en **`notes/0035`**. **T-088** (crash al atacar
  a un personaje) **resuelto y validado en Windows**: `object_info_card` no
  avanzaba `p` tras `text_block_build`; fix `p += 3`, test
  `test_fun_16dec_text_block` y replay de entrada. Ver **`notes/0038`**.

> Mantener esta sección y `RETOMAR.md` al día al cerrar cada sesión.
