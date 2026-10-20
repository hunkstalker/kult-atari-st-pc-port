# 0022 — Reinicio tras morir: el `bra 0x10158` que faltaba en `reset_clear`

> Bug detectado en la validación de T-061: al morir, el juego no volvía al
> Maestro/objeto; repetía la secuencia de muerte en bucle. **No se toca `src/`**:
> el salto de control que el C nativo no puede expresar se emula en `boot/` con
> `longjmp`, disparado por el hook de instrumentación.

## 1. Síntoma

- Al morir, tras la animación, volvía a ejecutarse la secuencia de muerte.
- El Maestro/objeto del inicio del bucle no aparecía.
- En `kult-marcas.log` se repiten `1682a` (`reset_clear`), `1842c`
  (`prepare_room`) y `181f0` (`room_reset`) cada ~126 VBLs (2264, 2391, 2516 en
  la run analizada).
- `106e2`/`106ea`/`106f8` (listas A/B) y `135e2` (`crt0_init`) solo aparecen en
  el VBL 0 (arranque inicial): **no** hay recarga completa de recursos.

## 2. Causa

El original termina `reset_clear` (`0x1682a`) con un salto al inicio del bucle:

```asm
movea.l #$1d1fe,a7
move.b  #$ff,$1ad0f
bsr.w   $16210
move.b  #$2,$1acd8
bra.w   $10158
```

`0x10158` es el arranque del bucle de juego:

```asm
wait_frame hasta soltar el clic
move.b  #$7,$1acc6
clr.b   $1ad0d
move.w  #$c001,$1a50e
run_command_keep_sp   ; Maestro/objeto
hint_icons
wait_frame_cursor
game_loop
```

La función C `reset_clear` (`src/fileio.c`) reproduce el estado (carga
`CLEAR.BIN`, `A_DAT_1AD0F=0xff`, `pal_next_byte`, `A_DAT_1ACD8=2`), pero **no
puede ejecutar el `bra.w 0x10158`**: retorna al llamante (`prepare_room` → guion
de muerte), y el guion continúa/repite.

No es `0x106f2`/`l_0101a8` (reinicio suave con recarga de listas): el original
salta a `0x10158`, no a `0x101a8`. El log lo confirma: no hay recarga de listas
en los reinicios.

## 3. Fix (solo `boot/` + hook de instrumentación)

- `platform/cpuwork.c` (`__cyg_profile_func_exit`): al salir de `reset_clear`,
  llama a `kult_request_restart()`.
- `boot/boot.c`:
  - `kult_request_restart()`: si el reinicio está armado, vacía la pila de
    instrumentación (`platform_cpuwork_reset_stack`) y hace
    `longjmp(g_jmp, 2)`.
  - `boot_enter_game(img)`: reproduce `0x10158` (espera a soltar clic, modo 7,
    `the_command=0xc001`, `run_command_keep_sp`, `hint_icons`,
    `wait_frame_cursor`).
  - El bucle de `kult_boot` usa `setjmp`: valor 1 = stop/límite de frames;
    valor 2 = reinicio → vuelve a `boot_enter_game` y a `game_loop`.
  - `g_restart_armed` evita reinicios durante el arranque/`0x10158`.
  - `longjmp` desde el exit hook no desenrolla la pila de instrumentación; se
    vacía explícitamente antes del salto.

`src/` queda intacto; el hook es solo del build del port.

## 4. Diagnóstico y pruebas

- `KULT_RESTART_AT=<vbl>`: fuerza un reinicio desde `0x10158` en ese VBL (solo
  diagnóstico). Verificado en headless: aparece
  `reinicio: reset_clear -> 0x10158`, se reentra en el bucle y el proceso
  termina limpio.
- `make test`: **499** (el core no cambia).
- `make test-port`: OK.
- `make port`, `make port-sdl`, CMake nativo/win y ZIP: OK.
- Smoke headless y SDL dummy: OK.

## 5. Verificación real pendiente

Morir en Windows con el exe nuevo:

- tras la animación de muerte debe volver el **Maestro/objeto** del inicio del
  bucle, no repetirse la muerte;
- conservar input, presentación y cursor de T-074/T-075.

Artefactos:

- `kult.exe` md5 `e3d0b6ee0bc98be046b5a02c732fe0ec`
- `kult-headless.exe` md5 `5ec4eb9dcfd68930b0813a31be92f853`
- ZIP md5 `c709bfb62f7bb8076dd9fb4e70d68d29`

## 6. Referencias

- `0x1682a` (`reset_clear`): `src/fileio.c`; `bra.w 0x10158`.
- `0x10158`: arranque del bucle (`boot/boot.c`, `boot_enter_game`).
- Log: `build-win/kult-0.1.0-win64/kult-marcas.log` (F12 2313/2399/2507;
  reinicios 2264/2391/2516).
- `notes/0010`, `notes/0011` (corrección de la suposición sobre `reset_clear`).
