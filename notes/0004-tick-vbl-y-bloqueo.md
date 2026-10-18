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

## Diagnóstico añadido (T-024, inicio)
Trazado con gdb: la plantilla de `0xc001` (`0x1d403`) empieza con byte 60
(`script_jump_zero`) y su expresión lee **`img[0x1acd8]`** (`A_DAT_1ACD8`, "flag
puesto por `0x1682a` tras cargar CLEAR.BIN"):

- con `0x1acd8 == 0` (valor del PRG, lo nuestro): `cond != 0` → avanza y por la
  pila de guion acaba en los scripts de reveal `0x23df5…0x23e22`, y `0x23e22`
  (byte 7) pide el elem idx 73 → **crash**;
- con `0x1acd8 == 2` (experimento, revertido): `cond == 0` → `script_push` a
  `TEMPL+0xd00` → `scene_mode_apply` → `icon_table` → `save_rect_body` con
  `cols=0` → **crash distinto**.

Es decir, el punto de bifurcación depende de estado que el arranque original fija
de otra forma (`reset_clear`/`prepare_room` no se llaman en `_start`). `0x1acd8` es
sólo el primero; la rama correcta necesita el estado de sala/guion completo. No se
ha dejado ningún cambio de código (experimento revertido).

## Siguiente (T-024)
Correr el **original** (Musashi / `kult_engine` del predecesor, o el oráculo del
kit) desde `0x10000` con los ficheros *staged* y capturar la imagen en el dispatch
de `0xc001`, para comparar `0x1acd8`, la pila de guion (`0x1a7d6`=`0x1a802`),
`0x1a752`, `0x1a252` y los contadores `ACD4/AB60/ACD7/1A579`. Alternativa:
instrumentar `script_expr`/`script_arg` y comparar el árbol de decisiones.
