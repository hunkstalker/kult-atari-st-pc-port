# 0059 — T-106: OSD con la fuente original del juego

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP nuevo.

## Motivo

El OSD de T-102/T-104 usaba `SDL_RenderDebugText`, la fuente bitmap 8×8 de
depuración de SDL. No era la fuente del juego. El proyecto Legacy dibujaba los
avisos con los glifos originales de la RAM (`0x1d202`, `CARAC.BIN`), 6 B por
glifo, 4×6 píxeles.

## Implementación

- `platform/osd.c`: nuevo `platform_osd_draw_rgba()` que pinta el aviso
  directamente en el buffer RGBA 320×200, después de `platform_fb_to_rgba()` y
  antes de `SDL_UpdateTexture`.
- Glifos leídos de `A_FONT_DATA` (`0x1d202`):
  - `0x10–0x19` dígitos;
  - `0x20` espacio;
  - `0x21–0x3A` A–Z.
- Texto en mayúsculas ASCII; minúsculas se convierten y los caracteres no
  soportados caen a espacio.
- Caja negra con texto blanco abajo-izquierda, 1:1 con los píxeles del juego;
  el viewport la escala igual que el resto de la imagen.
- Si el glifo `A` de la fuente aún no está cargado, el OSD no se dibuja.
- `platform/video_sdl.c`: eliminados `SDL_RenderDebugText` y el escalado SDL
  del OSD.

## Verificación

- `test_osd_font`: comprueba fondo, bits de los glifos, dígito, minúsculas,
  carácter no soportado y ausencia de fuente.
- `make test`: **519 core + 34 funciones / 218 `CHECK` HAL + diag + 2
  herramientas**.
- `make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win`
  OK.
- ZIP `build-win/kult-1.0.0-win64.zip`
  SHA256 `cf4d9f487b4edd7582745002326306be0b7a861c7023b55246520e54a591b975`
  (11 entradas).

## Pendiente

T-094: validación visual del OSD con la fuente del juego en Windows
(fullscreen/ventana y escalas), además de F5/F6/F7.
