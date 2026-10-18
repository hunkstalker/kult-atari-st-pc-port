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
| T-020 | Portar `_start` (0x10000), incluido el menú de idioma | 🚧 provisional en `boot/boot.c` (camino sin DRAP con `KULT_SKIP_DRAP`); falta validar el menú fiel y el clic |
| T-021 | Portar el tick de VBL `0x160c2` (paleta + blits GAUSS/MURSM) | ✅ en `boot/vbl.c`, con prueba (`make test-port`) |
| T-024 | **Diagnóstico del bloqueo de `0xc001`** | ✅ **causa raíz: el loader C `boot/prg.c` aplicaba una relocación de más tras cada `RELOC_SKIP` (byte 1)**, corrompiendo 544 bytes de la imagen (entre ellos un largo de ICONE) → el guion tomaba otra rama. Corregido; el loader coincide byte a byte con el oráculo y tiene prueba de regresión |
| T-022 | Portar `0x135ba` y `0x106f2` | ⬜ |
| T-023 | Omitir la comprobación FDC `0x19316` | ⬜ |

## M3 — Render y entrada reales (primera ventana)
| id | tarea | estado |
|---|---|---|
| T-030 | Ventana SDL3 + `SDL_Renderer`; planar→RGBA; paleta de 16 words; escalado entero | ⬜ |
| T-031 | Entrada real: teclado y ratón → paquete IKBD / `A_CURSOR_X/Y` | ⬜ |
| T-032 | Ritmo: simulación a cadencia fija + presentación al refresco | ⬜ |

## M4 — Audio
| id | tarea | estado |
|---|---|---|
| T-040 | Modelo del **MFP Timer A** que dispara las ISR C ya reconstruidas | ⬜ |
| T-041 | Emulación **PSG/YM2149** desde `psg_port_write/read` | ⬜ |
| T-042 | Salida por **SDL audio** | ⬜ |

## M5 — Filtros CRT
| id | tarea | estado |
|---|---|---|
| T-050 | `SDL_GPU` + shaders (escalado/CRT), opcional | ⬜ |

## M6 — Pulido y validación
| id | tarea | estado |
|---|---|---|
| T-060 | Menú de idioma (validar el fiel; valorar menú nativo) | ⬜ |
| T-061 | Validación: arranque, partida, guardado, audio | ⬜ |

## A — Workstream Amiga
| id | tarea | estado |
|---|---|---|
| T-A00 | Determinación y censo: comparar el binario Amiga por rutina con el ST | ⬜ |
| T-A01 | Decomp ASM del Amiga (spec byte a byte) | ⬜ |
| T-A02 | Core Amiga (lógica compartida + capa Amiga) + verificación | ⬜ |
| T-A03 | Arte/audio/textos Amiga | ⬜ |

## M7 — Empaquetado/release
| id | tarea | estado |
|---|---|---|
| T-070 | Builds Windows y Linux; paquete con ingesta | ⬜ |

## Documentación
| id | tarea | estado |
|---|---|---|
| T-D00 | Índice y ADRs (`docs/INDICE.md`, `docs/adr/`) | ✅ |
| T-D01 | Mantener `RETOMAR.md` y este `TODO.md` al cerrar cada sesión | 🚧 |
