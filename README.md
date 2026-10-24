# Kult: The Temple of Flying Saucers — Compilación PC (Atari ST + Amiga)

Port nativo a PC (**Windows y Linux**) de **Kult** (Exxos/ERE, 1989) que **une
las versiones Atari ST y Amiga**, con mejoras y **todos los idiomas**.

No parte de cero: el **C verificado** del Atari ST está importado en
`lib/kult-core/` y este repo añade un **backend de PC** (SDL3) para que ese mismo
C corra en Windows/Linux.

> **Estado:** **M0–M4 hechas**, **M6 cerrada** (T-060 ✅; **T-061 ✅ validada en
> Windows 2026-10-23**; **T-089 ✅** final del juego; **T-090 ✅** SOLAR EYES) y
> **M7 build/empaquetado interino hechos** (T-070/T-071: CMake Linux/Windows,
> `kult.exe` + ZIP, `kult.log` de diagnóstico). El juego arranca (menú de idioma
> fiel), se ve y suena (ventana SDL3 + audio); `make test` (519) y
> `make test-port` verdes. T-073 queda como feature opcional. El estado vivo está en **`PROYECTO.md`** y las tareas
> en **`TODO.md`**; el punto de retorno entre sesiones, en **`RETOMAR.md`**. Aquí
> no se duplica: `README` solo orienta.

## Core importado

El núcleo vive en **`lib/kult-core/`**, importado del repo
[`kult-atari-st-c`](https://github.com/hunkstalker/kult-atari-st-c) mediante
`git subtree`. El build usa solo `lib/kult-core/src` e `include`; el resto de
documentación, tests y herramientas del core quedan disponibles pero no se
compilan.

Los cambios propios del port dentro del core están documentados en
[`docs/CORE_DELTA.md`](docs/CORE_DELTA.md). Al actualizar el core:

```sh
git subtree pull --prefix=lib/kult-core \
  https://github.com/hunkstalker/kult-atari-st-c.git main --squash
```

## Documentación
- **`PROYECTO.md`** — contexto maestro, arquitectura y estado.
- **`TODO.md`** — tareas pendientes / en curso / completadas.
- **`RETOMAR.md`** — handoff de sesión (qué retomar y cómo).
- **`AGENTS.md`** — reglas y arranque para agentes/colaboradores.
- **`docs/INDICE.md`** — índice de toda la documentación.
- **`docs/CORE_DELTA.md`** — core importado y cambios locales del port.
- **`docs/RELEASE.md`** — formato portable, contenedores y errores de release.
- **`docs/TESTS.md`** — inventario y política de tests.
- **`notes/`** — notas de sesión (histórico).

## Objetivo (resumen)
- Juego nativo en C, **sin emulador** (Musashi solo se usa como oráculo en las
  pruebas del core).
- **Dos experiencias**: Atari ST y Amiga; perfil libre (arte/audio/ritmos).
- **Todos los idiomas**.
- Backend **SDL3** (ventana, entrada, audio, GPU) + renderizador planar→RGB.

## Datos y diagnóstico
- El usuario aporta su copia en `data/` (junto al ejecutable). En el paquete
  interino (T-071) deben ser los ficheros **extraídos** de los disquetes
  (`KULT.PRG`, `*.PI1`, `*.BIN`); las imágenes `.STX/.MSA/.ST` aún no se leen.
  La **ingesta automática** es **T-072** (siguiente; T-061 ya cerrada).
- **`kult.log`** se escribe junto al ejecutable (y a stderr) con directorios de
  datos, ruta de `KULT.PRG`, resultado de SDL/ventana/audio y exit code. En GUI,
  los errores fatales salen también en un `MessageBox`. `kult-headless.exe`
  (en el ZIP) da el mismo diagnóstico por consola. `KULT_LOG=<ruta>` lo redirige
  y `KULT_LOG=0` lo apaga.

## Origen y repos
- **`/app/kult-atari-st-decomp`** — decomp ASM del ST, byte a byte (spec, no se toca).
- **`kult-atari-st-c`** — C verificado del ST, importado en `lib/kult-core/`.
- **`/app/kult-atari-st-pc-port`** — este repo (la compilación).

Detalle en `PROYECTO.md`.

## Legal
El juego sigue bajo copyright (Piko Interactive). **No** se distribuyen assets ni
binarios; el usuario aporta su copia. Todo lo derivado del juego queda
**gitignored**.
