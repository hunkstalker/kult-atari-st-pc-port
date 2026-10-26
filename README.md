# Kult: The Temple of Flying Saucers — Port a PC (Atari ST)

Port nativo (**Windows, Linux y macOS**) de **Kult** (Exxos/ERE, 1989), con
**español y catalán**.

![Kult Settings](assets/Kult%20Settings.png)

No parte de cero: el **C verificado** del Atari ST está importado en
`lib/kult-core/` y este repo añade un **backend de PC** (SDL3) para que ese mismo
C corra en Windows, Linux y macOS.

> **Estado:** el port es jugable de principio a fin en Windows y Linux. La
> versión **1.1.0** está publicada (2026-10-26) con macOS Universal 2 (Intel +
> Apple Silicon), firmado en modo ad-hoc y no notarizado. La **1.2.0** está
> publicada (2026-10-26) y añade el cursor a refresco nativo. Incluye la ingesta
> de los discos originales, el selector de idioma,
> los packs español y catalán, guardado/carga y audio, además de **fullscreen
> nativo**, **F1**
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
- **Windows 10 o superior, 64 bits**, **Linux x86_64** o **macOS 11 o superior**
  (Intel o Apple Silicon) para los paquetes precompilados. Linux y macOS también
  se pueden compilar desde el código fuente.
  Release: <https://github.com/hunkstalker/kult-atari-st-pc-port/releases>.
- Una copia legal del juego con los dos discos en formato `.STX`:
  `Kult.STX` y `Kult (B).STX`. Los nombres no importan; se identifican por
  SHA-256.

## Primera ejecución

El paquete no incluye el juego: necesitas `Kult.STX` y `Kult (B).STX`. En la
primera ejecución se validan los discos, se genera `data/kult.data` y el juego
continúa arrancando en esa misma ejecución. **No hace falta ejecutarlo dos
veces.** En las siguientes ejecuciones, si existe `data/kult.data`, los `.STX`
ya no son necesarios y puedes borrarlos.

En Windows y Linux no uses una carpeta de solo lectura (`Program Files`,
`/opt`): el juego escribe `data/`, `saves/`, `kult.ini` y `kult.log` junto al
ejecutable o al AppImage. En macOS escribe en Application Support.

### Windows ZIP

1. Descomprime `kult-1.2.0-win64.zip` en una carpeta escribible.
2. Copia `Kult.STX` y `Kult (B).STX` junto a `kult.exe` o dentro de `data/`.
3. Ejecuta `kult.exe` con doble clic.
4. La primera ejecución crea `data/` y `saves/`, ingiere los discos y genera
   `data/kult.data`; al terminar, el juego arranca.
5. Para volver a jugar, ejecuta de nuevo `kult.exe`. Si mueves el juego, mueve
   también `data/` y `saves/`.

### Linux TGZ

1. Descomprime `kult-1.2.0-linux-x86_64.tar.gz` en una carpeta escribible.
2. Copia los `.STX` junto a `kult` o dentro de `data/`.
3. Ejecuta el juego:

   ```sh
   ./kult
   ```

   Si falta el permiso de ejecución: `chmod +x kult`.

4. La primera ejecución ingiere los discos, genera `data/kult.data` y arranca
   el juego en el mismo proceso.
5. Para volver a jugar, ejecuta otra vez `./kult`; si mueves la carpeta, mueve
   `data/` y `saves/` con ella.

### Linux AppImage

1. Copia `kult-1.2.0-x86_64.AppImage` a una carpeta escribible y dale permiso:

   ```sh
   chmod +x kult-1.2.0-x86_64.AppImage
   ```

2. Copia los `.STX` junto al AppImage. También puedes ponerlos en `data/` si ya
   existe esa carpeta.
3. Ejecuta:

   ```sh
   ./kult-1.2.0-x86_64.AppImage
   ```

4. El AppImage es un único archivo: su contenido interno es de solo lectura y se
   monta temporalmente. Al arrancar, `AppRun` crea `data/` y `saves/` junto al
   AppImage, copia `language.data` y `LEEME.txt`, ingiere los discos y arranca el
   juego en esa misma ejecución.
5. Para volver a jugar, ejecuta el mismo AppImage. Si lo mueves de carpeta,
   mueve también `data/` y `saves/`.
6. Si el sistema no tiene FUSE:

   ```sh
   ./kult-1.2.0-x86_64.AppImage --appimage-extract-and-run
   ```

### macOS (Universal 2)

1. Descarga `kult-1.2.0-macos-universal.zip`, descomprímelo y abre el
   `kult-1.2.0-macos-universal.dmg` que contiene. Copia `Kult.app` a una carpeta
   normal, por ejemplo `Aplicaciones` o `Escritorio`.
2. Arranca `Kult.app` con doble clic. Si macOS avisa de que no está verificada,
   usa clic derecho > **Abrir** o **Ajustes del Sistema > Privacidad y
   seguridad > Abrir igualmente**. La build es ad-hoc, no está notarizada.
3. Si no hay `kult.data`, la app pide seleccionar `Kult.STX` y `Kult (B).STX`.
   Selecciona los dos. Se copian a `~/Library/Application Support/Kult/data/`,
   se procesan y el juego arranca en esa misma ejecución.
4. Los datos y partidas quedan en `~/Library/Application Support/Kult/`. Las
   siguientes ejecuciones usan `data/kult.data` y no piden los STX.
5. **Desinstalar la app:** ciérrala y arrastra `Kult.app` a la Papelera. Los
   datos, ajustes y partidas de Application Support se conservan.
6. **Eliminación completa:** si quieres borrar también los datos y las partidas,
   haz antes copia de `saves/` y ejecuta:

   ```sh
   rm -rf "$HOME/Library/Application Support/Kult"
   ```

   Opcionalmente, si existe:

   ```sh
   rm -rf "$HOME/Library/Saved Application State/com.hunkstalker.kult.savedState"
   ```

## Controles
- **Ratón**: izquierdo/teclado = acción; derecho = acción + skip.
- **F1**: pantalla completa/ventana.
- **F2**: escala **Estirar a pantalla**, ×1–×6 o Auto; Estirar llena toda la salida.
- **F5**: muestra la ranura actual; otra pulsación mientras se ve el aviso cambia de ranura; **F6**: guarda; **F7**: carga.
- **F9/F11**: bajan/suben el volumen 10%; **F10**: silencia/restaura.
- **ESC**: cierra la ventana.
- Lista completa de atajos en `Kult Settings.txt`.

## Datos y diagnóstico
- Release **portable**: `data/` contiene los assets del usuario y `saves/` las
  escrituras del juego. En Windows y Linux TGZ viven junto al ejecutable; en el
  AppImage, junto al propio AppImage; en macOS, en
  `~/Library/Application Support/Kult/`. Las ranuras F5/F6/F7 van a
  `saves/slot-0.save`…`slot-9.save`; `kult.ini` recuerda pantalla, escala,
  ranura, volumen y silencio. La escala inicial por defecto es Estirar a
  pantalla.
- **`kult.log`** se escribe junto al ejecutable/AppImage o en Application
  Support en macOS, con timestamp. En macOS los errores fatales se muestran en
  un diálogo; en Windows/Linux van al log (salvo fallo del propio log).
  `kult-headless.exe` es solo una herramienta de desarrollo y no forma parte de
  la release.

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
