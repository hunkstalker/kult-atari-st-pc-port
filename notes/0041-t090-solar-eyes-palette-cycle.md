# 0041 — T-090: SOLAR EYES y el `dbra 0xffff` de `palette_cycle`

> **Resuelto en código y medido; pendiente de validación visual del usuario en
> Windows.** El fix es solo de timing en `src/video.c` (`palette_cycle`); no
> cambia la lógica ni la imagen.

## 1. Síntoma

La habilidad **SOLAR EYES** (poder PSI, fuera de combate) produce un flash que
en el port dura ~0,24 s y en el original ~1 s de CPU (más el ajuste al
siguiente VBL). Se percibía acelerado.

## 2. Localización

El replay nuevo y genérico (`assets/kult-replay-generic.bat`) desde un save
fuera de combate muestra, en el anillo/overlay:

```text
icono PODERES PSI -> 0xa002 (overlay_menu, f_014e66)
icono SOLAR EYES  -> 0xa00a (f_014f5e, índice 9)
```

En ASM:

- `cmd_jump_table[9]` = `f_014f5e` (`matching/src/data_009eaa.s`).
- `f_014f5e` resta 2 a `$1ad20` (energía PSI), ejecuta `f_016310` y fija
  `A_CUR_CLASS=0xff` (`auto_014f5e.s`).
- `f_016310` (`palette_cycle`) hace **por pasada**:
  `wait_vbl` + `dbra d0` con `d0=0xffff` + `set_palette`
  (`auto_016310.s:44-47` y `107-110`).

El C de `src/video.c` tenía los `wait_vbl` y `set_palette`, pero omitía los dos
`dbra` ("sin efecto observable").

## 3. Evidencia capturada

Replay:

```text
build-win/kult-0.1.0-win64/trazas-t090-20261023/
  kult-replay-solar-eyes.bin  (102.624 B)
  SHA256 be8bc2cd5cfd7b8a23e6e1e6592d02d0ea464872403e035c43bd409aa7b4ae58
```

Estado en la entrada de `palette_cycle` (gdb, `A_PALETTE_PTR=0x8ffe0`):

```text
0000 0112 0223 0232 0544 0334 0000 0702
0443 0325 0400 0530 0734 0754 0546 0777
```

Target `A_PAL_TABLE[$1acf4=0]`:

```text
0112 0223 0232 0544 0334
```

Eso exige **12 pasadas**: 6 hacia `0x777` y 6 de vuelta al target. La energía
PSI baja `0x3f -> 0x3d` en todas las capturas.

Medición del original con el oráculo (mismo estado, VBLs programadas):

```text
ciclos        = 7.879.396  (984,9 ms a 8 MHz)
instrucciones = 788.116
esperas VBL   = 12
stores sched  = 72 (12 x 6)
```

Port:

| momento | entrada `0xa00a` | fin `run_boundary_out` | duración |
|---|---:|---:|---:|
| antes del fix | VBL 4983 | VBL 4995 | 12 VBL (240 ms) |
| después del fix | VBL 4983 | VBL 5043 | 60 VBL (1200 ms) |

La entrada es el mismo frame en las dos capturas: mismo save, replay, clase
(`01aa9a`) y PSI. La única diferencia es el timing. Con el fix, `set_palette`
se ejecuta cada 5 VBL (12 pasadas): frames 4976, 4981, ..., 5031.

El oráculo mide 984,9 ms de ciclos; el port añade el ajuste al siguiente VBL
(~1,2 s). No se añade ningún retardo "a ojo": el `dbra` es el del ASM.

## 4. Fix

Solo `src/video.c` (`palette_cycle`), en las dos fases:

```c
wait_vbl(img);
cpu_delay(0x10000u);   /* dbra d0 con d0=0xffff, como 0x16396/0x16454 */
set_palette(img);
```

`cpu_delay(0x10000)` son 65.536 unidades (~655.360 ciclos a 10 ciclos/unidad):
el mismo coste que el `dbra` del original. En el diferencial es no-op; en el
port cobra tiempo real y cede VBLs. No se toca ningún otro comando ni se usan
gates por animación.

## 5. Pruebas

- Nueva `test_palette_cycle_solar_eyes_timing` (`test/test_video.py`): fija la
  paleta capturada, exige 12 esperas y `7.800.000 <= ciclos <= 7.950.000`.
- `make test`: **519 passed**.
- `make test-port`: OK (incluye la guardia de `cpu_delay(0x10000)`).
- Builds nativo, cross Windows y ZIP actualizados.

## 6. Artefactos

`build-win/kult-0.1.0-win64/trazas-t090-20261023/` contiene replay, save,
logs antes/después (`kult.log`, marcas, ring, input), vídeos, evidencia gdb
(`PAL_ENTRY`/`SETPAL`, paleta y target), medición del oráculo y `SHA256SUMS`.
Dos reproducciones (`determinism/play1`, `play2`) llegan al mismo comando y a
la misma bajada de PSI.

## 7. Nota sobre la duración esperada

El usuario estimaba ~2 s. La medición del original da 984,9 ms de ciclos para
esta paleta; el port queda en ~1,2 s por el ajuste de VBL. No se alarga a 2 s:
la fuente de verdad es el `dbra` del ASM. Si la percepción de 2 s viniera de
otra paleta/escena, se mide esa escena con el mismo método (oráculo + replay),
no se parchea esta animación.

## 8. Pausa entre fade-in y fade-out (meseta blanca)

Comprobado contra el ASM (matching byte a byte con el original):

- La fase 1, cuando todos los colores ya son `0x777`, detecta `d5 == 0` y salta
  a `l_0163a0` **sin** `wait_vbl`, `dbra` ni `set_palette`
  (`auto_016310.s:42-49`).
- En `l_0163a0` solo hay `sound_play_a`, el despacho de `A_DAT_1ABFA` (0 en
  SOLAR EYES), la copia del target y la entrada en fase 2
  (`auto_016310.s:50-64`). Ninguna de esas rutinas espera un VBL:
  `f_018f8e` inicia el sample y vuelve; `f_0103ae` con comando 0 cae en
  `f_010768`, que es un `rts`.
- La primera pasada de fase 2 ya hace `wait_vbl` + `dbra 0xffff` antes de su
  `set_palette` (`auto_016310.s:65-112`).

Por tanto la meseta blanca es **exactamente una pasada**, no una espera
adicional: el blanco se aplica al final de la última pasada de fase 1 y el
primer paso de vuelta se aplica tras el `wait_vbl + dbra` de la primera pasada
de fase 2. Duración esperada: ~82 ms de `dbra` + hasta ~20 ms de VBL, es decir
~82-102 ms.

En el port:
- gdb: último `set_palette` de fase 1 en VBL 5001 (blanco) y primero de fase 2
  en VBL 5006 → **5 VBL = 100 ms**, dentro del rango del ASM.
- Vídeo a 50 fps: pico blanco de **5 frames** (17,22-17,30 s) después del fix;
  antes del fix era **1 frame** (20 ms), porque faltaba el `dbra`.
- No hay ninguna pausa añadida por el port; la meseta que se ve es la del
  original. No se cambia nada por esto.

## 9. Hueco de audio en la meseta (Hatari) y relacion con T-073

El usuario grabo el audio de Hatari (`hatari.wav`, 22.050 Hz estereo):

- audio activo 1,830-3,481 s (1,651 s);
- desde 2,600 s hay un desvanecimiento progresivo;
- **2,6954-2,7164 s: 21 ms de casi silencio** (RMS ~15-20);
- en 2,715 s entra de golpe la segunda parte.

Eso es el hueco que el usuario percibia. El ASM no tiene un `wait` que lo
programe; el propio motor de samples lo produce:

- `sound_play_b` (`0x18f98` -> `f_01906a`) arranca el sample 0x17 y lo reproduce
  **hacia atras** (`$134 = 0x190f6`, puntero al final);
- `sound_play_a` (`0x18f8e` -> `f_018fc4`) lo **reinicia hacia delante** en la
  meseta (`$134 = 0x1910e`, puntero al inicio);
- el sample 0x17 tiene flag de loop 0 (`s_0192ce = 0`), asi que si la
  reproduccion backward llega al inicio antes de la transicion, el ISR para el
  Timer A y queda silencio hasta `sound_play_a`.

No es un fallo de emulacion de Hatari ni un `wait` oculto: es un artefacto de
timing del motor de audio del original. Si la fase 1 es lo bastante larga, el
sample termina y aparece el hueco; si el port va mas rapido, `sound_play_a`
interrumpe el sample antes y el hueco no aparece.

Comprobacion en el port con el mismo replay:

| build | fase visual | audio |
|---|---:|---|
| 8 MHz (defecto) | 60 VBL (1,2 s) | continuo 100,48-101,92 s, **sin hueco** |
| 5 MHz (`KULT_CPU_HZ=5000000`) | 84 VBL (1,68 s) | subida 106,75-107,52 s, **dos silencios de 30 y 20 ms** en 107,53/107,58 y segunda parte |

El 5 MHz reproduce la duracion de Hatari (1,65 s) y el hueco; el 8 MHz es
demasiado rapido y corta el sample antes de que termine. Por tanto, el hueco y
la duracion corta son la misma causa: **falta calibrar el reloj/CPU efectivo
(T-073)**, no un parche de `palette_cycle` ni de `sound.c`.

Recomendacion: no anadir un hueco artificial. Si se quiere fidelidad total al
Atari, seguir T-073 (medir CPU/ISR reales y calibrar `KULT_CPU_HZ`/
`KULT_ST_CONTEND`); si se prefiere evitar el artefacto, dejar el port a 8 MHz
con el audio continuo y documentar la diferencia.

Artefactos de audio anadidos al archivo de trazas:
`audio/hatari.wav`, `audio/port-8mhz-flash.wav`, `audio/port-5mhz-flash.wav`.

## 10. Estado

**T-090 cerrado (2026-10-23).** El fix visual del `dbra` esta aplicado y medido.
El hueco de audio de Hatari queda documentado como artefacto de timing del
original y **fuera del alcance de T-090**: no es una divergencia ASM<->C. La
calibracion del presupuesto efectivo de CPU/ritmo (y, si se quisiera, el hueco)
es **T-073, feature opcional no bloqueante**, no una tarea pendiente del core.
La validacion visual global de la partida sigue en **T-061**. **Sin commit.**
