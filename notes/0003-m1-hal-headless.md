# Nota 0003 — M1: HAL headless

- **Sesión 3.** Se implementa la **HAL de PC** (`platform/`), el **loader del
  PRG** y el **arranque provisional** (`boot/`) y el **driver headless**
  (`app/`). `make test-port` verde; `make port` arranca el juego y vuelca la
  sala cargada.

## Qué se hizo
- **Contrato (T-010)**: `platform/include/{os,hw,psg,sched}.h` **sombrean** las
  cabeceras del kit en el build del ejecutable (se pasa `-Iplatform/include`
  antes). Sólo reproducen lo que `src/` usa de verdad (medido con `nm -u` y
  grep): 13 `os_*`, 5 `OS_*`, `hw_write8/sub8/bclr8`, `psg_port_*`,
  `sched_poll8`, `os_refused`. `machine.h` se sigue tomando del kit.
- **Ficheros (T-011)**: `platform/fs.c` mapea `A:…` → `data/` (varios dirs con
  `KULT_DATA_DIR`), handles 6..37, `Fsfirst` vuelca tamaño/nombre en el DTA.
- **Sistema/vídeo (T-012/T-013)**: `os_super` con token; `platform/video.c`
  guarda base visible (Setscreen) y paleta (16 words ST), convierte planar→RGB
  y vuelca PPM/raw.
- **Entrada (T-014)**: `platform/input.c`: teclado (cola + auto-tecla `E`),
  paquete IKBD con cursor (160,100) y clic alterno (headless).
- **Tiempo (T-015)**: `platform/time.c`: `sched_poll8` avanza `A_DAT_1ACD4` y
  entrega el IKBD; `platform/{hw,psg,os,audio}.c` (MFP/PSG mínimo, `os_refused`
  no-op, `g_os_event` no-op, audio stub).
- **Driver + arranque (T-017/T-020 provisional)**: `boot/prg.c` (loader GEMDOS
  con tabla DRI de relocación, espejo de `loader.py`); `boot/boot.c` traduce
  `_start` (0x10000) a llamadas al core, con siembra del patrón FDC (`0xd8e8`),
  menú de idioma, y corte a N frames con `hook_loop_top` + `longjmp`.
  `app/main.c` headless con envs.

## Hallazgos
- Los recursos "empotrados" (CARAC/TEMPL/ICONE/ARPLA/…) **están dentro del PRG**
  en sus direcciones de runtime; no hay que inyectarlos (verificado contra
  `kult-port-legacy/work/embedded/`). En cambio `disk_b` sí hace falta:
  FOND.PI1, PUZZL.BIN, ECHE*.BIN.
- **El bucle principal queda bloqueado por el tick de VBL `0x160c2`** (M2): tras
  `_start`, `run_command(0xc001)` despacha a un índice de glifo inválido. El
  tick (rotación de paleta + blits MURSM/TEMPL) es la pieza que falta; el driver
  M1 se detiene **antes** de 0xc001 por defecto y vuelca la sala (coherente, 14
  colores). `KULT_RUN_LOOP=1` reproduce el fallo.
- El predecesor (`kult-port-legacy`) tiene un host nativo equivalente que también
  se queda en 0xc001, y una HAL Musashi que **parchea `_start`** para saltar el
  DRAP. Se ha añadido `KULT_SKIP_DRAP` como opción (no evita el bloqueo).

## Pruebas
- `make test-port`: ficheros (A:→dir, tamaño en DTA, lectura), vídeo
  (planar→RGB + paleta, golden de 2 píxeles), entrada (IKBD fija el cursor,
  Cnecin auto-tecla), reloj (el sondeo de VBL avanza el contador).
- `make port` + ejecución: vuelca `out/frame.ppm` coherente.

## Siguiente
- **M2/T-021**: portar `0x160c2` (con `0x1618a` y `0x16210`/`0x10768`) para
  desbloquear el `game_loop`. Después validar `_start` (menú fiel) y M3 (SDL).
