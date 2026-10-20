# 0026 — Esperas de disco: alcance global (arranque, idioma, salas, estrella)

> La espera añadida para la estrella final no es específica de `FOND.PI1` ni de
> esa escena: `pc_os_fread()` la aplica a **cada** lectura de recurso. Por eso
> ahora también se notan pausas al arrancar, tras elegir idioma/cambio de disco
> y al cargar salas. Es una estimación del tiempo de disquete del ST, no una
> emulación ciclo-exacta del FDC.

## Mecanismo

`platform_disk_delay(bytes)` (`platform/time.c`) duerme `bytes × 62,5 µs`
(≈ 16 KB/s) cediendo VBLs y presentando. Se llama desde `pc_os_fread()`
(`platform/fs.c`) con el número real de bytes leídos. En headless/diferencial
(`g_rt == 0`) es no-op; en ventana con `KULT_REALTIME=1` (defecto) se aplica.

Base de la estimación: disquete ST DD de 9×512 B/pista a 300 rpm = 4.608 B por
revolución cada 200 ms → **1,40 s para 0x7d22 = 32.034 B** (7 pistas). Sumando
búsqueda y arranque del motor se modela **~2,0 s** para ese tamaño.
El usuario indicó que la espera real es **> 1,4 s**.

## Dónde se carga FOND.PI1

`load_fond` (`0x13568`) fija `FOND.PI1` y `0x7d22` B y cae en `load_resource`
(`0x1076a` → Fread). Se usa en:

1. **Arranque / tras el menú de idioma y el cambio de disco**:
   `boot/boot.c` llama `load_fond(img)` después de `load_resource_list_a` y del
   aviso omitido de “INSERT DISK 2” (el original lo pide porque FOND vive en el
   disco B). Es la pausa que se ve tras elegir idioma.
2. **Preparación de cada sala**: `prepare_room` (`0x1842c`) llama `load_fond`
   tras el `icon_scale`/estrella, antes de `set_palette`, `room_reset` y
   `copy_screen`. Es la **espera de la estrella final** (`notes/0025`).
3. La misma `prepare_room` se usa al entrar/cambiar de sala y al reiniciar tras
   morir; por eso puede haber una pausa más en esos tránsitos.

No es que FOND se cargue “antes del arranque”: al inicio se cargan **PRES.PI1**
(título) y **DRAP.PI1** (menú de idioma) con `load_named_screen`, que usa el
mismo tamaño `0x7d22` y por tanto la misma espera. FOND entra después del
idioma/cambio de disco.

## Otras lecturas que ahora tienen espera

Con los ficheros de la versión E (tamaños reales; otras lenguas varían poco):

| Momento | Función / ficheros | Bytes aprox. | Espera aprox. |
|---|---|---:|---:|
| Título | `load_named_screen` → `PRES.PI1` | 32.034 | 2,0 s |
| Menú de idioma | `load_named_screen` → `DRAP.PI1` | 32.034 | 2,0 s |
| Tras idioma | `load_resource_list_a` → `DIAL*`, `DESC*`, `VERB*`, `MOTS*` | ~22.900 | ~1,4 s |
| Cambio de disco / inicio | `load_fond` → `FOND.PI1` | 32.034 | 2,0 s |
| Cambio de disco | `load_resource_list_b` → `PUZZL.BIN` + `BRUIT.BIN` | ~142.500 | ~8,9 s |
| Arranque | `load_eche_bank` → `ECHE0..7` | ~147.200 | ~9,2 s |
| Cambio de sala | `load_scena` → `SCENA.BIN` + 2×`0x7d00` | ~74.500 | ~4,7 s |
| Reinicio tras muerte | `reset_clear` → `CLEAR.BIN` | 10.524 | 0,66 s |
| Estrella final | `prepare_room` → `load_fond` | 32.034 | 2,0 s |

Los tamaños de las listas se leen con `Fsfirst` y se pasan a `Fread`; por eso la
espera es proporcional al fichero real, no una constante por llamada.

## Efecto y validación

- `make test`: 499 (el diferencial no tiene tiempo real; el delay es no-op).
- `make test-port`: OK; `test_disk_delay` comprueba no-op sin tiempo real y
  ≥2 VBL para 800 B (~50 ms) con tiempo real.
- En ventana: las pausas de arranque/idioma/cambio de disco/estrella son
  visibles y dan el ritmo del original. Si alguna resulta demasiado larga o
  corta, el único parámetro a ajustar es el factor de `platform_disk_delay`
  (62,5 µs/B), no una animación concreta.
- `src/` intacto: el modelo vive en la HAL.

## Referencias

- `notes/0025-espera-carga-fond.md` (estrella final).
- `platform/fs.c`, `platform/time.c`, `platform/platform.h`.
- `boot/boot.c`, `src/screens.c` (`load_fond`/`load_named_screen`),
  `src/loop.c` (`prepare_room`), `src/fileio.c` (`load_resource_list`,
  `load_scena`, `reset_clear`).
