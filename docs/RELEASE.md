# RELEASE — formato y comportamiento del paquete

Documento de decisión para la release del port. Prevalece sobre las partes de
`docs/BUILD.md`, `LEEME.txt` o notas antiguas que describan `data/`, appdata,
`kult-headless.exe` o `DIAGNOSTICO.txt` de forma distinta.

## Objetivo

Release **portable**: todo vive en la carpeta donde el usuario descomprime el
`.exe`. No hay instalador, appdata, `/tmp` ni rutas de datos configurables.

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

## Arranque

1. Crear/verificar `data/` y `saves/` junto al `.exe`.
2. Escribir `kult.log` en la raíz, junto al `.exe`.
3. Si existe `data/kult.data` válido, usarlo y no mirar los `.STX`.
4. Si no existe:
   - buscar `Kult.STX` y `Kult (B).STX` primero en `data/` y después junto al
     `.exe`;
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
- solo se muestra un diálogo de emergencia si **no se puede escribir**
  `kult.log`.

Códigos de salida:

```text
2 = faltan kult.data y los STX
3 = fallo del arranque
4 = fallo de ventana SDL
5 = carpeta no escribible
```

Sin fallback a appdata, `/tmp` ni directorios alternativos. `KULT_DATA_DIR` y
`KULT_LOG` se eliminan.

## Escrituras del juego

`CLEAR.BIN`, `SCENA.BIN` y cualquier estado modificable van a `saves/`, nunca al
contenedor ni a `data/`. Las ranuras F5/F6/F7 se guardan como
`saves/slot-0.save`…`slot-9.save`; `kult.ini`, junto al ejecutable, recuerda
la ranura activa. Los `partida-N.bin` anteriores se migran automáticamente.

## Empaquetado

La release contiene:

```text
kult.exe
SDL3.dll
LEEME.txt
LICENSE
THIRD_PARTY_NOTICES.md
LICENSES/
data/language.data
saves/ (vacía)
```

No contiene:

- assets del juego;
- `.STX`;
- `kult.data`;
- `kult-headless.exe`;
- `DIAGNOSTICO.txt`;
- tests ni documentación de desarrollo.

`LEEME.txt` debe explicar el flujo STX → `kult.data`, el log y los errores.
