# 0063 — T-110: orden y etiqueta de Estirar a pantalla

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP nuevo.

## Cambio

F2 queda así:

`Estirar a pantalla → ×1 → ×2 → ×3 → ×4 → ×5 → ×6 → Auto → Estirar a pantalla`

- **Estirar a pantalla**: llena la ventana/pantalla estirando 320×200; no usa
  escala entera ni conserva proporción.
- **×1–×6**: escala entera centrada, con bandas.
- **Auto**: mayor entero que quepa en la salida actual.
- El arranque sigue en Auto; desde Auto, F2 entra en Estirar a pantalla.
- El OSD del modo es literalmente `ESTIRAR A PANTALLA`.

## Implementación

- `platform_display_next()`: `Auto → Fill`, `Fill → ×1`, `×6 → Auto`.
- `platform/video_sdl.c`: OSD `ESTIRAR A PANTALLA` y log `estirar`.
- Las pruebas del ciclo y del viewport/ratón se mantienen; cambian los textos
  esperados.

## Verificación

`make test`: **519 core + 34 funciones / 227 `CHECK` HAL + diag + 2
herramientas**. `make port-sdl`, `make cmake-native`, `make cmake-win` y
`make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `1cfd3a55ccca841c77b45cfb4d42b18e47ea17a0bbb3511c2c3a14a65ecc9321`
(11 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual/funcional del ZIP en Windows.
