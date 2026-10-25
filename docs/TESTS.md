# TESTS — inventario y política

## Inventario actual

| conjunto | ubicación | cantidad | uso |
|---|---|---:|---|
| Tests diferenciales del core | `lib/kult-core/test/` | 519 funciones en 44 `test_*.py` | canónicos del core |
| Tests de herramientas del port | `test/` | 5 funciones en 2 ficheros | ringreport y packs de idioma |
| Tests del kit | `lib/kult-core/tools/recreate_kit/test/` | 339 funciones en 42 ficheros | validación del kit, no de `make test` |
| Tests funcionales HAL | `platform/test/test_platform.c` | 36 funciones / 261 `CHECK` | backend PC, contenedores, ingesta, packs, config, display/OSD, ranuras, audio y Estirar a pantalla |
| Humo de diagnóstico | `platform/test/test_diag.c` | 2 procesos (`off`/`on`) | logs desactivados/activados y F12 |
| Tests de convenciones/mapa/visibilidad | dentro del core | incluidos en los 519 | legibilidad y sincronización |

`make test` ejecuta las tres suites, una sola vez cada una:

- `test-core` → 519 tests del subtree (`lib/kult-core/test/`);
- `test-port` → binario C de la HAL (`platform/test/test_platform.c`) y humo
  del diagnóstico (`platform/test/test_diag.c`, target `test-diag`);
- la receta de `test` heredada de `kit.mk` ejecuta `pytest test/`, equivalente
  a `test-tools` (`test_ringreport.py` y `test_language_banks.py`).

`test-tools` no se añade como prerrequisito porque la receta del kit ya ejecuta
esa misma suite; duplicarla o sobreescribir la receta generaría dobles pasadas o
warnings de Make. Cada target se puede lanzar por separado. Los tests del kit no
forman parte de ninguna de estas suites.

## Dónde va cada test

1. **Core**
   - Los 519 tests viven solo en `lib/kult-core/test/`.
   - Son la fuente canónica y se ejecutan contra el core del subtree.
2. **HAL, ingesta y packs**
   - `platform/test/test_platform.c`.
   - `make test-port` es su entrada.
   - Los fixtures sintéticos se generan en `/tmp`; no se versionan.
3. **Herramientas del port**
   - `test/` (`test_ringreport.py` y `test_language_banks.py`).
   - `make test-tools` es su entrada.
4. **Kit vendor**
   - Los 339 tests del kit se quedan en el subtree como tests del vendor.
   - Se ejecutan aparte, no en `make test`, y no se empaquetan.

## ¿Es normal conservar los tests?

Sí. Los tests son parte del valor del proyecto: fijan el comportamiento
diferencial, las convenciones y las garantías del port. No deben borrarse para
“limpiar” el código ni eliminarse antes de publicar.

## Tests y release

- **Repositorio**: los tests se conservan; son la fuente de verdad.
- **Artefactos binarios** (ZIP Windows / TGZ Linux): no incluyen tests. El
  `install()` de CMake copia el ejecutable, SDL3, `data/language.data`,
  licencias, `Kult Settings.png`, `Kult Settings.txt`, `LEEME_LINUX.txt` en
  Linux y las carpetas `data/`/`saves/`. El ZIP Windows 1.0.0 tiene 13
  entradas; Linux añade TGZ y AppImage.
- **Archivos fuente de GitHub**: los ZIP/TAR de código que genera GitHub
  incluyen todo el repositorio, tests incluidos. Es lo esperado en una fuente.
- No se genera un paquete de fuentes propio. Si algún día se añade, habrá que
  decidir explícitamente si incluye tests (`CPACK_SOURCE_IGNORE_FILES`).
- Los tests que requieren datos no versionados (`KULT.PRG`, STX) son
  condicionales o se omiten con una nota; no bloquean a quien clone el repo.
  `make test-core` sí exige `work/disk_a/KULT.PRG`.

## Estado de la limpieza

Hecho:

- Añadido `make test-core`.
- Eliminada la copia duplicada de los tests del core en `test/`.
- `test/` reducido a `test_ringreport.py` + README.
- `make test` compone core + HAL + herramientas sin duplicar suites.

Hecho (T-118/T-119/T-120/T-121/T-122/T-123/T-124/T-125/T-126/T-127/T-128):

- `test_catalan_differential`: recuentos, entradas catalanas actualizadas,
  términos ERBE (`GOT`, `GLOP`, `XARXA TUNER`), preferencias `DAGA`, `FESOL`
  y `COMERCIANT`, y ausencia de `CALZE`, `TRAGO`, `BEINA`, `CANVIADOR`, `TELA`
  y `REZZO` en los cuatro bancos. T-121–T-125 añaden todo el diálogo deformado en clave fonética catalana y
  `DESCC 45` con `ELS CINC CRANIS`; quedan solo sonidos/nombres/control. T-126 añade la regresión de los diez verbos corregidos y la ausencia de
  `LLENCAR`/`LLANCAR` sueltos. T-127 comprueba `CARACC.BIN`, el glifo `Ç` y las
  palabras con `[0b]`. T-128 cubre `BARRIGA`, `ATRAVIESES`, `NAVE` y `ATREUS`. T-129 cubre F5 en dos pasos. T-130 añade la hoja de teclas al paquete; T-131 quita teclas inútiles y deja
  F12 solo con `KULT_MARKS`; T-132 la renombra a `Kult Settings.txt`.

Hecho (T-117):

- Importación ERBE del español y regresión `test_erbe_es_import` (recuentos,
  DADO, SCI FI, correcciones, centinela y contenedor).

Hecho (T-115/T-116):

- Regresión `test_desc_126_passerby` en `test/test_language_banks.py` para la
  descripción 126 en ES/CA (BIN y `language.data`).

Hecho (T-102–T-104):

- `test_display_viewport`: escala Auto/×1–×6, viewport centrado, Estirar a pantalla, mapeo de ratón y ciclos de F2.
- `test_osd`: texto y caducidad de los avisos por tiempo real, incluso sin VBLs.
- `test_audio_volume`: pasos ±10, mute/restauración, desilencio al ajustar y
  escalado real de la señal (50%/0%).
- `test_config`: `kult.ini`, defaults, reemplazo de claves y conservación de
  claves desconocidas (pantalla, escala, volumen, fullscreen, ranura).
- `test_osd_font`: glifos 4×6 de `A_FONT_DATA`, fondo, mapeo de caracteres y
  ausencia de fuente.
- `test_slots`: roundtrip de 1 MiB + paleta + base visible, ficheros truncados,
  ranura ausente, rango inválido, persistencia de `kult.ini` y peticiones
  F6/F7 dentro/fuera del bucle.

Hecho (T-095):

- Ingesta STX/Pasti y FAT12 con fixtures sintéticos en
  `platform/test/test_platform.c`.
- Validación de `KULT.PRG`, generación atómica de `kult.data` y pipeline
  completo condicional con los STX reales.
- Selección/carga de los packs `S`/`C` y comprobación del `language.data`
  versionado.

Sin hacer:

- Ejecutar los 339 tests del kit en CI; de momento siguen siendo un target
  separado del vendor.
