# 0017 — Handoff autoritativo: pantallas A/B/C y retrato del Maestro

> Este documento sustituye a `notes/0016` para todo lo relativo a T-073c.
> No se necesitan capturas nuevas: los logs actuales ya contienen C. Las
> Las capturas de las pantallas A, B y C son locales (`temp/screenshots/`, no
> versionadas) y solo evidencia.

## 0. Glosario y modelo correcto

- **Retrato**: rectángulo de la pantalla donde se pinta un personaje.
- **Pantalla A**: **3 retratos** + **descripciones**. Pixel-reveal. Retratos
  **quietos**. **Sin diálogo**.
- **Pantalla B**: **4 retratos** + **descripciones**. Lift-reveal. Retratos
  **quietos**. **Sin diálogo**. Aquí están los 4 bancos `15ac8` y el `0x36` de
  ~1095 (run 1258) / ~1352 (run 1560).
- **Pantalla C**: **1 retrato del Maestro** + lift-reveal + **diálogo**. Es el
  **único diálogo** y el **único retrato que se mueve**. Está **después de B**.
  - Run F12 1258: C en **1219–1249**; texto/diálogo en 1220–1225; frames del
    retrato en **1225–1247**; prompt en 1249.
  - Run F12 1560: C en **1513–1543**.
  - ~**8 frames / 16 blits** de retrato, a **1–3 VBL/frame**.
- Capturas de las pantallas A/B/C: archivos locales del usuario en
  `temp/screenshots/A.png`, `B.png` y `C.png` (carpeta gitignored; **no se
  versionan**).

## 1. Estado al inicio de T-073c (histórico; ver §8 para el fix)

- Commit **`47e1028`**.
- Exe estándar `build-win/kult-0.1.0-win64/kult.exe` md5
  **`900fd9939df1431a21ea8fa7e06c07d0`**; ZIP regenerado.
- `make test` **499**; `make test-port` OK.
- Input (histórico): en su momento se fijó “clic izquierdo salta; derecho sin
  función”. **Corregido en `notes/0020`**: izquierdo/teclado = acción sin skip;
  derecho = acción + skip.
- La build estándar **no** lleva traza `Y`/`V`; `platform/dispatch_trace.c`
  existe para una build de diagnóstico aparte (ver §5).
- La documentación actual está corregida para reflejar A/B/C. `src/` intacto.

## 2. Detección de C en los logs

Run F12 1258 (C en 1219–1249):

- `0x37`/`0x29`/`0x03`/`0x18`/`0x2e`: texto del **diálogo** y espera de clic.
- Frames del retrato: `12ad6`/`12b04` **16**, `124da`/`12510` **8**,
  `12950`/`1296c`/`129c8` **8**, `12b40` **~4665**, `120dc`/`123a0`.
- Reparto por VBL: el retrato se actualiza en 1225, 1226, 1227, 1228, 1229,
  1231, 1232, 1234, 1235, 1237, 1238, 1240, 1241, 1243, 1244, 1246, 1247.
- El texto/diálogo va en 1220–1225; el `0x2e` en 1249.
- **No** aparecen `15ac8` (son los 4 retratos de B) ni `15da6`/`S` en C.
- Los `15f6c/15fd4/1606e/16088/160aa` son helpers del guion del diálogo, no
  frames del retrato.

Run F12 1560: C en 1513–1543; mismos patrones (texto + frames de retrato).
La F12 después de C es correcta; el buffer de 150 VBL incluye C.

## 3. Por qué va acelerada (hipótesis de trabajo; resuelta en §8)

El bucle de frames del retrato de C se ejecuta a **1–3 VBL/frame** (unas 0,5 s
para ~8 frames). Hay que confirmar en código/ASM cuál de estas causas aplica:

1. El camino de C usa la variante rápida `f_012510` (sin espera de VBL) en vez
   de `f_0124da` (que sí espera VBL), o alterna ambas sin la pausa esperada.
2. El modo de C es `$1acc6=0x07`; `elem_mode_blit` solo actúa con
   `0x09/0x66/0x5f/0x67`, así que el retrato de C va por el camino de
   **texto/icono** (`0x37`/`0x29`/`0x03`), no por `elem_blit_pair_vbl`.
3. Ese camino de texto/icono no tiene el **reveal/typewriter** o la espera
   entre frames que sí tiene el original.
4. El click pegado (`btn=04` en la run antigua) pudo acelerar el texto/prompt;
   el estándar `900fd…` ya lleva el fix. Confirmar la run actual sin botón
   mantenido.
5. **No** es `KULT_FRAME_GATE` ni `15da6`; ambos están descartados para C.

> Plan anti-fallo detallado: **`notes/0018-plan-c.md`**. Guardia actual de
> input: `test_click_skip` en `make test-port` (izquierdo no salta; derecho sí;
> ver `notes/0020`).

## 4. Plan para confirmar y arreglar C

1. **Trazar el caller de los frames de C**:
   `0x37`/`0x29`/`0x03`/`0x18` → `text_entry_blit`/`run_text_entry_ex` →
   `120dc`/`124da`/`12510`/`12950…` → `12ad6`/`12b04`/`12b40`; ver qué
   variante (VBL o no) y qué espera hay entre frames.
2. **Comparar la cadencia**: C hace ~8 frames en ~23 VBL; determinar la cadencia
   esperada del original (los `f_0124da` esperan VBL; los `f_012510` no). Si
   falta una espera/reveal, añadirla **solo en el camino de C**.
3. **Descartar el click**: probar el estándar `900fd…` en C con el botón **no
   mantenido**; si con el fix ya va bien, cerrar sin más cambios.
4. **Fix**: en `platform/` o copia port-side justificada; no tocar A/B, no tocar
   input, no `KULT_FRAME_GATE`, no `15da6` a ciegas.
5. **Reconstruir/validar**: tests, build estándar, ZIP; actualizar docs.

No hace falta capturar frames nuevos: los logs ya tienen la secuencia de C; las
capturas de A/B/C son evidencia visual de las pantallas.

## 5. Build de diagnóstico Y/V (opcional)

La copia `platform/dispatch_trace.c` ya existe y contiene `platform_marks_byte`
(Y) y `platform_marks_state` (V). En la build estándar está excluida. Para una
build de diagnóstico:

- **Makefile**:
  - `PORT_CORE_SRC := $(filter-out src/dispatch.c,$(wildcard src/*.c))`;
  - `PLATFORM_SRC` debe incluir `platform/dispatch_trace.c` (quitar ese nombre
    del `filter-out`);
  - `PLATFORM_TEST_SRC` sigue filtrándolo.
- **CMakeLists.txt**:
  - quitar `src/dispatch.c` de `KULT_CORE_SRC`;
  - no excluir `platform/dispatch_trace.c` de `KULT_PLATFORM_SRC`.
- Reconstruir, copiar el exe a una carpeta de diagnóstico y ejecutar con
  `kult-marcas.bat`.
- **No** empaquetar esa build. Al terminar, revertir los filtros y reconstruir
  el estándar.

## 6. Reglas duras

- **Input (corregido en `notes/0020`)**: izquierdo/teclado = acción sin skip;
  derecho = acción + skip. No crear copias de `src/input.c`; el skip se corrige
  en la HAL (`platform_input_apply_skip`), no en `wait_frame`.
- `src/` intacto. No tocar repos de referencia externos ni datos del usuario.
- No usar `KULT_FRAME_GATE` ni retardos manuales como fix de C.
- No usar `15da6` como objetivo de C sin que la traza lo demuestre.
- No hacer commits salvo petición. Mantener `RETOMAR.md`, `TODO.md`,
  `PROYECTO.md` y `notes/`.

## 7. Prompt para la próxima sesión

```text
Retomas el port nativo de Kult en /app/kult-atari-st-pc-port. Lee primero
notes/0017-retomar-maestro.md, RETOMAR.md y TODO.md. Todo en español.

MODELO CORRECTO
- Pantalla A: 3 retratos + descripciones (pixel-reveal). Sin diálogo.
- Pantalla B: 4 retratos + descripciones (lift-reveal). Sin diálogo.
- Pantalla C: 1 retrato del Maestro + lift-reveal + diálogo. Único diálogo y
  único retrato que se mueve. Está después de B.
- C está en los logs: 1219–1249 / 1513–1543; ~8 frames/16 blits, 1–3 VBL/frame.
- No hay 15ac8 (eso es B) ni 15da6/S en C.

REGLAS
- Input: izquierdo/teclado = acción sin skip; derecho = acción + skip.
- No crear copias de src/input.c; el skip se corrige en la HAL (notes/0020).
- src/ intacto. No tocar repos de referencia externos ni datos del usuario.
- No usar KULT_FRAME_GATE ni retardos manuales como fix de C.
- No usar 15da6 como objetivo de C sin evidencia de la traza.
- No hacer commits salvo petición.

TAREA
1. Analizar el bucle de frames de C en los logs actuales y en el código/ASM:
   12ad6/12b04, 124da/12510, 12950/1296c/129c8, 12b40, texto.
2. Localizar el caller y la espera/reveal que falta (f_0124da espera VBL;
   f_012510 no; modo C 0x07; camino texto/icono).
3. Si hace falta, montar la traza Y/V en build aparte (0017 §5); no capturar
   frames.
4. Corregir solo C, sin tocar A/B ni el input; probar con el estándar 900fd…
5. Actualizar docs y, si se pide, regenerar ZIP.

No des por hecho que C es B ni que los 4 bancos son el Maestro.
```

## 8. Sesión 22 — T-073c resuelto (nota detallada: `notes/0019`)

- Causa: el `dbf` inline de `object_info_card` (`0x16dec`) no estaba anotado.
  El guion de C deja `wait=0x0a` → **40960 iteraciones × 18 ciclos ≈ 92 ms ≈
  4,6 VBL por entrada**.
- Fix (con permiso para tocar `src/`): `src/loop.c` usa `cpu_delay_cycles()`;
  no-op en el diferencial y cobro real en el port (mismo reloj que T-073b).
- C pasa de ~3 a ~**7,6 VBL/frame**; A/B no usan `16dec`; input: ver
  `notes/0020` (izquierdo/teclado sin skip; derecho con skip).
- `make test` **499**, `make test-port` OK (nueva `test_cpu_charge_cycles`).
- Exe estándar `fc4108e473a9d44c722245345a44fa32` (headless `e37f65…`, ZIP
  `8c9266…`). Validación visual confirmada por el usuario; commit `3235aa6`.
- Detalle completo, ASM, medición y verificación: **`notes/0019-fix-timing-c.md`**.
