# 0067 — T-114: arranque en Estirar y configuración F persistente

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Cambios

- La escala inicial por defecto es **Estirar a pantalla** (`scale = 7` en
  `kult.ini`, o si la clave no existe). `KULT_SCALE` sigue pudiendo forzarla.
- `kult.ini` guarda el estado de las teclas F:
  - `scale`: 0 Auto, 1..6 fijo, 7 Estirar a pantalla;
  - `fullscreen`: F1, 0/1;
  - `volume`: F9/F11, 0..100;
  - `muted`: F10, 0/1;
  - `save_slot`: F5, 0..9 (ya existía).
- Al arrancar se aplican volumen y silencio antes de abrir el dispositivo SDL.
- `platform/config.c` centraliza lectura/escritura de `kult.ini` y conserva
  claves desconocidas; `platform/slots.c` deja de tener su parser privado.
- `platform/video_sdl.c` escribe la configuración al cambiar F1/F2/F9/F10/F11.
- `KULT_SCALE`/`KULT_FULLSCREEN` siguen siendo overrides de arranque para
  desarrollo.

## Verificación

`make test`: **519 core + 36 funciones / 255 `CHECK` HAL + diag + 2
herramientas**. `test_config` cubre defaults, reemplazo de claves, conservación
de claves desconocidas y los valores de escala/pantalla/volumen/ranura.
`test_audio_volume` conserva el volumen recordado al silenciar.

`make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `07b91456fab0d5b1019d51e833f0b21655d9233dbcc210118cf7b2f3cf81dd92`
(12 entradas, `saves/` vacía, `Kult Settings.png` junto al `.exe`).

## Pendiente

T-094: validación visual/funcional del ZIP en Windows, incluyendo arranque
estirado y persistencia tras reiniciar.
