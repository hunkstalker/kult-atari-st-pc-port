# 0025 — Estrella final: la espera es la carga de FOND.PI1

> Punto 2 de la validación. La animación de la estrella ya tenía la duración
> correcta (3 elementos, `15da6` delay 20 = 5 VBL cada uno = 15 VBL), pero el
> usuario detectó que en el original la estrella **se queda en pantalla unos
> segundos** antes del reinicio. En el port desaparecía porque la carga de
> `FOND.PI1` es instantánea. La carga inicial (PRES/DRAP y, tras el idioma,
> listas A/B, FOND y ECHE) tiene una excepción: no espera, para no perder el
> inicio de la canción ni meter ~20 s de pausa; ver
> `notes/0026-esperas-de-disco.md`.

## Evidencia ASM

Tras la animación (`159e2`), `prepare_room` (`0x1842c`) hace:

```asm
018498: bsr.w  $159e2      ; estrella
0184a4: bsr.w  $13568      ; load_fond: FOND.PI1
...
0184bc: bsr.w  $135da      ; set_palette
0184c0: bsr.w  $181f0      ; room_reset
0184c4: bsr.w  $121c6      ; copy_screen
0184c8: bra.w  $1682a      ; reset_clear -> reinicio
```

No hay un `wait_vbl`/`dbf` programado entre la estrella y el reinicio. La espera
es el **Fread de `FOND.PI1`**: `load_named_screen` carga `0x7d22` = 32.034 B
(`0x13568` → `0x1076a`). En el ST el Fread bloquea la CPU y la estrella sigue
en pantalla; el port leía del disco del PC al instante y el `room_reset`
posterior pintaba la sala encima.

## Estimación del tiempo de carga

Disquete ST DD: 9 sectores × 512 B = 4608 B/pista; 300 rpm = 5 rev/s →
**1,40 s** para 32.034 B (7 pistas). Añadiendo búsqueda/encendido del motor,
el usuario estima **> 1,4 s**; se modela **~2,0 s** (62,5 µs/byte).

## Fix (solo HAL)

- `platform_disk_delay(bytes)` en `platform/time.c`: duerme `bytes × 62,5 µs`
  cediendo VBLs y presentando (como el bloqueo de Fread con la VBL viva).
  No depende de `KULT_CPU_HZ` (no es CPU, es E/S).
- `pc_os_fread()` (`platform/fs.c`) lo llama tras leer, con el número real de
  bytes. En headless/diferencial es no-op (`g_rt == 0`), así que `make test`
  y `test-port` no cambian de ritmo.
- `src/` intacto.

## Verificación

- `make test`: **499**.
- `make test-port`: OK, con nueva `test_disk_delay` (no-op sin tiempo real;
  800 B ≈ 50 ms → ≥ 2 VBL).
- `make port`, `make cmake-win`, `make package-win`: OK.
- Artefactos:
  - `kult.exe` md5 `ed17588978ec5560e2024f1fa995624b`
  - `kult-headless.exe` md5 `68ae05eaddcb6c33f4b66da437576aa3`
  - ZIP md5 `c27a193581bec5e613af547d76b7c209`

Validación visual pendiente: al morir, tras la estrella debe verse ésta ~2 s
mientras carga el fondo, y después el reinicio. Si dura demasiado/poco, el
único ajuste es el factor de `platform_disk_delay` (estimación, no medición
exacta del disquete).

## Nota sobre paleta

El intento de `palette_cycle` se revirtió: los F12 4516/4640/4767 están
dominados por `10fee/10ff4/12fb6` (`masked_word4`/revelado), no por
`palette_cycle`; era un cambio correcto de otro ritmo, pero no el fallo
reportado.
