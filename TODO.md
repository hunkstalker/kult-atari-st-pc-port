# TODO — tareas

Estados: **✅ hecho** · **🚧 en curso** · **⬜ pendiente** · **🧩 feature opcional (no bloqueante)**. Al cerrar una tarea,
cámbiala aquí y anota el resultado en `RETOMAR.md`. Los ids son estables
(`T-xxx`); los nuevos van al final de su fase.

## Índice rápido de IDs (abiertas/opcionales)
| id | tarea | estado |
|---|---|---|
| T-061 | Validación completa de la partida en Windows (checklist M6) | ✅ cerrada (2026-10-23) |
| T-072 | Ingesta automática de imágenes `.STX` a `data/kult.data` | ✅ implementada |
| T-073 | Calibración opcional de CPU/ritmo/fidelidad del port | 🧩 feature opcional (no bloqueante) |
| T-090 | SOLAR EYES: flash PSI acelerado (`palette_cycle`) | ✅ cerrado |
| T-093 | Revisión lingüística del catalán | ✅ validada por el usuario (2026-10-25) |
| T-094 | Validación visual final de la release Windows | ⬜ pendiente |
| T-095 | Tests automatizados de ingesta `.STX` y packs `S`/`C` | ⬜ pendiente |
| T-097 | Texto descuadrado al recibir un objeto (p. ej. “una mosca de pedra”) | ✅ corregido (`notes/0045`), pendiente validación |
| T-098 | Bloqueo en la prueba “La Horca” usando `kult.data`/`language.data` | ✅ corregido (`notes/0046`), pendiente validación |
| T-099 | Pasada de textos ingleses sin traducir | ✅ 240/241 corregidos (`notes/0047`) |
| T-092 | Publicar release 1.0.0: `main`, tag, GitHub Release y ZIP | ⬜ pendiente tras T-094/T-095 |
| T-096 | CI en GitHub: core + HAL + paquete sin assets | ⬜ pendiente tras publicar |

## M0 — Repositorio y base
| id | tarea | estado |
|---|---|---|
| T-000 | Documentación inicial del repo (`README`, `PROYECTO`, `TODO`, `RETOMAR`, `AGENTS`, `docs/`, `notes/`) | ✅ |
| T-001 | Importar el core desde `/app/kult-atari-st-c`: `src/`, `include/`, `test/`, `tools/` | ✅ |
| T-002 | Build propio: `CMake` + kit diferencial; verificar `make test` (511) en este repo | ✅ (kit/Makefile; CMake del ejecutable → M1) |
| T-003 | `project.toml` + `work/disk_a/KULT.PRG` (gitignored) para el oráculo | ✅ |

## M1 — HAL mínima (headless)
| id | tarea | estado |
|---|---|---|
| T-010 | `platform/`: cabeceras con el contrato de los 19 servicios | ✅ |
| T-011 | Ficheros: `os_fopen/close/create/read/write/fsfirst/fsetdta` (mapeo `A:` → `data/`) | ✅ |
| T-012 | Sistema: `os_super` (no-op) | ✅ |
| T-013 | Vídeo a memoria: `os_setscreen/setpalette/setcolor` + volcado del framebuffer | ✅ |
| T-014 | Entrada: `os_crawio`, `os_cnecin` + alimentar el paquete IKBD (`A_IKBD_PACKET`) | ✅ |
| T-015 | Tiempo/tick: `sched_poll8` + reloj que avanza `A_DAT_1ACD4` | ✅ (reloj; el trabajo por frame `0x160c2` es T-021) |
| T-016 | Audio: stub no-op | ✅ |
| T-017 | `app/main`: init → `_start` → `game_loop` (headless) | ✅ bring-up (carga sala + volcado); bucle real tras T-021 |

> Prueba del backend (sin oráculo): `make test-port` (ficheros, vídeo planar→RGB,
> entrada/IKBD, reloj, tick, loader). Ejecutable headless: `make port` → `build/kult`,
> que ejecuta el arranque y **avanza N frames de `game_loop`** volcando el
> framebuffer. Aceptación de M1 cumplida.

## M2 — Arranque y tick (reconstrucción)
| id | tarea | estado |
|---|---|---|
| T-020 | Portar `_start` (0x10000), incluido el menú de idioma | ✅ menú **fiel** en `boot/language.c` (drena teclas pendientes + espera D/E/F), clic de título por `prompt_wait` (sin el atajo `MODE_FLAG=0x87`) y suelta del clic en `l_010158`; pruebas en `make test-port` |
| T-021 | Portar el tick de VBL `0x160c2` (paleta + blits GAUSS/MURSM) | ✅ en `boot/vbl.c`, con prueba (`make test-port`) |
| T-024 | **Diagnóstico del bloqueo de `0xc001`** | ✅ **causa raíz: el loader C `boot/prg.c` aplicaba una relocación de más tras cada `RELOC_SKIP` (byte 1)**, corrompiendo 544 bytes de la imagen (entre ellos un largo de ICONE) → el guion tomaba otra rama. Corregido; el loader coincide byte a byte con el oráculo y tiene prueba de regresión |
| T-022 | Portar `0x135ba` y `0x106f2` | 🚧 `0x135ba` ✅ (`os_setscreen(-1,-1,0)` en `_start`). `0x106f2` (`l_0101a8`, reinicio suave) **pendiente/no portado y ABIERTO**. Análisis estático: `0x101a8` solo es **constante base de relocalización** de la tabla de pistas/iconos (`0x16f90`/`0x16fb4`) y no hay `jsr`/`jmp`/`pea` hacia `l_0101a8` → **parece inobservable**, pero puede necesitarse. **No confundir** con el reinicio tras morir (`0x1842c`→`reset_clear`; su `bra 0x10158` se emula ahora en `boot/`, ver T-076). Ver `notes/0010`, `notes/0011`, `notes/0022` |
| T-023 | Omitir la comprobación FDC `0x19316` | ✅ se omite; se siembra el patrón FDC (`0xd8e8`) para que el modo de escena (`0x14496`) no espere |

## M3 — Render y entrada reales (primera ventana)
| id | tarea | estado |
|---|---|---|
| T-030 | Ventana SDL3 + `SDL_Renderer`; planar→RGBA; paleta de 16 words; escalado entero | ✅ `make port-sdl` (`platform/video_sdl.c`): ventana, textura RGBA, `SDL_LOGICAL_PRESENTATION_INTEGER_SCALE`, ESC para salir |
| T-031 | Entrada real: teclado y ratón → paquete IKBD / `A_CURSOR_X/Y` | ✅ SDL→`platform_key_push`/`platform_mouse_set`; `Cnecin` bloqueante con bombeo (`idle`); prueba en `make test-port` |
| T-032 | Ritmo: simulación a cadencia fija + presentación al refresco | ✅ `platform_set_realtime` (1 VBL = 20 ms / 50 Hz) en modo ventana; `KULT_REALTIME=0` lo desactiva |

## M4 — Audio
| id | tarea | estado |
|---|---|---|
| T-040 | Modelo del **MFP Timer A** que dispara las ISR C ya reconstruidas | ✅ `platform/mfp.c`: periodo de `TACR`/`TADR` (2.4576 MHz), vector `$134` → ISR PCM (`0x1910e`/`0x190f6`) o reproductor de música (`0x3a872`, reconstruido en `boot/music_player.c`); ISRA bit 5 |
| T-041 | Emulación **PSG/YM2149** desde `psg_port_write/read` | ✅ `platform/psg.c` (fichero + **ledger** + seed) y `platform/audio.c` (3 ondas + ruido + mixer + volúmenes + envolvente + filtro DC) |
| T-042 | Salida por **SDL audio** | ✅ `platform/audio_sdl.c` (mono S16 48 kHz, callback drena el anillo); `KULT_AUDIO=0` lo apaga |

## M6 — Pulido y validación
| id | tarea | estado |
|---|---|---|
| T-060 | Menú de idioma (validar el fiel; valorar menú nativo) | ✅ fiel (ADR 0004): validado en T-020; se mantiene el menú del original |
| T-061 | Validación completa en Windows: arranque, partida, guardado, audio | ✅ **cerrada y validada en Windows (2026-10-23)**. Checklist M6 completo: título/idioma/colores, partida/controles, retrato C (T-073c), sangre/espacio (T-077), estrella (T-079), cursor (T-080), glitch del reveal (T-081), guardado/carga, muerte → reinicio (T-076), audio y puertas (T-082), input mantenido/anillo (T-084), combate/PSI (T-087), ficha al atacar sin crash (T-088), final en `attract` (T-089) y SOLAR EYES (T-090). Criterio de cierre: el C sigue los mismos caminos/comportamiento que el ASM **a nivel de experiencia del jugador**; no se exige timing ciclo-exacto. El hueco de audio de SOLAR EYES queda documentado como artefacto de timing del original y T-073 como feature opcional. Detalle: `docs/VERIFICACION.md` |
| T-073 | **Calibración opcional de CPU/ritmo (feature opcional)**: medir ciclos por función con el oráculo y aplicar un reloj virtual de CPU (~8 MHz); **no** ajustar animaciones a mano | 🧩 **Feature opcional (no bloqueante)**. **T-073a ✅** hook `cpu_delay`; **T-073b ✅** coste medido por función (`platform/cpuwork_costs.inc`, 79 funciones). **T-073c ✅ validado (Maestro C, commit `3235aa6`)**: A/B/C corregidas. La aceleración de C era el `dbf` inline de `object_info_card` (`0x16dec`): `wait=0x0a` → 40960 iteraciones × 18 ciclos ≈ 92 ms ≈ 4,6 VBL/entrada, sin anotar. Fix autorizado en `src/loop.c` con `cpu_delay_cycles()` (no-op en diferencial; cobra por el reloj de T-073b). C pasa de ~3 a ~7,6 VBL/frame; A/B no usan `16dec`. Exe actual: `c7b8dac1…`; `make test` 511 y `make test-port` verdes (nueva `test_cpu_charge_cycles`). Detalle: **`notes/0019-fix-timing-c.md`** (resumen en `notes/0017` §8); plan anti-fallo: **`notes/0018-plan-c.md`**; guardia: **`test_click_skip`**. Queda calibrar FPS (`KULT_FPSLOG`/`KULT_ST_CONTEND`/`KULT_CPU_HZ`). **Evidencia T-090 (2026-10-23)**: con `KULT_CPU_HZ=5000000` SOLAR EYES dura 84 VBL (1,68 s) y reproduce el hueco de audio de Hatari (1,65 s, 21 ms); a 8 MHz dura 60 VBL y no hay hueco. Apunta a calibracion CPU/ISR, no a parche. Detalle: **`notes/0041`** §9. |
| T-074 | **Input HAL + presentación (puertas / A-B-C)**: izquierdo/teclado = acción sin skip; derecho = acción + skip; presentar por VBL para ver animaciones que corren en `wait_vbl` | ✅ **sesión 23**: acción = izquierdo o derecho; skip = solo derecho; `platform_input_apply_skip()` pisa `$1a570` al salir de `wait_frame` (hook de instrumentación, como la referencia); `platform_tick` presenta en cada VBL. `src/` intacto; `test_click_skip` + `test_present_hook`. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0020-input-hal-y-presentacion.md`** |
| T-075 | **Cursor 16×16 / máximo IKBD**: al llegar al borde derecho el cursor reaparecía por el izquierdo; el juego configura el ratón absoluto a 304×184 | ✅ **sesión 24**: `crt0_init` envía `Ikbdws` `09 01 30 00 b8` (xmax 304, ymax 184); la HAL lo emula con `platform_ikbdws()` y `platform_mouse_set()` recorta a ese máximo. `boot/boot.c` lo llama tras `crt0_init`; `src/` intacto. Prueba `test_ikbd_mouse_max`. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0021-cursor-e-ikbd.md`** |
| T-076 | **Reinicio tras morir**: el juego repetía la secuencia de muerte en vez de volver al Maestro/objeto | ✅ **sesión 25**: `reset_clear` (`0x1682a`) termina en el original con `bra 0x10158`; el C nativo solo hacía el estado. Fix en `boot/` + hook: al salir de `reset_clear`, `kult_request_restart()` hace `longjmp` y `boot_enter_game()` reproduce `0x10158` (modo 7, `0xc001`, Maestro/objeto, `game_loop`). `src/` intacto; `KULT_RESTART_AT` diagnóstico. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0022-reinicio-tras-muerte.md`** |
| T-077 | **Ritmo de la muerte (punto 1)**: sangre (`181f0`/`13d0e`) y viaje espacial (`1834a`/`18316`) iban acelerados | ✅ **sesión 26**: la sangre se cobra por línea en `reveal_line` (`13d0e`) con la fórmula medida **1063 + 198 × pasos** (pasos = max(|dx|,|dy|)+1); `181f0` se deja a 0 para no meter un retardo previo. El viaje espacial cobra `1834a` con `1834a 2301836 101` ≈ 22.790 ciclos/pasada. Tabla regenerada (80 funciones). Sin tocar `src/`. **Validado en Windows**: sangre perfecta; espacio `1834a` 2373–2473 = 101 VBL (~2,02 s), igual que el ASM. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0023-costes-sangre-espacio.md`** |
| T-079 | **Espera de la estrella final (punto 2)** y **esperas de disco globales**: la estrella se queda en pantalla mientras carga `FOND.PI1`; al modelar el Fread aparecen pausas en salas/estrella (la carga inicial queda exenta) | ✅ **sesión 27**: el ASM no tiene espera programada tras `159e2`; `0x13568` lee `0x7d22` B de disco. `platform_disk_delay` en la HAL (~2 s para 32 KB: 9×512 B/pista a 300 rpm = 1,4 s + búsqueda/motor; usuario: >1,4 s) se aplica desde `pc_os_fread()` a **cada lectura** (FOND, listas, PUZZL/BRUIT, ECHE, SCENA, CLEAR...), no solo a la estrella. **Excepción de carga inicial**: `platform_disk_timing(0)` desde PRES/DRAP hasta después de ECHE (listas A/B, FOND incluidos) y `1` después; así el título aparece sin pantalla negra, la canción empieza desde el principio y no hay ~20 s de pausa tras el idioma. **Validado visualmente por el usuario** (commit `b599cf9`). No-op en headless/diferencial; `src/` intacto. En el momento del fix: `make test` 499; actual: **511**. Build de la estrella: exe `ed175889…`, headless `68ae05ea…`, ZIP `c27a1935…`; build actual: exe `c7b8dac1…`, headless `76e7974e…`, ZIP `787bd41d…`. Detalle: **`notes/0025-espera-carga-fond.md`**, **`notes/0026-esperas-de-disco.md`** |
| T-080 | **Cursor del sistema (SDL)**: se veía el cursor de Windows encima del cursor del juego | ✅ **sesión 28**: `platform/video_sdl.c` oculta el cursor con `SDL_HideCursor()` al abrir/recuperar foco; lo muestra al perder foco/cerrar. Dentro del juego solo queda el cursor de `src/cursor.c`; `src/` intacto. `make port-sdl` y `package-win` OK. Exe `3237a4d7…`, headless `68ae05ea…`, ZIP `ad7d30dd…`. Detalle: **`notes/0027-cursor-sistema-sdl.md`** |
| T-081 | **Retornos dobles de `elem_command` (`0x128a4`, H-el1)**: las salidas anticipadas (`addq.l #4,a7; rts`) no se propagan a los 12 llamantes; un reveal puede ejecutarse con `A_ELEM_*` viejo | ✅ **sesión 30**: estado de aborto en `elem_command_ex`/`elem_command_wrap_ex`; los 12 llamantes salen con el script avanzado sin blit/reveal ni tocar `A_ELEM_*`. Tests por llamante con `$1ad0a` y clase no encontrada. Auditoría: solo `0x1294c`, `0x14f4a` (ya modelado) y `0x1592e` (modelado y cerrado en T-083 con tail-call explícito); no quedan retornos dobles. `make test` **511**, `test-port`, nativo/win/ZIP OK. Exe `c7b8dac1…`, ZIP `787bd41d…`. **Validado visualmente por el usuario**: el glitch del punto 4 desaparece. Detalle: **`notes/0028`**. |
| T-082 | **Audio de puertas**: 3 samples (abrir, llegar al tope y cerrar); el del tope se cortaba porque la transición de escena llegaba a `sound_stop` antes de que sonara | ✅ **sesión 32, validado por el usuario**: log 88310: apertura 1192, tope `18f7a`/`0x1f` en 1321, `sound_stop` en 1322. La transición visual esperaba tarde: se cambió la sala antes de que terminara el sample. Ahora se espera al final de `textform_fwd`, antes de que la transición pinte/cargue la sala siguiente, a que `0x192c6` alcance `0x192ca`. `make test` 511 + build/ZIP OK. Exe `bd3764b9…`, headless `0fdda9c8…`, ZIP `1396dd00…`. Audio general (música/voces/efectos y puertas) **validado**. Detalle: **`notes/0029-audio-puertas.md`**. |
| T-083 | **Bugs al usar objetos en las serpientes** (primera: objeto incorrecto debe consumirse y no hacer nada; agua llena y flota dado; segunda: dado abre puerta) | ✅ **resuelto y validado (sesión 34)**: `icon_ring_loop` (0x15888) modela el retorno doble/tail-call de `0x1592e` con `run_command_keep_sp_a6` y tail-call explícito (`VM_TAIL_FLAG`/`icon_ring_loop_ex`); tests `test_run_command_keep_sp_a6_nibble0` y `test_fun_15888_ex_sin_tail`; **513 tests** + `test-port`. Validado con `kult.exe` (dado/2.ª y mosca/1.ª) y `kult-diag.exe`: el log `diag-logs/post-refactor-2026-10-21.log` no ejecuta `0x1fc2e`/`0x206b1`/`0x2563e`/`0x206b5` y muestra `TAIL_3D`/`VMTAIL a6=20726`. Detalle: **`notes/0030`** y **`notes/0031`**. |
| T-084 | **Input mantenido / clics repetidos**: mantener el clic izquierdo genera clics continuos; el anillo de acciones se abre/cierra y no se puede elegir | ✅ **sesión 36, resuelto y validado por el usuario**: el ASM/Hatari confirman que el IKBD (`0x0D`, INTERROGATE MOUSE POSITION) responde con **eventos desde la última interrogación**, no con nivel. La HAL entrega ahora un flanco de bajada una vez; mantener el botón no repite, soltar y pulsar sí, y el derecho sólo pide skip en su bajada. `src/` intacto; `test_click_held`, `test_click_skip` y `test_crawio_queue` (dos teclas = dos `Crawio`). `make test` **513** + `test-port`. Ejecutables: `kult-diag.exe` `64505805…`, `kult.exe` `cc4d5cee…`, headless `414a82c8…`. **Validado visualmente**: el anillo se abre una vez con el clic mantenido y queda abierto para elegir. Detalle: **`notes/0032-input-clic-mantenido.md`**. |
| T-085 | **Retrato/intro acelerado**: una animación de retrato/intro duraba <1 s cuando debía durar >2 s | ✅ **sesión 37, resuelto y validado por el usuario**: `portrait_intro` (`0x16a20`, comando `a013`) tenía dos `dbra d0` de `0xffff` por iteración transcritos como un `while` que nunca se ejecutaba. Ahora usan `cpu_delay(0x10000u)` (no-op en diferencial); la animación pasa de ~0,4 s a **~2,2 s**. Solo se toca ese timing; no se tocan `15da6`/`15ac8`/input. `test_fun_16a20` verde, guardia de `cpu_delay(0x10000)` en `test-port`; `make test` **513**. Ejecutables: `kult.exe` `5342b77c…`, headless `ccf5f7a9…`, diag `62710162…`. Detalle: **`notes/0033-portrait-intro-dbf.md`**. |
| T-086 | **Selección de acción en el anillo**: los tres destellos del icono elegido casi no se veían | ✅ **sesión 38, resuelto y validado por el usuario**: `icon_ring` (`0x1566a`) tenía tres `dbra d0` de `0xffff` entre los tres blits del icono y el C los omitía. Ahora usan `cpu_delay(0x10000u)` (no-op en diferencial); el cierre pasa de ~1 VBL a ~12-15 VBL. Auditoría de los `dbra 0xffff` de espera: `portrait_intro` (2, T-085), `icon_ring` (3, este fix), `palette_cycle` (2, no tocados). `make test` **513** + `test-port`. Ejecutables: `kult.exe` `4fc2cfa2…`, headless `8c222aff…`, diag `6628f205…`. Detalle: **`notes/0034-seleccion-anillo-dbra.md`**. |
| T-087 | **Incidencias tras matar un enemigo**: (a) el anillo sigue ofreciendo acciones de enemigo vivo sobre el cadáver; (b) recuadro negro con enemigo invisible | ✅ **resuelto y validado por el usuario (sesión 43)**. Dos caminos A7 corregidos: `run_command` (`0x103a8`) tiene boundary `setjmp`/`longjmp`; `run_command_keep_sp_a6` dispara el unwind directo para `nibble == 0xf000` (p. ej. `f2f8`) y `dispatch_a6` lo dispara para el `fxxx` que forma el byte `0x4d` (p. ej. `e1fc -> f1c2`). Fallback normal/anidado sin boundary; tail-call `c/a` y `$1abf8` intactos. La run final muestra `RUN_UNWIND4D` en VBL 2939 tras `e1fc -> f1c2`, sin `loop_reopen`, y `c357` en 3184 con clase `01ab30` en 3448; después `01aa86` muere con `c1d2`/`c1d7`/`c357`. Tests `test_run_command_f000_unwind` y `test_a7_unwind`; `make test` **516** + `test-port` OK. Ejecutables `kult.exe` `be2cd45a…`, `kult-diag.exe` `16605768…`; logs en `trazas-post-fix-20261021/validacion/`. Resumen: **`notes/0037-dos-caminos-unwind-a7.md`**; detalle: **`notes/0036`** §13; histórico: **`notes/0035`**. Salvedad: el camino directo `f2f8` no se reejecutó manualmente en esta run. |
| T-088 | **Crash al atacar a un personaje** (`object_info_card` / bloque de texto 3 B) | ✅ **resuelto y validado por el usuario en Windows**. El replay de entrada (`KULT_REPLAY_REC`/`KULT_REPLAY_PLAY`, cursor incluido) reprodujo el SIGSEGV: `object_info_card` no avanzaba `p` tras `text_block_build`; el ASM `0x129d6` deja A5 al final del bloque. El bucle releía `68 d2 00`, tomaba `0xd2` como índice y llegaba al patrón inválido 9. Fix en `src/loop.c`: `p += 3`. Test `test_fun_16dec_text_block`; `make test` **517** y `test-port` OK; replay nativo sin crash y validado visualmente en Windows. Replay/save/logs: `trazas-t088-crash-20261021-224446/`. Detalle: **`notes/0038`** §9–§11; evidencia: **`notes/0037`** §8. |
| T-089 | **Final del juego: bucle infinito de `attract`** | ✅ **resuelto y validado por el usuario en Windows (2026-10-22)**. El original no retorna de `attract` (`0x18a98`): en `l_018c80` (`0x18c80`) llama indefinidamente a `f_01834a` con `D5=0x64` (animación de estrellas). El C hacía `icon_list_reset` y retornaba, el VM seguía el guion y dejaba escena/paleta corruptas. Fix en `src/screens.c`: `attract_once` (primera pasada) + `attract_loop` (bucle real) + `attract` (no retorna). Evidencia Hatari: código relocalizado `+0xB018`; cadena `run_command_keep_sp -> vm_run -> attract -> l_018c80 -> f_01834a -> wait_vbl`. Test nuevo `test_fun_18a98_loop_once`; `make test` **518** + `test-port` OK; builds nativo/Windows/ZIP. Detalle: **`notes/0040-t089-final-attract-loop.md`**. |
| T-090 | **SOLAR EYES (habilidad PSI) acelerada** | ✅ **cerrado (2026-10-23)**. El comando `0xa00a` (`f_014f5e`) ejecuta `palette_cycle` (`0x16310`); el ASM hace por pasada `wait_vbl` + `dbra d0=0xffff` + `set_palette`, y el C omitía los dos `dbra`. Fix solo de timing en `src/video.c`: `cpu_delay(0x10000u)` en las dos fases. Replay/save y medición: `trazas-t090-20261023/` (original 7.879.396 ciclos ≈985 ms; port 12→60 VBL, 0,24→1,2 s). Nueva `test_palette_cycle_solar_eyes_timing`; `make test` **519** y `test-port` OK; builds/ZIP actualizados. El hueco de audio de Hatari queda documentado como artefacto de timing del original, **fuera del alcance de T-090** (no es divergencia ASM↔C); ver T-073 (feature opcional). T-061 quedó cerrada y validada en Windows (2026-10-23). Detalle: **`notes/0041`**. |

## M7 — Empaquetado/release
> Build, paquete portable, ingesta, idiomas y licencias completados. La release
> 1.0.0 está preparada. Orden de cierre: **validación de T-097/T-098 →
> validación visual → tests automatizados → publicar → CI**. El catalán ya está
> validado por el usuario (`notes/0043`).
| id | tarea | estado |
|---|---|---|
| T-070 | **Build**: portabilidad Windows (`platform/time.c` `_WIN32`, `platform/fs.c` `;`/data junto al exe) + CMake (`kult` headless, `kult-sdl`; en Windows → `kult.exe` GUI + `kult-headless.exe`) + `cmake/mingw-w64.cmake` + SDL3 **3.4.10** MinGW (`third_party/`, gitignored) + icono (`.rc`, RGBA, `SDL_SetWindowIcon`) + **log/diagnóstico** (`kult.log` con timestamp, preflight de datos) | ✅ build nativo CMake verificado; cross Windows enlaza (sin ejecutar aquí) |
| T-071 | **Empaquetado portable**: CPack ZIP Windows (`kult.exe`+`SDL3.dll`+`data/`+`saves/`+`LEEME.txt`+licencias) y TGZ Linux; **sin assets, sin headless ni DIAGNOSTICO** | ✅ |
| T-072 | **Ingesta + `kult.data`**: detección de `.STX` por SHA-256, extracción Pasti/FAT12, validación de `KULT.PRG` y generación atómica de `data/kult.data` | ✅ implementada |
| T-091 | **`language.data`**: DRAP + packs de idioma; español y catalán revisados incluidos | ✅ |
| T-093 | **Revisión lingüística del catalán**: textos, nombres propios y estilo | ✅ **validada por el usuario (2026-10-25)** |
| T-094 | **Validación visual final** de la release Windows: DRAP, fuentes, textos y menús | ⬜ pendiente |
| T-095 | **Tests automatizados de ingesta y packs**: fixtures sintéticos de `.STX`/`kult.data` y cobertura de `S`/`C` | ⬜ pendiente |
| T-097 | **Texto descuadrado al recibir objetos**: revisar espacios/códigos en la frase de entrega | ✅ corregido (`notes/0045`), pendiente validación |
| T-098 | **Bloqueo en “La Horca”**: reproducir con `kult.data` + `language.data` y localizar la causa | ✅ corregido (`notes/0046`), pendiente validación |
| T-092 | **Publicación 1.0.0**: push de `main`, tag, GitHub Release y ZIP Windows | ⬜ pendiente tras T-094/T-095 |
| T-096 | **CI en GitHub**: core + HAL + paquete sin assets | ⬜ pendiente tras publicar |

## A1 — Investigación Amiga (prerrequisito **DURO** de N3)
> Mapa de *lógica compartida vs específica* y diferencias de hardware/assets, para
> diseñar el modelo de datos de N3 a dos versiones. **No toca el motor.**
| id | tarea | estado |
|---|---|---|
| T-A00 | Determinación y censo: comparar el binario Amiga por rutina con el ST | ⬜ |
| T-A01 | Decomp ASM del Amiga (spec byte a byte) | ⬜ |

## N3 — Motor nativo y assets nativos (hito futuro: ADR + rama)
> Runtime 100% independiente del binario original: motor propio + assets nativos
> (**PNG/WAV/OGG/UTF-8** + **binario propio documentado**; sin mods). La ingesta
> pasa a ser un **conversor de un solo uso**. Modelo de datos preparado para
> **dos versiones (ST/Amiga)**. Detalle del plan en `notes/0011`.
| id | tarea | estado |
|---|---|---|
| T-N00 | Redactar el **ADR** de N3 (motor nativo + assets nativos + ingesta) | ⬜ |
| T-N01 | Formato de assets + cargador (PNG/WAV/OGG/UTF-8 + binario propio) | ⬜ |
| T-N02 | Conversores por subsistema: gráficos → PNG; audio → WAV/OGG/tracker; textos → UTF-8; datos de sala/guion → binario propio | ⬜ |
| T-N03 | Verificación por conversor + validación funcional | ⬜ |

## A2 — Implementación Amiga (después de N3, sobre el motor nuevo)
| id | tarea | estado |
|---|---|---|
| T-A02 | Core Amiga (lógica compartida + capa Amiga) + verificación | ⬜ |
| T-A03 | Arte/audio/textos Amiga | ⬜ |

## Documentación
| id | tarea | estado |
|---|---|---|
| T-D00 | Índice y ADRs (`docs/INDICE.md`, `docs/adr/`) | ✅ |
| T-D01 | Mantener `RETOMAR.md` y este `TODO.md` al cerrar cada sesión | 🚧 |
| T-D02 | Licencia raíz, avisos de terceros y textos públicos de release | ✅ `notes/0042` |
| T-099 | Pasada de textos ingleses sin traducir; 240/241 corregidos | ✅ `notes/0047` |

## M5 — Filtros CRT (nota: extra opcional, no prioritario)
> **Nota.** Fuera del camino crítico y **al final** (independiente de A). Es una
> **mejora de presentación**; se desarrollará en algún momento, tras completar el
> plan (`M7 → A1 → N3 → A2`). Se conserva el id **M5/T-050**. Detalle: `notes/0011`.
| id | tarea | estado |
|---|---|---|
| T-050 | `SDL_GPU` + shaders (escalado/CRT), opcional | ⬜ |
