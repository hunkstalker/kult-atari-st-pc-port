# 0018 — Plan para arreglar la animación de C (Maestro) sin repetir el fallo

> **Resuelto en `notes/0019-fix-timing-c.md`** (sesión 22): el problema era el
> `dbf` inline de `0x16dec` sin anotar como espera de CPU. El modelo de input se
> corrigió después en `notes/0020-input-hal-y-presentacion.md` (izquierdo/teclado
> sin skip; derecho con skip).
>
> Complementa a `notes/0017-retomar-maestro.md`. La sesión 17 falló porque
> inventó un cambio de input en vez de medir. Aquí está el protocolo y las
> pistas concretas para no repetirlo.

## 1. Por qué falló la sesión 17

- No existía el modelo A/B/C: se asociaron los 4 bancos `15ac8` de **B** con el
  Maestro.
- El prompt decía “diálogos” y “original” sin precisar; la sesión nueva fue a
  buscar el modelo de botones de otro repositorio y cambió el input.
- No se midió la animación de C: no se usaron los logs para ver el bucle de
  frames ni la cadencia.
- Se propuso un cambio de comportamiento (botón derecho) como “fix” sin
  evidencia y contra la decisión del usuario.

## 2. Reglas duras (obligatorias)

- **Input (corregido en `notes/0020`)**: izquierdo/teclado = acción sin skip;
  derecho = acción + skip. No crear copias de `src/input.c`; el skip se corrige
  en la HAL, no en `wait_frame`. No usar modelos de botones de otros
  repositorios.
- **A/B/C**: A = 3 retratos + descripciones (pixel-reveal); B = 4 retratos +
  descripciones (lift-reveal, 4 bancos `15ac8`/`0x36`); C = 1 retrato del
  Maestro + lift + **diálogo** (único diálogo y único retrato que se mueve),
  después de B.
- C está en los logs: **1219–1249 / 1513–1543**; ~8 frames/16 blits a 1–3
  VBL/frame; **sin** `15ac8`/`15da6`.
- **No** usar `KULT_FRAME_GATE` ni `15da6` como fix de C sin evidencia.
- **Medir antes de tocar**: una hipótesis, un cambio; si no mejora, revertir.
- No tocar A/B; `src/` intacto; no commits salvo petición.
- Deja prueba de cada cambio (`make test` 499 y `make test-port`).

## 3. Qué sabemos ya de C

- El texto/diálogo de C: `0x37`/`0x29`/`0x03`/`0x18`/`0x2e`; los
  `15f6c/15fd4/1606e/16088/160aa` son helpers del guion, no frames.
- Los frames del retrato de C: `12ad6`/`12b04` 16, `124da`/`12510` 8,
  `12950`/`1296c`/`129c8` 8, `12b40` ~4665, `120dc`/`123a0`.
- `124da` = `blit_rect_vbl` (**espera VBL**); `12510` = `blit_rect` (**sin
  espera**). `elem_blit_pair` usa `12510`; `elem_blit_pair_vbl` usa `124da`.
  `elem_mode_blit` (`0x14748`) despacha a la variante VBL solo con modo
  `0x5f`/`0x67`; el modo de C es `$1acc6=0x07`.
- C no pasa por `15ac8` (eso es B) ni por `15da6`/`S`.

## 4. Hipótesis de trabajo (a confirmar)

El retrato de C se actualiza a **1–3 VBL/frame** porque su bucle de frames no
tiene la espera/reveal que sí tenía el original. Candidatos:

1. El camino de C usa `f_012510` (sin VBL) donde debería usar `f_0124da` (con
   VBL), o alterna ambos sin la pausa esperada.
2. El modo de C (`0x07`) no entra en `elem_mode_blit`, así que el retrato va por
   el camino de **texto/icono** (`0x37`/`0x29`/`0x03`); ese camino no tiene la
   espera/reveal entre frames.
3. Falta un `wait_vbl`/reveal tipo 4 en el bucle de frames de C.
4. El click pegado (`btn=04` en la run antigua) pudo acelerar texto/prompt; el
   estándar `900fd…` ya lleva el fix. Confirmar la run actual sin botón
   mantenido.

## 5. Protocolo paso a paso

1. **Leer**: `AGENTS.md` (§ T-073c), `RETOMAR.md`, `notes/0017`, este `0018`.
2. **Trazar el caller de C** (sin tocar código): unir las líneas `F`/`Y`/`V` de
   la run de C con el código/ASM:
   `0x37`/`0x29`/`0x03`/`0x18` → `text_entry_blit`/`run_text_entry_ex` →
   `120dc`/`124da`/`12510`/`12950…` → `12ad6`/`12b04`/`12b40`.
   Escribir el grafo de llamadas y los VBL en `notes/`.
3. **Medir la cadencia de C**: frames, VBL/frame, qué variante (`124da` vs
   `12510`) usa cada frame. Comparar con la cadencia esperada del original.
4. **Una hipótesis, un cambio**: si falta una espera/reveal o hay que usar la
   variante VBL, hacer **un** cambio mínimo **solo en el camino de C**.
5. **Probar/medir**: `make test` 499, `make test-port`, run en C, comparar.
   Si no mejora, **revertir** el cambio y probar la siguiente hipótesis.
6. **No** tocar A/B, input, `KULT_FRAME_GATE` ni `15da6`.
7. Al cerrar: actualizar docs; no commit salvo petición.

## 6. Cómo mirar C sin capturas

- Usar `KULT_MARKS` y la traza `Y`/`V` en una build de diagnóstico
  (`notes/0017` §5). Pulsar **F12 después de C**; el buffer de 150 VBL incluye
  C. No hace falta capturar frames.
- Las capturas de las pantallas A/B/C están en `temp/screenshots/` (locales, no
  versionadas) solo como referencia visual.

## 7. Criterio de aceptación

- La animación del retrato de C queda al ritmo del original (visual y/o
  VBL/frame); A/B no cambian.
- Izquierdo/teclado: acción sin skip; derecho: acción + skip;
  `test_click_skip` sigue verde (`notes/0020`).
- `make test` 499 y `make test-port` OK; `src/` intacto.
- Build final sin traza; ZIP regenerado si aplica.

## 8. Primer mensaje sugerido para la sesión nueva

```text
Retomas T-073c (pantalla C del Maestro) en /app/kult-atari-st-pc-port.
Lee AGENTS.md (§ T-073c), RETOMAR.md, notes/0017-retomar-maestro.md y
notes/0018-plan-c.md. Todo en español.

Input: izquierdo/teclado = acción sin skip; derecho = acción + skip (HAL;
ver notes/0020). No crear copias de src/input.c; no tocar wait_frame; no usar
modelos de botones de otros repositorios. No usar KULT_FRAME_GATE ni 15da6
como fix de C sin evidencia.

A/B/C: A = 3 retratos + descripciones; B = 4 retratos + descripciones
(4 bancos 15ac8/0x36); C = 1 retrato del Maestro + lift + diálogo (único
diálogo y único que se mueve), después de B. C está en los logs 1219–1249 /
1513–1543; ~8 frames/16 blits a 1–3 VBL/frame.

Tarea: medir antes de tocar. Traza el caller de los frames de C
(0x37/0x29/0x03 → text_entry_blit/run_text_entry_ex → 120dc/124da/12510 →
12ad6/12b04/12b40; f_0124da espera VBL, f_012510 no; modo C 0x07). Propón
un cambio mínimo solo en C, pruébalo y compara. Si no mejora, revierte.
No toques A/B ni el input. Sin capturas nuevas.
```
