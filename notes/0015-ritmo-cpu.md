# Nota 0015 — Ritmo: el juego gasta ciclos de CPU; la HAL debe modelarlos (T-073)

- **Sesión 14/15.** Tras corregir colores y título, el usuario detecta que unas
  animaciones van aceleradas y otras no. Se investiga *qué consume tiempo* en el
  original (ASM) y *cómo debe responder la HAL*.

## Qué hace el juego (evidencia ASM)

Bucles `dbra` que **solo consumen CPU** (interrumpibles por el VBL), transcritos
por el core como `cpu_delay(unidades)`:

| dir | qué | bucle original | core |
|---|---|---|---|
| `0x15da6` | motor de elementos (criaturas/movimiento) | `d0=A_ANIM_1A470`; interno `d1=0xbb8` (3001) | `cpu_delay(delay*3200)` |
| `0x12e2e` | reveal pixelado (retrato/habla) | `d1=A_ANIM_1A56A` por punto | `cpu_delay(delay)` |
| `0x12f28` | reveal de elemento | `d1=0x64` fijo por paso | `cpu_delay(0x64)` |
| `0x16310` | `palette_cycle` (fundidos de color) | `wait_vbl` + `dbra 0xffff` + `set_palette` | **el `dbra` se omite** ("sin efecto observable") |

Además hay mucho coste CPU **no** marcado como `cpu_delay` (blits, `masked_word4`,
permuta de bits, `screen_address`…).

## Costes medidos del original (oráculo; referencia del predecesor)

`funcost_st_reference.txt` (ciclos por llamada, media medida con Musashi):

- `0x15da6`: ~835.000 ciclos/llamada ≈ **5 VBLs**.
- `0x12f28`: ~1.557.000 ≈ **10 VBLs**.
- `0x12fb6` (`masked_word4`): 244 × 12.800 llamadas.
- `0x12b40`, `0x123c0/123a0/12ad6/12b04`, `0x10ff4/11014`… miles de llamadas.
- `0x13594` (`wait_vbl`) y `0x13698` (`wait_frame`): su tiempo lo pone el VBL
  (no se cobran); `0x15da6` se dejaba a 0 porque su retardo va por la puerta.

## Por qué el `cpu_delay` solo no basta (T-073a ✅ / T-073b ⬜)

- **T-073a**: el core ya expone `cpu_delay` para los `dbf` transcritos. El port lo
  implementa (`platform/include/kult_port.h` sombra + `platform_cpu_delay` en
  `format/time.c`): convierte unidades (~10 ciclos/`dbra`) a tiempo con
  `KULT_CPU_HZ` (def. 8 MHz), y durante el retardo **corre el tick de VBL y
  presenta** (como el ISR en el original). Sin tiempo real es no-op (diferencial
  y headless determinista; `make test` intacto). `KULT_CPU=0` lo apaga.
- **Pero** eso solo cubre esos bucles. Las animaciones CPU-bound de otras
  funciones (reveal `0x12f28`, blits, etc.) siguen libres → **rápidas**.
- **T-073b (correcto)**: reloj virtual de CPU a 8 MHz **cobrando el coste medido
  por función** (oráculo), como el predecesor: instrumentar `src/*.c` solo en el
  build del port (`-finstrument-functions`, enlace `__cyg_profile_func_enter`) y
  una tabla de costes (generada del oráculo, mapeada a nuestros símbolos) que se
  cobra con el mismo reloj. El diferencial (imagen) no se toca. **No** ajustar
  animaciones a mano.

## Riesgos / notas

- El predecesor tuvo **una animación que nunca cuadró**: la calibración es
  sensible (por eso la tabla trae `TUNE`/gates y `KULT_CPU=0`). Hay que validar
  **escena a escena** contra el original, no de golpe.
- `0x16310` (`palette_cycle`) es el caso testigo de espera **no** transcrita
  (los fundidos de color); el modelo medido lo cubriría.

## Implementación T-073b (hecha)

- **Instrumentación**: `src/*.c` se compila con `-finstrument-functions` **solo**
  en el build del port (Makefile: objetos `build/port-obj/src/*.o`; CMake:
  `set_source_files_properties`). `make test` (diferencial) y `test-port` no se
  instrumentan.
- **Cobro**: `platform/cpuwork.c` implementa `__cyg_profile_func_enter` y cobra
  en cada llamada el coste medido con `platform_cpu_charge_cycles` (misma puerta
  que `cpu_delay`: convierte ciclos a tiempo con `KULT_CPU_HZ`, duerme y cede
  VBLs; `KULT_CPU=0` lo apaga; headless no-op).
- **Tabla**: `tools/gen_cpuwork_costs.py` cruza la referencia medida del oráculo
  (`tools/cpuwork/funcost_st_reference.txt`), `docs/mapa` (dirección ST ↔
  símbolo) y los símbolos globales reales (`nm`) → `platform/cpuwork_costs.inc`
  (**79 funciones** con coste > 0). Regenerar:
  `make cmake-native && python3 tools/gen_cpuwork_costs.py`.
- **Exclusiones** (evitan doble cobro): `0x13594/0x13698` (esperas de VBL/frame),
  `0x15da6/0x12e2e/0x12f28` (sus `dbf` van por `cpu_delay`) y audio
  `0x19xxx/0x3axxx`.
- **Datos del motor de elementos** (2500 frames auto): retardos `$1a470` =
  0 (30×), 10 (10×), 20 (56×), 60 (8×) → cadencia ST 1/3/5/13 VBLs. Con
  `cpu_delay` real, el retrato animado (15da6) queda paced.
- **Medido**: 600 frames, CPU off 26,7 s → CPU on 34,6 s (el modelo añade el
  trabajo de blits/reveal). `make test` **499**, `make test-port` verde,
  nativo/cross OK.

## Medida objetiva contra el ST/Hatari

- La velocidad "correcta" es la del original; no se adivina. El port expone
  **`KULT_FPSLOG=1`**: cada ~3 s imprime `fps juego`, contando iteraciones del
  bucle (= `wait_frame`, lo mismo que `Ikbdws` en el arnés Hatari del predecesor
  `tools/hatari_fps/`).
- Procedimiento: medir el FPS **en la misma escena** con Hatari (CPU
  cycle-exact, TOS real) y en el port. Si el port va más rápido, bajar
  `KULT_ST_CONTEND` (o `KULT_CPU_HZ`) hasta igualar. En fases dominadas por VBL
  el FPS no cambia con la CPU; en las CPU-bound (retratos) sí.
- `KULT_ST_CONTEND` modela la **velocidad efectiva** del ST (8 MHz nominales
  menos contención del shifter/wait states). Sin una medida no se toca el valor
  por defecto (100 %).

## Caso Maestro: retardo 0 (el que nunca cuadró en el legacy)

- El retrato del Maestro (último tramo del log) usa `$1a470 = 0`: el `dbf` de
  `15da6` **se salta** y el único pacer era `wait_vbl` -> **1 VBL/frame**
  (31 frames seguidos en el log). En el ST el **blit interno** de `15da6` sigue
  costando CPU y lo frena; ese trabajo no se cobraba porque `15da6` está excluido
  de la tabla (para no duplicar el `dbf`, que va por `cpu_delay`).
- Los otros retratos usan delay 20/60 -> 5/13 VBL y ya iban bien (fórmula ST
  `1 + delay/5`).
- **Corrección (usuario)**: el tramo de `15da6` a 1 VBL **no era el Maestro**,
  sino un **objeto que se mueve al inventario**, y su velocidad era correcta. El
  cobro extra se ha **desactivado por defecto** (`KULT_15DA6_BLIT = 0`); si se
  activa, afecta a *todos* los elementos con delay 0 (por eso se revirtió).
  El Maestro sigue **sin identificar**: se añade una marca manual **F12** que
  escribe `M f12 <vbl>` en `KULT_MARKS` para señalar el instante exacto de su
  animación y localizar la función/VBL sin tocar otras animaciones.
- `KULT_MARKS` ahora escribe también `S <vbl> <delay> <w> <h> <reveal>` por
  llamada a `15da6`, para calibrar con el tamaño real del retrato.

## Logs de marcas (F12 y runs múltiples)

- `kult.log` y `kult-marcas.log` ahora se abren en **append**: cada run añade su
  cabecera/`# RUN <epoch>` y no borra las anteriores. Antes se sobrescribían y se
  perdía la evidencia de la run del Maestro al volver a lanzar.
- **F12** escribe `M f12 <vbl>` en el log de marcas y `F12: marca manual` en
  `kult.log`; además:
  - vuelca un **buffer circular de los ~3 s ANTERIORES** (150 VBLs), y
  - sigue trazando **todo el core instrumentado, una vez por función y VBL**,
    durante ~1,5 s (75 VBLs).
  Así se puede pulsar **después** de una animación corta (<1 s) y aun así queda
  registrada; no hay que cronometrar la pulsación.
- Marcas añadidas además de `15da6/reveal/123c0/12b40/15a90/1215e`:
  `15d7e` (anim_restore), `159e2/159d2` (anim_entry), `15ac8` (anim_bank),
  `138a6` (scale_anim), `13740` (scale_loop), `12ce4` (view_mode_cycle),
  `12e08/12ea2` (reveal_box). Así se identifica el retrato del Maestro sin
  suposiciones.

## Calibración pendiente (T-073c)

- Ajustar escena a escena con `KULT_CPU_HZ` (def. 8 MHz), `KULT_ANIM_GATE` y
  `KULT_REVEAL_GATE`; `KULT_CPU=0` para A/B. No tocar animaciones a mano.
- `0x16310` (`palette_cycle`) **no** está en la referencia medida del predecesor
  (no se ejecutó en su escenario): el `dbra 0xffff` omitido sigue sin cobrarse.
  Si los fundidos de color van rápidos, hay que medirlo/cubrirlo aparte.

## Sesión 16 (T-073c): retrato del Maestro — resumen y handoff

- Se añadió una copia diagnóstica `platform/dispatch_trace.c` (sustituye a
  `src/dispatch.c` **solo en el port**) con líneas `Y <vbl> <byte> <script>` y
  `V <vbl> <byte> <script> <acc9> <acd2> <aaf9> <ad1d> <acc6> <skip> <a572>
  <a0f> <cx> <cy> <btn> <cmd> <obj>`.
- Los **3 primeros retratos** usan `reveal_pixel` (`0x0d`/`12f28`): VBL
  22/119/211 (deltas 97/92) en la run nueva frente a 29/127/219 (98/92) en la
  anterior; `S 15da6` del intro = 63 en ambas. **No se ha cambiado ningún otro
  reveal.**
- El **Maestro** usa `scene_mode_draw` (`0x36`; operando de guion `0x07` en
  `0x1d41c` de la imagen PRG, base 0x10000) y **no** llama a `12e2e/12e08/
  12ea2/12fb6`. Los 4 bancos `15ac8` son 4 objetos/partes (ASM `0x16582`/
  `0x16636`, el C coincide) y los blits `12ad6/12b04` son el dibujo del retrato.
- **`KULT_FRAME_GATE` descartado**: con 10 los bancos salen a 10 VBL exactos
  (comprobado en nativo), pero el frame visible sigue acelerado
  (`kult-frame10.log`, bancos 1365/1375/1385/1395; scene draw de 59→96 VBL). La
  puerta solo retrasa la construcción en el back buffer.
- Estado `V` del Maestro: `$1acc9=0x0a` (tras el byte `0x52`), `$1acd2=0x02`,
  `$1aaf9=0x63`, `$1ad1d=0`, modo `0x07`, skip 0. `anim_mode` (`0x188ce`) exige
  `0x3b..0x3e` y `$1aaf9==$1acd2`; no entra. `icon_repeat` (`0x18944`) retorna
  si `$1ad1d==0`; por eso no hay `15da6` en el Maestro. El `$1acc9` se fija
  después del scene draw y `$1aaf9` solo lo actualiza `icon_repeat` (que no
  corre): falta el **segundo scene draw/camino** del original tras la selección
  (hipótesis principal).
- Bug extra: `A_INPUT_BUTTONS` bit2 = `0x04` **pegado** desde VBL 2 (click
  fantasma). Fix en `platform/input.c` (sincroniza con el IKBD) y
  `platform/video_sdl.c` (refresca con `SDL_GetMouseState`); exe de traza
  `3fc2f940e11db072f1d0ce65785158ee`, **pendiente de probar**.
- **Build diagnóstico activo**: `platform/dispatch_trace.c` + filtros
  Makefile/CMake + `platform_marks_state`; **revertir antes del ZIP final**. El
  ZIP estándar no se ha regenerado. `make test` 499 y `make test-port` verdes.
- **Handoff completo y evidencia línea a línea: `notes/0016-maestro-handoff.md`.**

## Sesión 17: el Maestro no era (solo) ritmo de CPU — RECHAZADA (ver sesión 18)

- El caso T-073c quedó acotado a **input**: el reveal/lift comprueba
  `$1a570`; el `wait_frame` del core lo fijaba con el **clic izquierdo**, así
  que cada clic para avanzar saltaba el reveal. En el original el izquierdo solo
  interactúa y el **botón derecho** salta las transiciones
  (`kult-port-legacy/docs/RUNTIME.md`).
- Fix port-side: `platform/input_core.c` (copia justificada de `src/input.c`) +
  estado del botón derecho en la HAL (`platform/input.c`, `platform/video_sdl.c`)
  + filtros Makefile/CMake. `src/` intacto; prueba en `make test-port`.
- Verificado: izquierdo → `$1a570=0`; derecho → `$1a570=0xff`. El modelo de CPU
  (`cpu_delay`/`cpuwork`) no se toca; los reveals siguen su curso normal.

## Sesión 18: corrección (se rechaza el cambio de la sesión 17)

- Se revierte el mapeo al botón derecho: se elimina `platform/input_core.c`, el
  port vuelve a compilar `src/input.c` y el **clic izquierdo** sigue saltando
  transiciones/reveal. El botón derecho queda **sin función**.
- Se conserva el fix de click pegado (bit2 de `A_INPUT_BUTTONS` sincronizado con
  el botón real; refresco con `SDL_GetMouseState`) y el hallazgo de que los
  brazos del Maestro aparecen **después del primer prompt**.
- El exe `296ce6…` y la prueba del botón derecho quedan invalidados. T-073c
  sigue abierto: `15da6`/segundo scene draw.
