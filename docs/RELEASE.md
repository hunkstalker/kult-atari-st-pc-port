# RELEASE — formato y comportamiento del paquete

Documento de decisión para la release del port. Prevalece sobre las partes de
`docs/BUILD.md`, `LEEME.txt` o notas antiguas que describan `data/`, appdata,
`kult-headless.exe` o `DIAGNOSTICO.txt` de forma distinta.

## Estado

La release **1.0.0** está publicada con ZIP Windows, TGZ Linux y AppImage
(detalle: `notes/0092`). La **1.1.0** está en preparación y añade macOS
Universal 2; el ZIP/DMG de macOS se firma en modo ad-hoc y no se notariza.

## Objetivo

Release **portable** en Windows/Linux: todo vive en la carpeta donde el usuario
descomprime el paquete. En macOS, por ser un `.app`, los datos escribibles viven
en `~/Library/Application Support/Kult/`. No hay instalador, appdata de Windows,
`/tmp` ni rutas de datos configurables.

El usuario aporta únicamente sus dos imágenes de disquete:

```text
Kult.STX
Kult (B).STX
```

El port extrae los assets y genera su propio contenedor. Los `.STX` pueden
borrarse después de una ingesta correcta.

## Estructura

```text
<carpeta del exe>/
├── kult.exe
├── SDL3.dll
├── LEEME.txt
├── Kult Settings.txt
├── Kult Settings.png
├── LICENSE
├── THIRD_PARTY_NOTICES.md
├── LICENSES/
├── kult.log              # se crea al ejecutar
├── data/
│   ├── Kult.STX          # entrada opcional; se puede borrar tras ingerir
│   ├── Kult (B).STX      # entrada opcional; se puede borrar tras ingerir
│   ├── language.data     # DRAP + packs español/catalán del port
│   └── kult.data         # generado desde los STX
└── saves/                # escrituras del juego
```

No se incluyen `kult-headless.exe` ni `DIAGNOSTICO.txt` en la release.

## Linux

El TGZ Linux contiene:

```text
kult
lib/libSDL3.so.0
LEEME.txt
Kult Settings.txt
Kult Settings.png
LICENSE
THIRD_PARTY_NOTICES.md
LICENSES/
data/language.data
saves/
```

`kult` lleva RUNPATH `$ORIGIN/lib`; el usuario solo aporta los dos STX. También
se publica un AppImage: `kult-1.1.0-x86_64.AppImage`. Su `AppRun` usa
`KULT_BASE_DIR` para crear `data/` y `saves/` junto al AppImage, y copia
`language.data` y `LEEME.txt` a esa misma carpeta.

Dependencias base: glibc/musl del sistema, X11 o Wayland y audio
ALSA/Pulse. SDL3 va incluida.

## macOS

La release macOS es Universal 2 (Intel `x86_64` + Apple Silicon `arm64`),
requiere macOS 11+ y se publica como ZIP y DMG:

```text
kult-1.1.0-macos-universal.zip
kult-1.1.0-macos-universal.dmg
```

Contenido:

```text
Kult.app/
├── Contents/MacOS/Kult
├── Contents/Frameworks/libSDL3.0.dylib
├── Contents/Resources/LEEME.txt
├── Contents/Resources/kult.icns
├── Contents/Resources/data/language.data
├── Contents/Info.plist
└── Contents/Resources/{settings,licencias}
LEEME.txt
Kult Settings.txt
Kult Settings.png
LICENSE
THIRD_PARTY_NOTICES.md
LICENSES/
```

`Kult.app` va firmada en modo ad-hoc si no se configura
`MACOS_SIGN_IDENTITY`; no está notarizada. Si faltan `data/kult.data` y los STX,
la app muestra un diálogo para seleccionar los dos discos y los copia a
`~/Library/Application Support/Kult/data/`. Los datos, partidas, `kult.ini` y
`kult.log` viven en esa carpeta.

### Desinstalación

- **Solo la app**: cerrar `Kult.app` y arrastrarla a la Papelera. No hay
  instalador, LaunchAgents ni archivos en `/Library`; Application Support se
  conserva.
- **Eliminación completa**: borrar también
  `~/Library/Application Support/Kult/`, que contiene `data/`, `saves/`,
  `kult.ini` y `kult.log`. Hacer copia de `saves/` antes si se quieren conservar
  las partidas.
- El estado opcional de ventanas de macOS puede quedar en
  `~/Library/Saved Application State/com.kultport.kult.savedState`; se puede
  borrar si existe.
- El DMG es de solo lectura: expulsarlo no elimina la app instalada.

## Arranque

1. Crear/verificar `data/` y `saves/` en la base de datos (carpeta del
   ejecutable/AppImage o Application Support en macOS).
2. Escribir `kult.log` en esa base.
3. Si existe `data/kult.data` válido, usarlo y no mirar los `.STX`.
4. Si no existe:
   - buscar `Kult.STX` y `Kult (B).STX` primero en `data/` y después en la base;
   - en macOS, pedir los discos con el diálogo nativo si no aparecen;
   - ingerir, validar y generar `data/kult.data` de forma atómica;
   - no borrar los `.STX` automáticamente.
5. Si no hay `kult.data` ni `.STX`, registrar el error y salir con código 2.

## Contenedores

`kult.data` y `language.data` usan el mismo formato:

```text
magic + versión + tipo (GAME | LANG) + nº de entradas + índice + blobs
```

Índice:

```text
{ nombre, offset, tamaño }
```

`kult.data` (generado desde los STX):

- `KULT.PRG` como fuente de datos, nunca ejecutado;
- `PRES.PI1`, `DRAP.PI1`, `FOND.PI1`;
- `ECHE*.BIN`, `DIAL*.BIN`, `MOTS*.BIN`, `VERB*.BIN`, `DESC*.BIN`;
- `PUZZL.BIN`, `BRUIT.BIN`, `CLEAR.BIN`, etc.

`language.data` (lo aporta el port):

- `DRAP.PI1` de la pantalla de idioma;
- packs de idioma (`es`, `ca`, ...);
- fuente/glifos necesarios;
- manifiesto y checksums.

Prioridad de recursos:

```text
language.data > kult.data > data/ sueltos (solo desarrollo)
```

`language.data` incluye el DRAP de cinco banderas y los packs español (`S`) y
catalán (`C`). Si falta, se usa D/E/F y el `DRAP.PI1` de `kult.data`. Si incluye
el DRAP de cinco banderas, debe incluir ambos packs; si no, se rechaza.

## Errores y log

Las trazas opcionales (`KULT_MARKS`, anillo, F12, etc.) se describen en
`DIAGNOSTICO.md`; no se incluye un binario de diagnóstico separado.

Política **B**:

- todos los errores se registran en `kult.log` con timestamp y prefijo `ERROR:`;
- no se muestran ventanas por errores normales;
- en Windows solo se muestra un diálogo de emergencia si **no se puede escribir**
  `kult.log`; en macOS los errores fatales lanzados desde Finder muestran un
  diálogo visible.

Códigos de salida:

```text
2 = faltan kult.data y los STX
3 = fallo del arranque
4 = fallo de ventana SDL
5 = carpeta no escribible
```

En Windows/Linux no hay fallback a appdata ni `/tmp`; en macOS el fallback es
`~/Library/Application Support/Kult`. `KULT_DATA_DIR` y `KULT_LOG` se eliminan.

## Escrituras del juego

`CLEAR.BIN`, `SCENA.BIN` y cualquier estado modificable van a `saves/`, nunca al
contenedor ni a `data/`. Las ranuras F5/F6/F7 se guardan como
`saves/slot-0.save`…`slot-9.save`; `kult.ini` recuerda la pantalla, escala,
ranura, volumen y silencio. En macOS, `saves/`, `kult.ini` y `kult.log` están en
`~/Library/Application Support/Kult/`.

## Empaquetado

La release contiene:

```text
kult.exe
SDL3.dll
LEEME.txt
Kult Settings.txt
Kult Settings.png
LICENSE
THIRD_PARTY_NOTICES.md
LICENSES/
data/language.data
saves/ (vacía)
```

`Kult Settings.png` es una imagen de referencia del port, no un asset del juego.

## CI y release automática

`.github/workflows/release.yml` se ejecuta en `main` y en tags `v*`:

- `test`: `make test-port` + `make test-tools`.
- `windows`: `make deps-win` + `make package-win`, ZIP y `.sha256`.
- `linux`: `make deps-linux`, CMake, TGZ, AppImage y sus `.sha256`.
- `macos`: `make deps-macos`, CMake Universal 2, ZIP/DMG, firma ad-hoc y
  `.sha256`.
- `release`: en tags `v*`, adjunta ZIP Windows, TGZ/AppImage Linux y ZIP/DMG
  macOS con sus checksums.

Los artefactos de CI pueden no ser byte-idénticos al ZIP validado localmente;
cada paquete incluye su `.sha256`.

No contiene:

- assets del juego;
- `.STX`;
- `kult.data`;
- `kult-headless.exe`;
- `DIAGNOSTICO.txt`;
- tests ni documentación de desarrollo.

Cada bundle lleva su `LEEME.txt`: Windows, Linux TGZ y macOS lo instalan como
`LEEME.txt`; el AppImage lo lleva dentro y su `AppRun` lo copia al lado del
AppImage. Debe explicar el flujo STX → `kult.data`, el log y los errores.
