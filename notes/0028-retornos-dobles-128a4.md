# 0028 — Retornos dobles de `elem_command` (`0x128a4`): deuda H-el1

> **Estado:** deuda documentada, **no implementada**. Esta nota fija el análisis,
> el alcance y el plan para una sesión nueva. No se ha tocado `src/` para esto.
>
> **Síntoma que la destapó:** artefactos gráficos en el reveal de la estrella /
> elemento (punto 4). El reveal sigue ejecutándose después de una salida
> anticipada de `elem_command` y pinta con `A_ELEM_*` viejo.

## 1. Evidencia en el log

Run antigua con F12 4516/4640/4767:

- `12f28` (`reveal_pixel`) entra en **VBL 4471**.
- `12fb6` (`masked_word4`, el “dot” del reveal) se ejecuta desde **4471 hasta
  4919**: ~449 VBL ≈ 9 s.
- Los tres F12 caen dentro de ese reveal.
- No aparecen `palette_cycle` ni `16310`; **no es un fallo de paleta**.

`12f28` llama a `128a4` (`elem_command`). En el C actual, `elem_command` retorna
normalmente y `reveal_pixel` continúa; en el original, una salida anticipada de
`128a4` aborta también `reveal_pixel`.

## 2. Semántica ASM que falta

`0x128a4` tiene dos salidas:

- **Normal:** `rts` a `reveal_pixel`.
- **Anticipada:** desde `0x1294c`:

```asm
01294c: 588f        addq.l #4,a7   ; descarta la dirección de retorno del llamante
01294e: 4e75        rts            ; vuelve al llamante de 0x12f28
```

Es un **retorno doble**: no vuelve a `reveal_pixel`, sino a quien llamó a
`reveal_pixel`. La salida anticipada ocurre al menos en:

- `0x128b8`: `$1ad0a != 0`.
- `0x128d2`: clase no encontrada en `0x1285a`.

En ambos casos `A6` ya ha avanzado +2. El C actual devuelve el script avanzado,
pero **no comunica el aborto**; el llamante sigue usando `A_ELEM_1A528/52C/52E/
530/532`, que pueden ser de otro gráfico o no estar inicializados.

## 3. Los 12 llamantes (H-el1)

`REVISION.md` del repo base marca H-el1 como **ALTA** y documenta 12 llamantes.
Mapeo actual:

| # | Función C | Fichero | Tipo |
|---:|---|---|---|
| 1 | `blit_dispatch_a` | `src/handlers.c` | `elem_command_wrap` |
| 2 | `blit_dispatch_b` | `src/handlers.c` | `elem_command_wrap` |
| 3 | `overlay_dispatch_1` | `src/handlers.c` | `elem_command_wrap` |
| 4 | `overlay_dispatch_2` | `src/handlers.c` | `elem_command_wrap` |
| 5 | `reveal_pixel` | `src/reveal.c` | `elem_command_wrap` |
| 6 | `reveal_box_build` | `src/handlers.c` | `elem_command_wrap` |
| 7 | `reveal_box_wait_a` | `src/handlers.c` | `elem_command_wrap` |
| 8 | `reveal_box_wait_b` | `src/handlers.c` | `elem_command_wrap` |
| 9 | `elem_blit_vbl` | `src/handlers.c` | `elem_command_wrap` |
| 10 | `scale_prep` | `src/scale.c` | `elem_command_wrap` |
| 11 | `scale_loop` | `src/scale.c` | `elem_command_wrap` |
| 12 | `attract` | `src/screens.c` | `elem_command` directo |

Arreglar solo `reveal_pixel` corrige el glitch de la estrella, pero deja 11
rutas con la misma semántica sin modelar.

## 4. Por qué produce artefactos

Secuencia actual:

1. `reveal_pixel` llama a `elem_command_wrap`.
2. `elem_command` detecta `$1ad0a` o clase no encontrada y retorna el script
   avanzado.
3. `reveal_pixel` **no sabe que debe abortar**.
4. Usa `A_ELEM_*` viejo, calcula `total`, y ejecuta el bucle de `masked_word4`.
5. El resultado es una transición/reveal que en el original no existiría, con
   gráficos viejos o no cargados.

## 5. Otros retornos dobles conocidos

- `0x14f24` (`anim_threshold`): **modelado** en `anim_threshold2`
  (`src/commands.c`): devuelve 2 y el llamante se salta `palette_cycle`.
- `0x15888` (`icon_ring_loop`): **modelado** sustituyendo el
  `addq.l #4,a7` + `bra 0x103ae` por el despacho.
- `0x128a4`: **no modelado** (esta nota).
- Quedan por auditar otros `addq.l #N,a7` + `rts`/`bra` (ver §8).

## 6. Por qué los tests no lo detectan

- `test/test_element.py::test_fun_128a4_ff` verifica las salidas tempranas con
  `stop_pc=0x1294c`: comprueba la función, **no** que el llamante aborte.
- `test/test_reveal.py::test_fun_12f28` no cubre el camino de salida anticipada.
- En general, un test con `stop_pc` en una salida temprana es sospechoso de
  ocultar este tipo de fallo de control de flujo.

## 7. Diseño de arreglo recomendado

No usar un parche en la HAL: la HAL no conoce la salida anticipada de
`elem_command`.

Propuesta:

1. Añadir un estado de aborto a `elem_command`:
   - `int elem_command_ex(..., int *aborted)`, o
   - un `elem_result { uint32_t script; int aborted; }`.
2. Mantener `elem_command_wrap()` actual como compatibilidad para tests y
   añadir `elem_command_wrap_ex(..., int *aborted)`.
3. En salida anticipada:
   - devolver el script ya avanzado (el A6 real);
   - marcar `*aborted = 1`.
4. En cada llamante: si `aborted`, retornar el script **sin** blit/reveal y sin
   tocar `A_ELEM_*`.
5. En `attract` (llamada directa): si aborta, devolver `entry_out` y no seguir
   con el resto del cuerpo.

Alternativa descartada: un flag global `g_elem_abort`. Es fácil de olvidar y
hace los tests dependientes de estado global; un out-param/estructura es más
seguro.

## 8. Plan de pruebas

### 8.1 Casos mínimos

Para cada llamante, dos casos:

- `$1ad0a = 1`.
- Clase no encontrada en `0x1285a`.

Comprobar:

- la imagen no cambia (no hay reveal/blit);
- el A6 devuelto coincide con el del original;
- no se ejecuta `masked_word4`/`render` posterior.

### 8.2 Orden

1. `reveal_pixel` (reproduce el glitch del punto 4).
2. `scale_prep` / `scale_loop`.
3. `attract`.
4. Dispatchers de `handlers.c` (`blit_dispatch_*`,
   `overlay_dispatch_*`, `reveal_box_*`, `elem_blit_vbl`).

### 8.3 Regresión

- `make test`: 499 + nuevos casos.
- `make test-port`.
- Build nativo/Windows/ZIP.
- Validación visual: punto 4 sin artefactos; A/B/C sin regresiones.

## 9. Auditoría de otros retornos dobles

Buscar sistemáticamente en el PRG:

- `addq.l #N,a7` seguido de `rts`.
- `addq.l #N,a7` seguido de `bra`/`jmp` a despacho.
- `lea ...(a7),a7` / `adda.l #N,a7` con `rts` (limpiezas que puedan saltarse un
  marco).

Para cada aparición:

1. Dirección y función en `work/names.txt` / `docs/mapa`.
2. Llamantes en C (`rg`).
3. ¿Se modela el salto? ¿Hay `stop_pc` en la salida?
4. Añadir a la tabla de deuda o marcar como falso positivo.

Sospechosos ya conocidos:

- H-el1 (`0x128a4`, esta nota).
- H-SC1 (`scale_prep`/`scale_loop`); se resuelve con H-el1.
- H-an2 (`dbra` do-while).
- H-el2 (`walk_entry` firma).
- H-el3 (`mulu.w` 16×16).
- `0x14f24` y `0x15888` ya modelados.

## 10. Riesgos

- Cambiar 12 funciones del core puede afectar a varias escenas; hacer commits
  pequeños y validar por fases.
- No tocar `src/` sin autorización explícita.
- No usar `stop_pc` en `0x1294c` como única prueba.
- Mantener el comportamiento actual del A6 cuando no hay aborto.

## 11. Checklist para la sesión nueva

1. Leer esta nota, `notes/0017` §5, `REVISION.md` H-el1/H-SC1 y
   `test/test_element.py`/`test/test_reveal.py`.
2. Construir la build de diagnóstico Y/V (swap `src/dispatch.c` ↔
   `platform/dispatch_trace.c`; no empaquetar).
3. Reproducir: F12 en el reveal; confirmar `12f28`/`12fb6` y la salida
   anticipada (`$1ad0a` o clase no encontrada).
4. Añadir test de `reveal_pixel` que falle con el C actual.
5. Implementar `elem_command`/`elem_command_wrap` con estado de aborto.
6. Corregir `reveal_pixel` y validar visualmente el punto 4.
7. Extender a los otros 11 llamantes con sus tests.
8. Auditar otros retornos dobles (§8).
9. `make test` + `make test-port` + builds.
10. Actualizar esta nota/TODO/RETOMAR y hacer commit.

## 12. Referencias

- `/app/kult-atari-st-c/docs/REVISION.md`: H-el1 (ALTA), H-SC1, H-an2,
  H-el2, H-el3.
- `/app/kult-atari-st-c/docs/REFERENCIA_F4.md`: salidas tempranas de `0x128a4`
  y `0x14f24`.
- `src/element.c` (`elem_command`), `src/reveal.c` (`reveal_pixel`),
  `src/scale.c`, `src/handlers.c`, `src/screens.c`.
- `test/test_element.py` (`stop_pc=0x1294c`), `test/test_reveal.py`.
- Log: `build-win/kult-0.1.0-win64/kult-marcas.log` (run con F12
  4516/4640/4767; `12f28` 4471–4919).
