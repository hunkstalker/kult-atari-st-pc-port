# 0035 — T-087: incidencias tras matar un enemigo (handoff)

> **ACTUALIZACIÓN SESIÓN 40 (2026-10-21):** ya hay captura limpia y causa raíz
> localizada. El fallo tras `PSI Attack` no es la transición `38 ... 34` ni la
> ruta `c1e5`: es el **A7/unwind de los comandos `0xfxxx` en `0x103ae`** que el
> port no modela. La clase sí queda marcada; el anillo se reabre porque `f2f8`
> retorna a `palette_cmd`/`b002` en vez de volver al marco de `run_command`.
> Evidencia, traza y plan robusto en **`notes/0036-a7-unwind-combate.md`**.
> El texto de abajo se conserva como histórico de la sesión 39; las hipótesis
> de `use_object`/`38 ... 34`/`c1e5` quedaron descartadas o superadas.
>
> **CERRADO (sesión 43).** El unwind A7 se resolvió en dos fixes
> complementarios: el `fxxx` directo (`run_command_keep_sp_a6`) y el indirecto
> de `dispatch_a6` (byte `0x4d`). Resumen y validación final:
> **`notes/0037-dos-caminos-unwind-a7.md`**; detalle técnico:
> **`notes/0036`**. Las hipótesis históricas de abajo quedaron descartadas.

## 1. Incidencias

### A. Anillo de acciones con el enemigo ya muerto (confirmada por el usuario)

Después de matar a un enemigo, el cadáver queda en el suelo, pero el anillo de
acciones sigue ofreciendo las acciones de un enemigo vivo (atacar/matar, etc.).
No ocurre siempre que se abre el anillo; aparece en el contexto de combate tras
la muerte.

### B. Enemigo invisible dentro de un recuadro negro (sin reproducir todavía)

El usuario describe un recuadro negro que normalmente marca otro enemigo, pero
sin el sprite del enemigo dentro. En la última run revisada **no se pudo
confirmar**: el log mezclaba runs y la asociación no era fiable. No hay todavía
una captura limpia de esta incidencia.

## 2. Evidencia parcial (tratar con reservas)

En runs anteriores se observó, sin poder asociarlo de forma fiable a una
observación concreta:

- El guion del anillo en `21f1a` (`icon_ring`) y un salto en `21f28` cuyo destino
  `0x4b18` es relativo a `A_TEMPL_DATA` (`0x1d402`), es decir, vuelve a
  `0x21f1a`: bucle del propio guion.
- Reaperturas del anillo con `the_command = ffff` (sin acierto de acción).
- Objetos/clases de enemigos como `1b3d0`/`1ab30` y `1b474`/`1aa77`, con sprites
  que pasan de `0x38×0x41` a `0x10×0x41` durante la muerte.
- En una run se vio `prepare_room` (`0x1842c`) → `reset_clear` (`0x1682a`)
  después de un comando de muerte, aunque el usuario indica que **no murió y no
  cambió de sala**. Queda pendiente confirmar si ese reset es espurio o parte
  normal de la secuencia.

**No usar** el `S ... 65280 255` ni la ausencia de líneas `B`/`S` como prueba
del enemigo invisible: fue una hipótesis sin correlación fiable con la pantalla.

## 3. Estado de la investigación

No hay causa raíz confirmada. Hipótesis a comprobar:

1. El estado del objeto/clase/flags del enemigo no se actualiza al morir, por lo
   que el guion sigue ofreciendo acciones de vivo.
2. El anillo se reabre por el bucle `21f1a`/`21f28` y el estado del cadáver no
   permite salir de él.
3. El enemigo invisible es un sprite que no se vuelve a dibujar, se dibuja con
   banco vacío o queda fuera de pantalla. Puede compartir causa con el estado
   del cadáver, pero **no está demostrado**.
4. El `prepare_room`/`reset_clear` visto tras la muerte podría ser una rama
   errónea del guion, no una transición normal.

No tocar código hasta reproducir de forma determinista y comparar con el
original/Hatari.

## 4. Problema del log (importante)

`kult-marcas-diag.log` es **append-only**, contiene varias runs separadas solo
por `# RUN` y no incluye timestamp por run. F12 solo deja `M f12 <vbl>`. Con un
fichero grande es fácil analizar una run distinta de la que el usuario está
mirando; ya ocurrió en esta sesión.

Protocolo para la próxima captura:

1. **Borrar o renombrar** el log antes de la run.
2. Una sola run y **una sola incidencia**.
3. Pulsar **una vez F12** en el momento exacto y anotar el VBL del título.
4. No lanzar otra run hasta copiar/guardar ese log.
5. Pasar el log limpio o el bloque completo de esa run.
6. Mejor aún: usar un `KULT_MARKS` distinto por run (por ejemplo
   `kult-marcas-enemigo.log`, `kult-marcas-anillo.log`).

Mejora futura recomendada: añadir timestamp al encabezado `# RUN` en
`platform/cpuwork.c` para que cada bloque sea autoidentificable.

## 5. Plan para la próxima sesión

### Captura A — enemigo invisible

- Matar al enemigo y esperar a que el recuadro negro esté estable.
- F12 con el enemigo invisible en pantalla, sin cambiar de sala.
- Anotar sala, enemigo y si el recuadro sigue al cadáver.

### Captura B — anillo con acciones de vivo

- Repetir la muerte del mismo tipo de enemigo.
- F12 con el anillo abierto sobre el cadáver.
- Anotar qué acciones aparecen y si el enemigo está muerto en pantalla.

### Instrumentación temporal (solo después de tener la captura)

- Objeto/clase/flags antes y después de la muerte (`A_SELECTED_OBJECT`,
  `A_ELEM_CLASS`, bits de `+1`, `+3`, `+4`).
- `A_MODE_FLAG` (`0x1acc6`), `A_DAT_1ACC7`, `A_DAT_1ACD2`, `A_DAT_1AD09`,
  `A_DAT_1ACF7`, `A_CLASS_IDX` (`0x1ace6`).
- Anillo: script/máscara, `the_command`, acciones y bucle `21f1a`/`21f28`.
- Dibujo: `obj_list_iter` (`0x16636`), `anim_bank` (`0x15ac8`),
  `blit_dispatch_b`/`0x12b04` y `15da6`: x/y/w/h, puntero/banco, hash y primeros
  bytes del sprite.
- Muerte: punto donde se marca el enemigo como muerto y quién llama a
  `prepare_room`/`reset_clear`.

### Validación y cierre

- Comparar objeto/flags/banco con el original/Hatari en la misma sala y acción.
- Corregir solo la causa confirmada, con prueba de regresión.
- `make test` (**514**) y `make test-port`.
- Validar con una run limpia: enemigo vivo → acciones de vivo; enemigo muerto →
  acciones de cadáver o sin anillo; sprite visible.

### Traza R del anillo (implementada 2026-10-21)

Con `KULT_RINGLOG=<ruta>` (solo `R`) o `KULT_MARKS` activo (normal o diag),
cada anillo escribe líneas `R` en vivo:

```text
R <vbl> <evento> a=.. b=.. c=.. cr2=.. abf8=.. ace7=.. acf2=.. ace8=..
  ad09=.. w19316=.. a826=.. sel=.. cls=.. [selb=..] [clsb=..]
```

- `ring_open`: A6 del guion, máscara y nº de acciones.
- `ring_entry`: `a=icono`, `b=comando`, `c=bit`; el icono se traduce con
  `work/texts/VERBE.BIN.json` (índice JSON = icono).
- `ring_pick` / `ring_return`: selección y retorno.
- `loop_*`: `in`, `sel`, `cmd`, `after_cmd`, `threshold`, `pending`,
  `pending_tail`, `reopen`, `clear_flags`, `exit_ffff`, `tail_ac`.
- `run_cmd`, `run_f`, `run_f_ret`, `walk4d`, `walk4d_ret`, `vm_tail`,
  `run_boundary_in/out`.
- `cr2=$1a4c2`, `abf8=$1abf8`, `a826=A_RUN_STACK`, `w19316=palabra 0x19316`.

Esto permite distinguir: mismo anillo reabierto vs guion distinto, si el corte
fue por `0xffff`, por `a/c`, por `$1abf8` o por reapertura, y si `$1abf8`/A7
estaban en el valor esperado.

Uso rápido:

```bat
kult-anillo.bat                 rem KULT_RINGLOG=%~dp0kult-anillo.log
```

```sh
python3 tools/ringreport.py kult-anillo.log
python3 tools/ringreport.py kult-anillo.log --all
```

El informe traduce cada icono a su verbo (`work/texts/VERBE.BIN.json`) y marca
como sospechosa una reapertura tras un pick cuyo comando no es `a/c`.

### Traza de dano/ALEAT (implementada 2026-10-21)

La linea `R` incluye ahora `hp` (`$1ad20`), `ad03`, `ad02`, `rnd_idx`
(`$1a579`) y `rnd_byte` (`$1acd7`). Con `KULT_RANDLOG=<ruta>` se registra cada
lectura de la tabla ALEAT (`0x2741e`) desde `pal_next_byte`/`pal_next_word`
(`D <vbl> rnd_b|rnd_w ... idx=.. val=.. hp=.. ad03=.. ad02=..`).

Capturas preparadas:
- `kult-dano-normal.bat` -> `dano-normal-{marcas,ring,rnd,input}.log`
- `kult-dano-psi.bat` -> `dano-psi-{marcas,ring,rnd,input}.log`
- `kult-dano-todo.bat` -> `dano-todo-{marcas,ring,rnd,input}.log` (una run con
  todos los ataques, normal + PSI).

`KULT_INPUTLOG` anade lineas `I` para clic crudo (`mouse_down/up`) y clic
entregado/procesado (`ikbd_click`/`click`). La traza `R` incluye ademas
`click_cmd`, `queue_abf8/ac06/acaa/acae`, `dispatch_queued`, `set_ac06`,
`set_abf8`, `set_acaa`, `cmd_handler` e `inline_cmd`, para que ningun comando
quede fuera aunque no pase por el anillo.

Plan: misma sala/enemigo, varios golpes por tipo; medir `hp_antes - hp_despues`,
correlacion con `rnd_idx`/`rnd_byte`, rama `21d75 -> 2162a` vs `0x25b82`, y
comparar con el original/Hatari.

### Test de regresion pendiente (unwind de A7)

Se intento un test diferencial de `icon_ring_loop` con un anillo sintetico de una
accion `e010` que ejecuta `0x4d`. El mecanismo del `0x4d` esta montado
(`$1a826=STACK_TOP`, pila de guion y RET), pero el oraculo no selecciona la
accion de forma fiable con el fixture actual de clic/cursor: el origen del
anillo, los bytes X/Y del paquete IKBD y los schedules IKBD/VBL no quedan
sincronizados. No se dejo en `make test` para no introducir un falso verde.

La validacion practica es la traza `R`: tras el fix, la misma secuencia no debe
mostrar `loop_reopen` despues del primer `e1fc`; `test/test_ringreport.py` prueba
que `ringreport.py` detecta esa firma. El test diferencial queda como trabajo
pendiente cuando el fixture de clic del anillo se pueda fijar con el oraculo.

## 6. Comandos y artefactos

- Diag actual: `build-win/kult-0.1.0-win64/kult-diag.exe`
  (`f43ebde4…`), lanzado con `kult-diag-marcas.bat`.
- Build estándar: `build-win/kult-0.1.0-win64/kult.exe` (`cc4a2cd0…`).
- Tests: `make test` (514) y `make test-port`.
- **No hay commits de esta fase.** Cualquier cambio de código debe esperar a la
  reproducción limpia.

## 7. Referencias

- Anillo: `src/commands.c` (`icon_ring`, `icon_ring_loop_ex`).
- Preparación/reinicio de escena: `src/loop.c` (`prepare_room`,
  `reset_clear`) y `notes/0022`.
- Input: `notes/0032` (no se ha tocado en esta incidencia).
- Timing del anillo: `notes/0034` (T-086, validado; no confundir con este bug).
