# 0049 — Tests automatizados de ingesta y packs (T-095)

Fecha: 2026-10-25. Estado: **cerrado**.

## Objetivo

Cubrir en `make test` la ingesta de los discos originales y los packs de idioma
sin depender siempre de los datos del usuario:

- detección de STX por SHA-256;
- extracción Pasti/STX (pistas sin comprimir y comprimidas);
- extracción FAT12 y validación de `KULT.PRG`;
- generación atómica de `kult.data`;
- selección/carga de los packs `S` y `C`.

## Cambios

### Código de producción

La parte pura de `platform/ingest.c` se extrajo a `platform/diskimage.{h,c}`:

- `kult_stx_name()` — extensión `.STX`/`.stx`;
- `kult_stx_to_raw()` — STX Pasti a imagen de sectores;
- `kult_fat12_extract()` — FAT12 a `kult_filelist`;
- `kult_filelist_find()` / `kult_filelist_free()`;
- `kult_ingest_validate()` — 28 ficheros obligatorios + SHA-256 de `KULT.PRG`;
- `kult_ingest_known_disk()` — discos A/B conocidos por SHA-256.

`platform/ingest.c` conserva la orquestación: búsqueda de candidatos, detección
por hash, generación de `kult.data` y escritura atómica. No cambia el
comportamiento ni el formato; el pipeline real sigue generando 28 ficheros.

### Pruebas (`platform/test/test_platform.c`)

Se añadieron 7 funciones (30 en total, 163 `CHECK`):

1. `test_diskimage_stx` — STX sintético sin comprimir (9 sectores en su LBA),
   comprimido con tabla de cabeceras (pista 1), cabecera inválida y extensión.
2. `test_diskimage_fat12` — imagen FAT12 sintética con cadena de clusters,
   entradas borradas/LFN/directorio ignoradas, duplicado sin distinguir
   mayúsculas y BPB inválido.
3. `test_ingest_validate` — falta de un fichero obligatorio y `KULT.PRG` con
   hash desconocido. Camino positivo condicional con el PRG real.
4. `test_ingest_pipeline` — condicional: usa los STX reales de
   `build/data`, `build-win/data` o `data`, comprueba la detección A/B por
   SHA-256, ejecuta `platform_ingest_ensure()`, verifica `kult.data` (28
   entradas, `KULT.PRG`/`PRES.PI1`, sin `.tmp`) y que la capa de ficheros sirve
   el PRG. Sin discos, exige un fallo limpio.
5. `test_container_atomic` — el temporal desaparece tras el rename; un fallo de
   escritura no toca el contenedor anterior; tipo/magic inválidos se rechazan.
6. `test_language_packs` — `language.data` sintético con `VERBS`/`VERBC`:
   lectura exacta, selección `s`/`c` y caída a `E` si no hay pack.
7. `test_language_assets` — condicional: el `assets/language.data` real contiene
   `DRAP.PI1`, `CARACS`, los cuatro pares S/C no vacíos y los pares no son
   idénticos.

Las pruebas que necesitan datos no versionados se omiten con una nota si no
están; el resto son siempre deterministas y no usan red.

## Verificación

```sh
make test      # 518 core + HAL (30 funciones) + 1 port
make port      # build headless
cmake --build build-cmake -j
make cmake-win # cross MinGW
```

Todo en verde. `platform/ingest.c` real genera 28 ficheros desde los STX del
árbol de trabajo.

## Organización (ajuste posterior)

- `make test` queda documentado: `test-core` + `test-port` y la receta heredada
  de `kit.mk` que ejecuta `test/` (equivalente a `test-tools`). No se añade una
  segunda receta ni otra dependencia, para no duplicar suites ni emitir warnings.
- `test/README.md` y `docs/TESTS.md` fijan dónde vive cada suite.
- La política queda documentada: los tests permanecen en el repositorio, los
  binarios de release no los incluyen y las fuentes de GitHub sí.
- T-096 (CI) se limita a `test-port` + paquete sin assets; el core diferencial
  necesita `work/disk_a/KULT.PRG`.
