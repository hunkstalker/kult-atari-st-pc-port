# 0034 — Selección de acción en el anillo: destellos del icono

> **Resuelto y validado por el usuario.** El cambio es solo de timing en
> `src/commands.c` (`icon_ring`); no cambia lógica ni imagen.

## 1. Síntoma

Al seleccionar una acción en el anillo, el icono elegido apenas se ve: el anillo
pasa al comando siguiente casi en el VBL de la selección. Debería mostrar tres
destellos separados por retardos del original.

## 2. Causa

En el ASM de `0x1566a` (`icon_ring`), tras el clic y el sonido `0x13`, hay tres
blits del icono seleccionado **separados por tres `dbra d0` con `d0=0xffff`**:

```asm
bsr.w f_011020        ; icono 0x18
move.w #$ffff,d0
l_015840:
dbra d0,l_015840

bsr.w f_011020        ; icono 0x02
move.w #$ffff,d0
l_015858:
dbra d0,l_015858

bsr.w f_011020        ; icono 0x19
move.w #$ffff,d0
l_01586c:
dbra d0,l_01586c
```

El C tenía los tres `text_blit_ovl_b`, pero **no los tres retardos**. Cada uno
son 65.536 iteraciones (~82 ms a 8 MHz), así que el cierre del anillo quedaba en
~1 VBL en lugar de ~12-15 VBL.

## 3. Fix

En `src/commands.c`, dentro del cierre de `icon_ring` (solo cuando hay una
acción válida, `the_command != 0xffff`), tras cada blit:

```c
text_blit_ovl_b(img, x, yy, 0x18);
cpu_delay(0x10000u);   /* dbra d0 con d0=0xffff: 65536 iteraciones */
text_blit_ovl_b(img, x, yy, 0x02);
cpu_delay(0x10000u);
text_blit_ovl_b(img, x, yy, 0x19);
cpu_delay(0x10000u);
```

`cpu_delay` es no-op en el diferencial y cobra tiempo real en el port. **No se
toca** el cuadro de texto posterior, el anillo en sí, `15da6`, `15ac8` ni el
input.

## 4. Auditoría de bucles `dbra 0xffff` de espera

Se revisaron todos los `move.w #$ffff,d0` del ASM que forman un bucle `dbra`
(no cuentan los que son solo un valor de retorno):

| función | bucles | estado |
|---|---|---|
| `portrait_intro` (`0x16a20`) | 2 | corregidos en T-085 (`notes/0033`) |
| `icon_ring` (`0x1566a`) | 3 | corregidos aquí |
| `palette_cycle` (`0x16310`) | 2 | **no se tocan**: documentados como bucles de espera sin efecto observable; requerirían su propia validación de fundidos |

No aparecen más bucles de espera `dbra 0xffff` en el ASM. No se han añadido
retardos a ciegas ni a otras animaciones.

## 5. Pruebas

- `test_fun_16a20` y el resto del diferencial: **513** verdes (la imagen no
  cambia; el diferencial no mide tiempo).
- `test-port`: OK; incluye la guardia de `cpu_delay(0x10000u)` (>=4 VBL).
- **Validado por el usuario en Windows**: al seleccionar acciones se ven los
  tres destellos del icono elegido; el cierre del anillo pasa de ~1 VBL a
  **~12-15 VBL**.

## 6. Ejecutables

- `kult.exe` `4fc2cfa2fd148bdd4169848eaa1eadca`
- `kult-headless.exe` `8c222affc847beb67650aab8210b3e13`
- `kult-diag.exe` `6628f205fb6f0bfbbf55cf8691ee8be7`
