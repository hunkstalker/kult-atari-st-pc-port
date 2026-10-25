# TODO — tareas

Estados: **✅ hecho** · **🚧 en curso** · **⬜ pendiente** · **🧩 feature opcional (no bloqueante)**. Al cerrar una tarea,
cámbiala aquí y anota el resultado en `RETOMAR.md`. Los ids son estables
(`T-xxx`); los nuevos van al final de su fase.

## Índice rápido de IDs (abiertas/opcionales)
| id | tarea | estado |
|---|---|---|
| T-022 | Completar `0x106f2` + `l_0101a8` (`0x135ba` ya portado) | ✅ cerrada (2026-10-25; `notes/0053`) |
| T-061 | Validación completa de la partida en Windows (checklist M6) | ✅ cerrada (2026-10-23) |
| T-072 | Ingesta automática de imágenes `.STX` a `data/kult.data` | ✅ implementada |
| T-073 | Calibración opcional de CPU/ritmo/fidelidad del port | 🧩 feature opcional (no bloqueante) |
| T-090 | SOLAR EYES: flash PSI acelerado (`palette_cycle`) | ✅ cerrado |
| T-093 | Revisión lingüística del catalán | ✅ validada por el usuario (2026-10-25) |
| T-094 | Validación visual final de la release Windows | ✅ cerrada (2026-10-25; `notes/0088`) |
| T-095 | Tests automatizados de ingesta `.STX` y packs `S`/`C` | ✅ cerrada (2026-10-25) |
| T-097 | Texto descuadrado al recibir un objeto (p. ej. “una mosca de pedra”) | ✅ validado por el usuario (2026-10-25) |
| T-098 | Bloqueo en la prueba “La Horca” usando `kult.data`/`language.data` | ✅ validado por el usuario (2026-10-25) |
| T-099 | Pasada de textos ingleses sin traducir | ✅ 240/241 corregidos (`notes/0047`) |
| T-092 | Publicar release 1.0.0: `main`, tag, GitHub Release y paquetes Windows/Linux | ✅ publicada (2026-10-25; CI OK; `notes/0092`) |
| T-135 | Paquetes Linux TGZ + AppImage | ✅ cerrada; CI generó TGZ/AppImage (`notes/0091`, `notes/0092`) |
| T-096 | CI en GitHub: HAL + paquetes Windows/Linux | ✅ cerrada; primera ejecución OK (`notes/0092`) |
| T-136 | macOS Universal 2 (Intel + Apple Silicon) | 🚧 implementado; CI/QA pendientes (`notes/0093`) |
| T-137 | Publicar release 1.1.0 con macOS | ⬜ pendiente de tag y GitHub Release |
| T-100 | Diagnósticos separados del timing y guía de uso | ✅ cerrada (2026-10-25) |
| T-101 | «DIE» (objeto) traducido como MORIR; corregir a DADO/DAU | ✅ cerrada (2026-10-25; `notes/0054`) |
| T-102 | Arranque fullscreen nativo, F1 fullscreen/ventana y F2 escala entera | ✅ cerrada (2026-10-25; `notes/0055`) |
| T-103 | Savestate completo por ranura (0-9) y persistencia de la ranura activa | ✅ cerrada (2026-10-25; `notes/0056`) |
| T-104 | Atajos F5/F6/F7 integrados en el bucle | ✅ cerrada (2026-10-25; `notes/0057`) |
| T-105 | Documentación de release, ZIP y hashes tras los atajos | ✅ cerrada (2026-10-25; `notes/0058`) |
| T-106 | OSD con la fuente original del juego | ✅ cerrada (2026-10-25; `notes/0059`) |
| T-107 | Ranuras `slot-N.save` | ✅ cerrada (2026-10-25; `notes/0060`; migración retirada en T-108) |
| T-108 | Retirar la migración de `partida-N.bin` (formato no publicado) | ✅ cerrada (2026-10-25; `notes/0061`) |
| T-109 | F2 Rellenar: estirar a toda la ventana/pantalla | ✅ cerrada (2026-10-25; `notes/0062`; orden/etiqueta final en T-110) |
| T-110 | F2: ciclo Estirar a pantalla → ×1..×6 → Auto y OSD | ✅ cerrada (2026-10-25; `notes/0063`) |
| T-111 | OSD: caducidad por tiempo real (no por VBL) | ✅ cerrada (2026-10-25; `notes/0064`) |
| T-112 | Volumen F9/F10/F11 con OSD del estado | ✅ cerrada (2026-10-25; `notes/0065`) |
| T-113 | Incluir `Kult Settings.png` junto al `.exe` en release/CI | ✅ cerrada (2026-10-25; `notes/0066`) |
| T-114 | Arranque en Estirar y configuración F en `kult.ini` | ✅ cerrada (2026-10-25; `notes/0067`) |
| T-115 | DESC 126: “caminante salva a algunos…” en ES/CA | ✅ cerrada (2026-10-25; `notes/0068`; sustituida por T-116) |
| T-116 | DESC 126 con base ERBE en ES y catalán derivado | ✅ parcial (solo DESC 126); sustituida por T-117 |
| T-117 | Español completo desde ERBE + correcciones | ✅ cerrada (2026-10-25; `notes/0070`) |
| T-118 | Catalán diferencial desde ERBE + correcciones | ✅ cerrada (2026-10-25; `notes/0071`; terminología en T-119/T-120) |
| T-119 | Terminología catalana alineada con ERBE (`CALZE`→`GOT`, `XARXA TUNER`) | ✅ cerrada (2026-10-25; `notes/0072`; preferencias en T-120) |
| T-120 | Preferencias catalanas: mantener `DAGA`, `FESOL` y `COMERCIANT` | ✅ cerrada (2026-10-25; `notes/0073`) |
| T-121 | Traducir el diálogo deformado del personaje de `STA KLAROS TA VIEN` | ✅ cerrada (2026-10-25; `notes/0074`; clave en T-122) |
| T-122 | Reescribir el diálogo deformado con clave catalana (`STA KLAR, STA BE`) | ✅ cerrada (2026-10-25; `notes/0075`; clave final en T-123) |
| T-123 | Clave fonética catalana creativa (`LASIT UAC IOESDES…`) | ✅ cerrada (2026-10-25; `notes/0076`; ajuste en T-124) |
| T-124 | Ajuste de `DIALC 162` (`EMBOLUPA AM NORMA LYTAT`) | ✅ cerrada (2026-10-25; `notes/0077`) |
| T-125 | Revisión completa de todo el diálogo en clave fonética | ✅ cerrada (2026-10-25; `notes/0078`) |
| T-126 | Erratas de verbos catalanes (`LLENCAR`/`LLANCAR`, cambios y duplicados) | ✅ cerrada (2026-10-25; `notes/0079`) |
| T-127 | Añadir la `Ç` catalana a la fuente y a todos los textos | ✅ cerrada (2026-10-25; `notes/0080`) |
| T-128 | Correcciones de textos catalanes (`BARRIGA`, `ATRAVIESES`, `NAVE`, `ATREUS`) | ✅ cerrada (2026-10-25; `notes/0081`) |
| T-129 | F5 en dos pasos: mostrar ranura y luego rotar | ✅ cerrada (2026-10-25; `notes/0082`) |
| T-130 | Añadir `TECLAS_FN.txt` a la release/CI | ✅ cerrada (2026-10-25; `notes/0083`; renombrado en T-131) |
| T-131 | Instrucciones de atajos y F12 inerte en release | ✅ cerrada (2026-10-25; `notes/0084`; renombrado en T-132) |
| T-132 | Renombrar las instrucciones a `Kult Settings.txt` | ✅ cerrada (2026-10-25; `notes/0085`) |
| T-133 | Limpieza de artefactos y fuentes de diseño | ✅ cerrada (2026-10-25; `notes/0086`) |
| T-134 | `make deps-win` reproducible | ✅ cerrada (2026-10-25; `notes/0087`) |

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
| T-022 | Portar `0x135ba` y `0x106f2` | ✅ **sesión 69**: `0x135ba` (`os_setscreen(-1,-1,0)` en `_start`) y `0x106f2` (`load_resource_list_c`, lista C `0x1a2c8`) portados; 519 tests diferenciales verdes y `make regscan` `ids_reg=0`. `l_0101a8` reconstruido en `boot/` como **ruta latente verificada** (`KULT_SOFT_RESTART_AT`), sin flujo de juego artificial. Smokes de reinicio suave y arranque normal OK. **No confundir** con el reinicio tras morir (`0x1842c`→`reset_clear`; su `bra 0x10158` se emula en `boot/`, ver T-076). Detalle: `notes/0053` |
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
> 1.0.0 está preparada. Orden de cierre: **validación visual final → tests
> automatizados → publicar → CI**. T-097/T-098 ya están validados. El catalán ya está
> validado por el usuario (`notes/0043`).
| id | tarea | estado |
|---|---|---|
| T-070 | **Build**: portabilidad Windows (`platform/time.c` `_WIN32`, `platform/fs.c` `;`/data junto al exe) + CMake (`kult` headless, `kult-sdl`; en Windows → `kult.exe` GUI + `kult-headless.exe`) + `cmake/mingw-w64.cmake` + SDL3 **3.4.10** MinGW (`third_party/`, gitignored) + icono (`.rc`, RGBA, `SDL_SetWindowIcon`) + **log/diagnóstico** (`kult.log` con timestamp, preflight de datos) | ✅ build nativo CMake verificado; cross Windows enlaza (sin ejecutar aquí) |
| T-071 | **Empaquetado portable**: CPack ZIP Windows (`kult.exe`+`SDL3.dll`+`data/`+`saves/`+`LEEME.txt`+licencias) y TGZ Linux; **sin assets, sin headless ni DIAGNOSTICO** | ✅ |
| T-072 | **Ingesta + `kult.data`**: detección de `.STX` por SHA-256, extracción Pasti/FAT12, validación de `KULT.PRG` y generación atómica de `data/kult.data` | ✅ implementada |
| T-091 | **`language.data`**: DRAP + packs de idioma; español y catalán revisados incluidos | ✅ |
| T-093 | **Revisión lingüística del catalán**: textos, nombres propios y estilo | ✅ **validada por el usuario (2026-10-25)** |
| T-094 | **Validación visual final** de la release Windows: DRAP, fuentes, textos, menús y atajos F1/F2/F5/F6/F7 | ✅ **validada por el usuario (2026-10-25)**: ZIP de T-134, 13 entradas, SHA256 `5ed0e019…`. Detalle: **`notes/0088`** |
| T-095 | **Tests automatizados de ingesta y packs**: fixtures sintéticos de `.STX`/`kult.data` y cobertura de `S`/`C` | ✅ **sesión 65**: `platform/diskimage.{h,c}` (STX Pasti + FAT12) y 7 pruebas nuevas en `platform/test/test_platform.c` (30 funciones/163 `CHECK`): STX sin/comprimir, FAT12, validación de `KULT.PRG`, pipeline condicional con los STX reales, contenedor atómico, selección/lectura de `S`/`C` y `assets/language.data`. `make test` 518 core + HAL + 1 port OK. Detalle: **`notes/0049`** |
| T-097 | **Texto descuadrado al recibir objetos**: revisar espacios/códigos en la frase de entrega | ✅ **validado por el usuario (2026-10-25)** |
| T-098 | **Bloqueo en “La Horca”**: reproducir con `kult.data` + `language.data` y localizar la causa | ✅ **validado por el usuario (2026-10-25)** |
| T-092 | **Publicación 1.0.0**: push de `main`, tag, GitHub Release y paquetes Windows/Linux | ✅ **publicada (2026-10-25)**: `main`/`v1.0.0` en `8eb079a`; [release](https://github.com/hunkstalker/kult-atari-st-pc-port/releases/tag/v1.0.0) con ZIP Windows, TGZ Linux, AppImage y checksums; CI en `success`. Detalle: **`notes/0092`** |
| T-096 | **CI en GitHub**: `test-port` + build/paquete sin assets. El core diferencial requiere `work/disk_a/KULT.PRG` (no versionado): queda como ejecución local o con un fixture privado, no como gate del CI público. | ✅ **cerrada (2026-10-25)**: workflow `release.yml` con tests HAL/herramientas, ZIP Windows, TGZ Linux, AppImage, checksums y GitHub Release; primera ejecución en `main` y `v1.0.0` en `success`. Detalle: **`notes/0089`**, **`notes/0090`**, **`notes/0091`**, **`notes/0092`** |
| T-100 | **Diagnósticos separados del timing y guía de uso** | ✅ **sesión 66**: `platform/diag.{c,h}` con todos los logs opcionales; `cpuwork.c` solo timing/hooks; buffer F12 de 2 MiB ahora perezoso (BSS 2.225.448 → 128.328 B en `build/kult`). Nuevo `make test-diag` (off/on) integrado en `test-port`. Guía `docs/DIAGNOSTICO.md`. Validación funcional del usuario (2026-10-25), sin cerrar T-094. Detalle: **`notes/0050`**, **`notes/0051`** |
| T-101 | **«DIE» (objeto) mal traducido como MORIR**: el verbo 116 es el sustantivo «dado», no «morir» | ✅ **sesión 70**: `VERB 116` corregido a **DADO** (bytes oficiales Erbe `e4 24 2f`) y a **DAU** en catalán (`e4 35`); `language.data` regenerado (77.747 B, `e69cae5a…`). Nueva regresión `test/test_language_banks.py` (BIN + contenedor); `make test` 519 core + HAL + diagnóstico + 2 herramientas; ZIP `aba7eca8…`. Los «die» verbales de `DIAL` no se tocan. Detalle: **`notes/0054`** |
| T-102 | **Arranque fullscreen nativo + F1/F2**: fullscreen sin bordes a resolución nativa, F1 alterna ventana/fullscreen y F2 cicla Auto/1x–6x con viewport entero propio | ✅ **sesión 71**: `platform/display.c` (escala/viewport puro), `platform/osd.c` (aviso breve), `platform/video_sdl.c` sin presentación lógica y con ratón corregido por viewport, `app/main.c` con `KULT_SCALE=0` Auto y `KULT_FULLSCREEN=0`; tests de display/OSD en `test-port`; `make port`/`port-sdl` OK. Detalle: **`notes/0055`** |
| T-103 | **Savestate completo por ranura (0-9)**: `KSLOTP01` (1 MiB + paleta + base visible), `saves/slot-N.save`, persistencia en `kult.ini`, OSD y ciclo de ranura | ✅ **sesión 71**: `platform/slots.c`, `platform_video_restore()`, `platform_saves_dir()`, `kult.ini` con `save_slot`; tests de roundtrip, corrupción, ausencia, rango y persistencia en `test-port`. Detalle: **`notes/0056`** |
| T-104 | **Atajos F5/F6/F7**: F5 cicla ranuras, F6 guarda y F7 carga en el tope del bucle | ✅ **sesión 71**: F5/F6/F7 interceptadas en `platform_sdl_pump`, `platform_in_loop()`, peticiones y `platform_slots_apply()` en `present_cb`; guardado con cursor restaurado y carga con `sound_stop`/`cursor_draw`; tests de solicitud dentro/fuera del bucle y avisos. Detalle: **`notes/0057`** |
| T-105 | **Documentación y ZIP** con F1/F2 y ranuras | ✅ **sesión 71**: LEEME/README/BUILD/TESTS/VERIFICACION/LIMITACIONES/RELEASE_NOTES actualizados; ZIP regenerado con 11 entradas; `make test` 519 core + 33 funciones / 209 `CHECK` + diag + 2 herramientas. ZIP SHA256 `66bd1af5…` (sustituido por T-106). Detalle: **`notes/0058`** |
| T-106 | **OSD con la fuente original del juego** | ✅ **sesión 71**: `platform_osd_draw_rgba()` lee los glifos 4×6 de `A_FONT_DATA` (`0x1d202`) y pinta en el RGBA del juego; se elimina `SDL_RenderDebugText`; nueva prueba de fuente/fondo/mapeo. ZIP `cf4d9f48…` (sustituido por T-107); `make test` 519 core + 34 funciones / 218 `CHECK`. Detalle: **`notes/0059`** |
| T-107 | **Ranuras `slot-N.save`** | ✅ **sesión 71**: guardado en `saves/slot-0.save`…`slot-9.save`; `kult.ini` intacto. La migración inicial se retiró en T-108 por no haberse publicado el formato anterior. ZIP `1589185f…` (sustituido por T-108). Detalle: **`notes/0060`** |
| T-108 | **Retirar la migración de `partida-N.bin`** | ✅ **sesión 71**: `platform/slots.c` usa solo `slot-N.save`; eliminados escaneo/renombrado y fallback; pruebas de migración retiradas. ZIP `862b20b2…` (sustituido por T-109); `make test` 519 core + 34 funciones / 218 `CHECK`. Detalle: **`notes/0061`** |
| T-109 | **F2 Rellenar**: estirar 320×200 a toda la salida | ✅ **sesión 71**: `PLATFORM_SCALE_FILL`, viewport completo, mapeo de ratón no entero y OSD de relleno; T-110 fijó el orden y la etiqueta final. ZIP `9a62f3d4…` (sustituido por T-110); `make test` 519 core + 34 funciones / 227 `CHECK`. Detalle: **`notes/0062`** |
| T-110 | **F2: Estirar a pantalla → ×1..×6 → Auto** | ✅ **sesión 71**: nuevo orden del ciclo, OSD literal `ESTIRAR A PANTALLA`, log `estirar`; pruebas de ciclo actualizadas. ZIP `1cfd3a55…` (sustituido por T-111); `make test` 519 core + 34 funciones / 227 `CHECK`. Detalle: **`notes/0063`** |
| T-111 | **OSD por tiempo real** | ✅ **sesión 71**: TTL con `platform_now_ns()` (2,5 s); `platform_osd_show_ms()`; caduca aunque el juego esté bloqueado en `Cnecin`; nueva prueba sin VBL. ZIP `528de4ef…` (sustituido por T-112); `make test` 519 core + 34 funciones / 228 `CHECK`. Detalle: **`notes/0064`** |
| T-112 | **Volumen F9/F10/F11** | ✅ **sesión 71**: volumen maestro ±10%, mute/restauración y OSD `VOLUMEN N`/`SILENCIO`; F11 deja de ser marca de animación. ZIP `d4c125ee…` (sustituido por T-113); `make test` 519 core + 35 funciones / 240 `CHECK`. Detalle: **`notes/0065`** |
| T-113 | **`Kult Settings.png` en la release** | ✅ **sesión 71**: imagen versionada e instalada junto a `kult.exe`; paquete de 12 entradas; verificado con `unzip -l`. ZIP `16c004ba…` (sustituido por T-114). Detalle: **`notes/0066`** |
| T-114 | **Arranque en Estirar + config F persistente** | ✅ **sesión 71**: `scale` por defecto 7 (Estirar); `kult.ini` guarda escala, fullscreen, volumen, mute y ranura; `platform/config.c` centraliza el parser. ZIP `07b91456…` (sustituido por T-115); `make test` 519 core + 36 funciones / 255 `CHECK`. Detalle: **`notes/0067`** |
| T-115 | **DESC 126** | ✅ **sesión 71**: primera corrección ES/CA; sustituida por T-116. ZIP `1384cec5…`. Detalle: **`notes/0068`** |
| T-116 | **DESC 126 con base ERBE** | ✅ **parcial**: solo se cambió DESC 126; el alcance completo se hizo en T-117/T-118. ZIP `e0e2993b…`. Detalle: **`notes/0069`** |
| T-117 | **Español completo desde ERBE** | ✅ **sesión 71**: bancos ES desde ERBE + `make_es_fixed`, con `SAI FAI`→`SCI FI`, DADO y centinela DESC 406; `language.data` 77.881 B; regresión `test_erbe_es_import`. Detalle: **`notes/0070`** |
| T-118 | **Catalán diferencial desde ERBE** | ✅ **sesión 71**: 1.051 entradas comparadas; 93 actualizadas (9/34/24/26), resto conservado por equivalencia semántica; T-093 mantenido. ZIP `3592ff91…`; 5 herramientas. Detalle: **`notes/0071`** |
| T-119 | **Terminología catalana alineada con ERBE** | ✅ **sesión 71**: `CALZE`→`GOT`, `TRAGO`→`GLOP`, `TELA`→`TERANYINA`, `FRASCO`→`FLASQUET` y `NETWERK`/`REZZO`→`XARXA TUNER`; alienígena 184/238/239 intacto. `language.data` 77.896 B, `0d9582d9…`; ZIP `3d8cf7d6…`; 5 herramientas. Detalle: **`notes/0072`** |
| T-120 | **Preferencias catalanas del usuario** | ✅ **sesión 71**: se conservan `DAGA`, `FESOL` y `COMERCIANT` (revertidos `GANIVET`, `BEINA` y `CANVIADOR`). `language.data` 77.903 B, `a3e606b5…`; ZIP `de37885a…`; 5 herramientas. Detalle: **`notes/0073`** |
| T-121 | **Diálogo deformado traducido** | ✅ **sesión 71**: `DIALC` 143/147/155/156/157/162/164/165/166 pasan de inglés fonético a catalán; `DESCC 45` usa `ELS CINC CRANIS`. `language.data` 77.887 B, `3c554a1d…`; ZIP `79450f59…`; 5 herramientas. Detalle: **`notes/0074`** |
| T-122 | **Diálogo deformado en clave catalana** | ✅ **sesión 71**: se rehacen las nueve líneas con la deformación deliberada (`STA KLAR, STA BE`, `TU NER?`, `DESAKTIVAT`, `SITUASIO KONFUSA`). `language.data` 77.880 B, `cd4c9a04…`; ZIP `a6a982ee…`; 5 herramientas. Detalle: **`notes/0075`** |
| T-123 | **Clave fonética catalana creativa** | ✅ **sesión 71**: cortes y cambios fonéticos (`LASIT UAC IOESDES EBOLUPA AM NORMA YTAT`, `DES BRAN XAT`, `AI TA UN KIT TANT`, `DES BRAN XEU LOS`). `language.data` 77.903 B, `923dba18…`; ZIP `229db647…`; 5 herramientas. Detalle: **`notes/0076`** |
| T-124 | **Ajuste de `DIALC 162`** | ✅ **sesión 71**: `EMBOLUPA AM NORMA LYTAT`. `language.data` 77.905 B, `4a2cae27…`; ZIP `237d8274…`; 5 herramientas. Detalle: **`notes/0077`** |
| T-125 | **Diálogo en clave completo** | ✅ **sesión 71**: 75 entradas pasan a clave fonética catalana usando EN/FR/DE; solo quedan sonidos, nombres y control. `language.data` 77.904 B, `2e83f042…`; ZIP `295465c6…`; 5 herramientas. Detalle: **`notes/0078`** |
| T-126 | **Erratas de verbos catalanes** | ✅ **sesión 71**: `60 ESTIRAR`, `81 LLANÇAR SOBRE`, `164 TIRAR`; además 1, 13/17, 52, 71/83 y 145. `language.data` 77.916 B, `99a1816e…`; ZIP `466ae50b…`; 5 herramientas. Detalle: **`notes/0079`** |
| T-127 | **Ç catalana** | ✅ **sesión 71**: `CARACC.BIN` con glifo `1C 10 10 10 1C 04`, código `0x0b`; conversión de todas las palabras con cedilla. `language.data` 78.513 B, `1809189c…`; ZIP `1c1e8f6f…`; 5 herramientas. Detalle: **`notes/0080`** |
| T-128 | **Correcciones de textos catalanes** | ✅ **sesión 71**: `BARRIGA`→`VENTRE`, `ATRAVIESES`→`TRAVESSES`, `NAVE`→`NAU`, `T'ATREUS`→`T'ATREVEIXES`, `S'ATREVE`→`S'ATREVEIX` y `DIALC 176` acortado; `130` en tres líneas. `language.data` 78.511 B, `38ff7f70…`; ZIP `435df4be…`; 5 herramientas. Detalle: **`notes/0081`** |
| T-129 | **F5 en dos pasos** | ✅ **sesión 71**: primer F5 muestra la ranura actual; segundo F5 con el aviso visible rota. `language.data` sin cambios, `38ff7f70…`; ZIP `2a127b57…`; 5 herramientas. Detalle: **`notes/0082`** |
| T-130 | **`TECLAS_FN.txt` en release/CI** | ✅ **sesión 71**: hoja F1–F12 instalada por CMake; paquete de 13 entradas. ZIP `2316f64a…`; 5 herramientas. Detalle: **`notes/0083`** |
| T-131 | **Instrucciones y F12 en release** | ✅ **sesión 71**: archivo renombrado a `Instrucciones - Atajos de teclado.txt`, sin teclas inútiles ni F12; F12 solo con `KULT_MARKS`. ZIP `d45392a3…`; 13 entradas. Detalle: **`notes/0084`** |
| T-132 | **`Kult Settings.txt`** | ✅ **sesión 71**: el archivo de atajos pasa a `Kult Settings.txt`; contenido sin cambios. ZIP `5ed0e019…`; 13 entradas. Detalle: **`notes/0085`** |
| T-133 | **Limpieza y fuentes** | ✅ **sesión 71**: `.af` en `assets/design/`, xlsx fuera del índice, `temp/` y builds limpiados; ZIP/trazas conservados. Detalle: **`notes/0086`** |
| T-134 | **`make deps-win`** | ✅ **sesión 71**: SDL3 3.4.10 con hashes y extracción idempotente; ZIP sin cambios. Detalle: **`notes/0087`** |
| T-135 | **Linux TGZ + AppImage** | ✅ **cerrada (2026-10-25)**: `kult` + `libSDL3.so.0` con RPATH, `LEEME.txt` Linux, AppRun con `KULT_BASE_DIR`, `make deps-linux`; CI generó TGZ y AppImage. Detalle: **`notes/0091`**, **`notes/0092`** |

## M8 — macOS y release 1.1.0
| id | tarea | estado |
|---|---|---|
| T-136 | **macOS Universal 2**: `Kult.app` Intel + Apple Silicon, selector STX, Application Support, ZIP/DMG y CI | 🚧 **implementación local**; falta ejecutar CI macOS, artifact y QA Intel. Sin notarizar (ad-hoc). Detalle: **`notes/0093`** |
| T-137 | **Release 1.1.0**: Windows ZIP, Linux TGZ/AppImage, macOS ZIP/DMG, checksums y notas | ⬜ pendiente de tag/publicación |

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
