# 0095 — T-138: cursor como overlay (merged a main)

Fecha: 2026-10-26. Estado: **implementado, merged y validado en
Windows/Linux/macOS; release 1.2.0 publicada (tag `v1.2.0`)**.

## Alcance

- El juego sigue simulando a 50 Hz.
- El cursor del juego se presenta como overlay, con el sprite y la paleta
  originales.
- No se reescala ni se interpolan las animaciones ni el OSD.
- El cursor está activo por defecto; no hay variable de usuario.
- Las capturas PPM/planar incluyen el cursor.
- El savestate conserva el estado del cursor, no sus píxeles; al cargar se
  reconstruye el overlay.

## Implementación

- `lib/kult-core/src/cursor.c` no se modifica: en el build del port sus funciones
  se renombran a `kult_core_cursor_draw/restore`.
- `platform/cursor_overlay.c` envuelve esas funciones, mantiene la visibilidad y
  compone una copia de la imagen con el cursor recolocado a `A_CURSOR_X/Y`.
- `platform/video.c` usa la copia compuesta para PPM/planar.
- `platform/video_sdl.c` compone el frame antes de subirlo a la textura; el OSD
  se sigue dibujando encima del cursor (comportamiento actual).
- `platform/time.c` añade `platform_set_render_pump()` y
  `sleep_with_render()`: durante las esperas de VBL se presenta la misma imagen
  a hasta ~120 Hz para que el cursor responda. La simulación no se acelera.
- `app/main.c` registra el pump solo con ventana SDL abierta.
- El overlay usa la posición nueva del ratón aunque el IKBD aún no la haya
  entregado; si el juego recoloca el cursor (ficha de objeto), respeta esa
  posición hasta que el usuario mueva el ratón.
- `KULT_PRESENT_LOG=1` registra la frecuencia real de presentación.

## Tests

- `make test-port`: OK.
- `build/test_cursor_overlay`: comprueba composición a la misma posición, a una
  posición nueva y con el cursor oculto.
- `make test-port` verifica que el pump se ejecuta durante `cpu_delay`.
- CMake Linux y Windows: build OK.
- pytest: 5 passed.
- Medición con Xvfb: 60 Hz de presentación (límite VSync del display de prueba);
  el código intenta hasta ~120 Hz. En un monitor de 60 Hz el máximo útil es 60;
  en 120/144 Hz puede subir hasta el límite del código.

## QA

- **Windows (2026-10-26)**: validado por el usuario; el cursor se comporta
  correctamente y sin duplicados.
- **Linux/macOS (2026-10-26)**: validado por el usuario; cursor correcto, sin
  duplicados.

## Seguimiento opcional (post-1.2.0)

- Medir carga de CPU/GPU con el pump a ~120 Hz.
- Revisar interacción con OSD, F1/F2, fullscreen y escalado Estirar.
- Decidir si el OSD debe quedar por encima o por debajo del cursor.

## Rama

`feature/cursor-refresh`, basada en `eee8744`; actualizada con main (`d54dc28`)
y mezclada en main (`dbca6d3`).
