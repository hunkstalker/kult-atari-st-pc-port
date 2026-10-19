# Nota 0011 — Plan de hitos: M7 (build), ingesta/assets nativos y N3

- **Sesión 11.** Sin cambios de **código** (solo documentación y el icono copiado a
  `assets/`): se **fija y documenta el plan** para que una sesión nueva lo retome
  sin perder contexto.

## Orden de hitos (acordado)

```
[ahora] Arreglos (portabilidad Windows + docs)  →  M7 build (Windows/Linux)  →
T-061 (validación del usuario)  →  M7-c (empaquetado + ingesta)  →
A1 (investigación Amiga; prerrequisito DURO de N3)  →  N3 (ADR + motor nativo +
assets nativos)  →  A2 (implementación Amiga)  →  M5 (extra opcional, al final)
```

- **M5** (CRT) es un extra de presentación independiente; no se junta con A.
- **T-022 / `0x106f2`** queda **abierto** (posible futuro), no cerrado.
- **T-D01** (mantener docs) es continuo.

## M7 — Build Windows/Linux (detalle)

Objetivo: un `kult.exe` que arranque, se vea y suene, **sin tocar `src/`**, para
la validación del usuario en Windows.

1. **Portabilidad mínima** (solo `platform/`):
   - `platform/time.c`: reloj/espera multiplataforma. POSIX usa
     `clock_gettime`/`nanosleep`; **Windows** necesita shim (`_WIN32`:
     `QueryPerformanceCounter` + `Sleep`), sin depender de SDL en headless.
   - `platform/fs.c`: el separador de `KULT_DATA_DIR` es `:` hoy y **choca con
     `C:\`**; en Windows usar `;`, y el directorio por defecto `data/` **junto al
     `.exe`**.
   - `app/main.c`: revisar que no quede nada POSIX.
2. **CMake** (raíz, nuevo): targets `kult` (headless) y `kult-sdl` (SDL3). Mismas
   fuentes/flags que el Makefile (`src/` + `boot/` + `platform/` + `app/`, con
   `-Iplatform/include` **el primero**, que sombrea `os.h`/`hw.h`/`psg.h`/`sched.h`).
   El **Makefile se mantiene** para core/diferencial.
   - `cmake/mingw-w64.cmake` (patrón del predecesor).
   - **SDL3 3.4.10** (la misma que la nativa) para MinGW:
     `SDL3-devel-3.4.10-mingw.zip` en `third_party/` (gitignored) y
     `SDL3.dll` del `SDL3-3.4.10-win32-x64.zip` para el paquete.
3. **Icono** (ya copiado a `assets/kult_icon.ico`, md5 `21fac068…`, idéntico al
   del predecesor): `assets/kult_icon.rc` (`101 ICON "kult_icon.ico"`) para el
   recurso del `.exe` (windres + `OBJECT_DEPENDS`), y `SDL_SetWindowIcon` en
   `platform/video_sdl.c` con RGBA embebido.
4. **Empaquetado interino**: ZIP `kult.exe` + `SDL3.dll` + carpeta `data/` vacía +
   instrucciones (el usuario copia sus ficheros). **Sin assets en el paquete.**
   La ingesta/`kult.data` NO entra en este primer build (va con M7-c/N3).
5. **Verificación**: Linux `make test` (499) + `make test-port` + smoke
   `make port`/`port-sdl`; build cross Windows OK; el **usuario** ejecuta el `.exe`
   (T-061).

## Ingesta y archivo único `kult.data` (para M7-c/N3)

Idea: en vez de leer ficheros sueltos, **todo se empaqueta una vez** en un
archivo único (tipo ZIP) que el juego abre al arrancar.

- **Contenedor `kult.data`**: cabecera (`magic` + versión + nº de entradas) +
  índice `{nombre sin unidad, offset, tamaño}` + blobs. Se carga en memoria;
  `platform/fs.c` sirve `os_fopen`/`os_fread`/`os_fsfirst` desde él.
- **Escrituras del juego** (`CLEAR.BIN`, `SCENA.BIN`): van a `saves/` junto al
  exe (el archivo es de solo lectura).
- **Modos**: `kult.exe --ingest <ruta>` construye `kult.data` y sale;
  **auto-ingesta** al primer arranque si falta `kult.data` y `ingest/` tiene
  contenido.
- **Entradas soportadas, por coste**:
  - **A · carpeta de ficheros ya extraídos** → empaquetar. Barato (desbloquea).
  - **B · imágenes** `.st`/`.msa` (FAT12) y `.adf` (OFS/FFS). Medio (C).
  - **C · `.stx` (Pasti/ST protegido)**: caro; decidir portar o pre-convertir.
- **Contenido**: `kult.data` incluye **los recursos** (`*.PI1`, `*.BIN`, …) **y el
  `KULT.PRG`**, porque el `KULT.PRG` se usa como **fuente de datos** (tablas,
  recursos empotrados CARAC/TEMPL/ICONE/ARPLA) que el C reconstruido **lee**;
  aunque el port **no ejecuta** el código 68000 (la lógica es la reconstrucción C
  dentro de `kult.exe`). Es decir: el `.data` lleva datos, no el programa
  ejecutable.
- **Legal**: el paquete va **sin** assets; el usuario aporta su copia y la ingesta
  genera su `kult.data` (uso personal).

## N3 — Motor nativo y assets nativos (hito futuro: ADR + rama)

Objetivo: el runtime **no depende** del binario original (ni del `KULT.PRG` ni de
blobs originales); el motor es **nuestro** y lee **assets nativos**.

- **Decisiones (a fijar en el ADR de N3):**
  - **Media**: gráficos **PNG**; audio **WAV/OGG**; textos **UTF-8**; música en un
    **formato tracker propio**.
  - **Datos de sala/guion/tablas**: **binario propio documentado** (se decidió
    **no** JSON/YAML: **no habrá mods**, se prioriza rendimiento/autenticidad).
  - **Ingesta = conversor de un solo uso** (tu copia → assets nativos).
  - **Modelo de datos preparado para dos versiones (ST/Amiga)**.
  - La regla "`src/` no se toca" **se replantea en la rama de N3** (ahí nace el
    motor nuevo).
- **Fases por subsistema** (una tras otra): gráficos → audio → textos → datos de
  sala; cada conversor con su prueba.

## A — Workstream Amiga, en dos fases

- **A1 — Investigación** (prerrequisito **DURO** de N3): `T-A00` censo Amiga↔ST por
  rutina + `T-A01` decomp ASM Amiga (spec byte a byte). Salida: mapa de *lógica
  compartida vs específica* + diferencias de hardware (Paula/audio, paleta/planos)
  y de assets. **No toca el motor**; sirve para diseñar el modelo de N3 a dos
  versiones.
- **A2 — Implementación** (tras N3): `T-A02` core Amiga sobre el motor nuevo +
  `T-A03` arte/audio/textos Amiga. Cierra con verificación contra el binario Amiga.
- Ids `T-A00…T-A03` se conservan; solo cambia el **agrupamiento** de fase (A→A1/A2).

## T-022 — estado (se deja ABIERTO)

- `0x135ba` (XBIOS `Setscreen(-1,-1,0)`) ✅ implementado como `os_setscreen` no-op.
- `0x106f2` (`l_0101a8`, "reinicio suave" de `_start`): **pendiente/no portado**.
  Análisis estático (esta sesión): `0x101a8` **solo** aparece como **constante base
  de relocalización** de la tabla de pistas/iconos (`0x16f90` suma / `0x16fb4`
  resta; el bucle salta entradas nulas) y **no se ve ningún `jsr`/`jmp`/`pea`** ni
  puntero hacia `l_0101a8`; el único vector cercano es `$502 = l_01019a`. Por
  tanto **parece inobservable**, pero **se deja abierto**: puede necesitarse más
  adelante. Si se retoma, portar `0x106f2` (lista C + `crt0` + lista A) en `boot/`.
- **No confundir** con el **reinicio tras morir**, que es `0x1842c`
  (`prepare_room`) → `0x1682a` (`reset_clear`), **ya en el core** (`src/loop.c`,
  `src/fileio.c`) y que se valida en T-061.
- **Transcripción exacta de `l_0101a8` (0x101a8)** por si se retoma:
  `bsr f_0106f2` (carga la lista C: `load_resource_list` con tabla **`0x1a2c8`**),
  `movea.l #STACK,a7`, `bsr f_0135e2` (`crt0_init`), `bsr f_0106e2`
  (`load_resource_list_a`, tabla `0x1a3aa`), `bra l_0100ec` (vuelve al bucle de
  carga). Nota: `load_resource_list(img, table)` es **interno** del core (no está
  en `kult_api.h`); un port en `boot/` lo declararía `extern` (o usaría solo las
  variantes `_a`/`_b` exportadas).

## Deriva de documentación corregida (T-D01)

- `AGENTS.md`: estado *"Fase M0: código aún no importado"* → estado real (M0–M4 +
  M6 en curso).
- `docs/PROMPT_SESION.md`: *"M0–M3 hechas"* / *"Tarea inmediata: M4"* → estado real
  y **M7 build** como tarea inmediata.
- `PROYECTO.md`: §6 (fila M6 "pendiente" → en curso; **A** dividida en **A1/A2**;
  **N3** añadido; **M7** aclarado) y §8 actualizado.
- `docs/BUILD.md`: §"Estado (M0/M1)" → estado real + **plan de build M7** (CMake,
  SDL3 3.4.10 MinGW, icono, portabilidad, empaquetado).
- `docs/LIMITACIONES.md`: reinicio `0x106f2` (**abierto**, no "diferido") y
  distinción con `reset_clear`; Amiga A1/A2; ingesta con N3.
- `TODO.md`/`docs/PLAN.md`: fases M7/A1/N3/A2, T-022 abierto, T-061 con
  morir→reiniciar.
