# Kult: The Temple of Flying Saucers — Port 1.1.0

Segunda release pública del port nativo de **Kult: The Temple of Flying
Saucers** (Exxos/ERE, 1989): **Windows, Linux y macOS**, con español y catalán.

Fecha: 2026-10-25. Tag: `v1.1.0`.

El port ejecuta el C verificado del Atari ST sobre un backend SDL3, sin emulador.
No incluye el juego original: el usuario aporta su propia copia legal.

## Destacado

- **macOS Universal 2**: un único `Kult.app` para Macs Intel (`x86_64`) y Apple
  Silicon (`arm64`) con macOS 11+, con SDL3 incluida.
- **Primera ejecución guiada en macOS**: si falta `kult.data`, la app pide los
  dos `.STX` y los copia a `~/Library/Application Support/Kult/data/`.
- **Datos macOS en Application Support**: `data/`, `saves/`, `kult.ini` y
  `kult.log` quedan en `~/Library/Application Support/Kult/`.
- **Un `LEEME.txt` por plataforma**: Windows, Linux y macOS llevan sus propias
  instrucciones. El AppImage copia su `LEEME.txt` junto al archivo al arrancar.
- Se mantienen las mejoras de 1.0.0: fullscreen nativo, F1/F2, ranuras F5/F6/F7,
  OSD con la fuente original, volumen F9/F10/F11, español ERBE y catalán con `Ç`.

## Paquete

- Windows x64: `kult-1.1.0-win64.zip` (`kult.exe`, `SDL3.dll`, datos, ajustes y
  `LEEME.txt`).
- Linux x86_64: `kult-1.1.0-linux-x86_64.tar.gz` (`kult`,
  `lib/libSDL3.so.0`, datos, ajustes y `LEEME.txt`).
- Linux AppImage: `kult-1.1.0-x86_64.AppImage` (un solo archivo; crea `data/` y
  `saves/` junto a él y copia `LEEME.txt`).
- macOS Universal 2: `kult-1.1.0-macos-universal.zip` y `.dmg`
  (`Kult.app`, `LEEME.txt`, ajustes y licencias).

Las instrucciones completas de primera ejecución están en `README.md` y en el
`LEEME.txt` de cada plataforma. El usuario solo aporta `Kult.STX` y
`Kult (B).STX`; los nombres no importan, se identifican por SHA-256.

## macOS: firma y Gatekeeper

`Kult.app` es Universal 2 y está **firmada en modo ad-hoc**, pero **no está
notarizada por Apple**. Al descargarla, macOS puede mostrar un aviso de
seguridad. Para abrirla:

1. Clic derecho sobre `Kult.app` > **Abrir**.
2. O ir a **Ajustes del Sistema > Privacidad y seguridad** y permitir la app.

No hace falta tener una cuenta Apple Developer para usar esta build. Una
notarización oficial queda para una release posterior. La build Universal 2 se
verifica en CI para `arm64` y `x86_64`; la prueba visual manual prevista es en un
Mac Intel. Si aparece un problema específico de Apple Silicon, se corregirá en
una revisión posterior.

## Primera ejecución

- **Windows/Linux**: copia los `.STX` junto al ejecutable o en `data/` y
  ejecuta el juego. La primera ejecución ingiere los discos y arranca; no hace
  falta ejecutarlo dos veces.
- **AppImage**: coloca los `.STX` junto al AppImage y ejecútalo desde una carpeta
  escribible. La primera ejecución crea `data/`, `saves/` y `LEEME.txt` a su
  lado.
- **macOS**: abre `Kult.app`; si faltan los discos, selecciónalos en el diálogo.
  Si existe `~/Library/Application Support/Kult/data/kult.data`, las siguientes
  ejecuciones no piden los STX.

Los checksums SHA-256 de los artefactos se adjuntan a la GitHub Release.
Los artefactos de CI no son necesariamente byte-idénticos a builds locales por
los timestamps del empaquetado.

## Licencias

El port y sus packs de idioma se distribuyen bajo licencia MIT. Las licencias
de terceros están en `THIRD_PARTY_NOTICES.md` y `LICENSES/`. El repositorio no
incluye el juego original.
