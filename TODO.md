# TODO — tareas

Estados: **✅ hecho** · **🚧 en curso** · **⬜ pendiente**. Al cerrar una tarea,
cámbiala aquí y anota el resultado en `RETOMAR.md`. Los ids son estables
(`T-xxx`); los nuevos van al final de su fase.

## M0 — Repositorio y base
| id | tarea | estado |
|---|---|---|
| T-000 | Documentación inicial del repo (`README`, `PROYECTO`, `TODO`, `RETOMAR`, `AGENTS`, `docs/`, `notes/`) | ✅ |
| T-001 | Importar el core desde `/app/kult-atari-st-c`: `src/`, `include/`, `test/`, `tools/` | ✅ |
| T-002 | Build propio: `CMake` + kit diferencial; verificar `make test` (499) en este repo | ✅ (kit/Makefile; CMake del ejecutable → M1) |
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
| T-061 | Validación: arranque, partida, guardado, audio | 🚧 checklist y comandos en `docs/VERIFICACION.md` y `docs/LIMITACIONES.md`; **la ejecuta el usuario** con el paquete Windows (`build-win/kult-0.1.0-win64.zip`). Sus `.STX` ya se extrajeron a `data/` (sesión 13); **colores validados por el usuario** (sesión 14). **Primera pantalla validada por el usuario** (título visible, sin negro; espera tecla o clic) → idioma D/E/F. Falta confirmar ritmo/guardado/audio y **morir → reiniciar sala** (`reset_clear`, core) |
| T-073 | **Ritmo del original (trabajo de CPU)**: medir ciclos por función con el oráculo y aplicar un reloj virtual de CPU (~8 MHz); **no** ajustar animaciones a mano | 🚧 **T-073a ✅** hook `cpu_delay`; **T-073b ✅** coste medido por función (`platform/cpuwork_costs.inc`, 79 funciones). **T-073c ✅ (Maestro C), pendiente validación visual**: A/B/C corregidas. La aceleración de C era el `dbf` inline de `object_info_card` (`0x16dec`): `wait=0x0a` → 40960 iteraciones × 18 ciclos ≈ 92 ms ≈ 4,6 VBL/entrada, sin anotar. Fix autorizado en `src/loop.c` con `cpu_delay_cycles()` (no-op en diferencial; cobra por el reloj de T-073b). C pasa de ~3 a ~7,6 VBL/frame; A/B no usan `16dec`. Exe actual: ver T-074; `make test` 499 y `make test-port` verdes (nueva `test_cpu_charge_cycles`). Detalle: **`notes/0019-fix-timing-c.md`** (resumen en `notes/0017` §8); plan anti-fallo: **`notes/0018-plan-c.md`**; guardia: **`test_click_skip`**. Queda calibrar FPS (`KULT_FPSLOG`/`KULT_ST_CONTEND`/`KULT_CPU_HZ`) y casos no medidos (`palette_cycle`) |
| T-074 | **Input HAL + presentación (puertas / A-B-C)**: izquierdo/teclado = acción sin skip; derecho = acción + skip; presentar por VBL para ver animaciones que corren en `wait_vbl` | ✅ **sesión 23**: acción = izquierdo o derecho; skip = solo derecho; `platform_input_apply_skip()` pisa `$1a570` al salir de `wait_frame` (hook de instrumentación, como el legacy); `platform_tick` presenta en cada VBL. `src/` intacto; `test_click_skip` + `test_present_hook`. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0020-input-hal-y-presentacion.md`** |
| T-075 | **Cursor 16×16 / máximo IKBD**: al llegar al borde derecho el cursor reaparecía por el izquierdo; el juego configura el ratón absoluto a 304×184 | ✅ **sesión 24**: `crt0_init` envía `Ikbdws` `09 01 30 00 b8` (xmax 304, ymax 184); la HAL lo emula con `platform_ikbdws()` y `platform_mouse_set()` recorta a ese máximo. `boot/boot.c` lo llama tras `crt0_init`; `src/` intacto. Prueba `test_ikbd_mouse_max`. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0021-cursor-e-ikbd.md`** |
| T-076 | **Reinicio tras morir**: el juego repetía la secuencia de muerte en vez de volver al Maestro/objeto | ✅ **sesión 25**: `reset_clear` (`0x1682a`) termina en el original con `bra 0x10158`; el C nativo solo hacía el estado. Fix en `boot/` + hook: al salir de `reset_clear`, `kult_request_restart()` hace `longjmp` y `boot_enter_game()` reproduce `0x10158` (modo 7, `0xc001`, Maestro/objeto, `game_loop`). `src/` intacto; `KULT_RESTART_AT` diagnóstico. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0022-reinicio-tras-muerte.md`** |
| T-077 | **Ritmo de la muerte (punto 1)**: sangre (`181f0`/`13d0e`) y viaje espacial (`1834a`/`18316`) iban acelerados | ✅ **sesión 26**: la sangre se cobra por línea en `reveal_line` (`13d0e`) con la fórmula medida **1063 + 198 × pasos** (pasos = max(|dx|,|dy|)+1); `181f0` se deja a 0 para no meter un retardo previo. El viaje espacial cobra `1834a` con `1834a 2301836 101` ≈ 22.790 ciclos/pasada. Tabla regenerada (80 funciones). Sin tocar `src/`. **Validado en Windows**: sangre perfecta; espacio `1834a` 2373–2473 = 101 VBL (~2,02 s), igual que el ASM. Exe `b7edc936…`, headless `7faac3d3…`, ZIP `97d56789…`. Detalle: **`notes/0023-costes-sangre-espacio.md`** |

## M7 — Empaquetado/release
> Primero el **build** (para validar en Windows); la **ingesta `.STX`** se
> adelanta tras T-061 (el usuario tiene imágenes de disquete, no `.PRG`). El
> contenedor `kult.data` puede ir con N3.
| id | tarea | estado |
|---|---|---|
| T-070 | **Build**: portabilidad Windows (`platform/time.c` `_WIN32`, `platform/fs.c` `;`/data junto al exe) + CMake (`kult` headless, `kult-sdl`; en Windows → `kult.exe` GUI + `kult-headless.exe`) + `cmake/mingw-w64.cmake` + SDL3 **3.4.10** MinGW (`third_party/`, gitignored) + icono (`.rc`, RGBA, `SDL_SetWindowIcon`) + **log/diagnóstico** (`kult.log`, MessageBox, preflight de datos) | ✅ build nativo CMake verificado; cross Windows enlaza (sin ejecutar aquí) |
| T-071 | **Empaquetado interino**: CPack ZIP Windows (`kult.exe`+`kult-headless.exe`+`SDL3.dll`+`data/`+`LEEME.txt`+`DIAGNOSTICO.txt`) y TGZ Linux; **sin assets** | ✅ interino (la ingesta va con T-072) |
| T-072 | **Ingesta + `kult.data`**: primero extracción automática `.STX`/`.ST`/`.MSA` → `data/` (o `--ingest`); después contenedor `kult.data` (puede ir con N3) | ⬜ **siguiente tras T-061** |

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

## M5 — Filtros CRT (nota: extra opcional, no prioritario)
> **Nota.** Fuera del camino crítico y **al final** (independiente de A). Es una
> **mejora de presentación**; se desarrollará en algún momento, tras completar el
> plan (`M7 → A1 → N3 → A2`). Se conserva el id **M5/T-050**. Detalle: `notes/0011`.
| id | tarea | estado |
|---|---|---|
| T-050 | `SDL_GPU` + shaders (escalado/CRT), opcional | ⬜ |
