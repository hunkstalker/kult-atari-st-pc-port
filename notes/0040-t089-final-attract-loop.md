# 0040 — T-089: el final entra en el bucle infinito de `attract`

> **Estado:** ✅ **resuelto y validado por el usuario en Windows** (2026-10-22).
> Tras el final se queda en la animación de estrellas, sin volver al guion ni
> dejar la escena/paleta corruptas. T-061 sigue abierta por la partida completa.

## 1. Síntoma

En el original, tras la última secuencia, se ve la animación de viaje espacial
con estrellas (la misma que al morir, sin la pantalla de sangre) y se queda ahí.
En el port se volvía al guion y aparecía una escena/paleta corrupta.

## 2. Causa

El original no retorna de `attract` (`0x18a98`). Al acabar la primera pasada:

`/app/kult-atari-st-decomp/matching/src/auto_018a98.s:133-140`

```asm
l_018c78:
    movea.l (a7)+, a6
    addq.l  #1, a6
    bsr.w   f_0182f0
l_018c80:
    move.l  #$64, d5
    bsr.w   f_01834a          ; icon_scale_loop
    bra.w   l_018c80
```

El C modelaba la primera pasada, hacía `icon_list_reset` y **retornaba**
(`src/screens.c`). Al volver, el VM/guion seguía ejecutando comandos y dejaba
la pantalla/paleta erróneas.

### Evidencia en Hatari: código relocalizado +0xB018

El original ejecuta una copia del código en la zona de SPRIT/PERSO; por eso los
breakpoints en las direcciones estáticas no saltaban. La relación es:

```text
runtime = dirección del ASM + 0xB018
```

Retornos vistos en la pila:

```text
0x2343C = 0x18424 + 0xB018   ; wait_vbl dentro de f_01834a
0x23CA2 = 0x18C8A + 0xB018   ; bsr f_01834a dentro de l_018c80
0x1B67A = 0x10662 + 0xB018   ; vm_run
0x1B436 = 0x1041E + 0xB018   ; run_command_keep_sp
```

La cadena real es:

```text
run_command_keep_sp -> vm_run -> attract
  -> l_018c80 (bucle infinito) -> f_01834a -> wait_vbl
```

## 3. Fix

En `src/screens.c`:

- la primera pasada pasa a `attract_once()` (devuelve `script+1` en 0x18c78);
- `attract_loop(img, script, max_loops)` reproduce `l_018c80` llamando a
  `icon_scale_loop(img, 0, 0x64)`;
- `max_loops == 0` es el comportamiento real (infinito);
  `max_loops > 0` solo lo usan las pruebas para acotar la ejecución;
- `attract()` llama a `attract_loop(..., 0)` y por tanto **no retorna**, igual
  que el original.

La salida anticipada de `elem_command` se conserva: si `attract_once` devuelve
algo distinto de `script+1`, `attract_loop` la propaga.

## 4. Pruebas

- `test_fun_18a98` / `_first` / `_abort` siguen comparando la primera pasada
  contra el oráculo con `stop_pc=0x18C80` (ahora llaman a `attract_once`).
- Nueva `test_fun_18a98_loop_once`: `attract_loop(..., 1)` contra el oráculo
  parado en `0x18C8A` (el `bra` del bucle), comprobando que se ejecuta
  `f_01834a` con `D5=0x64`.
- `make test`: **518 passed**.
- `make test-port`: OK.
- `make port`, `make port-sdl`, `make cmake-win`, `make cmake-win-diag` y
  `make package-win`: OK.

## 5. Build

```text
kult.exe               a132e8e35cb63336f51664c2a9ab8e4c71110b94c5f597fe85f0abe5961e3761
kult-headless.exe      376021643dc00088c2ca6fca89717af4041e9329cfe40a8d2ae322b931ed4166
kult-diag.exe          d9f5775cbc78fc77184fe3247ba8511ae642f6659fbde23038ff957ba20f78cb
kult-diag-headless.exe 7cbf6587422917c660b19f6d473b60b3bf3cf82f61ad251cffe6994a318840e9
ZIP                    450739ecf0d946a811b10b1c89b1f7b1035b9028b40e2a00a0bc265f570fb0e8
```

## 6. Validación

Validado por el usuario en Windows el **2026-10-22** con `save-endgame` +
`kult-replay-endgame.bin` y el build de §5. Resultado:

- tras la última secuencia se queda en la animación de estrellas;
- **no** vuelve al guion ni aparece la escena/paleta corrupta;
- el proceso se cierra cerrando la ventana, como corresponde al bucle infinito
  del original.

Con esto T-089 queda cerrado. T-061 sigue abierta: ya es posible una partida
completa de principio a fin, pero el usuario quiere revisar algún punto más.
