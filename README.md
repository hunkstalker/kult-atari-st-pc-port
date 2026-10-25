# Kult: The Temple of Flying Saucers — Port a PC (Atari ST)

Port nativo a PC (**Windows y Linux**) de **Kult** (Exxos/ERE, 1989), con
**español y catalán**.

![Kult Settings](assets/Kult%20Settings.png)

No parte de cero: el **C verificado** del Atari ST está importado en
`lib/kult-core/` y este repo añade un **backend de PC** (SDL3) para que ese mismo
C corra en Windows/Linux.

> **Estado:** el port es jugable de principio a fin en Windows y Linux. Incluye
> la ingesta de los discos originales, el selector de idioma, los packs español y
> catalán, guardado/carga y audio, además de **fullscreen nativo**, **F1**
> ventana/fullscreen, **F2** escala Estirar a pantalla/×1–×6/Auto y **F5/F6/F7** para
> ranuras de savestate (`saves/slot-N.save`), con avisos OSD en la **fuente original del
> juego**.
> El seguimiento técnico del desarrollo está en **`PROYECTO.md`**, **`TODO.md`** y
> **`RETOMAR.md`**.

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
- **`docs/REPOSITORIO.md`** — qué se versiona y qué queda fuera.
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

## Requisitos
- **Windows 10 o superior, 64 bits**, para el paquete precompilado. En Linux se
  compila desde el código fuente.
- Una copia legal del juego con los dos discos en formato `.STX`:
  `Kult.STX` y `Kult (B).STX`. Los nombres no importan; se identifican por
  SHA-256.

## Controles
- **Ratón**: izquierdo/teclado = acción; derecho = acción + skip.
- **F1**: pantalla completa/ventana.
- **F2**: escala **Estirar a pantalla**, ×1–×6 o Auto; Estirar llena toda la salida.
- **F5**: muestra la ranura actual; otra pulsación mientras se ve el aviso cambia de ranura; **F6**: guarda; **F7**: carga.
- **F9/F11**: bajan/suben el volumen 10%; **F10**: silencia/restaura.
- **ESC**: cierra la ventana.
- Lista completa de atajos en `Kult Settings.txt`.

## Datos y diagnóstico
- Release **portable**: todo vive junto al `.exe`. `data/` contiene los assets
  del usuario y `saves/` las escrituras del juego. Las ranuras F5/F6/F7 van a
  `saves/slot-0.save`…`slot-9.save`; `kult.ini` recuerda pantalla, escala,
  ranura, volumen y silencio. La escala inicial por defecto es Estirar a pantalla.
  No hay appdata ni rutas de datos alternativas.
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
