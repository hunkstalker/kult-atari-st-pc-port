# Nota 0007 — M3/T-031+T-032: entrada real y ritmo

- **Sesión 7.** Se completa M3: el juego se ve, se controla y va al ritmo del
  original.

## T-031 — entrada real
- `platform/input.c`: `platform_mouse_set(x,y,buttons)` y `platform_input_set_auto`.
  - IKBD: con entrada real el paquete lleva el ratón tal cual (cursor + botón
    izquierdo); en headless sigue el clic alterno.
  - `Cnecin`: con `auto=0` **bloquea** bombeando `platform_set_idle` (el
    callback presenta y sale al cerrar la ventana); sin idle, cae a la auto-tecla
    para no colgar en tests.
- `platform/video_sdl.c`: `platform_sdl_pump` traduce eventos SDL —
  teclado → `platform_key_push` (ESC = salir); ratón → coordenadas de juego vía
  `SDL_RenderCoordinatesFromWindow` (respeta el `INTEGER_SCALE` lógico) →
  `platform_mouse_set`.
- `app/main.c`: en ventana registra `present_cb`/`idle_cb`; `KULT_AUTOINPUT=1`
  fuerza el modo auto (para pruebas sin display).

## T-032 — ritmo
- `platform/time.c`: `platform_set_realtime(on)`. Con él, **cada VBL consume
  20 ms** de reloj monotónico (`clock_gettime`+`nanosleep`), con resincronización
  si se va >0.5 s por detrás. Así el bucle (1 paso por `wait_frame`) corre a
  50 Hz como el ST, sin depender del vsync. `KULT_REALTIME=0` lo apaga (headless).
- Medido: 333 VBL → 6.7 s (≈ 20 ms/VBL).

## Verificación
- `make test-port`: entrada real (ratón→IKBD, `Cnecin` consume la tecla) y el
  resto de la HAL.
- `make port-sdl` + `SDL_VIDEODRIVER=dummy`: 60 frames rápidos
  (`KULT_REALTIME=0`) y 100 frames a 50 Hz (6.7 s). Sin warnings.

## Pendiente / siguientes
- **T-020**: validar `_start` (menú de idioma fiel, hoy con `KULT_SKIP_DRAP` o
  auto-tecla) — el menú ya espera tecla real en modo ventana.
- **M4**: audio (MFP Timer A + PSG/YM2149 + salida SDL).
