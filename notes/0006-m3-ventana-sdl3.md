# Nota 0006 — M3/T-030: primera ventana SDL3

- **Sesión 6.** Se añade la ventana con SDL3 (ADR 0001/0002: `SDL_Renderer` como
  base) y el ejecutable `make port-sdl` → `build/kult-sdl`.

## Qué se hizo
- `platform/video.c`: `platform_fb_to_rgba()` — conversión planar→RGBA8888
  **compartida** con el volcado PPM (misma lectura de planos y paleta, así el
  render y el golden no divergen).
- `platform/video_sdl.c` (sólo en `port-sdl`): `SDL_CreateWindow` (320×200×escala,
  redimensionable), `SDL_CreateRenderer`, textura `RGBA32` con
  `SDL_SCALEMODE_NEAREST`, `SDL_SetRenderLogicalPresentation(INTEGER_SCALE)` y
  `SDL_SetRenderVSync(1)`. `platform_sdl_pump()` cierra con la X o ESC.
- Enlace con el motor: `kult_set_frame_cb()` (el hook del bucle llama al
  callback cada iteración) y `kult_request_stop()` (lo pide el presentador al
  cerrar). `app/main.c` abre la ventana si `KULT_WINDOW=1` (0 frames = sin
  límite) y presenta cada frame.
- `Makefile`: `port` (headless) excluye `platform/video_sdl.c`; `port-sdl` lo
  añade con `-DHAVE_SDL3` y `pkg-config sdl3`.

## Verificación
- `make port-sdl` compila sin warnings; con `SDL_VIDEODRIVER=dummy
  SDL_RENDER_DRIVER=software KULT_WINDOW=1 KULT_FRAMES=120` corre y vuelca el PPM.
- `make test-port` añade comprobación de `platform_fb_to_rgba` (píxeles
  conocidos) además del PPM.

## Pendiente (M3)
- **T-031**: entrada real (teclado → `os_crawio/cnecin`; ratón → IKBD /
  `A_CURSOR_X/Y`); hoy sigue el auto-tecla/clic headless.
- **T-032**: separar simulación (cadencia fija) de presentación (vsync ya limita,
  pero el bucle avanza un paso por iteración).
