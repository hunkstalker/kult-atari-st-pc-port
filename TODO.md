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
| T-022 | Portar `0x135ba` y `0x106f2` | 🚧 `0x135ba` ✅ (`os_setscreen(-1,-1,0)` en `_start`). `0x106f2` (`l_0101a8`, reinicio suave) **pendiente/no portado y ABIERTO**. Análisis estático: `0x101a8` solo es **constante base de relocalización** de la tabla de pistas/iconos (`0x16f90`/`0x16fb4`) y no hay `jsr`/`jmp`/`pea` hacia `l_0101a8` → **parece inobservable**, pero puede necesitarse. **No confundir** con el reinicio tras morir (`0x1842c`→`reset_clear`, ya en el core). Ver `notes/0010`, `notes/0011` |
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
| T-073 | **Ritmo del original (trabajo de CPU)**: medir ciclos por función con el oráculo y aplicar un reloj virtual de CPU (~8 MHz); **no** ajustar animaciones a mano | 🚧 **T-073a ✅** hook `cpu_delay`; **T-073b ✅** coste medido por función (`platform/cpuwork_costs.inc`, 79 funciones). **T-073c 🚧 (Maestro C)**: **A** 3 retratos pixel + descripciones; **B** 4 retratos lift + descripciones (4 bancos `15ac8`/`0x36`); **C** 1 retrato del Maestro + lift + **diálogo** (único diálogo y único retrato que se mueve), después de B. C en los logs: 1219–1249 / 1513–1543; ~8 frames/16 blits a 1–3 VBL; sin `15ac8`/`15da6`. Input: clic izquierdo salta; derecho sin función. Exe estándar `900fd993…` (commit `47e1028`), sin traza. Siguiente: analizar el bucle de frames de C y la espera/reveal que falta (`f_0124da` espera VBL; `f_012510` no; modo C 0x07); no capturar frames. **No cambiar el input.** Handoff autoritativo: **`notes/0017-retomar-maestro.md`**. Queda también calibrar FPS (`KULT_FPSLOG`/`KULT_ST_CONTEND`/`KULT_CPU_HZ`) y casos no medidos (`palette_cycle`) |

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
