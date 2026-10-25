# 0055 — T-102: arranque fullscreen, F1 y F2

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-103 (slots).

## Alcance

La build SDL arranca en **fullscreen sin bordes a la resolución nativa** del
monitor y mantiene el juego a 320×200 con escalado entero y nearest-neighbour.

- **F1**: alterna fullscreen ↔ ventana. Al volver a ventana se usa la última
  escala fija (×3 por defecto) y se centra.
- **F2**: cicla **Auto → ×1 → ×2 → ×3 → ×4 → ×5 → ×6 → Auto**.
  - En ventana, una escala fija redimensiona la ventana; Auto mantiene el
    tamaño actual y ajusta el viewport.
  - En fullscreen, la escala fija deja bandas negras centradas; Auto usa el
    mayor entero que quepa.
- `KULT_SCALE` sigue existiendo: `0`/ausente = Auto, `1..6` = escala fija.
  `KULT_FULLSCREEN=0` permite arrancar en ventana para desarrollo/pruebas.

## Implementación

- `platform/display.c`/`.h`: cálculo puro de escala y viewport, sin SDL.
- `platform/osd.c`/`.h`: aviso breve con caducidad por VBL (125 frames = 2,5 s
  a 50 Hz). T-102 lo usa para F1/F2; T-103/T-104 lo reutilizan.
- `platform/video_sdl.c`:
  - se elimina `SDL_LOGICAL_PRESENTATION_INTEGER_SCALE` y se pinta la textura
    en un `SDL_FRect` centrado;
  - el ratón se convierte con `SDL_RenderCoordinatesFromWindow` y se corrige
    con el viewport y la escala;
  - F1/F2 no se envían a la cola GEMDOS;
  - los eventos de resize/display/fullscreen recalculan el viewport;
  - OSD discreto; la versión inicial de T-102 usó `SDL_RenderDebugText` (fuente
  8×8 de SDL), sustituido en T-106 por los glifos originales de `A_FONT_DATA`.
- `app/main.c`: `platform_sdl_open(title, scale_mode, fullscreen)`.

## Verificación

- `make test-port`: nuevas pruebas de viewport (1080p, 1280×800, 1366×768,
  salidas pequeñas, escala fija recortada, ciclo de F2) y OSD.
- `make port`, `make port-sdl`, `make test` y builds CMake en verde.
- Validación manual pendiente de la release: F1/F2 en ventana y fullscreen,
  ratón en los bordes/bandas y OSD.

## Limitaciones

- El cambio de fullscreen es asíncrono en algunos sistemas; el viewport se
  recalcula en cada frame y en los eventos de ventana.
- Auto en ventana no cambia el tamaño de la ventana: solo ajusta la imagen.
