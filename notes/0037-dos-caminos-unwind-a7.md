# 0037 — T-087: los dos caminos del unwind A7 (cierre)

> **RESUELTO Y VALIDADO POR EL USUARIO (2026-10-21).** T-087 necesitó dos
> fixes porque hay dos entradas distintas a `0x103ae` que forman comandos
> `0xfxxx`. La causa raíz, la captura inicial y el detalle técnico están en
> **`notes/0036-a7-unwind-combate.md`** (§11 y §12); esta nota resume el
> mecanismo, los dos cambios, la evidencia de la run final y el estado de
> cierre.

## 1. Mecanismo original

En `run_command` (`0x103a8`) el original guarda A7:

```asm
0103a8  move.l a7,$1a826
```

En `run_command_keep_sp` (`0x103ae`), para `nibble == 0xf000`:

```asm
0103e0  cmpi.w #$f000,d2
0103e4  bne.w  $103ee
0103e8  movea.l $1a826.l,a7    ; restaura el marco exterior
```

Después camina la plantilla y hace tail-jump al VM (`0x10630`). El `rts` final
del VM (`0x10666`) vuelve al **llamante de `run_command`**, saltándose el resto
del despacho anidado. Es un retorno no local, no una llamada normal.

El port ya modelaba el tail-call `c/a` y el pendiente `$1abf8`, pero no este
unwind de A7.

## 2. Los dos caminos

| Camino | Ejemplo | Entrada en C | Fix |
|---|---|---|---|
| Directo | `b002 -> a010/f2f8` | `run_command_keep_sp_a6` (`0x103ae`), rama `nibble == 0xf000` | sesión 41 |
| Indirecto | `e1fc -> byte 0x4d -> f1c2` | `dispatch_a6` (`0x10390`), que forma un `fxxx` y ejecuta la lógica de `0x103ae` | sesión 42 |

El primer fix no podía resolver el segundo caso: `e1fc` tiene nibble `e`, no
entra por la rama `0xf000`, y el `f1c2` nace dentro de `dispatch_a6`. A la
inversa, el segundo fix no sustituye al primero: `f2f8` sí pasa por
`run_command_keep_sp_a6`.

## 3. Fix 1 — sesión 41: boundary central

- `src/handlers.c` — `run_command` (`0x103a8`): boundary `setjmp`/`longjmp`
  apilable; si hay unwind, el comando termina como el `rts` original y vuelve
  al `game_loop`.
- `include/run_boundary.h`: pila de boundaries (`kult_run_boundary`), helper de
  consulta y disparo del unwind.
- `src/dispatch.c` — `run_command_keep_sp_a6`: para `nibble == 0xf000`, tras
  completar el VM, dispara el unwind si hay boundary activo; sin boundary
  conserva el retorno normal.
- `platform/dispatch_trace.c`: espejo para `kult-diag.exe`.

Trazas: `run_boundary_in/out`, `run_unwind`, `run_unwind_out` y
`RUN_UNWIND` en el log `D`.

## 4. Fix 2 — sesión 42: `dispatch_a6`

- `src/dispatch.c` — `dispatch_a6` (`0x10390`): tras ejecutar el sub-VM `fxxx`,
  si hay boundary activo dispara el mismo unwind. Fuera de boundary mantiene
  el retorno anidado que ejercitan los tests directos.
- `platform/dispatch_trace.c`: mismo cambio con traza `RUN_UNWIND4D`.

No se tocaron el tail-call `c/a`, el pendiente `$1abf8` ni el fallback anidado
sin boundary.

## 5. Run de validación

Logs archivados en:

```text
build-win/kult-0.1.0-win64/trazas-post-fix-20261021/
  README.txt
  SHA256SUMS
  dano-todo-{input,marcas,ring,rnd}.log   # append-only, varias runs
  kult.log
  validacion/                             # bloque limpio de la run validada
    dano-todo-{input,marcas,ring,rnd}.log
    kult.log
    README.txt
    SHA256SUMS
```

La run validada empieza en `kult.log` a las 22:18:48 local y termina a las
22:21:38. En `validacion/` está extraído solo el último bloque; los ficheros de
la carpeta padre conservan el histórico completo.

Enemigo fuerte `01aab3`, en `dano-todo-ring.log`:

```text
2657  e1fc -> WALK_RUN entry=21d6c
2657  byte 0x4d -> WALK_4D cmd=f1c2 (dispatch_a6)
2939  walk4d_ret
2939  run_unwind a=21d86 b=1e14d c=0f1c2
2939  run_unwind_out
      (no hay loop_reopen al anillo 21f1a)
3184  c357 -> run_f_ret 3448 con clase 01ab30
```

En `dano-todo-marcas.log`:

```text
D 2939 RUN_UNWIND4D cmd=f1c2 a6=1e14d
```

Después, el enemigo `01aa86` muere con la secuencia normal:

```text
5700 c1d2 -> 5772 c1d7 -> 5930 c357 -> 6168 clase 01ab30
```

**Salvedad:** esta run valida manualmente el camino indirecto
(`e1fc -> 0x4d/f1c2`). El camino directo `b002 -> f2f8` queda cubierto por el
test automático y por el diagnóstico de `notes/0036`, pero no se reejecutó
manualmente en esta última captura.

## 6. Pruebas

- `test/test_dispatch.py::test_run_command_f000_unwind`: `run_command` con
  `0xf010` y `STACK_TOP`.
- `test/test_a7_unwind.py`: helper C con `setjmp` real; falla sin el unwind de
  `dispatch_a6` y pasa con él.
- `test_fun_10390` y `test_vm_byte_4d_nested_a6`: siguen cubriendo el fallback
  anidado sin boundary.
- `make test`: **516 passed**.
- `make test-port`: **OK**.

## 7. Ejecutables y hashes

```text
kult.exe               be2cd45a734f98ecb01a4ec80de84e59a55105fdb0eb5cdd1ce1b8c65f6789a6
kult-headless.exe      3e44ba7da08efb2fc26eff604ba5521ece3f73f93b5535d9141c163faa59574c
kult-diag.exe          166057685ee712ec657b5e6a9c6c6a2e731b61bbed4a51161c3f166dc64c924d
kult-diag-headless.exe 5065fcc1f8331a82a4e57d1ced4402cf482890c75cdd312af1679b99832d83bc
ZIP                    bf9ad1abf79001c5593da995d94d47c1861b67cfa6aa32d9a889e32d4bb32c0e
```

Bloque limpio de validación (`validacion/SHA256SUMS`):

```text
input  171159eafd8b63f86e7475b82361bf79aec8d1aa827f71e202a5c14ac36f0333
marcas f5d178f2556d880ff6ab85e0f39a2f8bc0e87c3bcd5d8b84802096cb98a604a1
ring   ec447d91093e66a633d3413586a7e71b6ab41c7fd138ebba17fa8f79379968f5
rnd    1839a2a2160e51ed0a81919634d5b9d972b9e514dab09ddefd443c6f21eefe4b
kult   d9531d725371ccfe8569f137556416bcd30c91aabe61318a846ca7a3732d0561
```

Las capturas append-only completas tienen sus hashes en el `SHA256SUMS` de la
carpeta padre.

## 8. Nota sobre el final de la run

El bloque de validación no termina con `fin:` en `kult.log`: el usuario confirmó
que **el proceso se cerraba/crasheaba** al atacar a un personaje, no por cerrar
la ventana. La secuencia final es:

```text
8932/8934  clic izquierdo -> cmd=e105
8936       mouse_up
8949       V 29 20224 -> text_window_entry (byte handler 41)
8950       V 18 20229 -> object_info_card (0x16dec, byte handler 24)
8964       B fn=12b04 sel=1ba6c cls=1aaef src=9abe0
           w=0056 h=0041 x=0016 y=0018
8964       F 12b40 x216 (misma VBL)
           [fin del log]
```

El punto de crash está por tanto en **`object_info_card` (`0x16dec`)** o en el
blit `0x12b04` que lanza para la ficha del personaje. No hay mensaje de error ni
`fin:`; la ventana desaparece. No está relacionado con el unwind A7 de T-087,
que ocurre antes, en VBL 2939.

El seguimiento de este crash fue **T-088**, ya **resuelto y validado en
Windows**. La causa era que `object_info_card` no avanzaba `p` tras
`text_block_build` (el ASM `0x129d6` deja A5 al final del bloque de 3 bytes).
Se añadió un replay de entrada para reproducirlo sin depender del objeto inicial
aleatorio y se corrigió con `p += 3`. Ver
**`notes/0038-replay-input-t088.md`** §9–§11.

## 9. Seguimiento

T-087 queda cerrado. Queda como auditoría posterior revisar los demás handlers
con restauración de A7 (`0x14ee8`, `0x14f74`, `0x15052`, `0x151b0`, `0x152ee`,
`0x15382`, `0x15530`, `0x1699c/a4/b4`), sin parchear el anillo por clase ni
solo `palette_cmd`. Siguiente tarea de M6/M7: **T-061** (validación completa en
Windows) y **T-072** (ingesta `.STX`).
