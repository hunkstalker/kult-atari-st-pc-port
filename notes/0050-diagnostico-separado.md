# 0050 — Diagnóstico separado del timing (T-100)

Fecha: 2026-10-25. Estado: **cerrado**.

## Objetivo

Preparar la organización antes de publicar 1.0.0:

- documentar cómo se aplican los diagnósticos;
- separar los logs opcionales del modelo de timing;
- no reservar memoria de diagnóstico cuando no se usa;
- cubrir el comportamiento con una prueba de humo.

Sin cambios de comportamiento del juego, variables ni formatos de log.

## Cambios

### `platform/diag.c` + `platform/diag.h`

Todo el diagnóstico activable por entorno sale de `platform/cpuwork.c`:

- ficheros `KULT_MARKS`, `KULT_RINGLOG`, `KULT_RANDLOG`, `KULT_INPUTLOG`;
- marcas `F/K/B/S/U/P`, eventos `R/T/D/I`, líneas `Y/V`, F12 (`X/M`);
- buffer circular F12 y última línea de texto;
- puntos internos `platform_diag_tick`, `platform_diag_func_begin/end` y
  `platform_diag_bank` para la instrumentación.

`platform/cpuwork.c` conserva el cobro de ciclos, la tabla función→dirección y
los hooks de HAL (`wait_frame`, `reset_clear`). El core no cambia: sus hooks
siguen llegando a través de `platform/include/kult_port.h` y son no-op en el
diferencial.

### Buffer F12 perezoso

Antes: `MARK_RING = 1<<18` entradas estáticas (~2 MiB de BSS) en todos los
binarios. Ahora se hace `calloc` solo al abrir `KULT_MARKS`.

Medida con `size build/kult`:

- antes: BSS 2.225.448 B;
- después: BSS 128.328 B;
- ahorro: 2.097.120 B.

### Prueba de humo

Nuevo `platform/test/test_diag.c` y target `make test-diag`, incluido en
`make test-port`:

- proceso `off`: sin variables no se crea ningún fichero;
- proceso `on`: comprueba `F/K/B/S/U/P/R/T/D/I/Y/V/X/M`, la ventana F12 y los
  cuatro ficheros de log.

Solo enlaza `platform/diag.c` con stubs de imagen y reloj; no necesita assets.

### Documentación

Nueva guía `docs/DIAGNOSTICO.md`: arquitectura, variables, build de traza VM,
política de release, pruebas y mantenimiento de `dispatch_trace.c`. Referencias
actualizadas también en `ARQUITECTURA.md`, `VERIFICACION.md`, `RELEASE.md`,
`test/README.md`, `platform/include/kult_port.h` y `tools/ringreport.py`.

Además se aclara `make test`: la receta heredada de `kit.mk` ya ejecuta
`test/`; no se añade `test-tools` como prerrequisito ni se sobreescribe la
receta, evitando dobles pasadas y warnings.

## Verificación

- `make test-diag`: off/on OK.
- `make test-port`: HAL + diagnóstico OK.
- `make test`: 518 core + HAL/diagnóstico + 1 herramienta OK.
- `make port`, CMake nativo y cross MinGW OK.
- `make package-win` OK; ZIP con 11 entradas, sin tests ni `kult-diag`.
- `build-win/kult.exe`: BSS 2.482.080 → 384.960 B.
- Humo real headless con `KULT_MARKS`/`KULT_AUTOLOG`: arranque OK, log con
  marcas `F/K/S` y volcados `M auto`.
- ZIP candidato: `build-win/kult-1.0.0-win64.zip`, SHA256
  `b959d76ad0aa2a1a4175a00ac2a0705efa567c13d3abe63c7ab7509ba334bec3`.
