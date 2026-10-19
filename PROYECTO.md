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
| `/app/kult-port-legacy` | predecesor de modding | solo referencia; no se toca |

## 3. Arquitectura de capas (objetivo)

```
src/        juego en C (copiado de kult-atari-st-c) — INTACTO
include/    addrs.h / kult_api.h / …               — INTACTO
test/       499 pruebas diferenciales              — INTACTO
boot/       _start (0x10000) y tick VBL (0x160c2)  — NUEVO (reconstrucción)
platform/   HAL PC: os_* / hw_* / psg_* / sched_* + render + audio — NUEVO
app/        main + driver + presentación           — NUEVO
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
| `0x135ba`, `0x106f2` | rutinas pequeñas del arranque |
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
| **M2** | portar `_start` y `0x160c2` | ✅ `_start` **fiel** (menú validado en T-020; `0x106f2` diferido) + tick `0x160c2` |
| **M3** | render SDL3 + entrada real (primera ventana jugable) | ✅ ventana + entrada real + ritmo 50 Hz |
| **M4** | audio (MFP Timer A + PSG + SDL audio) | ✅ hecho (T-040/041/042) |
| **M6** | pulido y validación | pendiente |
| **A** | workstream **Amiga**: spec (decomp) + core | pendiente |
| **M7** | empaquetado/release (Windows/Linux) | pendiente |
| **M5** | filtros CRT (SDL_GPU + shaders) — **nota: extra opcional, no prioritario** | ⬜ nota (desarrollar en algún momento, tras validar) |

Detalle por fase: **`docs/PLAN.md`**.

> **Nota (M5).** La prioridad es **terminar el juego** (M6 validación → M7
> empaquetado → Amiga); los filtros CRT son una **mejora de presentación
> opcional** que se desarrollará en algún momento, fuera del camino crítico. Se
> conserva el id **M5/T-050** como referencia en el plan.

## 7. Decisiones cerradas
Recogidas como ADR en `docs/adr/`:
- SDL3 (`0001`), `SDL_Renderer` base y `SDL_GPU` para CRT (`0002`),
  audio en hito propio (`0003`), menú de idioma fiel (`0004`), `boot/` fuera de
  `src/` (`0005`), headless primero (`0006`).

## 8. Estado (punto de control)
- **Fase M0 cerrada**: core importado y **499 diferenciales verdes** (`make test`).
- **Fase M1 hecha**: HAL PC (`platform/`), loader + `_start` provisional (`boot/`)
  y driver headless (`app/`). **`make test-port` verde**; `make port` arranca el
  juego y vuelca la sala cargada.
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
  el usuario**. `0x106f2` (reinicio) queda diferido.
- **Siguiente:** validación del usuario (M6/T-061) → **M7 (empaquetado)** → Amiga.
  **M5 (CRT) como extra opcional.** Sin bloqueantes.

> Mantener esta sección y `RETOMAR.md` al día al cerrar cada sesión.
