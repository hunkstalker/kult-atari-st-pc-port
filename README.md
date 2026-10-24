# Kult: The Temple of Flying Saucers — Port a PC (Atari ST)

Port nativo a PC (**Windows y Linux**) de **Kult** (Exxos/ERE, 1989), con
**español y catalán**.

No parte de cero: el **C verificado** del Atari ST está importado en
`lib/kult-core/` y este repo añade un **backend de PC** (SDL3) para que ese mismo
C corra en Windows/Linux.

> **Estado:** **M0–M4 hechas**, **M6 cerrada** (T-060 ✅; **T-061 ✅ validada en
> Windows 2026-10-23**; **T-089 ✅** final del juego; **T-090 ✅** SOLAR EYES) y
> **M7 build/empaquetado interino hechos** (T-070/T-071: CMake Linux/Windows,
> `kult.exe` + ZIP, `kult.log` de diagnóstico). El juego arranca (menú de idioma
> fiel), se ve y suena (ventana SDL3 + audio); `make test` (518 core + test propio)
> y `make test-port` verdes. T-073 queda como feature opcional. El estado vivo está en **`PROYECTO.md`** y las tareas
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
- **`docs/DATA_FORMAT.md`** — formato de `kult.data`/`language.data` e ingesta STX.
- **`docs/LANGUAGE_PACKS.md`** — packs español/catalán y limitaciones de fuente.
- **`docs/TESTS.md`** — inventario y política de tests.
- **`notes/`** — notas de sesión (histórico).

## Objetivo (resumen)
- Juego nativo en C, **sin emulador** (Musashi solo se usa como oráculo en las
  pruebas del core).
- Base **Atari ST**; el trabajo sobre Amiga queda para fases posteriores.
- **Español y catalán**, además de D/E/F.
- Backend **SDL3** (ventana, entrada, audio, GPU) + renderizador planar→RGB.

## Datos y diagnóstico
- Release **portable**: todo vive junto al `.exe`. `data/` contiene los assets
  del usuario y `saves/` las escrituras del juego. No hay appdata ni rutas de
  datos alternativas.
- Copia `Kult.STX` y `Kult (B).STX` en `data/` o junto al `.exe`. En el primer
  arranque se extraen, se validan por SHA-256 y se genera `data/kult.data`.
  Después puedes borrar los `.STX`.
- **`kult.log`** se escribe en la raíz, junto al `.exe`, con timestamp. Los
  errores no abren ventanas; solo si el log no se puede escribir aparece un
  diálogo de emergencia. `kult-headless.exe` es solo una herramienta de
  desarrollo y no forma parte de la release.

## Origen y repos
- **`kult-atari-st-decomp`** — decomp ASM del ST, byte a byte (spec, no se toca).
- **`kult-atari-st-c`** — C verificado del ST, importado en `lib/kult-core/`.
- **`kult-atari-st-pc-port`** — este repo (el port).

Detalle en `PROYECTO.md`.

## Licencia
El código, las herramientas, la documentación y los packs de idioma del port se
distribuyen bajo licencia **MIT** (ver `LICENSE`). Los componentes de terceros
conservan sus licencias (ver `THIRD_PARTY_NOTICES.md` y `LICENSES/`).

Este repositorio no incluye el juego original. El usuario aporta su copia legal;
los assets del juego quedan **gitignored**.
