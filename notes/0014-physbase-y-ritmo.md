# Nota 0014 — Physbase 0x98000 (colores del juego) y ritmo pendiente

- **Sesión 14.** Tras desbloquear T-061, el usuario reporta: arranque/idioma bien,
  pero el **juego con colores distorsionados** (ruido cian/rojo; captura en
  `temp/screenshots/`) y la **lógica acelerada**.

## Colores: la base de pantalla (Physbase) es del HAL
- Instrumentado un dump de paleta con `KULT_DEBUG`: la HAL aplicaba
  `[pal] 7f80 112 223 ...` y coincidía con la imagen en `0x77fe0`; solo 5 words
  coincidían con `FOND.PI1` (justo los que escribe `set_palette_entry`). El resto
  eran **datos del juego**, no colores.
- Causa: con `OS_SCREEN_BASE = 0x80000` (valor del kit), pantalla B = `0x78000`
  y paleta = `0x77fe0` caen dentro de la zona que el juego llena:
  - bancos **ECHE 0..7** contiguos desde `0x64c8e` → acaban en `~0x88b84`;
  - el heap/buffers del juego llegan a `~0x88c00`.
  El juego pisaba pantalla y paleta → ruido y colores mal. El predecesor ya lo
  documentó y lo corrigió igual (`Physbase = 0x98000`).
- **Fix**: `platform/include/os.h` → `OS_SCREEN_BASE = 0x98000` (es la HAL:
  Physbase lo fija el sistema; el juego solo lo lee). Quedan pantalla B
  `0x90000`, paleta `0x8ffe0` y A `0x98000` (fin `0x9fd00`), por encima de
  ECHE/heap y dentro de la imagen de 1 MiB. **`src/` intacto**.
- El diferencial (`make test`) compila con el `os.h` del kit (`0x80000`), no con
  esta cabecera: **499 verde**. `make test-port` también.
- Verificación: `[pal]` = paleta exacta de `FOND.PI1`
  (`000 112 223 232 544 334 000 702 443 325 400 530 734 754 546 777`) y estable;
  frame del juego correcto ("THE MASTER'S ORBIT"); `visible=0x98000`.
- De paso: `make port-sdl` no dependía de `platform/include/*.h` (no recompilaba
  al cambiar la base); añadido a las dependencias.

## Ritmo: aceleración (abierto → T-073)
- Medido en ventana: **20,1 ms/tick = 50 VBL/s** y 1 tick por iteración de
  `game_loop`; el *pacing* del port es correcto.
- El original también corre a 50 VBL/s, pero su CPU (7 MHz) no siempre termina el
  trabajo de un frame dentro de un VBL y la reconstrucción **descartó busy-waits**
  sin efecto observable en imagen (p. ej. el `dbra 0xffff` de `palette_cycle`).
  En el ST eso baja el ritmo efectivo; el port, sin coste de CPU, va "a tope".
- No es la base de pantalla. La solución correcta es la del predecesor: **medir**
  ciclos por función con el oráculo y aplicar un **reloj virtual de CPU** (~8 MHz)
  que duerma el trabajo. **No** ajustar animaciones a mano (provoca velocidades
  distintas). Queda como **T-073**.

## Corrección: la primera pantalla NO es negra (es el título)
- El usuario corrigió la conclusión anterior: la primera pantalla es el **título
  (PRES.PI1)** con música, esperando **tecla o clic**; no debe verse negra.
- Causa del negro: `_start` carga PRES y lo copia a A, pero no llama a
  `set_palette` hasta después de la espera de DRAP; el ST hereda la paleta del
  escritorio, pero el HAL arranca con la paleta a cero.
- **Fix** (`boot/boot.c`): aplicar la paleta del PI1 recién cargado (cabecera en
  `screen_ptr_b-0x20` = `A_PALETTE_PTR`) tras copiar PRES y antes de la espera.
  El título se ve con sus colores; después se mantiene el `set_palette`/swap de
  DRAP. El `_start` original queda igual (era una carencia del estado inicial
  del HAL, no del juego).
- **Verificación**: `KULT_DUMP_TITLE=title.ppm` vuelca el frame del título antes
  de la espera; es **idéntico píxel a píxel a `PRES.PI1`** (0 diferencias).
  Criterio en `docs/VERIFICACION.md` (título visible; tecla o clic → idioma;
  fallo si negro/no avanza). El teclado **sí** avanza esta espera (vía
  `wait_frame`/Crawio), además del clic.
- **El usuario confirma** que la pantalla de inicio ya se ve correctamente (sin
  negro).

## Estado
- ZIP nuevo con el fix de colores. **El usuario confirma que los colores ya se
  ven correctos** (T-061, 2026-10-19). Queda por confirmar el **ritmo**; si la
  aceleración sigue, atacar T-073 con medición (antes o después de T-072, según
  prioridad).
