# 0030 — Build de diagnóstico Y/V completo (run hasta el fallo)

> Para fallos de lógica difíciles (p. ej. usar un objeto incorrecto en la boca
> de la serpiente y acabar en un retrato gigante/artefacto), la traza estándar
> no basta: `S`/`F` no incluyen el byte de guion ni el estado de la VM.

## Build

```sh
make cmake-win-diag
```

Compila en `build-win-diag/` con `-DKULT_DIAG_TRACE=ON`: sustituye
`src/dispatch.c` por `platform/dispatch_trace.c`. El resto del port es el
estándar.

En el paquete de pruebas se copia como:

- `kult-diag.exe` (md5 `b5e94d0645dc3d86fee3909eb9180a47`)
- `kult-diag-headless.exe` (md5 `b178d8413167acd2b29c81146bb9c4d3`)
- `kult-diag-marcas.bat` (activa `KULT_MARKS=kult-marcas-diag.log`)

**No se empaqueta ni sustituye al exe estándar.** Es solo diagnóstico.

## Qué registra

Con `KULT_MARKS` activo, cada byte de guion escribe:

- `Y <vbl> <byte> <script>`: byte ejecutado y puntero de guion.
- `V <vbl> <byte> <script> ...`: estado de la VM (comando, objeto seleccionado,
  modo, skip, cursor, botones, etc.).
- `U <vbl> sel=... cls=... cmd=... ... selb=... clsb=...`: al entrar en
  `use_object` (0x17ca4); incluye los 8 bytes del objeto seleccionado y de la
  clase.
- `P <vbl> sel=... cls=... ...`: al entrar en `scale_prep` (0x13702), para ver
  qué objeto/guion acaba en el escalado.
- `B <vbl> fn=... sel=... cls=... cmd=... src=... w=... h=... x=... y=...`:
  al entrar en `blit_dispatch_b` (0x11f2e) o `elem_blit_rev` (0x12b04);
  incluye las dimensiones/fuente del elemento antes del blit.
- `D <vbl> ...`: traza de control de la VM (T-083b). Tipos:
  - `VMIN a6=... tmpl=... stack=...` / `VMOUT a6=... why=... tmpl=... stack=...`:
    entrada y salida de `vm_run`.
  - `RUN cmd=... script=... nibble=... tmpl=... stack=...`: entrada de
    `run_command_keep_sp` (incluye también el caso `0x0000`, que no pasa por
    `walk_entry`).
  - `WALK_RUN` / `WALK_INLINE` / `WALK_4D`: índice, `entry` y `end` que devuelve
    `walk_entry` en cada despacho.
  - `JMP` / `CALL` / `RET` (y `*_END`): destino, retorno y pila antes/después de
    los bytes `0x33`/`0x34`/`0x35`.
- `F`, `B`, `S`, `M`: marcas de funciones/bancos/animación/F12.

La build estándar solo escribe las líneas `F/S/B/M`; las `Y/V` son exclusivas de
esta build.

## Cómo usarla

1. Copiar `kult-diag.exe`, `kult-diag-marcas.bat` y `SDL3.dll` junto a los
   `data/` del juego (o usar la carpeta de pruebas ya preparada).
2. Ejecutar `kult-diag-marcas.bat`.
3. Reproducir el fallo (p. ej. meter el cuchillo en la serpiente).
4. Cerrar el juego **en el momento del fallo**. No hace falta F12: `Y/V` se
   escriben durante toda la run.
5. Analizar el `kult-marcas-diag.log` **desde el final hacia atrás**: el último
   `Y/V` antes del cierre es el estado que provocó el fallo.

## Interpretación

- El `script` de `Y/V` es una dirección de imagen (`0x10000` + offset del PRG).
  Para leer los bytes del guion, restar `0x10000` y sumar la cabecera del PRG
  (28 bytes) para obtener el offset del fichero.
- Bytes relevantes: `0x0e` = `scale_loop`, `0x10` = `scale_prep`,
  `0x31` = `use_object` (comando), etc.
- El `V` incluye `the_command`, objeto seleccionado, modo y flags; sirve para
  saber qué rama tomó el juego antes de `scale_prep`.

## Limpieza

Al terminar, borrar `build-win-diag/` y los `kult-diag*` del paquete de pruebas
si no se van a seguir usando. No commitear binarios ni logs.

## Hallazgo T-083 (2026-10-21): retorno doble de `icon_ring_loop` (0x15888)

La traza `D` localizó la transición exacta del fallo:

```text
D 1303 VMIN a6=1fbc0 tmpl=1fc2e
Y 1303 3c 1fbc0            (entrada 0xd2)
...
Y 1303 3d 1fc1a            (byte 0x3d = icon_ring_loop)
D 1590 RUN cmd=c32f ...    (comando c32f, nibble c)
D 1653 WALK_INLINE cmd=0134 idx=0133 entry=206c3 end=20727
...
D 1711 VMOUT a6=20726 why=end_byte tmpl=20727
Y 1711 34 1fc2e            (¡cabecera de la entrada siguiente!)
```

ASM `0x15888` guarda `A_TEMPL_PTR` en `0x1a4be` y, para comando con nibble
`0xc`/`0xa` (y para el comando pendiente `0x1abf8`), sale por `0x1592e`:

```asm
01592e: 588f        addq.l #4,a7   ; descarta el retorno al VM exterior
015930: 6000aa7c    bra.w $103ae   ; tail-call a run_command_keep_sp
```

Es un **retorno doble/tail-call**: el VM exterior se abandona; el `vm_run`
anidado vuelve directamente a quien llamó al VM exterior. El C actual de
`icon_ring_loop` hace:

```c
run_command_keep_sp(img, script);
return script;              // vuelve al VM exterior, que sigue vivo
```

El `run_command_keep_sp` anidado ya ha pisado `A_TEMPL_PTR` (queda en `0x20727`),
así que el VM exterior compara su `A6=0x1fc2e` con `0x20727`, no coincide y
ejecuta la cabecera `0x1fc2e` (longitud `0x34` como `call`) → `0x2563e`
(`scale_prep`) o, en la 2.ª serpiente, cabecera `0x206b1` → `0x206b5` con
`py=0` → índice `0xffff` → bucle `12b40`.

La auditoría de `notes/0028` dio `0x15888` por modelado por “sustituir el
`addq`+`bra` por el despacho”, pero faltaba modelar que el VM exterior **no
debe continuar**.

### Fix propuesto (requiere tocar `src/`)

1. Exponer el `A6` final de `run_command_keep_sp` (p. ej.
   `uint32_t run_command_keep_sp_a6(...)`, manteniendo el wrapper `void`).
2. En `icon_ring_loop`, en las dos ramas de `0x1592e`, hacer
   `return run_command_keep_sp_a6(img, script);` en vez de
   `run_command_keep_sp(...); return script;`.
3. Replicar en `platform/dispatch_trace.c` (build de diagnóstico).
4. Test diferencial de la rama `0xc`/`0xa` de `0x15888` y regresión
   `make test` 511 + `make test-port`.

### Fix aplicado (T-083, 2026-10-21)

- `src/dispatch.c`: `run_command_keep_sp_a6()` devuelve el `A6` final del
  despacho; `run_command_keep_sp()` queda como wrapper compatible.
- `src/commands.c`: las dos ramas de `0x1592e` de `icon_ring_loop` (nibble
  `0xc`/`0xa` y comando pendiente) hacen `return run_command_keep_sp_a6(...)`,
  modelando el tail-call.
- `platform/dispatch_trace.c`: mismo despacho en la build de diagnóstico.
- `kult_core.map` / `kult_api.h`: exporta `run_command_keep_sp_a6`.
- Tests nuevos: `test_run_command_keep_sp_a6_nibble0` y `test_fun_15888_ex_sin_tail` (513 tests).
- Artefactos: `kult.exe` `41be915acfd6eedccae459e5be014a84`,
  `kult-headless.exe` `a7b180cdfdec055f70165ce2b17bec4a`.
  **Validado en `kult.exe` y `kult-diag.exe`:** dado/2.ª y mosca/1.ª sin error.
  El log `diag-logs/post-fix-dado-mosca-2026-10-21.log` no contiene ninguna
  ejecución de `0x1fc2e`, `0x206b1`, `0x2563e` ni `0x206b5`; tras el `0x3d` se
  ve `VMOUT a6=20726 why=end_byte tmpl=20727` y el retorno normal.
