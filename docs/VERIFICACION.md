# VERIFICACIÓN

## Core (heredado, no se toca)
- **499 pruebas diferenciales** contra el oráculo (Musashi): dan la misma entrada
  al original (emulado) y al C, y comparan memoria + registros + ledger.
- Son la **red de seguridad** de que el porte **no cambia la lógica**. Deben
  seguir verdes al importar el core (M0/T-002).
- Comandos: `make test`, `make test PYTEST_ARGS=-n0`, `make regscan` (`ids_reg=0`),
  `make core`.

## Backend (nuevo)
Pruebas propias de la HAL, sin oráculo:
- **Ficheros:** mapeo `A:…` → `data/`; lectura de recursos reales (fixtures).
- **Vídeo/render:** conversión **planar→RGBA** contra una imagen **golden**;
  paleta de 16 words; escalado entero (sin interpolación inesperada).
- **Entrada:** mapeo teclado/ratón → `os_crawio`/`os_cnecin` y paquete IKBD;
  `A_CURSOR_X/Y` se actualizan.
- **Tiempo/tick:** el contador de VBL avanza; las esperas retornan; el número de
  pasos por segundo es el esperado (simulación a cadencia fija).
- **Audio:** comparación del **stream de registros PSG** del reproductor de música
  (`0x3a872`) con el **oráculo** (ledger del kit, 23 eventos en una escena
  controlada), y **RMS/energía** de la emulación PSG/YM2149. Además: periodo/ISR
  del **Timer A** del MFP y el bombeo en modo tiempo real. Todo en `make test-port`
  (`test_music_player_golden`, `test_mfp_timer_a`, `test_audio_pump`,
  `test_audio_rms`, `test_integration_music`).

## Integración (hitos)
- **M1:** el arranque headless avanza N frames y el framebuffer es coherente.
- **M2:** las esperas no cuelgan (el tick avanza el contador).
- **M3:** primera ventana jugable y controlable, sin artefactos.
- **M4:** la intro/música suena (RMS/energía como comprobación básica).

## Regla
Toda corrección o feature nueva deja su prueba. Si algo es inobservable, se
documenta como *intencional* con su justificación (igual que en el core).
