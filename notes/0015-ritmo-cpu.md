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
  `cpu_delay` real, el motor de elementos 15da6 queda paced.
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

## Caso 15da6 delay 0 (objeto al inventario, no la pantalla C)

El tramo de `15da6` con delay 0 que se veía en el log **no era la pantalla C
del Maestro**: era un **objeto al inventario**. El `dbf` de `15da6` se salta y
el único pacer era `wait_vbl`; el blit interno seguía costando CPU. El cobro
extra se desactivó (`KULT_15DA6_BLIT=0`) porque afectaba a todos los elementos
con delay 0. La pantalla C del Maestro **no usa `15da6`**: usa el camino de
texto/retrato (`12ad6/12b04`, `124da/12510`, `12950/1296c/129c8`, `12b40`).
- Los otros retratos usan delay 20/60 -> 5/13 VBL y ya iban bien (fórmula ST
  `1 + delay/5`).
- El cobro extra sigue **desactivado por defecto** (`KULT_15DA6_BLIT=0`); si se
  activa, afecta a todos los elementos con delay 0 (por eso se revirtió).
- La pantalla C (Maestro) se localiza con F12 y el análisis de logs; ver
  `notes/0017-retomar-maestro.md`.
- `KULT_MARKS` escribe también `S <vbl> <delay> <w> <h> <reveal>` por llamada a
  `15da6`, pero **C no aparece en `S`**: su animación es texto/retrato, no
  `15da6`.

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

## Sesiones 16–20 (T-073c) — corregido

- **Sesión 16**: trazas `Y`/`V`; click pegado (`A_INPUT_BUTTONS` bit2=0x04);
  reveals de A (`0x0d`/`12f28` en 22/119/211); pantalla **B** con `0x36` y los
  4 bancos `15ac8`. La asociación inicial “4 bancos = Maestro” era **errónea**:
  esos 4 bancos son **B**.
- **Modelo correcto**:
  - **A**: 3 retratos + descripciones (pixel-reveal), quietos, sin diálogo.
  - **B**: 4 retratos + descripciones (lift-reveal), quietos, sin diálogo.
  - **C**: 1 retrato del Maestro + lift + **diálogo** (único diálogo y único
    retrato que se mueve), **después de B**.
- **C en los logs**: 1219–1249 / 1513–1543; ~8 frames/16 blits
  (`12ad6/12b04` 16, `124da/12510` 8, `12950/1296c/129c8` 8, `12b40` ~4665) a
  1–3 VBL/frame; texto `0x37`/`0x29`/`0x03`/`0x18`/`0x2e`; sin `15ac8`/`15da6`.
- **Causa de la aceleración (a confirmar)**: el bucle de frames de C va a 1–3
  VBL/frame; probablemente falta la espera/reveal del camino texto/icono
  (`f_0124da` espera VBL; `f_012510` no; modo C `0x07`). No usar
  `KULT_FRAME_GATE` ni `15da6` como fix.
- **Sesión 17**: intento de cambiar el input, **rechazado**. **Sesión 18**:
  revertido; clic izquierdo salta; derecho sin función; fix de click pegado
  conservado. **Sesión 20**: documentación A/B/C corregida.
- **Estado**: commit `47e1028`; exe `900fd…`; ZIP; 499/test-port. Handoff:
  **`notes/0017-retomar-maestro.md`**.
