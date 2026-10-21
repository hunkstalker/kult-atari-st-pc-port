# 0033 — T-085: retrato/intro acelerado (`0x16a20`, comando `a013`)

> **Resuelto y validado por el usuario.** `src/` cambia solo en el timing de
> `portrait_intro`; la lógica y la imagen no cambian.

## 1. Síntoma

Una animación de retrato/intro terminaba en menos de 1 s cuando debía durar más
de 2 s. No era el motor de elementos (`15da6`) ni los objetos animados de sala
(`15ac8`): esos tramos ya tenían la cadencia correcta.

## 2. Causa

La animación es `portrait_intro` (`0x16a20`, comando `a013`). El ASM repite 11
veces un bloque con **dos bucles `dbra` de `0xffff`**:

```asm
move.w #$ffff, d0
l_016a76:
dbra   d0, l_016a76
...
move.w #$ffff, d0
l_016a90:
dbra   d0, l_016a90
```

Cada bucle son **65.536 iteraciones**. El C los había transcrito así:

```c
uint32_t spin = 0xffff;
while (spin != 0xffff)      /* condición falsa desde el inicio */
    spin = (spin - 1) & 0xffff;
```

Nunca se ejecutaba el retardo: se perdían 2 × 65.536 × 11 iteraciones
(~14,4 M ciclos ≈ 1,8 s). Por eso la animación quedaba en ~0,44 s.

El diferencial no podía detectarlo: compara imagen/estado y `cpu_delay` es
no-op en `make test`. Es un fallo **solo de tiempo**, como los `dbf` ya
transcritos en `anim.c`/`reveal.c`.

## 3. Fix

En `src/loop.c`, dentro de `portrait_intro`, los dos bloques `spin` se
sustituyen por la puerta de timing existente:

```c
/* dbra d0 con d0=0xffff: 65536 iteraciones (no-op en diferencial). */
cpu_delay(0x10000u);
```

- En el ejecutable cobra ~655.360 ciclos por bucle y cede VBLs/presenta.
- En el diferencial es no-op.
- **No se toca** `15da6`, `15ac8`, input ni ninguna otra animación.

## 4. Alcance

El cambio afecta **solo** a `portrait_intro` (`0x16a20`). No hay otras llamadas
a esos dos bucles ni se modifica ninguna otra función. La animación que ya
funcionaba no se toca.

**Audio:** el audio no se tocó y ya era correcto. El desfase percibido era
consecuencia de que la animación se acortaba; al restaurar los `dbra`, el audio
existente queda sincronizado. No hay ningún cambio pendiente en el sistema de
audio.

## 5. Pruebas y validación

- `test_fun_16a20` (diferencial) sigue verde: la imagen/estado no cambia.
- Nueva guardia en `platform/test/test_platform.c`: `cpu_delay(0x10000u)` debe
  ceder **>= 4 VBLs** (~82 ms a 8 MHz), que es lo que cuesta cada `dbra`.
- Medición funcional del escenario de `portrait_intro` con el reloj real:
  **110 VBL / 2202 ms** (antes se quedaba en ~22 VBL).
- `make test`: **513**; `make test-port`: OK.
- **Validado por el usuario en Windows**: la animación ya dura lo esperado y no
  se han observado regresiones en otras animaciones.

## 6. Artefactos

- `kult.exe` `5342b77ca9a1ece4d7a495439634366b`
- `kult-headless.exe` `ccf5f7a92d2087e2d9631ae6e9c1c8c3`
- `kult-diag.exe` `6271016265dbcaf3d8edff6f3824311f`

## 7. Referencias

- ASM: `0x16a20` (`portrait_intro`), dos `dbra d0` con `d0=0xffff` por
  iteración.
- Core: `src/loop.c` (`portrait_intro`).
- Timing: `include/kult_port.h` / `platform/include/kult_port.h`
  (`cpu_delay`), `platform/time.c` (`platform_cpu_delay`).
- Prueba funcional: `platform/test/test_platform.c` (`test_cpu_delay`).
