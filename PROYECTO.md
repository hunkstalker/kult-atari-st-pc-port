# PROYECTO — contexto maestro y estado

Documento raíz de **`kult-port-pc-compilation`**. Es el punto de entrada: qué es
el proyecto, cómo se organiza, qué piezas hay y en qué estado están. Las tareas
viven en **`TODO.md`**; el punto de retorno de sesión, en **`RETOMAR.md`**; el
índice de documentación, en **`docs/INDICE.md`**.

## 1. Qué es

Port nativo a **Windows y Linux** de **Kult: The Temple of Flying Saucers**
(Exxos/ERE, 1989) que:
- **une las versiones Atari ST y Amiga**,
- corre como **C nativo sin emulador**,
- incluye **mejoras** y **todos los idiomas**.

**Idea central:** el juego no habla con el Atari, sino con una **capa de
servicios**. Esa capa es lo que se sustituye por un backend de PC.

```
              ┌───────────────────────────────┐
App/driver → │  juego (src/, C verificado)   │  ← 415 funciones, 499 tests
              └───────────────┬───────────────┘
                              │ 19 servicios (contrato fijo)
              ┌───────────────▼───────────────┐
              │  backend PC = "nuestro TOS"   │  SDL3 + SO (ficheros, entrada,
              └───────────────────────────────┘  vídeo, audio, tiempo)
```

## 2. Repos implicados

| repo | papel | regla |
|---|---|---|
| `/app/kult-atari-st-decomp` | decomp ASM del ST, byte a byte | spec congelada; no se toca |
| `/app/kult-atari-st-c` | C verificado del ST | **base**; se copia aquí (M0) |
| **este repo** | la compilación (ST+Amiga, PC) | el trabajo |

## 3. Arquitectura de capas (objetivo)

```
src/        juego en C (copiado de kult-atari-st-c) — INTACTO
include/    addrs.h / kult_api.h / …               — INTACTO
test/       499 pruebas diferenciales              — INTACTO
boot/       _start (0x10000) y tick VBL (0x160c2)  — NUEVO (reconstrucción)
platform/   HAL PC: os_* / hw_* / psg_* / sched_* + render + audio — NUEVO
app/        main + driver + presentación           — NUEVO
assets/     icono de la app (`kult_icon.ico`), recurso del .exe — NUEVO
docs/       documentación · notes/ · adr/
```

`src/` **no se modifica**: si algo hubiera que cambiarlo, sería señal de que la
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
- Los 415 funciones de `src/` y los 499 tests diferenciales.

### 5.2 Se reconstruye en C (lógica del original que falta)
| pieza | qué es |
|---|---|
| `_start` (0x10000) | arranque + **menú de idioma** (único nombre sin C) |
| `0x160c2` | tick de VBL: contador `A_DAT_1ACD4` + trabajo por frame |
| `0x135ba` | XBIOS `Setscreen(-1,-1,0)` → `os_setscreen` no-op (✅) |
| `0x106f2` | reinicio suave `l_0101a8` (**abierto**; parece inobservable). **No** es el reinicio tras morir (ese es `0x1842c`→`reset_clear`, ya en el core) |
| `0x19316` | comprobación FDC → **se omite** en PC |

### 5.3 Se implementa (capa de PC)
- La HAL (los 19 servicios) con **SDL3** + SO.
- **Renderizador**: framebuffer planar 320×200/16 colores → RGBA → escalado.
- **Audio**: emulación **MFP Timer A** + **PSG/YM2149** + salida SDL.
- **Tiempo/tick**: reloj que avanza el contador de VBL y ejecuta el tick.

## 6. Fases

| fase | contenido | estado |
|---|---|---|
| **M0** | repo y documentación; importar `src/include/test/tools` | ✅ hecho |
| **M1** | HAL mínima **headless** + driver (sin ventana) | ✅ hecho |
| **M2** | portar `_start` y `0x160c2` | ✅ `_start` **fiel** (menú validado en T-020; `0x106f2` **abierto**) + tick `0x160c2` |
| **M3** | render SDL3 + entrada real (primera ventana jugable) | ✅ ventana + entrada real + ritmo 50 Hz |
| **M4** | audio (MFP Timer A + PSG + SDL audio) | ✅ hecho (T-040/041/042) |
| **M6** | pulido y validación | 🚧 T-060 ✅ (menú fiel); **T-061** = validación del usuario |
| **M7** | build/empaquetado (Windows/Linux) + ingesta | 🚧 **build ✅** (T-070) y **ZIP/TGZ interino ✅** (T-071); **ingesta `.STX` adelantada** tras T-061 (T-072) |
| **A1** | **investigación** Amiga (censo + decomp) | ⬜ **prerrequisito duro de N3** |
| **N3** | **motor nativo + assets nativos** (ADR + rama) | ⬜ hito futuro (tras validación) |
| **A2** | **implementación** Amiga (sobre el motor de N3) | ⬜ |
| **M5** | filtros CRT (SDL_GPU + shaders) — **nota: extra opcional, no prioritario** | ⬜ nota (al final) |

Detalle por fase: **`docs/PLAN.md`**; plan de hitos: **`notes/0011`**.

> **Orden acordado:** `arreglos (portabilidad Windows + docs) → M7 build → T-061
> (validación del usuario) → M7-c (empaquetado + ingesta) → A1 → N3 → A2 → M5`.
> **M5** es un extra de presentación independiente (no se junta con A). **A1** es
> **prerrequisito duro de N3** (su censo/decomp informa el modelo a dos versiones).
> **T-022/`0x106f2`** se deja **abierto** (posible futuro). Detalle: `notes/0011`.

## 7. Decisiones cerradas
Recogidas como ADR en `docs/adr/`:
- SDL3 (`0001`), `SDL_Renderer` base y `SDL_GPU` para CRT (`0002`),
  audio en hito propio (`0003`), menú de idioma fiel (`0004`), `boot/` fuera de
  `src/` (`0005`), headless primero (`0006`).

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
- **M6 en curso (T-060 ✅ / T-061 🚧)**: se cierra el **menú de idioma fiel** de
  `_start` (T-020: clic de título, drenado de teclas, D/E/F; `0x135ba` como
  `os_setscreen` no-op) y se documenta la **validación** (checklist en
  `docs/VERIFICACION.md`, limitaciones en `docs/LIMITACIONES.md`), que **ejecuta
  el usuario**. `0x106f2` (reinicio suave) queda **abierto/pendiente**.
- **Plan fijado (sesión 11, sin código)**: orden `arreglos (portabilidad Windows +
  docs) → M7 build → T-061 validación → M7-c (empaquetado + ingesta) → A1
  (investigación Amiga, prerrequisito duro de N3) → N3 (motor nativo + assets
  nativos, ADR + rama) → A2 (Amiga) → M5 (opcional)`. Detalle: **`notes/0011`**.
- **M7 build hecho (sesión 12, T-070)**: portabilidad Windows (`platform/time.c`
  reloj/espera, `platform/fs.c` `;` + `data/` junto al exe, `KULT_PRG` del dir de
  datos), **CMake** nativo (`kult`, `kult-sdl`) y **cross MinGW**
  (`kult.exe` GUI con icono + `kult-headless.exe`), `cmake/mingw-w64.cmake`,
  SDL3 **3.4.10** MinGW en `third_party/` (gitignored) e icono (`.rc` + RGBA +
  `SDL_SetWindowIcon`). Detalle: **`notes/0012`**.
- **Empaquetado interino hecho (T-071)**: CPack → ZIP Windows
  (`build-win/kult-0.1.0-win64.zip`: `kult.exe`+`SDL3.dll`+`data/`+`LEEME.txt`)
  y TGZ Linux; sin assets. `make test` 499 y `make test-port` siguen verdes.
- **Datos + diagnóstico (sesión 13)**: el `.exe` "no hacía nada" porque en
  `data/` solo había imágenes `.STX` (el paquete interino necesita ficheros
  extraídos). Los `.STX` del usuario (idénticos a los discos del proyecto) se
  extrajeron y se dejaron los 28 ficheros en `data/`. Se añadió **`kult.log`**
  junto al `.exe` (+stderr, `MessageBox` sin consola), **fallo de ventana =
  fatal** (antes bucle invisible) y **preflight de los 26 ficheros** del juego
  con lista clara. ZIP con `kult-headless.exe` + `DIAGNOSTICO.txt`. Detalle:
  **`notes/0013`**.
- **Colores del juego (sesión 14)**: corregidos cambiando la **base de pantalla
  del HAL** (`OS_SCREEN_BASE = 0x98000` en `platform/include/os.h`). Con el valor
  del kit (`0x80000`), pantalla B y su paleta caían dentro de los datos del juego
  (ECHE + heap) → ruido/colores mal. Paleta de `FOND.PI1` correcta y frame
  validado; `src/` intacto; **499 + test-port verdes**. Detalle: `notes/0014`.
- **Primera pantalla (sesión 14)**: corregida; es el **título (PRES)** visible
  con música y espera **tecla o clic**. El negro era un fallo del estado inicial
  del HAL (faltaba aplicar la paleta del PI1 antes de la espera); ahora
  `boot/boot.c` la aplica. Verificado: frame del título idéntico a `PRES.PI1`.
- **Ritmo (T-073a/b, sesión 15)**: implementado el modelo de CPU: `cpu_delay`
  real para los `dbf` transcritos + **coste medido por función** del oráculo
  (`-finstrument-functions` solo en el port, `platform/cpuwork_costs.inc`, 79
  funciones). El port cobra ciclos y cede VBLs; `KULT_CPU=0` lo apaga.
  **T-073c** resuelto (ver abajo); queda calibración por escenas/casos no
  medidos (p.ej. `palette_cycle`). Detalle: `notes/0015`.
- **T-073c — Maestro (pantalla C, resuelto; pendiente validación visual)**:
  **A** 3 retratos pixel + descripciones; **B** 4 retratos lift + descripciones
  (4 bancos `15ac8`/`0x36`); **C** 1 retrato del Maestro + lift + **diálogo**
  (único diálogo y único retrato que se mueve), después de B. La aceleración de
  C era el `dbf` inline de `object_info_card` (`0x16dec`): `wait=0x0a` → 40960
  iteraciones × 18 ciclos ≈ 92 ms ≈ 4,6 VBL/entrada, que el core no anotaba.
  Fix autorizado en `src/loop.c` con `cpu_delay_cycles()` (no-op en diferencial;
  en el port cobra con el reloj de T-073b). C pasa de ~3 a ~7,6 VBL/frame; A/B
  no usan `16dec`. Exe actual: ver T-074. Detalle:
  **`notes/0019-fix-timing-c.md`** (resumen en `notes/0017` §8); plan
  anti-fallo: **`notes/0018-plan-c.md`**; guardia: **`test_click_skip`**.
- **Input/presentación (T-074, sesión 23)**: bug de puertas/A-B-C. El core
  deriva acción y skip del bit0 del IKBD; la HAL lo tenía invertido (izquierdo =
  acción+skip; derecho ignorado). Se corrige en la HAL, como el legacy: acción =
  izquierdo **o** derecho; skip = solo derecho; `platform_input_apply_skip()`
  pisa `$1a570` al salir de `wait_frame` (hook de instrumentación). Además,
  `platform_tick` presenta en **cada VBL**, así se ven las animaciones que corren
  en `wait_vbl` (puertas, wipes). `src/` intacto; `test_click_skip` y
  `test_present_hook`. Exe `d67ad168…`, headless `52fb9898…`, ZIP
  `f7b268e9…`. Detalle: **`notes/0020-input-hal-y-presentacion.md`**.
- **Cursor 16×16 / máximo IKBD (T-075, sesión 24)**: el cursor reaparecía por el
  lado contrario en el borde. El juego configura el ratón absoluto con
  `Ikbdws` `09 01 30 00 b8` → máximo **304×184**; la HAL lo emula con
  `platform_ikbdws()` y `platform_mouse_set()` recorta a ese máximo. `boot/boot.c`
  lo llama tras `crt0_init`; `src/` intacto. Prueba `test_ikbd_mouse_max`.
  Detalle: **`notes/0021-cursor-e-ikbd.md`**.
- **Siguiente:** validar el exe nuevo en Windows: izquierdo/teclado = acción y
  animaciones visibles; derecho = acción + skip; A/B/C y puertas; cursor sin
  wrap en los bordes. Si C no cuadra de ritmo, calibrar con
  `KULT_CPU_HZ`/`KULT_ST_CONTEND`, sin parchear animaciones. Después **T-061**
  (validación Windows: título/idioma, partida/colores, guardado, audio, morir →
  reiniciar) → **T-072** (ingesta `.STX`) → **M7-c** → **A1** → **N3** → **A2**
  → **M5**.

> Mantener esta sección y `RETOMAR.md` al día al cerrar cada sesión.
