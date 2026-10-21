# 0036 — T-087: A7/unwind de comandos `0xfxxx` (sesión 40)

> **RESUELTO Y VALIDADO POR EL USUARIO (SESIÓN 43).** Resumen de cierre y
> evidencia final: **`notes/0037-dos-caminos-unwind-a7.md`**. La causa raíz del
> fallo tras `PSI Attack` era el retorno no local (unwind de A7) de `0x103ae`
> para comandos `0xfxxx`. El boundary `setjmp`/`longjmp` vive en
> `run_command`; la sesión 41 cubrió el `fxxx` directo de
> `run_command_keep_sp_a6` y la sesión 42 el `fxxx` indirecto que forma
> `dispatch_a6` desde el byte `0x4d`. No es un parche en `palette_cmd` ni está
> condicionado a una clase/enemigo concreto. Ver §11, §12 y §13.

## 1. Resumen ejecutivo

La captura limpia de la sesión 40 confirma el síntoma:

- Enemigo 1 (normal, clase `01aa77`): `Attack` normal -> animación de muerte y
  secuencia de muerte correcta.
- Enemigo 2 (normal, clase `01aa86`): `PSI Attack` -> animación de muerte, pero
  el anillo vuelve a abrirse; después `Attack` deja un recuadro negro donde
  estaba el enemigo y el jugador muere.

La clase del enemigo 2 **sí queda marcada como golpeada** (`byte0 = 0`, bit6 de
`byte1` puesto). Lo que no ocurre es el **retorno no local** que en el original
saca la ejecución de `palette_cmd`/`b002`/bucle del anillo y vuelve al marco de
`run_command`, desde donde el juego continúa con la secuencia de muerte.

Causa raíz: `run_command_keep_sp_a6` (`0x103ae` en C) ejecuta el guion `f2f8` y
retorna normalmente. El original, para `nibble == 0xf`, restaura A7 desde
`$1a826` y el `rts` final del VM vuelve al llamante de `run_command`. Es un
tail-call/unwind no local que el port no trasladó.

## 2. Captura limpia

Antes de la run se archivaron los logs antiguos en:

```text
build-win/kult-0.1.0-win64/trazas-antes-run-combates-20261021-2035/
```

La run nueva (20:55, hora del juego) está guardada en:

```text
build-win/kult-0.1.0-win64/trazas-run-combates-20261021-2055/
  dano-todo-input.log
  dano-todo-marcas.log
  dano-todo-ring.log
  dano-todo-rnd.log
  kult.log
  README.txt
  SHA256SUMS
```

`README.txt` contiene la observación textual del usuario y la asociación de
clases. Los `dano-todo-*` de la raíz del paquete son los mismos ficheros de esta
run; el archivo de `trazas-run-combates-20261021-2055/` es la copia de
referencia con SHA256.

## 3. Traza: asociación enemigo/clase/secuencia

La run tiene tres combates, pero los dos relevantes para este handoff son:

| # | Clase inicial | Bytes iniciales | Acción | VBL | Clase tras el golpe | Resultado |
|---|---|---|---|---|---|---|
| 1 | `01aa77` | `14 04 31 19 06` | normal `c2f8` | 1365 | `00 44 31 19 06` | secuencia de muerte correcta |
| 2 | `01aa86` | `0e 04 31 d9 0b` | PSI: `b002` -> `a010`/`f2f8` | 5472/5549 | `00 44 31 d9 0b` en 5926 | `loop_reopen` al anillo `20c88`; no muere |

Enemigo 1, secuencia posterior (traza `ring`/`marcas`):

```text
c2f8 VBL 1365  -> loop_tail_ac -> run_f 2337f -> inline 21747
clase 00 44 31 19 06 en VBL 1745
c1d2 VBL 1970  -> anillo 2187d
c1d7 VBL 2052  -> 90aa -> c357 VBL 2163
clase final 01ab30 en VBL 2426
```

Enemigo 2:

```text
b002 VBL 5472 -> a010/f2f8 VBL 5549 -> run_f 2337f -> inline 21747
clase 00 44 31 d9 0b en VBL 5926
loop_after_cmd / loop_threshold / loop_reopen al anillo 20c88 en VBL 5926
NO c1d2, NO c1d7, NO c357
```

Después, ya con el anillo reabierto:

```text
c2f8 VBL 8251 -> c357 VBL 8421 -> clase 01ab30 en VBL 8687
```

El enemigo `01aa86` sigue vivo; el recuadro negro es el enemigo que no se ha
eliminado. El jugador muere después.

Datos adicionales:

- `$1ad20` (energía PSI) baja `0x3f -> 0x37` (-8) en la acción PSI; no es la
  vida del enemigo.
- No aparece `c1e5` ni la ruta `A_1AAE1` en esta run.
- No se observa la transición de clase a `38 ... 34`; la muerte correcta de
  estos enemigos usa `c1d2`/`c1d7`/`c357`, no ese estado que se había supuesto
  en notas anteriores.
- La clase final `01ab30` aparece en ambos combates; es la continuación normal
  del juego tras la secuencia de muerte, no una prueba de que el segundo
  enemigo haya muerto.

## 4. Hipótesis descartadas o corregidas

- **`use_object` no marca la muerte:** descartado. En el enemigo 2 la clase pasa
  a `00 44 31 d9 0b`; el golpe se aplica.
- **Falta la transición `38 ... 34` de `script_step`:** no es la ruta usada por
  estos enemigos en esta captura. No seguir por ahí sin nueva evidencia.
- **La ruta `c1e5`/`A_1AAE1`:** no se dispara. No es la causa del caso PSI.
- **Es solo un problema de tipo de enemigo:** el enemigo 1 y el 2 tienen clases
  distintas (`01aa77` vs `01aa86`), pero el fallo específico de PSI se explica
  por el despacho `f2f8`, no por la clase. La diferencia de clase puede explicar
  resistencia/daño, pero no que el anillo se reabra después del PSI.
- **El ataque normal también falla siempre:** no. `c2f8` (nibble `c`) sí entra
  por el tail-call del anillo y el enemigo 1 muere correctamente.

## 5. Causa raíz: A7 y retorno no local en `0x103ae`

En el original, `run_command` (`0x103a8`) guarda A7:

```asm
0103a8  move.l a7,$1a826
```

`run_command_keep_sp` (`0x103ae`) comprueba el nibble alto del comando. Para
`0xfxxx`:

```asm
0103e0  cmpi.w #$f000,d2
0103e4  bne.w  $103ee
0103e8  movea.l $1a826.l,a7    ; restaura el A7 del marco exterior
0103ee  ...
0103fa  bsr.w  $12950           ; walk_entry; el bsr deja $103fe en pila
0103fe  movea.l a5,a6
010400  move.l a4,$1a4e0
010406  bra.w  $10630           ; tail-jump al VM
```

El VM termina en `0x10666` con `rts`. Al haberse restaurado A7 al valor de
entrada de `run_command`, ese `rts` **vuelve al llamante de `run_command`**, no
a `palette_cmd`/`b002`/anillo. Es un retorno no local; el decompilador lo
describe como un push de `0x103fe`, pero realmente el `0x103fe` es el retorno
del `bsr.w $12950` y la pila queda en el marco exterior.

La misma restauración de A7 aparece en otros handlers que despachan comandos
`fxxx`:

- `0x14ee8` `cmd135`
- `0x14f74` `grid_wait`
- `0x15052` `obj_action_select`
- `0x151b0` (familia de acciones)
- `0x152ee` `cursor_anim_wait`
- `0x15382` `palette_cmd`
- `0x15530` `mode_queue`
- `0x1699c` / `0x169a4` / `0x169b4`

Por eso el fix debe ser **central en el despacho**, no un caso especial de PSI.

## 6. Estado del port C

En `src/handlers.c`:

```c
void run_command(uint8_t *img, uint32_t stack) {
    wr32(img + A_RUN_STACK, stack);
    ...
    run_command_keep_sp(img, 0);
    ...
}
```

`A_RUN_STACK` (`$1a826`) se guarda, pero **no se lee en ningún sitio del port**.
No hay equivalente al `movea.l $1a826,a7` ni al `rts` no local.

En `src/dispatch.c`, `run_command_keep_sp_a6` para el nibble `f`:

```c
walk_entry(...);
wr32(img + A_TEMPL_PTR, end_out);
uint32_t r = vm_run(img, entry_out);
return r;
```

Es una llamada normal: al terminar `f2f8`, la ejecución vuelve a `palette_cmd`,
luego a `b002` y al bucle del anillo, que reabre. Por eso aparece `loop_reopen`.

El port solo modela el tail-call de `c/a` y del pendiente `$1abf8` en
`icon_ring_loop_ex` (`VM_TAIL_FLAG`). El unwind de `0xfxxx` quedó pendiente; la
propia nota de `test/test_dispatch.py` lo dice: la rama `0xf000/otra` restaura
A7 y no se prueba con `run_command_keep_sp` porque el oráculo usaría un A7
basura.

## 7. Solución robusta recomendada

**Recomendación:** implementar el unwind en el boundary de `run_command` con
`setjmp`/`longjmp`, que es el equivalente C del `rts` con A7 restaurado.

Diseño:

1. En `run_command`:
   - marcar un boundary activo;
   - `setjmp` al entrar;
   - si se produce unwind, salir del comando y volver al `game_loop` como haría
     el `rts` original;
   - limpiar la marca al salir.
2. En `run_command_keep_sp_a6`, rama `nibble == 0xf000`:
   - ejecutar el VM completo;
   - si hay un boundary activo, hacer el unwind;
   - si se llama fuera de un boundary, volver normalmente (fallback seguro).
3. Mantener el comportamiento actual para `c/a` y `$1abf8`; no cambiarlo.
4. Añadir traza `run_unwind`/`run_unwind_out` para validar en los logs que tras
   `f2f8` no hay `loop_reopen`.
5. No tocar `dispatch_a6` en el primer cambio. También pasa por `0x103ae` en el
   original, pero su semántica de tail-call con la VM exterior merece una
   auditoría aparte después de validar el caso PSI.

Por qué es robusta y no un parche:

- Corrige el mecanismo central de `0x103ae`, aplicable a todos los `fxxx`.
- No condiciona el comportamiento a la clase del enemigo, a `f2f8` ni a que el
  byte0 de la clase sea 0.
- Respeta la semántica real del ASM (retorno no local), en vez de simular solo
  el síntoma cerrando el anillo a mano.
- Se puede probar de forma diferencial con `run_command` y un comando `fxxx`.

Riesgos y guardas:

- `longjmp` se salta limpiezas de C; el original también se salta esas tramas,
  pero hay que comprobar cursor/overlay/sonido en la run real.
- Evitar saltar a un `jmp_buf` caducado: usar un flag de boundary activo y
  limpiarlo en todos los caminos.
- Si en el futuro `run_command` pudiera anidarse, usar una pila de boundaries en
  vez de un único `jmp_buf`.
- Variables locales que deban sobrevivir al `setjmp` no deben depender de
  registros no `volatile`; en este caso el boundary solo necesita volver al
  bucle.
- Auditar `dispatch_a6` y los otros handlers con A7 en una segunda pasada.

Alternativa (más mantenible, más invasiva): cambiar el despacho para devolver
un estado explícito `NORMAL`/`TAIL`/`UNWIND` y propagarlo por `cmd_handler`,
`palette_cmd`, `b002` y el anillo. Es un refactor grande; no es necesario para
cerrar T-087 si el unwind central queda bien probado.

## 8. Plan de implementación y validación

Siguiente sesión, en este orden:

1. Implementar el boundary con `setjmp` en `run_command` y el unwind central
   para `fxxx` en `run_command_keep_sp_a6`.
2. Añadir traza de unwind y un test diferencial de `run_command` con un comando
   `fxxx` y `STACK_TOP` (el caso que quedó pendiente en `test_dispatch.py`).
3. `make test` (suite actual conocida: 514; añadir el nuevo test) y
   `make test-port`.
4. Recompilar `kult-diag.exe` y `kult.exe`, copiar al paquete, y repetir la
   misma captura: enemigo 1 con `Attack` normal y enemigo 2 con `PSI Attack`.
5. En la traza nueva comprobar:
   - el enemigo 1 sigue muriendo;
   - tras `f2f8` aparece el unwind y **no** `loop_reopen` al anillo `20c88`;
   - se ejecuta `c1d2`/`c1d7`/`c357`;
   - no aparece el recuadro negro ni muere el jugador por el enemigo vivo.
6. Si falla la limpieza visual, revisar el estado de cursor/overlay antes de
   ampliar el fix; no parchear el anillo por clase.
7. Actualizar `notes/0036`, `RETOMAR.md` y `TODO.md` con el resultado.

## 9. Artefactos y comandos

Logs de referencia:

```text
build-win/kult-0.1.0-win64/trazas-antes-run-combates-20261021-2035/
build-win/kult-0.1.0-win64/trazas-run-combates-20261021-2055/
```

Build/diag de esta sesión (sin cambios de código):

```text
kult-diag.exe  f43ebde468bea7456523e23b805b40b3d6b708dc0f0d857ff771870eca10475e
kult.exe       cc4a2cd0fcd962ca1f9114ba3e92170a586cb756bcffe46edb9af0a9c757b2f6
```

Comandos de referencia:

```sh
make test
make test-port
make cmake-win
make cmake-win-diag
```

Para la captura manual se usa `kult-dano-todo.bat`, que escribe
`dano-todo-{input,ring,marcas,rnd}.log`; antes de cada run hay que archivar o
renombrar los anteriores.

## 10. Estado al cerrar la sesión

- No se ha modificado la lógica de `src/` en esta sesión.
- Los logs de la run nueva están guardados y hasheados.
- La causa raíz está localizada y el plan robusto está definido.
- `T-087` sigue **abierto**; el siguiente trabajo es el unwind A7 central.

## 11. Sesión 41 — fix central implementado (pendiente validación)

### Implementación

- **`src/handlers.c` — `run_command` (0x103a8):** registra un boundary
  `setjmp` antes de despachar y lo desapila en todos los caminos. Si se produce
  unwind, sale del comando como el `rts` original y vuelve al `game_loop`, sin
  ejecutar el resto de `run_command`. La pila de boundaries
  (`kult_run_boundary` en `include/run_boundary.h`) permite anidamiento; las
  variables que sobreviven al `longjmp` se guardan de forma segura.
- **`src/dispatch.c` — `run_command_keep_sp_a6` (0x103ae):** solo para
  `nibble == 0xf000`, tras completar el VM, dispara el unwind si hay boundary
  activo. Sin boundary activo mantiene el retorno normal (fallback seguro). No
  se toca el tail-call `c/a` ni el pendiente `$1abf8`.
- **`platform/dispatch_trace.c`:** mismo cambio para que `kult-diag.exe` emita
  la traza nueva (`D ... RUN_UNWIND` y `run_unwind`).
- **Trazas:** `run_unwind` (antes del `longjmp`) y `run_unwind_out` (al salir del
  boundary). Se conservan `run_boundary_in/out`, `run_cmd`, `run_f*`.
- **`dispatch_a6`:** no se toca en este primer fix; queda para auditoría
  posterior, como estaba previsto.

### Pruebas

- Nuevo test diferencial `test_run_command_f000_unwind` en
  `test/test_dispatch.py`: `run_command` (0x103a8) con comando `0xf010` y
  `STACK_TOP`, el caso que estaba pendiente en la nota de la rama `0xf000`.
- `make test`: **515 passed** (514 + nuevo).
- `make test-port`: **OK** (fs, log, presentación, timing, vídeo, input, Crawio,
  reloj, loader, idioma, música, Timer A, RMS).
- `make cmake-win` y `make cmake-win-diag`: OK.

### Ejecutables del paquete

```text
kult.exe               2575d8bcc5b14402157eb23f732f219e99191dfadafc989c7f5802ecd533c7d3
kult-headless.exe      465cd985f0aa1c2ff2066558252b7b7f023e8a35eb5b589e1518239100fd10ed
kult-diag.exe          b4cf4c4e5df3d3bccd26798012a7fe3d46ed9e4d675cf8674799357df10dd145
kult-diag-headless.exe da9249da08de0370cb3190a767605726402b86f3f98f9f02e9fc21226f10973a
```

Copiados a `build-win/kult-0.1.0-win64/` (junto a `kult-dano-todo.bat`).

### Validación pendiente (usuario, Windows)

Repetir la misma run de `trazas-run-combates-20261021-2055` con
`kult-dano-todo.bat`:

1. Enemigo 1 (`01aa77`, `Attack` normal): sigue muriendo con
   `c2f8 -> c1d2 -> c1d7 -> c357`.
2. Enemigo 2 (`01aa86`, `PSI Attack`): tras `b002 -> a010/f2f8` debe aparecer
   `run_unwind`/`RUN_UNWIND` y **no** `loop_reopen` al anillo `20c88`.
3. Deben ejecutarse `c1d2`/`c1d7`/`c357` y no debe quedar el recuadro negro ni
   morir el jugador por el enemigo vivo.
4. Guardar los nuevos `dano-todo-*.log` + `kult.log` en una carpeta
   `trazas-post-fix-*` y compararlos con la traza de referencia.

### Siguiente paso

Con la validación visual/logs OK: cerrar T-087 y auditar `dispatch_a6` y los
demás handlers con restauración de A7 (`0x14ee8`, `0x14f74`, `0x15052`,
`0x151b0`, `0x152ee`, `0x15382`, `0x15530`, `0x1699c/a4/b4`) en una segunda
pasada. No se ha hecho commit.

## 12. Sesión 42 — segundo camino: unwind en `dispatch_a6` (0x10390)

La run posterior al fix de la sesión 41 (binario `b4cf4c4e…`) seguía fallando en
el **último combate**, enemigo fuerte `01aab3`:

```text
4356 c1dc   -> primer ataque normal
4672 e1fc   -> primer PSI: WALK_RUN entry=21d6c, byte 0x4d en 21d86
4672 WALK_4D cmd=f1c2 -> dispatch_a6 (0x10390)
4954 walk4d_ret -> run_f_ret e1fc -> loop_after_cmd -> loop_reopen (anillo 21f1a)
5083 e1fc   -> segundo PSI: vuelve a WALK_4D/f1c2
5292 c357   -> clase final 01ab30
```

No aparecía ningún `run_unwind`: `e1fc` tiene nibble `e`, no entra por la rama
`nibble == 0xf000` de `run_command_keep_sp_a6`. El `f1c2` lo forma
`dispatch_a6` (`0x10390`), que en el port ejecutaba el sub-VM y **devolvía
anidado** al VM exterior. El original, al ser `0xfxxx`, restaura A7 desde
`$1a826` y su `rts` vuelve al llamante de `run_command`: el anillo no debe
reabrirse.

### Cambio

- **`src/dispatch.c` — `dispatch_a6`:** tras `vm_run`, si hay boundary de
  `run_command` activo, dispara `run_unwind_to_boundary()` con traza
  `run_unwind`. Sin boundary mantiene el retorno anidado anterior.
- **`platform/dispatch_trace.c`:** mismo cambio con traza `RUN_UNWIND4D`.
- No se toca el fix directo de `run_command_keep_sp_a6`, ni el tail-call `c/a`,
  ni `$1abf8`.

Los dos caminos son complementarios:

| Camino | Ejemplo | Función C | Estado |
|---|---|---|---|
| Directo | `f2f8` | `run_command_keep_sp_a6` | sesión 41 |
| Indirecto byte `0x4d` | `e1fc -> f1c2` | `dispatch_a6` | sesión 42 |

### Prueba

Nuevo `test/test_a7_unwind.py`: compila un helper C mínimo contra
`build/libkult.so`, instala un boundary real con `setjmp` y comprueba que
`dispatch_a6` hace unwind al despachar `f003`. El test falla sin el cambio y
pasa con él. Los tests existentes `test_fun_10390` y
`test_vm_byte_4d_nested_a6` siguen cubriendo el fallback sin boundary.

- `make test`: **516 passed**.
- `make test-port`: **OK**.
- `make cmake-win` y `make cmake-win-diag`: OK.

### Ejecutables del paquete

```text
kult.exe               be2cd45a734f98ecb01a4ec80de84e59a55105fdb0eb5cdd1ce1b8c65f6789a6
kult-headless.exe      3e44ba7da08efb2fc26eff604ba5521ece3f73f93b5535d9141c163faa59574c
kult-diag.exe          166057685ee712ec657b5e6a9c6c6a2e731b61bbed4a51161c3f166dc64c924d
kult-diag-headless.exe 5065fcc1f8331a82a4e57d1ced4402cf482890c75cdd312af1679b99832d83bc
```

### Validación pendiente (usuario, Windows)

Repetir el último combate con `kult-dano-todo.bat`:

1. Primer ataque normal y dos `PSI Attack`.
2. Tras `e1fc -> 0x4d/f1c2` debe aparecer `RUN_UNWIND4D`/`run_unwind` y **no**
   `loop_reopen`.
3. Debe ejecutarse `c357` una sola vez y el enemigo debe morir, sin repetir la
   animación.
4. Si se reproduce el enemigo `01aa86`, validar también `b002 -> f2f8`
   (`RUN_UNWIND`).

No se ha hecho commit.

## 13. Sesión 43 — validación final y cierre

La run de `kult-dano-todo.bat` del 2026-10-21 22:21 (local) validó el segundo
camino y cerró T-087. Logs completos (append-only) en
`build-win/kult-0.1.0-win64/trazas-post-fix-20261021/`; bloque limpio de la run
validada en `.../validacion/` (con `README.txt` y `SHA256SUMS`).

Evidencia principal, enemigo fuerte `01aab3`:

```text
2657 e1fc -> WALK_RUN entry=21d6c
2657 byte 0x4d -> WALK_4D cmd=f1c2 (dispatch_a6)
2939 walk4d_ret
2939 run_unwind a=21d86 b=1e14d c=0f1c2
2939 run_unwind_out
     (sin loop_reopen al anillo 21f1a)
3184 c357 -> clase 01ab30 en 3448
```

En `dano-todo-marcas.log`: `D 2939 RUN_UNWIND4D cmd=f1c2 a6=1e14d`. Después, el
enemigo `01aa86` muere con `c1d2`/`c1d7`/`c357` (5700/5772/5930) y clase
`01ab30` en 6168.

Tests y build: `make test` **516**, `make test-port` OK, `kult.exe`
`be2cd45a…`, `kult-diag.exe` `16605768…`. Resumen completo y hashes:
**`notes/0037-dos-caminos-unwind-a7.md`**.

Salvedad: la run manual valida el camino indirecto `dispatch_a6`; el camino
directo `b002/f2f8` queda cubierto por `test_run_command_f000_unwind` y el
diagnóstico de §11, pero no se reejecutó manualmente en esta captura.
