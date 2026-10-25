# 0053 — T-022: `0x106f2` (lista C) y reinicio suave `l_0101a8`

Fecha: 2026-10-25. Sesión 69. Estado: **cerrada**; T-094 queda pendiente sobre
el ZIP regenerado.

## Alcance

- **Repo C (`kult-atari-st-c`):** completar `f_0106f2` (`0x106f2`), el cargador
  de la lista C (`0x1a2c8`), para dejar el core 1:1 con el original.
- **Port:** reconstruir `l_0101a8` como **ruta latente verificada**, sin añadir
  menús, botones, atajos ni eventos de juego. Solo se fuerza con el diagnóstico
  `KULT_SOFT_RESTART_AT=<vbl>`.
- **No confundir** con el reinicio tras morir (`0x1842c` → `reset_clear` →
  `0x10158`), ya resuelto en `boot/` (`notes/0022`).

## Repo C

Commit `94c0be1` (`feat(fileio): porta f_0106f2 (lista C)`):

- `src/fileio.c`: `load_resource_list_c(img)` → `load_resource_list(img, 0x1a2c8)`.
- `include/kult_api.h` y `kult_core.map`: símbolo público `load_resource_list_c`.
- `symbols/names.txt`: `fn 0x106f2 load_resource_list_c`.
- `test/test_fileio.py`: `test_fun_106f2` diferencial contra el oráculo
  (memoria, DTA, handles y retorno).
- `make mapa` regenerado; censo actualizado (424 funciones, 519 pruebas).

Verificación:

```sh
cd /app/kult-atari-st-c
make test          # 519 passed
make regscan       # ids_reg=0
make core
make mapa
```

## Port

Core importado con `git subtree pull` desde el repo C:

- `fbeaff3` — merge `chore(core): importa f_0106f2 (lista C)`.
- `dd417c5` — squash `beebb52..94c0be1`.

`boot/boot.c`:

- `boot_load_0100ec()` factoriza el tramo `l_0100ec` que ya existía en el
  arranque en frío: FOND, lista B, ECHE 0..7, paleta, copia de pantalla,
  `save_clear_bin`, `music_stop` y `sound_stop_supexec`.
- `boot_l0101a8()` implementa la secuencia del original:
  1. vaciar la pila de instrumentación;
  2. `platform_disk_timing(1)`;
  3. `load_resource_list_c` (si falla, se registra y se continúa);
  4. `crt0_init` + `platform_ikbdws` (mismo ajuste de ratón que en frío);
  5. `load_resource_list_a` (si falla, se registra y se continúa);
  6. `boot_load_0100ec` y vuelta al camino común `0x10158`.
- No se escribe `A7`: `STACK=0x1d1fe` es la pila del 68000 y no tiene
  equivalente directo en C.
- No se repiten `init_hardware`, menú de idioma ni parches de idioma/MOTS: la
  imagen ya los conserva.
- El `KULT_SOFT_RESTART_AT` del hook de frames hace `longjmp(g_jmp, 3)`; el
  bucle de `kult_boot` llama a `boot_l0101a8` y después al camino común
  (`boot_enter_game` + `game_loop`). `g_restart_armed` permanece a 0 durante la
  recarga y se arma justo antes de `game_loop`. `KULT_RESTART_AT` no cambia.

## Verificación del port

```sh
cd /app/kult-atari-st-pc-port
make test          # 519 core + HAL + diagnóstico + 1 port OK
make package-win   # ZIP regenerado
```

Smoke con datos reales:

```sh
KULT_WINDOW=0 KULT_FRAMES=400 KULT_SOFT_RESTART_AT=200 \
KULT_SKIP_DRAP=1 KULT_KEYS=E KULT_DEBUG=1 ./build/kult
```

Resultado: `exit=0`, sin crash. Secuencia observada en `kult.log`/stderr:

```text
reinicio suave: KULT_SOFT_RESTART_AT=200
reinicio suave: lista C no disponible; se continúa
[boot] lista_c
[boot] crt0 (reinicio)
[boot] lista_a (reinicio)
[boot] fond
[boot] lista_b
[boot] eche
[boot] clear/music
[boot] wait_frame l_010158
[boot] run_command c001
[boot] hint_icons
[boot] entrando game_loop
```

Arranque normal sin `KULT_SOFT_RESTART_AT`: `exit=0` y misma secuencia de
entrada al juego, sin reinicio suave.

## Limitaciones

- La lista C no tiene ficheros sueltos en el paquete: `Fsfirst` falla y
  `l_0101a8` **continúa**, igual que el original. No es un error fatal.
- `0x101a8` sigue pareciendo **inobservable** en el flujo normal (solo aparece
  como constante base de relocalización en `0x16f90`/`0x16fb4`, sin
  `jsr`/`jmp`/`pea`). Se porta por completitud y queda cubierto por el
  diagnóstico.
- El reinicio suave no es una funcionalidad de juego: no hay tecla, menú ni
  condición de final que lo dispare.
- T-094 no se cierra aquí: la validación visual final debe hacerse sobre el ZIP
  regenerado.

## Artefactos

- ZIP: `build-win/kult-1.0.0-win64.zip`
  SHA256 `ca6eec877d8ef8e75c6c9918c1d08fdffb0dc5a084b920a8a30a9fe58bfa252b`.
- Core C: `94c0be1`; port: `fbeaff3` (merge del subtree) y `bef0896` (código de
  `boot/`).
