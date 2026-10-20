# 0021 — Cursor 16×16: máximo absoluto del IKBD (304×184)

> Bug detectado tras corregir el input/presentación: al llegar al borde derecho,
> el cursor aparecía por el lado contrario. **No se toca `src/`**: la HAL emula
> el comando IKBD que el propio juego envía.

## 1. Síntoma

- Al llegar al borde derecho de la ventana, el cursor reaparecía por la
  izquierda (y algo análogo en el borde inferior).
- El cursor del juego es un sprite de **16×16** (`cursor_draw`, `0x10526`).

## 2. Causa

El juego configura el ratón absoluto del IKBD en `crt0_init`:

```asm
; Ikbdws(0x19, 0x1a459, 4) -> 5 bytes (la longitud XBIOS es d0+1)
09 01 30 00 b8
```

- `0x09`: absolute mouse reporting.
- `0x0130` = **304** = máximo X.
- `0x00b8` = **184** = máximo Y.

Es decir, el IKBD original **nunca reporta x > 304 ni y > 184**, justo el área
para que el sprite de 16×16 quepa entero (320−16, 200−16).

El port ignoraba ese comando (el core lo documenta como no-op) y recortaba el
ratón a 319×199. Con `x > 304`, `cursor_draw` escribe fuera de la fila y el
sprite reaparece por el borde opuesto. Comprobado llamando a `cursor_draw` con
`x=319`: aparecen píxeles en `x=0..` y en la fila siguiente.

## 3. Fix (solo HAL)

- `platform/input.c`:
  - `g_mouse_max_x/y` (por defecto 319/199).
  - `platform_ikbdws(buf, len)`: si `buf[0] == 0x09` y `len >= 5`, lee
    `xmax`/`ymax` big-endian y los guarda.
  - `platform_mouse_set` recorta a esos máximos, no a 319/199.
  - Al cambiar el máximo, se recorta también la posición actual.
- `boot/boot.c`: tras `crt0_init(img)`:
  ```c
  platform_ikbdws(img + 0x1a459, 5);
  ```
  Es exactamente el `Ikbdws` que hace el juego. `src/` intacto.
- No es un clamp “a ojo”: el valor 304×184 sale del comando del juego. Si el
  juego cambiara el rango, la HAL se adapta.

## 4. Pruebas

`make test-port` (`test_ikbd_mouse_max`):

- `platform_ikbdws({09 01 30 00 b8}, 5)` fija 304×184.
- `platform_mouse_set(999, 999)` entrega `A_CURSOR_X/Y = 304/184`.
- `platform_mouse_set(-10, -10)` entrega `0/0`.
- Se restaura 319×199 al final para no afectar a otras pruebas.

`make test`: **499** (el core no cambia).

## 5. Verificación

- `make test`: 499.
- `make test-port`: OK.
- `make port`, `make port-sdl`, `make cmake-native`, `make cmake-win`,
  `make package-win`: OK.
- Smoke headless (incluido `KULT_CURSOR_X/Y=999`) y SDL dummy: sin cuelgues.
- Artefactos:
  - `kult.exe` md5 `b7edc93660ea7caadd87191f8ffeee7a`
  - `kult-headless.exe` md5 `7faac3d3351c24c46f825b4096557130`
  - ZIP md5 `97d567898127ab8290d4155113c312d1`

Validación visual pendiente: el cursor debe quedarse en el borde derecho/inferior
sin reaparecer por el lado contrario; izquierdo/teclado y derecho conservan el
comportamiento de T-074.

## 6. Referencias

- TOS `bios/chardev.S`: comando `0x09` = absolute mouse, 4 bytes (xmax/ymax).
- `src/video.c` (`crt0_init`): `Ikbdws(0x19, 0x1a459, 4)`.
- `src/cursor.c` (`cursor_draw`): sprite 16×16.
- `notes/0020-input-hal-y-presentacion.md`.
