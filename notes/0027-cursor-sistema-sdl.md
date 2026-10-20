# 0027 — Cursor del sistema (SDL): oculto; solo el cursor del juego

> En la ventana se veía el cursor de Windows encima del cursor de 16×16 que
> dibuja el juego. El cursor del sistema lo muestra SDL por defecto; hay que
> ocultarlo explícitamente.

## Fix (solo SDL)

En `platform/video_sdl.c`:

- `SDL_HideCursor()` al abrir la ventana.
- Al recuperar el foco (`SDL_EVENT_WINDOW_FOCUS_GAINED`), volver a ocultarlo (hay
  plataformas que lo muestran al recuperar foco).
- Al perder el foco (`SDL_EVENT_WINDOW_FOCUS_LOST`), `SDL_ShowCursor()` para que
  el usuario pueda usar el escritorio, y limpiar los botones.
- Al cerrar (`platform_sdl_close`), `SDL_ShowCursor()` para restaurar el cursor
  del sistema.

El cursor visible dentro del juego sigue siendo el sprite de `src/cursor.c`;
la HAL solo oculta el cursor del SO. No se toca `src/`.

## Verificación

- `make port-sdl`: OK.
- `make package-win`: OK (exe y ZIP).
- `make test` / `make test-port`: sin cambios (el código SDL no entra en ellos).
- Artefactos:
  - `kult.exe` md5 `3237a4d7cc6d1a82423a2b95e017f814`
  - `kult-headless.exe` md5 `68ae05eaddcb6c33f4b66da437576aa3`
  - ZIP md5 `ad7d30dded0c1afe2fee2d4417fb5258`

Validación visual: con la ventana activa no debe verse el cursor de Windows;
solo el cursor del juego. Al salir de la ventana/cerrar, el cursor del sistema
vuelve a mostrarse.
