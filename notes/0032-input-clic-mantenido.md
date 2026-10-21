# 0032 — T-084: input mantenido / clics repetidos (handoff)

> **Estado: ABIERTO — requiere reanálisis antes de tocar código.**
> No hay fix aplicado. El candidato del legacy (pestillo de clic) **no se debe
> copiar a ciegas**: puede estar tapando una diferencia más profunda entre el
> modelo IKBD del port y el original.

## Síntoma del usuario

Al mantener pulsado el **clic izquierdo**, el juego interpreta clics continuos.
El anillo de acciones se abre y se cierra repetidamente; con una sola pulsación
es imposible elegir una opción. El usuario sospecha que puede pasar con más
entradas (teclado mantenido, botón derecho, etc.).

## Evidencia

Run de diagnóstico del 2026-10-21 (log conservado en
`build-win/kult-0.1.0-win64/diag-logs/input-click-held-2026-10-21.log`):

- 22.528 líneas, `kult-diag.exe` md5 `b5e94d0645dc3d86fee3909eb9180a47`.
- `Y 3d 1fb5a` (`icon_ring`/`icon_ring_loop`) se repite en VBL
  1478, 1485, 1492, 1499 y 1506.
- En esas líneas `V` el campo de botones es `05` (`$1a456` = bit0 + bit2 clic).
- `V ... 05` aparece 135 veces; hay 167 líneas `K` (cambios de skip).

Eso es consistente con: un botón físico mantenido se entrega como “clic nuevo”
en cada `wait_frame`, y el juego vuelve a ejecutar la acción.

## Cadena de código actual

1. `platform/video_sdl.c`: eventos SDL de ratón → `platform_mouse_set()`.
   - `SDL_GetMouseState` sincroniza la máscara cada pump.
   - `mouse_buttons()` traduce LMASK/RMASK a bit0/bit1.
2. `platform/input.c`:
   - `platform_mouse_set()` colapsa cualquier botón a `g_buttons = 1`.
   - `platform_ikbd_deliver()` entrega **el nivel** de `g_buttons` en cada
     paquete: si el botón sigue pulsado, `A_IKBD_PACKET = 1` siempre.
3. `src/input.c` (`wait_frame`, 0x13698, fiel al ASM):
   - copia `A_IKBD_PACKET` a `A_INPUT_BUTTONS`;
   - si el bit0 está puesto, marca bit2 = clic.
4. El juego comprueba `A_INPUT_BUTTONS & 0x04` en sus esperas y en
   `game_loop`; por eso un nivel mantenido se interpreta como clic repetido.

## Lo que ya se sabe (y lo que NO)

**Confirmado por código y log:** el port entrega el botón como nivel en cada
`wait_frame`; no hay pestillo de “una pulsación = un clic”.

**No confirmado:** que la solución correcta sea copiar el pestillo del legacy.
En `/app/kult-port-legacy/runtime/native/kult_native_host.c` existe:
- `g_click_hold = 12` VBLs y `g_click_latch`;
- `kult_native_click_consumed()` al leer el botón;
- soltar no cancela el clic pendiente;
- comentario: “una pulsación ejecutaba varios clics”.

Ese pestillo es un **candidato**, no una prueba de fidelidad. Hay que decidir si
el original modela el IKBD como evento (flanco) o como nivel, y si la solución
correcta es:
- un pestillo de flanco en la HAL (como legacy),
- modelar de forma más fiel el comando/respuesta IKBD de `0x13698`,
- o corregir otra parte del contrato de entrada.

## Sospechas adicionales (teclado)

- `platform/input.c:pc_os_crawio()` avanza `g_head` **dos veces**; descarta una
  tecla de cada dos. Es un bug independiente.
- `platform/video_sdl.c` encola `SDL_EVENT_KEY_DOWN` también cuando
  `e.key.repeat` es cierto; mantener una tecla puede repetir acciones. Legacy
  ignora `e.key.repeat`.

## Reanálisis obligatorio antes del fix

1. Leer el ASM de `0x13698` y el uso de `Ikbdws`/buffer `0x1a44e`/`0x1a459`;
   determinar qué responde el IKBD y si el botón es nivel o flanco.
2. Comparar con el comportamiento del original en Hatari/oráculo para:
   - clic mantenido sobre el anillo de acciones;
   - clic corto;
   - clic derecho mantenido (skip);
   - una tecla mantenida.
3. Revisar el legacy, pero **sin asumirlo correcto**: documentar por qué su
   pestillo de 12 VBLs es fiel o si es un parche empírico.
4. Decidir la capa del fix. Preferencia: `platform/` (modelo de máquina); no
   tocar `src/` salvo que el ASM demuestre que el fallo es del core.
5. Añadir pruebas en `platform/test/test_platform.c`:
   - clic mantenido → un solo clic;
   - soltar y volver a pulsar → nuevo clic;
   - clic derecho → acción + skip mantenido;
   - dos teclas encoladas → `Crawio` devuelve ambas.
6. Validar con `kult-diag-marcas.bat` manteniendo el clic: `btn=05` debe salir
   una sola vez por pulsación y el anillo debe quedar abierto para elegir.

## Referencias rápidas

- `platform/input.c`: `platform_mouse_set` (~47), `pc_os_crawio` (~106),
  `platform_ikbd_deliver` (~147).
- `platform/video_sdl.c`: `platform_sdl_pump` (~120), key repeat (~130).
- `src/input.c`: `wait_frame` (~28).
- `src/loop.c`: `game_loop` (~2141), click en bucle principal.
- `src/commands.c`: `icon_ring` (~469), `overlay_menu` (~147),
  `sample_menu` (~90).
- `src/handlers.c`: `wait_count` (~220), `obj_wait_hit` (~790).
- Legacy: `/app/kult-port-legacy/runtime/native/kult_native_host.c`
  (`kult_native_input`, `kult_native_frame_tick`, `click_consumed`),
  `/app/kult-port-legacy/runtime/native/kult_native_input.c`.

## Base actual

- `make test`: **513**; `make test-port`: OK.
- Artefactos: `kult.exe` `41be915a…`, `kult-diag.exe` `b5e94d06…`.
- Log de esta incidencia: `diag-logs/input-click-held-2026-10-21.log`.
