# 0094 — Plan T-138: cursor a refresco nativo (opción B)

Fecha: 2026-10-25. Estado: **planificado**, sin implementar.

## Decisiones

- Se elige la **opción B**: overlay propio con el sprite del juego.
- Se mejora **solo el cursor**; las animaciones y el OSD siguen a 50 Hz. No se
  reescala ni se interpola el motor.
- El cursor se integra **por defecto**, sin `KULT_*` de usuario.
- El cursor debe aparecer en capturas PPM/planar; se compone en una copia al
  volcar.
- El savestate guarda el **estado** del cursor (posición, sprite, clip y
  hotspot) dentro de la imagen, pero no hornea sus píxeles. Al cargar se
  reconstruye el overlay, igual que la pantalla actual.
- La presentación a más Hz es **oportunista** mientras el hilo del juego espera
  un VBL. En tramos CPU-bound el cursor puede bajar a 50 Hz o menos; no se
  rompe la lógica.
- **T-139** (separar render y simulación en hilos) queda **descartada**: T-138
  se validó en Windows y no se necesita. Solo se reevaluaría si apareciera
  stutter; SDL/window en macOS, audio, input y el estado del core no son
  thread-safe sin un diseño y QA específicos.

## Arquitectura prevista

- `platform_tick()` sigue avanzando la simulación a 50 Hz.
- Se añade un pump de presentación durante las esperas de VBL para volver a
  presentar la misma imagen y mover el overlay del cursor.
- El cursor software del core no se pinta en el framebuffer del port; se
  conserva la API `cursor_draw`/`cursor_restore` y el estado en la imagen.
- El overlay usa `A_CURSOR_X/Y`, `A_CLIP_X/Y`, `A_CURSOR_SPRITE_PTR` y
  `A_CURSOR_HOTSPOT`, con la paleta actual.
- Orden de capas: framebuffer → cursor → OSD, como hasta ahora (el OSD sigue
  siendo la capa superior).
- No se usa una librería externa: SDL3, ya presente.

## Fases

1. Auditoría de `cursor_draw`, `cursor_restore`, `wait_frame_cursor`, slots,
   capturas y savestates.
2. Overlay a 50 Hz y supresión del cursor software; pruebas de no duplicado.
3. Composición del cursor en capturas.
4. Presentación oportunista a más Hz durante esperas; medición en Windows,
   Linux y macOS.
5. QA: clics, anillo, bordes 304×184, OSD, fullscreen, escala, guardado/carga y
   capturas.

## Rama

```text
git switch -c feature/cursor-refresh
```

Base: `ffcbebe` (commit probado en CI para 1.1.0). No se toca `main` ni el tag
`v1.1.0` hasta publicar la release.
