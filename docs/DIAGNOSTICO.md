# Diagnóstico del port

## Principio

El diagnóstico es **opcional y está separado del juego**: sin variables de
entorno no se abre ningún fichero de traza. El log normal `kult.log` sí está
siempre activo para arranque y errores.

La regla de organización es:

- la lógica del juego solo llama a hooks `kult_diag_*`/`kult_text_ev`;
- la implementación de esos hooks está en `platform/`;
- el diferencial del core los compila como no-op;
- ningún `printf` de investigación se añade directamente al core.

## Dónde vive cada pieza

| pieza | responsabilidad |
|---|---|
| `platform/log.c` | `kult.log`, errores y `platform_fatal` |
| `platform/diag.c` | marcas, F12, anillo, texto, RNG, entrada y buffer circular |
| `platform/diag.h` | puente interno entre instrumentación y diagnóstico |
| `platform/cpuwork.c` | **solo timing de CPU**, mapa función→dirección y hooks de entrada/salida |
| `platform/time.c` | avisa a F12/autolog (`KULT_AUTOLOG`, `KULT_F12_AT`) |
| `platform/input.c` | entrega los eventos de clic al hook de diagnóstico |
| `app/main.c` | FPS periódico (`KULT_FPSLOG`) |
| `platform/dispatch_trace.c` | variante de `dispatch.c` con traza Y/V/D completa |
| `platform/include/kult_port.h` | conecta los hooks del core con `platform/diag.c` |
| `lib/kult-core/include/kult_port.h` | mismos hooks como no-op en el diferencial |

Los ~50 puntos de instrumentación del core son el delta documentado en
`CORE_DELTA.md`. No contienen formato de log ni acceso a ficheros.

## Activación en ejecución

| variable | efecto |
|---|---|
| `KULT_MARKS=<ruta>` | marcas `F/K/B/S/U/P`, texto y F12; abre el log en append |
| `KULT_RINGLOG=<ruta>` | eventos `R` del anillo y `T` de texto, sin el resto de marcas |
| `KULT_RANDLOG=<ruta>` | lecturas `D` de la tabla ALEAT |
| `KULT_INPUTLOG=<ruta>` | eventos `I` de entrada |
| `KULT_AUTOLOG=1` | volcado del historial cada 150 VBL |
| `KULT_F12_AT=<vbl>` | simula F12 en un VBL concreto |
| `KULT_SOFT_RESTART_AT=<vbl>` | fuerza `l_0101a8` (lista C → `crt0` → lista A → `0x100ec`) y vuelve a `0x10158` |
| `KULT_FPSLOG=1` | FPS del bucle cada ~3 s en `kult.log` |

F12 vuelca los ~150 VBL anteriores, la última línea de texto (`X`) y abre una
ventana de ~75 VBL de traza de funciones. **Solo actúa si `KULT_MARKS` está
activo**; en una ejecución normal no escribe en `kult.log` ni genera traza. El
historial circular (2 MiB) se reserva **solo al activar `KULT_MARKS`**; no ocupa
BSS en una ejecución normal.

Ejemplo Windows:

```bat
set KULT_MARKS=%~dp0kult-marcas.log
set KULT_AUTOLOG=1
kult.exe
```

Ejemplo Linux/headless:

```sh
KULT_MARKS=/tmp/kult-marks.log KULT_FRAMES=20 ./build/kult
```

## Traza completa de la VM

`KULT_DIAG_TRACE=ON` cambia `lib/kult-core/src/dispatch.c` por
`platform/dispatch_trace.c` (líneas `Y/V/D`). Se compila con:

```sh
make cmake-win-diag
```

No forma parte del build estándar ni del paquete. Al cambiar
`lib/kult-core/src/dispatch.c` hay que revisar también esa variante: es una
copia de desarrollo y debe mantenerse sincronizada.

## Release

- El ZIP/TGZ no incluye tests, `kult-diag` ni logs.
- El ejecutable estándar conserva los diagnósticos activables por entorno para
  poder diagnosticar una instalación; sin variables no generan ficheros.
- El modelo de CPU (`-finstrument-functions` + `cpuwork.c`) **no es
  diagnóstico**: es necesario para el ritmo del juego y no debe desactivarse al
  limpiar logs.

## Pruebas

```sh
make test-diag   # humo off/on, sin assets
make test-port   # incluye test-diag + pruebas HAL
make test        # core + HAL + herramientas
```

`test-diag` comprueba que sin variables no se crea ningún fichero y que con
ellas aparecen las líneas `F/K/B/S/U/P/R/T/D/I/Y/V/X/M`, incluido el volcado
F12. Los formatos de log también los consume `tools/ringreport.py`: no
cambiarlos sin revisar esa herramienta.
