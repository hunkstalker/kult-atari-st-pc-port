# Nota 0013 — Desbloqueo T-061 (datos) + log de diagnóstico

- **Sesión 13.** El usuario reportó que `kult.exe` "no hacía nada". Causa: en
  `data/` solo había **imágenes `.STX`** (`Kult.STX`, `Kult (B).STX`); el paquete
  interino necesita los **ficheros extraídos** (`KULT.PRG`, `*.PI1`, `*.BIN`), y
  al no encontrarlos el `.exe` (subsistema GUI, sin consola) salía con código 2
  sin mensaje.
- Los `.STX` del usuario son **byte-idénticos** (`md5`) a los discos conocidos
  del proyecto: `0debece3…` (A) y `b2a0c73d…` (B).

## Desbloqueo de la validación
- Extraídos con `stx.py` + `fat12.py` (copiados a `/tmp`, sin tocar
  `kult-port-legacy`): 16 ficheros de A + 12 de B = **28**; `KULT.PRG` con md5
  `e4fdc6fa…` (el esperado).
- Copiados **aplanados** (sin subcarpetas) a
  `build-win/kult-0.1.0-win64/data/`, junto a los `.STX` originales (inofensivos).
- `CLEAR.BIN` restaurado desde el disco: la copia de `/tmp/kult_data` estaba
  **parcheada** por una partida de pruebas (el juego lo reescribe).

## Segundo hallazgo (log del usuario): ventana solo con KULT_WINDOW=1
- El primer `kult.log` del usuario mostró `fin: frames=733 ... exit=0` y el aviso
  de `out/frame.ppm`: había arrancado en **modo headless** aunque era la build
  SDL, porque `KULT_WINDOW` no estaba definido (`int window = getenv(...) != 0`).
- **Corregido**: en la build con SDL la **ventana es el modo por defecto**
  (doble clic = juego); `KULT_WINDOW=0` fuerza headless (diagnóstico/pruebas).
  En la build sin SDL se ignora y siempre va headless. Documentado en
  `LEEME`/`DIAGNOSTICO`/`BUILD`/`VERIFICACION`/`RETOMAR`.

## Tercer hallazgo: aviso «INSERT DISK 2» al arrancar (0x918e)
- `_start` (`l_0100e0`) emite **siempre** la caja de texto `0x918e`
  ("INSERT DISK 2 IN DRIVE, THEN PRESS ANY KEY"; índice de texto 394) antes de
  cargar `FOND`/`PUZZL`/`BRUIT` (disco B), y espera un **clic** (`prompt_wait`).
  En el original hay que cambiar el disquete físico; en headless lo descarta el
  input automático (por eso no aparecía en los smokes), pero en ventana con
  entrada real dejaba el juego colgado en el aviso (el texto pide una tecla,
  pero se espera el botón del ratón).
- **Fix**: el port sirve ambos discos desde el mismo directorio de datos, así que
  el aviso se **omite** (y el bucle de reintento con `0x918e`). `load_fond` ahora
  comprueba `A_FILE_HANDLE` (FOND ausente → arranque falla, exit 3) y
  `load_resource_list_b` devuelve -1 si faltan `PUZZL`/`BRUIT`. El preflight de
  `app/main.c` sigue listando con claridad los ficheros que falten.
- **Flujo resultante**: título (PRES) visible con música esperando tecla/clic →
  pantalla de idioma (DRAP) + D/E/F → partida, **sin** pedir disco 2. (El título
  se veía negro por falta de paleta; corregido en `notes/0014`.)

## Cuarto hallazgo: se bloquea tras elegir idioma (entrada no bombeada)
- `wait_frame` (0x13698) lee el botón del IKBD por `sched_poll8`, pero **SDL solo
  se bombeaba** en `Cnecin` (idle) y en el bucle (`frame_hook`). En las esperas de
  arranque con `wait_frame` (`prompt_wait` del título y, sobre todo,
  `wait_frame l_010158` tras el idioma) **no se bombeaba ni presentaba**: si el
  botón seguía pulsado (o el release no se había procesado), `l_010158` quedaba en
  bucle infinito y la ventana se quedaba congelada en el último frame (negro).
- **Fix**: `platform_tick` ahora **presenta y bombea SDL en cada VBL del
  arranque** (`platform_set_present`); `boot/` lo desactiva al entrar en
  `game_loop` (`platform_set_loop(1)`), donde ya presenta el frame hook. Así las
  esperas leen ratón/teclado y la pantalla avanza. Prueba: `test_present_hook`
  en `make test-port`. (`platform/time.c`, `platform.h`, `boot/boot.c`,
  `app/main.c`.)

## Log de diagnóstico (M7, para no repetir el silencio)
- **`platform/paths.c`**: `platform_exe_dir()` compartido (fs y log); antes vivía
  dentro de `fs.c`.
- **`platform/log.c`**:
  - `kult.log` **junto al `.exe`** (`KULT_LOG=<ruta>` lo redirige, `KULT_LOG=0`
    lo apaga); fallback `%LOCALAPPDATA%\Kult\kult.log` (Windows) o `/tmp`.
  - Todo va también a stderr (headless).
  - `platform_fatal()`: log + **MessageBox** si no hay consola
    (`GetConsoleWindow()==NULL`), pensado para la build GUI; en consola no
    interpone diálogo.
- **`app/main.c`**:
  - `platform_log_init()` al empezar; registra versión (`KULT_VERSION` de CMake),
    directorios de datos, ruta de `KULT.PRG`, PRG cargado, SDL/ventana/audio,
    frames, dumps y **exit code**.
  - **Fallo de ventana = fatal** (antes seguía en *headless* con `frames=0`,
    invisible).
  - **Preflight** de los 26 ficheros del juego (además del PRG): si falta alguno,
    error claro con la lista y la explicación (ficheros extraídos, no `.STX`).
    Evita el crash opaco con datos incompletos.
- `platform/video_sdl.c` y `platform/audio_sdl.c`: errores de SDL al log.
- CMake: `KULT_VERSION` público; `user32` en Windows para el diálogo. ZIP:
  `kult-headless.exe` + `DIAGNOSTICO.txt`; `LEEME.txt` actualizado.
- Prueba nueva `test_log` en `make test-port`.

## Verificación
- `make test` → **499 passed**; `make test-port` → OK (con `test_log`);
  `make regscan` → `ids_reg=0`.
- `make port`/`port-sdl`, CMake nativo y **cross Windows** (con `user32`), sin
  warnings.
- Smoke: datos buenos → exit 0, log completo; solo `.STX`/datos incompletos →
  exit 2 con lista de ficheros y mensaje; log creado junto al exe.
- CPack: ZIP (`kult.exe`, `kult-headless.exe`, `SDL3.dll`, `data/`,
  `LEEME.txt`, `DIAGNOSTICO.txt`) y TGZ; **sin assets**.

## Pendiente
- **T-061**: el usuario valida en Windows (ya con `data/` extraído y el ZIP
  nuevo). Después, **T-072 — ingesta `.STX` en C**: que el usuario solo tenga que
  copiar sus imágenes de disquete (meta: no pedir nunca un `.PRG`).
