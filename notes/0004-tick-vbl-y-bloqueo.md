# Nota 0004 — Tick de VBL (0x160c2) y bloqueo de 0xc001

- **Sesión 4 (M2/T-021).** Se porta el tick de VBL del original a `boot/vbl.c` y
  se conecta al reloj de la HAL. **No desbloquea el bucle**: la hipótesis de la
  sesión anterior (que el tick era lo que faltaba para `0xc001`) queda **falsada**.

## Qué se hizo
- `boot/vbl.c` — reconstrucción de `0x160c2`:
  - `addq.b #1,A_DAT_1ACD4`;
  - según `A_DAT_1AD0D`: rotación de paleta (12..15) cuando vale 5, o el trabajo
    normal cuando vale 0 (cualquier otro valor: `rts`);
  - cada 64 VBLs: `A_DAT_1AB60++` + strip 16×30 desde GAUSS (`0x268de`) a las dos
    pantallas;
  - en VBLs impares con `A_FRAME_1AB4F`: blit enmascarado de 3 planos desde la
    lista que arranca en `0x1a82a` (MURSM `0x23e9c`; al cruzarla, TEMPL `0x1d402`).
  - `pal_next_byte` (0x16210) y `0x10768` (rts) del core.
- `platform_set_tick()`: el driver registra `kult_vbl_tick` antes de arrancar;
  sin hook, el reloj sólo incrementa el contador.
- Prueba en `make test-port` (contador, `AB60` en el 64.º VBL, rotación de paleta).

## Diagnóstico del bloqueo (refinado)
`run_command(0xc001)` → `vm_run(0x1d403)` → byte 60 `script_jump_zero`:
`script_expr` da 0 → `script_push` salta a `TEMPL + 0x6a20 = 0x23e22` → byte 7
`blit_dispatch_a` → `elem_command(py=0x4a)` → idx 73 → glifo 17, **fuera** de la
tabla `0x1a252` (que sólo tiene ~8-10 patrones y luego mezcla la lista de recursos).
Con `width=0` el bucle de `elem_bitmap_build` desborda la imagen.

Es una **divergencia de estado/evaluación** con el original, no del tick: el
original no toma ese camino (o la tabla tiene más entradas en runtime). El tick
ya está puesto y el fallo persiste.

## Siguiente (T-024)
Comparar con el **oráculo** (Musashi) el estado en el dispatch de `0xc001`:
tabla `0x1a252`, operandos de `script_arg` (`0x1a752`), contadores
`ACD4/AB60/ACD7/1A579`. Alternativa: instrumentar `script_expr`/`script_arg`.
