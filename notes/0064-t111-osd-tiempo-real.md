# 0064 — T-111: caducidad del OSD por tiempo real

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-112 (volumen).

## Bug

El OSD caducaba por frames VBL (`platform_frame_count()` + 125). Ese contador
solo avanza cuando corre `platform_tick()`. Si el juego está bloqueado en
`Cnecin` (título, menú de idioma, cuadros de texto o menús), `idle_cb` presenta y
bombea SDL pero no avanza VBLs: el aviso se quedaba indefinidamente.

## Cambio

- `platform/osd.c` usa `platform_now_ns()` (reloj monótono real).
- TTL por defecto: **2500 ms**.
- API interna: `platform_osd_show_ms(text, ms)`; `platform_osd_show()` usa 2,5 s.
- `platform_osd_active()` compara el deadline real, sin depender del juego.
- El dibujo sigue en `platform_sdl_present()`; si el juego está parado, el OSD
  desaparece en la siguiente presentación de `idle_cb`.

## Verificación

`make test`: **519 core + 34 funciones / 228 `CHECK` HAL + diag + 2
herramientas**. Nueva comprobación de caducidad tras 10 ms **sin avanzar VBLs**.

`make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `528de4eff78ecd7851a90a8d454db95310ed5274f7220d3e55290db82e19c33a`
(11 entradas, `saves/` vacía).

## Pendiente

T-112: volumen F9/F10/F11; después T-094 sobre el ZIP final.
