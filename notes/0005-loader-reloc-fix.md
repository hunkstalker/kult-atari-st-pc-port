# Nota 0005 — Causa raíz del bloqueo: relocación DRI del loader C

- **Sesión 5 (T-024).** El bloqueo de `run_command(0xc001)` no era del tick ni
  del core, sino del **loader del PRG del port** (`boot/prg.c`): aplicaba una
  relocación **de más** después de cada `RELOC_SKIP` del stream DRI.

## El bug
El stream de relocación de un `.PRG` es: un `u32` con el offset del primer
longword a relocalizar, y luego **un byte por paso**: `0` = fin, `1` = salto de
254 **sin relocalizar**, cualquier otro = delta al siguiente. El C hacía:

```
apply(cur);
c = fgetc();
if (c == 1) { cur += 254; continue; }   // <-- el continue vuelve arriba y APLICA
...
```

El `continue` volvía al `apply` del principio, así que **cada salto de 254 añadía
una relocación fantasma** en la posición saltada. Resultado: **544 longwords**
corruptos en la imagen (uno de ellos alteraba el largo de una entrada de la tabla
ICONE `0x2751e`), lo que cambiaba el recorrido del guion de `_start` y llevaba a
`elem_command` con un índice de glifo inválido.

## La corrección
Reestructurado para aplicar la relocación **sólo** al primer offset y tras cada
delta normal, nunca tras un salto. `boot/prg.c`:

```
apply(cur);
for (;;) {
    c = fgetc();
    if (c==EOF || c==0) break;
    if (c==1) { cur += 254; continue; }   // salto: NO aplica
    cur += c;
    apply(cur);
}
```

## Verificación
- La imagen que produce `boot/prg.c` coincide **byte a byte** con la del loader
  del oráculo (`tools/recreate_kit/oracle/loader.py`): **0 diferencias** en 1 MiB
  (antes: 544).
- El port **ejecuta el arranque y `game_loop`** (1200+ frames, framebuffer
  coherente de 15 colores).
- Prueba de regresión en `make test-port` (`test_prg_loader`): un `.PRG`
  sintético con un fixup, un SALTO de 254 y otro fixup; comprueba que la posición
  saltada **no** se relocaliza.

## Lección
Comparar el loader del port contra el del oráculo (misma imagen byte a byte) es
una comprobación barata que debería haber estado desde T-001; queda como parte de
`make test-port`.
