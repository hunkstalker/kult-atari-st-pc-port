# 0065 — T-112: control de volumen F9/F10/F11

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Comportamiento

- **F9**: baja el volumen maestro 10% (mínimo 0).
- **F11**: sube el volumen maestro 10% (máximo 100).
- **F10**: silencia/restaura. Guarda el volumen anterior y lo repone al quitar
  el silencio.
- Si se pulsa F9/F11 estando silenciado, se desilencia y se aplica el paso desde
  el volumen guardado.
- OSD:
  - normal: `VOLUMEN N` (sin `%`, la fuente del juego no tiene ese glifo);
  - silenciado: `SILENCIO`.
- F11 deja de usarse como marca de “fin de animación”; F12 sigue siendo la marca
  manual de diagnóstico.

## Implementación

- `platform/audio.c`: volumen maestro independiente de `KULT_MUSIC_GAIN`.
  La señal final multiplica por `g_volume / 100`; afecta a música y efectos.
- Estado: `g_volume`, `g_volume_saved`, `g_muted`.
- API: `platform_audio_volume()`, `platform_audio_muted()`,
  `platform_audio_set_volume()`, `platform_audio_volume_step()` y
  `platform_audio_toggle_mute()`.
- `platform/video_sdl.c`: F9/F10/F11 interceptadas, sin `key.repeat`, con OSD.
- El volumen no se persiste entre sesiones: arranca al 100%.

## Verificación

`make test`: **519 core + 35 funciones / 240 `CHECK` HAL + diag + 2
herramientas**. `test_audio_volume` cubre pasos ±10, topes 0/100, mute y
restauración, desilencio al ajustar y escalado real de la señal a 50%/0%.

`make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `d4c125eea9ba6bdcc355f08bf379daf18503e3b12d833acaec67ac087aea2caa`
(11 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual/funcional del ZIP en Windows (incluido el volumen y el
OSD).
