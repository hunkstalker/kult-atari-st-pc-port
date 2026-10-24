# 0032 — T-084: input mantenido / clics repetidos (resuelto y validado)

> **Resuelto en sesión 36 y validado por el usuario en Windows.** El clic
> izquierdo mantenido ya no repite la acción: el anillo se abre una vez y queda
> abierto para elegir. Fix **sólo en `platform/`**; `src/` (el juego) queda
> intacto.

## 1. Síntoma

Al mantener pulsado el clic izquierdo, el juego interpretaba clics continuos: el
anillo de acciones se abría y se cerraba y no se podía elegir. La sospecha de
que afectaba a más entradas mantenidas era correcta en el mecanismo (se
entregaba un nivel), pero sólo el ratón reproducía el fallo con la evidencia
disponible.

Log de partida:
`build-win/kult-0.1.0-win64/diag-logs/input-click-held-2026-10-21.log`
(`kult-diag.exe` `b5e94d06…`): `Y 3d 1fb5a` (`icon_ring`) se repite en VBL
1478/1485/1492/1499/1506 con `$1a456 = 05`.

## 2. Qué hace el original (ASM + Hatari)

`wait_frame` (`src/input.c`, ASM `0x13698`) hace:

```asm
move.b  #$ff,A_IKBD_PACKET
move.l  #s_01a44e,-(a7)     ; buffer de 1 byte
clr.w   -(a7)               ; longitud XBIOS = d0+1 = 1
move.w  #$19,-(a7)          ; XBIOS Ikbdws
trap    #$e
...
```

El byte en `0x1a44e` es **`0x0d`** (comprobado en `KULT.PRG`). No es un “clic”:
es el comando IKBD **`0x0D`, INTERROGATE MOUSE POSITION**. El IKBD responde:

```text
0xF7            ; cabecera de posición absoluta
BUTTONS         ; 0000dcba: a=derecho baja, b=derecho sube,
                ;            c=izquierdo baja, d=izquierdo sube
XMSB XLSB YMSB YLSB
```

TOS consume la cabecera y llama al `mousevec` con un buffer de 5 bytes
(`mousebuf`): `BUTTONS,X,Y`. El handler del juego (`0x13686`) copia esos 5 bytes
a `A_IKBD_PACKET` (`0x1a44f`).

Lo decisivo: esos bits son **eventos “desde la última interrogación”**, no el
nivel actual. Un botón mantenido sólo produce su bit de bajada en la primera
interrogación; las siguientes reciben `BUTTONS = 0` mientras no cambie. Hatari
lo implementa igual en `IKBD_Cmd_ReadAbsMousePos()`: compara el estado actual
con `PrevReadAbsMouseButtons` y devuelve sólo los bits cambiados.

Por eso el pestillo de la referencia (`g_click_hold = 12`, `g_click_latch`,
`kult_native_click_consumed`) **no es la especificación**, pero su idea de “una
pulsación = un clic” sí es fiel. El timeout de 12 VBL es empírico; el original
no lo tiene: el flanco queda pendiente hasta la siguiente interrogación.

## 3. Causa en el port

`platform_mouse_set()` colapsaba cualquier botón a un nivel (`g_buttons`), y
`platform_ikbd_deliver()` entregaba ese nivel en **cada** paquete. El
`wait_frame` fiel al ASM marcaba bit2 de clic en cada `wait_frame`; el anillo
volvía a ver “clic” cada 7 VBL.

## 4. Decisión de diseño

- **Original = flanco/evento**, no nivel.
- **Capa = `platform/`**: es el modelo del IKBD (máquina), no lógica del juego.
  `src/input.c` ya era fiel; no se toca.
- La HAL conserva la convención ya validada de acción en **bit0** del paquete
  (notas 0020): izquierdo o derecho generan **un** flanco de bajada; el
  `wait_frame` lo normaliza a bit2 (`$1a456 = 05`). Así el log de diagnóstico
  sigue mostrando `btn=05`, pero **una sola vez por pulsación**.
- El **skip** también es de flanco: sólo la bajada del derecho lo arma. Se pisa
  `$1a570` al salir de `wait_frame` con ese evento, no con el nivel del botón.
- No se copia el timeout de 12 VBL de la referencia.

## 5. Fix aplicado (`platform/input.c`)

- `platform_mouse_set()` mantiene el estado físico de izquierdo/derecho y arma
  `g_action_pending` / `g_skip_pending` **sólo en una bajada**.
- `platform_ikbd_deliver()` consume los flancos: entrega `bit0 = 1` una vez y
  después `0` mientras el botón siga pulsado. También actualiza
  `A_INPUT_BUTTONS` para las rutas que leen el paquete sin pasar por
  `wait_frame`.
- `platform_input_apply_skip()` aplica el skip del **flanco entregado** y lo
  consume.
- El clic sintético headless (`KULT_CLICK`) mantiene su alternancia para liberar
  las esperas “soltar y pulsar”.
- `platform_input_set_auto()` limpia botones/flancos al cambiar de modo (evita
  arrastrar estado entre pruebas y entre arranque real/headless).

## 6. Teclado (revisado)

- `pc_os_crawio()` avanza `g_head` **una sola vez** en el código actual; la
  sospecha de doble avance era de un estado anterior. Se añade
  `test_crawio_queue`: dos teclas encoladas → dos lecturas, la segunda no se
  pierde.
- `platform_sdl_pump()` sigue encolando `e.key.repeat`. **Es fiel**: TOS también
  auto-repite (`kb_initial`/`kb_repeat` en `bios/chardev.S`). Filtrarlo haría el
  port menos fiel y no es la causa del anillo (el anillo se dispara con el ratón).
  Si en el futuro se quiere igualar el ritmo exacto de repetición del TOS, será
  una tarea aparte.

## 7. Pruebas

En `platform/test/test_platform.c` (todas en `make test-port`):

- `test_click_held`: bajada izquierda = un clic; **32 `wait_frame` mantenidos**
  = ningún clic nuevo ni skip; soltar no es clic; nueva pulsación = otro clic.
- `test_click_skip`: bajada derecha = acción + skip; mantenido no repite ni
  acción ni skip; sin botón no hay nada.
- `test_crawio_queue`: dos teclas encoladas = dos `Crawio` y cola vacía.
- Se mantiene `test_ikbd_mouse_max` (máximo 304×184) y el resto de la HAL.

`make test`: **513**; `make test-port`: OK.

## 8. Validación

- `make test` **513** + `make test-port` verdes.
- Diag nativo SDL (mismo código que `kult-diag.exe`) bajo Xvfb, con xdotool:
  pulsación mantenida 5 s → **un solo flanco de entrada**. La traza no muestra
  repeticiones de `Y 3d` durante el mantenimiento.
- `test_click_held` recorre 32 frames con el botón pulsado sin clics nuevos.
- **Validado por el usuario en Windows** con `kult-diag.exe` (`64505805…`):
  manteniendo el clic, el anillo de acciones se abre **una sola vez** y se queda
  abierto para poder elegir.

Ejecutables de esta sesión:

- `kult-diag.exe` `6450580595c4fe623d42d17b5cb6a817`
- `kult.exe` `cc4d5cee1a297a5dfdef93821a0f2c35`
- `kult-headless.exe` `414a82c85329022188c1e9c5781251d1`
- Build local de diagnóstico (no conservado): `kult-sdl`
  `cded5de825b6bba3c68b2f4e90376cd6`

## 9. Referencias

- ASM: `kult-atari-st-decomp/matching/src/auto_013698.s`,
  `matching/src/fun_13686.s`; `KULT.PRG` byte `0x1a44e = 0x0d`.
- TOS: `matching/toolchain/bios/chardev.S` (`mousebuf`/`mousevec`).
- IKBD: comando `0x0D` (Linux `atarikbd.rst`); Hatari
  `IKBD_Cmd_ReadAbsMousePos()`.
- Referencia (sólo consulta, no fuente de verdad):
  `runtime/native/kult_native_host.c`.
- Notas previas de input: `notes/0020`, `notes/0021`, `notes/0027`.
