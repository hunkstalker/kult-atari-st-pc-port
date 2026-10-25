# Datos, contenedores e ingesta

## Carpetas

```text
<dir del exe>/
├── kult.log
├── data/
│   ├── Kult.STX          # opcional; se puede borrar tras ingerir
│   ├── Kult (B).STX      # opcional; se puede borrar tras ingerir
│   ├── language.data     # overlay de idioma del port (opcional)
│   └── kult.data         # assets del juego generados desde los STX
└── saves/
```

Orden de lectura:

```text
saves/ > language.data > kult.data > data/ sueltos
```

Las escrituras del juego van siempre a `saves/`.

## Savestates del port (T-103)

Las ranuras F5/F6/F7 viven en `saves/slot-0.save`…`slot-9.save`:

```text
offset  tamaño  contenido
0       8       "KSLOTP01"
8       4       versión = 1 (u32 BE)
12      4       ranura 0..9 (u32 BE)
16      32      paleta: 16 words ST (u16 BE)
48      4       base visible (u32 BE)
52      4       frame/VBL (u32 BE)
56      4       0x100000 (u32 BE)
60      4       reservado
64      1 MiB   imagen completa del motor
```

`kult.ini` (junto al ejecutable) guarda `save_slot = 0..9`. La carga valida
magia, versión, ranura y tamaño antes de modificar la partida.

## Contenedor `KULTDTA1`

`kult.data` y `language.data` comparten formato:

```text
offset  tamaño  contenido
0       8       "KULTDTA1"
8       4       versión = 1 (u32 LE)
12      4       tipo: 1 = GAME, 2 = LANG (u32 LE)
16      4       número de entradas (u32 LE)
20      ...     índice de entradas
        ...     blobs
```

Cada entrada del índice ocupa 72 bytes:

```text
64 B  nombre ASCII (mayúsculas, terminado en NUL)
4 B   offset absoluto (u32 LE)
4 B   tamaño (u32 LE)
```

`kult.data` contiene los 28 ficheros extraídos:

- `KULT.PRG` (fuente de datos, nunca se ejecuta);
- `PRES.PI1`, `DRAP.PI1`, `FOND.PI1`;
- `ECHE0..7.BIN`, `PUZZLE`/`PUZZL.BIN`, `BRUIT.BIN`, `CLEAR.BIN`;
- `DESC*`, `DIAL*`, `MOTS*`, `VERB*`;
- `DESKTOP.INF`.

`language.data` contiene los recursos de idioma del port, empezando por
`DRAP.PI1` y los packs que se añadan.

La release incluye `assets/language.data`, generado desde `assets/lang/`:

```sh
python3 tools/build_language_data.py
```

Contiene `DRAP.PI1`, el pack español (`VERBS`, `DESCS`, `MOTSS`, `DIALS`,
`CARACS`) y el pack catalán (`VERBC`, `DESCC`, `MOTSC`, `DIALC`). El catalán es
inicial, sin tildes ni `Ç` por límites de la fuente, pendiente de revisión
lingüística.

## Ingesta `kult.data`

1. Si `data/kult.data` existe y es válido, se usa y no se miran los STX.
2. Si no existe o está corrupto, se buscan candidatos `.STX`/`.stx` en:
   - `data/`
   - carpeta del ejecutable
3. Cada candidato se identifica por **SHA-256**:

```text
Disco A: d34e1efa34c2f4d3073ca61221d52f4eedd190fc2d2a07f5cbdef525eab017c1
Disco B: f4ef13da5039821baedf8378b47358cf354d471afc7f647b35ded1988b8a064e
```

4. Se exige un A y un B; nombres de fichero irrelevantes.
5. Se parsea Pasti/STX y se reconstruye la imagen raw.
6. Se lee FAT12 y se extraen los ficheros.
7. Se valida `KULT.PRG` por SHA-256:

```text
2e85f223167439425ab2096bd21716de135f9b472bde90f4d40e8b0e03132fe6
```

8. Se escribe `data/kult.data.tmp` y se renombra a `data/kult.data`.

Los STX no se borran automáticamente. Tras una ingesta correcta el usuario puede
eliminarlos; el juego ya no los necesita.

## `language.data`

El port carga `data/language.data` antes que `kult.data`. Sus entradas tienen
prioridad para los mismos nombres. Si falta o está incompleto, se usa el
contenido D/E/F de `kult.data`.

La pantalla `DRAP.PI1` de cinco banderas debe ir acompañada de los packs de los
idiomas que anuncia. Si se incluye S y C, deben existir ambos; si no, se usa el
`DRAP.PI1` base.

## Errores

- Sin `kult.data` y sin los dos STX: error y salida 2.
- STX desconocido o duplicado: error en `kult.log` y salida 2.
- `kult.data` corrupto: se regenera si hay STX; si no, error.
- Carpeta no escribible: error y salida 5.
- Todo se registra en `kult.log` con timestamp; no hay ventanas salvo el diálogo
  de emergencia cuando no se puede escribir el log.
