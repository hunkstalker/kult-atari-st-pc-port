# 0062 — T-109: F2 Rellenar (estirado a toda la salida)

Fecha: 2026-10-25. Estado: **cerrada**. T-110 fijó el orden final
`Estirar a pantalla → ×1..×6 → Auto` y la etiqueta `ESTIRAR A PANTALLA`.
Siguiente: T-094 sobre el ZIP nuevo.

## Comportamiento

F2 pasa a ciclar:

`Auto (ajustar) → ×1 → ×2 → ×3 → ×4 → ×5 → ×6 → Rellenar → Auto`

- **Auto/×1–×6**: mantiene 320×200 con escala entera, centrada y con bandas.
  Auto elige el mayor entero que quepa (puede superar ×6 en 4K).
- **Rellenar**: no usa escala entera; dibuja la textura 320×200 en todo el
  rectángulo de salida. Usa factores independientes `out_w/320` y `out_h/200`.
  En 1920×1080 equivale a ×6 horizontal y ×5,4 vertical; llena la pantalla pero
  deforma el 8:5 original.
- El modo persiste al cambiar con F1 entre ventana/fullscreen; Rellenar llena
  la ventana o la pantalla según el modo activo.
- Al salir de Rellenar, F2 vuelve a Auto y recalcula el mayor entero que quepa.

## Implementación

- `platform/display.c/.h`:
  - `PLATFORM_SCALE_FILL` y campo `fill` en `platform_viewport`;
  - `platform_display_scale()` devuelve 0 para Rellenar;
  - `platform_display_viewport()` devuelve el rect completo;
  - `platform_display_point()` mapea el ratón para escala entera y para relleno.
- `platform/video_sdl.c`: ciclo de F2, OSD `ESCALA RELLENAR`, ratón mediante
  `platform_display_point()` y sin redimensionar la ventana en Rellenar.
- No se toca `lib/kult-core/`.

## Verificación

`make test`: **519 core + 34 funciones / 227 `CHECK` HAL + diag + 2
herramientas**. Se cubren viewport de relleno, escala 0, ciclo ×6→Rellenar→Auto
y mapeo de ratón en Auto y Rellenar.

`make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `9a62f3d4f7b988163343e4265462bdf9a29844c3381b76709ec5da69b4849bca`
(11 entradas, `saves/` vacía).

## Limitación

Rellenar no conserva proporción ni píxeles cuadrados; en 16:9 el arte se estira.
