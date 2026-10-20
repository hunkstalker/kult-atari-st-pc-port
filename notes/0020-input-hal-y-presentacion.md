# 0020 — Input (botones) y presentación por VBL: dos fallos de HAL

> Cierra el bug de puertas/A-B-C detectado durante la validación de T-061. **No
> se toca `src/`**: el core verificado se queda intacto. La solución replica la
> del motor legacy, que ya interceptaba `$1a570` en la HAL.

## 1. Síntoma

En el port:

- En A/B/C, el clic izquierdo y el derecho casi funcionaban, pero el derecho no
  saltaba hasta pulsar el izquierdo.
- En el loop del juego, el izquierdo estaba desactivado; el derecho saltaba las
  puertas.
- Al pulsar una tecla, la puerta **sonaba pero no se veía**; al terminar el
  audio/tiempo, se pasaba a la sala siguiente.

En el original (Hatari):

- **izquierdo / teclado**: acción normal, **sin** skip.
- **derecho**: acción normal **+** skip (A/B/C y puertas).
- La animación de puerta se ve con izquierdo/teclado; con derecho se salta.

## 2. Causa

### 2.1 Botones

El TOS documenta el paquete IKBD como `bit0 = botón derecho`, `bit1 = izquierdo`
(`vdi/mouse.S`). El core `wait_frame` (`src/input.c`, `0x13698`) hace:

```c
img[A_INPUT_BUTTONS] = img[A_IKBD_PACKET];
if (img[A_INPUT_BUTTONS] & 0x01) {   /* bit0 */
    img[A_INPUT_BUTTONS] |= 0x04;    /* acción */
    moved = 0xff;                    /* skip */
}
img[A_SKIP_TRANSITION] = moved;
```

Es decir, el core deriva **acción y skip del mismo bit0**. Nuestra HAL entregaba
el **izquierdo** como `bit0` y **ignoraba el derecho**: por eso el izquierdo
hacía acción + skip y el derecho no hacía nada. El modelo correcto del original
necesita separar acción (izquierdo/derecho/teclado) de skip (solo derecho).

El legacy ya lo resolvía en la HAL (`runtime/kult_engine.c`):

```c
ram[0x1a44f] = g_mouse_btn ? 1 : 0;          /* izquierdo = acción */
if (a == 0x1a570) {                          /* escritura del skip */
    ram[a] = g_mouse_skip ? 0xff : 0;        /* derecho = skip */
    return;
}
```

Es un intercept de la escritura de `$1a570`; el core no se toca.

### 2.2 Presentación

`platform_tick` solo presentaba durante el arranque:

```c
if (g_present && (!g_in_loop || g_in_delay))
    g_present(img);
```

En el bucle, el único que presentaba era el `frame_hook` al inicio de cada
iteración de `game_loop`. Las animaciones de puerta corren **dentro** de un
comando, en un bucle de `wait_vbl`; no pasan por el frame hook. El audio y el
tiempo avanzaban, pero la pantalla no se refrescaba. Al terminar el comando, el
bucle presentaba ya la sala nueva. De ahí “se oye pero no se ve”.

## 3. Fix

### 3.1 Input (solo HAL)

- `platform/video_sdl.c`: se rastrea la máscara completa de SDL
  (`SDL_BUTTON_LMASK` y `SDL_BUTTON_RMASK`) y se traduce a la máscara del puerto
  (`bit0=izquierdo`, `bit1=derecho`). **OJO SDL3**: `SDL_BUTTON_RIGHT = 3` y
  `SDL_BUTTON_RMASK = 1<<2 = 4`; no se puede pasar la máscara SDL tal cual a
  `platform_mouse_set` (ese fue el motivo de que el derecho no hiciera nada al
  principio).
- `platform/input.c`:
  - `buttons` usa la convención SDL: `bit0 = izquierdo`, `bit1 = derecho`.
  - Acción = izquierdo **o** derecho (`g_buttons`).
  - Skip = solo derecho (`g_mouse_skip`).
  - `platform_input_apply_skip(img)` pisa `A_SKIP_TRANSITION` con
    `g_mouse_skip ? 0xff : 0`.
- `platform/cpuwork.c`: en `__cyg_profile_func_exit`, si la función que sale es
  `wait_frame`, se llama a `platform_input_apply_skip()`. Es el equivalente
  port-side del intercept `m68k_write_memory_8($1a570)` del legacy: el core
  acaba de escribir el flag y la HAL lo corrige antes de que el llamante lo lea.
- El paquete IKBD sigue llevando la acción en `bit0`; el skip va aparte.
- `src/input.c` **no se modifica**.

Resultado:

- izquierdo: acción, skip 0 → animación visible;
- derecho: acción, skip 0xff → salta;
- teclado: acción vía `Crawio`, skip 0 → animación visible.

### 3.2 Presentación (solo HAL)

- `platform/time.c`: `platform_tick` presenta en **cada VBL** (arranque y bucle).
- `app/main.c`: `present_cb` (frame hook) deja de presentar; sigue bombeando SDL
  y contando FPS. La presentación la hace `tick_cb` en cada VBL.
- Se elimina el flag `g_in_delay`, que ya no hace falta.

Así se ven las animaciones que corren en `wait_vbl` dentro de un comando
(puertas, wipes, transiciones) y las que pasan por el bucle.

## 4. Pruebas

`make test-port`:

- `test_present_hook`: presenta en arranque **y** en bucle.
- `test_click_skip`:
  - izquierdo → acción, `A_SKIP_TRANSITION = 0`;
  - derecho → acción, `A_SKIP_TRANSITION = 0xff`;
  - sin botón → ni acción ni skip.
- `test_real_input` y `test_mouse_click_state` actualizados a la máscara
  izquierdo/derecho.

`make test`: **499** (el core no cambia; el hook de skip es solo del build del
port).

## 5. Verificación

- `make test`: 499.
- `make test-port`: OK.
- `make port`, `make port-sdl`, `make cmake-native`, `make cmake-win`,
  `make package-win`: OK.
- Smoke headless y SDL dummy: arrancan y terminan sin colgar.
- Con `KULT_MARKS`, el log muestra pares `K <vbl> 255` → `K <vbl> 0`: el core
  escribe el skip y el hook de HAL lo corrige con el botón derecho.
- Artefactos: `kult.exe` md5 `d67ad168b578ae37663f588b4638bf2f`;
  `kult-headless.exe` `52fb9898e56aeb93fcdd08ef1ad053c3`; ZIP
  `f7b268e9cf953baefe10c77a61fc7ec1`.

Validación visual pendiente del usuario:

- puerta con izquierdo/teclado: se ve la animación;
- puerta con derecho: se salta;
- A/B/C con izquierdo/teclado: reveal visible; con derecho: skip;
- el izquierdo vuelve a interactuar en el loop del juego.

## 6. Referencias

- `/app/kult-port-legacy/runtime/kult_engine.c`: intercept de `$1a570` y
  separación `g_mouse_btn`/`g_mouse_skip`.
- TOS `vdi/mouse.S`: `IKBD <00>:rt btn <01>:lf btn`.
- `notes/0017-retomar-maestro.md` y `notes/0019-fix-timing-c.md`.
