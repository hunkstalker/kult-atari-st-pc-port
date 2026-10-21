# 0028 — Retornos dobles de `elem_command` (`0x128a4`): deuda H-el1

> **Estado:** **implementado, probado y validado visualmente por el usuario**
> (sesión 30, T-081). El aborto se propaga por
> `elem_command_ex`/`elem_command_wrap_ex` a los 12 llamantes; los tests
> diferenciales cubren las dos salidas (`$1ad0a` y clase no encontrada).
>
> **Síntoma que la destapó:** artefactos gráficos en el reveal de la estrella /
> elemento (punto 4). El reveal seguía ejecutándose después de una salida
> anticipada de `elem_command` y pintaba con `A_ELEM_*` viejo.

## 0. Aclaración: el supuesto “punto 3”

El F12 **4905** que se usó para hablar de un “punto 3” de
`object_info_card`/`16dec` cae dentro del reveal largo **4471–4919**, que es el
mismo fallo H-el1 del punto 4. `object_info_card`/`16dec` aparece después
(4919/5048) como ficha normal, no como un fallo observado por el usuario. Por
tanto **no hay un punto 3 independiente**: era una inferencia del log antiguo,
anterior a este fix. No se le asigna tarea.

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

Arreglar solo `reveal_pixel` corregiría el glitch de la estrella, pero dejaría 11
rutas con la misma semántica sin modelar. En la sesión 30 se corrigieron las 12.

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
- `0x128a4`: **modelado** en la sesión 30 con `elem_command_ex` (esta nota).
- Auditoría de otros `addq.l #N,a7` + `rts`/`bra`: ver §9 (no queda ninguno).

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

- `make test`: **511** actuales + nuevos casos.
- `make test-port`.
- Build nativo/Windows/ZIP.
- Validación visual: punto 4 sin artefactos; A/B/C sin regresiones.

## 9. Auditoría de otros retornos dobles

> **Resultado (sesión 30):** barrido del PRG con Capstone sobre `addq.l #N,a7`
> (34 apariciones) y sobre `adda.l`/`lea ...(a7),a7`. Solo hay **tres retornos
> dobles reales**: `0x1294c` (esta nota, ya modelado), `0x14f4a`
> (`anim_threshold`, ya modelado por `anim_threshold2`) y `0x1592e`
> (`icon_ring_loop`, ya modelado sustituyendo el `addq` + `bra 0x103ae`). El
> resto son limpiezas normales de argumentos tras `trap`/`trap #14` o de
> `movem`; no quedan retornos dobles sin modelar.

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

## 11. Checklist de la sesión 29 (histórico; completado en la 30)

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

## 12. Implementación (sesión 30)

- `src/element.c`: `elem_command_ex(..., int *aborted)` marca el aborto en las
  dos salidas; `elem_command` y `elem_command_wrap` se mantienen como envoltorios
  compatibles sin estado, y se añaden `elem_command_wrap_ex`.
- `src/reveal.c`, `src/handlers.c`, `src/scale.c`, `src/screens.c`: los 12
  llamantes consultan el aborto y salen con el script avanzado **sin** blit,
  reveal ni tocar `A_ELEM_*`. En `attract` se devuelve `entry_out`.
- Tests nuevos (12): `test_fun_12f28_abort`, abortos de `blit_dispatch_a/b`,
  `overlay_dispatch_1/2`, `reveal_box_build/wait_a/wait_b`, `elem_blit_vbl`,
  `scale_prep`, `scale_loop` y `attract`; cada uno con `$1ad0a` y clase no
  encontrada, comparando imagen y A6.
- `make test`: **511** verdes; `make test-port` OK; nativo/Windows/ZIP OK.
- Exe Windows `c7b8dac1531acd828cf037ac8f309d2d`; headless
  `76e7974e1f52ce5eb65f9c542dec4327`; ZIP
  `787bd41d600b3d0ec19046d296d4df08`.
- **Validación visual:** confirmada por el usuario; el glitch del punto 4
  desaparece con el aborto propagado.

## 13. Referencias

- `/app/kult-atari-st-c/docs/REVISION.md`: H-el1 (ALTA), H-SC1, H-an2,
  H-el2, H-el3.
- `/app/kult-atari-st-c/docs/REFERENCIA_F4.md`: salidas tempranas de `0x128a4`
  y `0x14f24`.
- `src/element.c` (`elem_command`), `src/reveal.c` (`reveal_pixel`),
  `src/scale.c`, `src/handlers.c`, `src/screens.c`.
- `test/test_element.py` (`stop_pc=0x1294c`), `test/test_reveal.py`.
- Log: `build-win/kult-0.1.0-win64/kult-marcas.log` (run con F12
  4516/4640/4767; `12f28` 4471–4919).

## 14. Reapertura de T-061 (sesión 48)

- El usuario confirmó una pasada visual actual **sin glitches gráficos** en Windows.
- El ítem “reproducir el glitch gráfico aislado” de T-061 se retira: no tenía
  caso, log ni captura propios y era un resto del punto 4 (H-el1), ya cubierto
  por T-081. No hay un segundo glitch reproducible.
- El checklist base de T-061 está validado, pero T-061 **sigue abierta**: falta
  jugar de principio a fin y corregir los bugs que aparezcan. T-073 sigue
  abierta como calibración fina no bloqueante.
