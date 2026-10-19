# 0016 — Histórico de la sesión 16 (corregido)

> **No es el handoff actual.** El documento autoritativo es
> `notes/0017-retomar-maestro.md`. Este archivo conserva la evidencia técnica de
> la sesión 16 y la corrección posterior: la asociación inicial de los 4 bancos
> con el Maestro era errónea. No se usa ningún modelo de botones de otros ports.

## Qué se hizo en la sesión 16

- Se añadió una copia diagnóstica `platform/dispatch_trace.c` (sustituye a
  `src/dispatch.c` solo en el port) con:
  - `Y <vbl> <byte> <script>` (bytes del guion de la VM);
  - `V <vbl> <byte> <script> <acc9> <acd2> <aaf9> <ad1d> <acc6> <skip> <a572>
    <a0f> <cx> <cy> <btn> <cmd> <obj>` (estado).
- Se descubrió el **click pegado**: `A_INPUT_BUTTONS` bit2 (`0x04`) quedaba
  activo desde VBL 2 sin clic real. Se corrigió en `platform/input.c`
  (sincroniza el bit con el IKBD) y `platform/video_sdl.c` (refresca con
  `SDL_GetMouseState`). El fix se conserva.
- Se probó `KULT_FRAME_GATE`: espacia los bancos, pero **no** cambia el frame
  visible; se descartó como fix.
- Se identificaron los 3 reveals de la pantalla A (`0x0d`/`12f28` en
  22/119/211) y los 4 bancos `15ac8` de la pantalla B.

## Corrección posterior (A/B/C)

- **Pantalla A**: 3 retratos + descripciones (pixel-reveal). Sin diálogo.
- **Pantalla B**: 4 retratos + descripciones (lift-reveal). Sin diálogo. Aquí
  están los 4 bancos `15ac8` y el `0x36` de ~1095/1352. **No es el Maestro.**
- **Pantalla C**: 1 retrato del Maestro + lift-reveal + **diálogo**. Es el único
  diálogo y el único retrato que se mueve. Está después de B.
  - C en los logs: 1219–1249 (run 1258) / 1513–1543 (run 1560).
  - ~8 frames / 16 blits (`12ad6`/`12b04` 16, `124da`/`12510` 8,
    `12950`/`1296c`/`129c8` 8, `12b40` ~4665) a 1–3 VBL/frame.
  - Texto/diálogo: `0x37`/`0x29`/`0x03`/`0x18`/`0x2e`; los
    `15f6c/15fd4/1606e/16088/160aa` son helpers del guion.
  - **No** hay `15ac8` (eso es B) ni `15da6`/`S` en C.
  - La F12 después de C es correcta; el buffer de 150 VBL incluye C.

## Evidencia técnica que sigue siendo válida

- ASM `0x16582` (`update_icon_list`) y `0x16636` (`obj_list_iter`): recorren las
  entradas de la tabla `0x1aa72`, marcan el objeto de la clase actual y cargan
  el banco `0x15ac8`. En la pantalla B aparecen 4 bancos `6x37/6x35`; el C
  `src/loop.c` coincide.
- `0x159bc` (`row_palette`) es el lift; `12ad6`/`12b04` son blits de elementos;
  `124da`/`12510` son blits de sprite (el primero espera VBL); `12950/1296c/
  129c8` y `120dc` son el camino de icono/retrato.
- Los 3 primeros reveals de A (`0x0d`/`12f28`) no se tocaron y siguen su curso.
- `KULT_FRAME_GATE` no es la causa de C; la puerta solo afecta a la
  construcción de bancos en el back buffer.

## Sesión 17 (rechazada)

- Intentó cambiar el mapeo de entrada para mover el salto de reveal a otro
  botón. No era la causa de C, contradecía la decisión del usuario y se
  revirtió. No se conserva ese enfoque.

## Estado actual

- Commit `47e1028`; exe estándar `900fd993…`; ZIP regenerado; `make test` 499 y
  `make test-port` OK.
- Input: clic izquierdo salta; derecho sin función; fix de click pegado
  conservado.
- La build estándar no lleva `Y`/`V`; la traza es diagnóstica y va aparte.
- Detalle actual y plan: **`notes/0017-retomar-maestro.md`**.
