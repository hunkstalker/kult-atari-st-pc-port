# 0029 — Audio de puertas (T-082): el sample del tope se cortaba

> **Validado por el usuario.** El sonido de puertas ya se oye entero: apertura,
> tope y cierre, y la sala no cambia hasta que termina el sample del tope.
> Diagnóstico con la run de marcas del 2026-10-20 (`# RUN` en la línea 88310;
> F12 1341).

## Evidencia del log

Secuencia al cruzar una puerta:

- **VBL 1192**: empieza el sonido de apertura desde `textform_fwd` (sample
  `0x1d`, `18f8e`/`18fc4`). La animación/transición dura hasta 1320.
- **VBL 1321**: `18f7a`/`18fc6` (`supexec_b(0x31, 0x1f)`) desde
  `textform_fwd`; es el sample del **tope**.
- **VBL 1322**: `18fa4`/`19084` (`sound_stop`, ASM `0x14704` de
  `body_146d2`) **un VBL después**; después `textform_rev` arranca los samples
  del cierre (`0x1d`/`0x1e`).

No falta el cableado: el sample del tope se inicia, pero `sound_stop` lo corta
antes de que el audio lo reproduzca.

## Causa

En `body_146d2`, antes de `sound_stop`, el original gasta trabajo de CPU
(hints, copia de pantalla, etc.). Medido con el oráculo en
`scene_mode_draw` (`a572=1`, `stop_pc=0x14704`): **~242.030 ciclos ≈ 1,5 VBL**.
El C llegaba a `sound_stop` en el mismo VBL, así que el Timer A nunca sacaba el
sample `0x1f`.

No es un problema de mezcla ni de PSG: es el mismo patrón de T-073 (trabajo de
CPU no anotado).

## Fix

El primer intento esperaba antes de `sound_stop`, pero para entonces
`copy_screen` ya había cambiado la sala: el juego mostraba la sala siguiente
mientras el sample seguía sonando.

El fix correcto espera **al final de `textform_fwd`**, dentro de
`scene_mode_apply`, antes de que la transición continúe:

```c
textform_fwd(img, script, img[A_SEL_1ACC9]);
uint32_t cur = be32(img + 0x192c6);
uint32_t end = be32(img + 0x192ca);
for (int i = 0; i < 50 && cur < end; i++) {
    wait_vbl(img);
    cur = be32(img + 0x192c6);
    end = be32(img + 0x192ca);
}
```

El puntero del sample (`0x192c6`) lo avanza el ISR del Timer A; `wait_vbl`
bombea el audio. Así la sala actual sigue visible hasta que termina el sample
del tope y solo después se pinta/carga la siguiente. Tope de 50 VBL para no
colgar si un sample repite. En diferencial es no-op.

## Verificación

- `make test`: **511**.
- `make test-port`: OK.
- `make package-win`: OK.
- Artefactos:
  - `kult.exe` md5 `bd3764b904a4c679b0caed6ea467e76e`
  - `kult-headless.exe` md5 `0fdda9c894a4fb1f5a5dad1b9d64dfef`
  - ZIP md5 `1396dd00eb054ca6d3995e1d7a398816`

**Validación visual:** confirmada por el usuario. La sala actual permanece
visible hasta que termina el sample del tope y después se pasa a la siguiente.
El audio general (música, voces y efectos) queda validado.

## ¿Es robusto?

Para este caso, sí: ataca el orden real del problema y no depende de una
duración adivinada. Lo que aún es mejorable es que `mode.c` lee directamente el
estado interno del reproductor (`0x192c6`/`0x192ca`). Si aparecen más secuencias
de audio, lo limpio sería exponer un `sound_wait_sample()` en `sound.c` y usarlo
desde la transición. No se añade otra capa ahora que está validado; si vuelve a
aparecer un caso parecido, una sesión nueva debe valorar si compensa
generalizarlo.

## Referencias

- Log: `build-win/kult-0.1.0-win64/kult-marcas.log` (run 88310).
- `src/mode.c` (`scene_mode_apply`/`body_146d2`), `src/textform.c`
  (`textform_fwd`/`textform_rev`).
- `notes/0015`, `notes/0019` (modelo de CPU).
