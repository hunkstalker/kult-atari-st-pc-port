# 0039 — Handoff: analizar el final del juego (¿debe volver al título?)

> **Actualización (sesión 50):** análisis cerrado. El original **no vuelve al
> título**: entra en el bucle infinito de `attract` (`0x18c80`) y muestra la
> animación de estrellas. El port retornaba de `attract` y seguía el guion, de
> ahí la escena/paleta corruptas. Fix **T-089** implementado y **validado en
> Windows (2026-10-22)**; detalle y evidencia en
> **`notes/0040-t089-final-attract-loop.md`**. T-061 se cerró y validó en
> Windows en la sesión 53 (2026-10-23).
>
> **Estado original de esta nota:** incidencia abierta de **T-061**
> (validación de Windows). Esta sesión era **solo análisis de divergencia**:
> sin fix. Se conserva el plan y las hipótesis como histórico.

## 1. Objetivo

Explicar por qué, al terminar la partida:

- el original debería volver a la **pantalla de título** con su música principal;
- el port carga una **escena corrupta** y deja la **paleta rota**.

El entregable de la próxima sesión es un informe con:

1. ruta real del original tras la escena final;
2. ruta real del port;
3. **primer punto de divergencia** (PC/función/comando/estado);
4. evidencia (RAM, registros, log, captura puntual);
5. causa probable.

No se implementa ninguna corrección en esta fase.

## 2. Contexto de tareas

- **T-061** es la validación completa de Windows; **cerrada y validada en
  Windows el 2026-10-23** (sesión 53). No crear otra campaña de validación.
- Los fixes anteriores (T-073c, T-076, T-077, T-079…T-088) fueron **incidencias
  hijas** de esa validación.
- **T-073** queda como **feature opcional** de calibración fina no bloqueante.
- **T-072** (ingesta `.STX`) es la siguiente tarea tras cerrar T-061.
- Si el análisis confirma un bug de código, la corrección podrá registrarse como
  **T-089 — final del juego: volver al título**. **No crear T-089 todavía**:
  primero hay que demostrar la divergencia.

## 3. Síntoma y evidencia actual

### 3.1. Reproducción

Bats creados para el endgame (no usan `save-t088`):

- `assets/kult-replay-record-endgame.bat`
- `assets/kult-replay-play-endgame.bat`
- copias en `build-win/kult-0.1.0-win64/`

Restauran `save-endgame\SCENA.BIN` y `CLEAR.BIN` sobre `data/` y lanzan
`kult.exe`. El replay se guarda en `kult-replay-endgame.bin`.

> Los bats antiguos `kult-replay-record.bat` / `kult-replay-play.bat` **no
> sirven para esto**: restauran `save-t088/SCENA.BIN`.

### 3.2. Evidencia preservada

Carpeta:

```text
build-win/kult-0.1.0-win64/trazas-endgame-20261022-125714/
```

Contiene:

- `kult-replay-endgame.bin`
- `kult.log`
- `README.txt`
- `kult-replay-record-endgame.bat`
- `kult-replay-play-endgame.bat`
- `save-endgame/SCENA.BIN`
- `save-endgame/CLEAR.BIN`
- `save-endgame/SHA256SUMS`
- `SHA256SUMS` de toda la carpeta

Hashes principales:

```text
0f3989cf4bb487cd84c6191c692405bfa23c81b288ca5052cf1a62a307368c66  kult-replay-endgame.bin
65046b20f2589202146acdf460bafae7c3dbd3124ccb3b04defda20dd388ea30  kult.log
fc1ea367bc50dbf2a64fac7f1fe241139b5024558cd0000e94c7d2915754fc94  save-endgame/SCENA.BIN
fb53e7d66866041d8057b2f84b4f8fac03c29d9c671951995e528a9ff0404195  save-endgame/CLEAR.BIN
```

No se ha modificado el save original; el manifiesto del save está en
`save-endgame/SHA256SUMS` y se verificó con `sha256sum -c`.

> `build-win/` está **gitignored**: esta carpeta de evidencia no viaja en git.
> Si el nuevo entorno no la tiene, hay que recuperar `save-endgame` +
> `kult-replay-endgame.bin` del workspace original o pedírselos al usuario. No
> versionar los binarios del juego.

### 3.3. Datos del replay

- Tamaño: **41.984 B** = cabecera de 16 B + 2.623 eventos de 16 B.
- Cabecera `KULTREP1`, versión 1.
- Eventos: 1.310 IKBD, 1.311 `Crawio`, 2 `Cnecin`.
- Último clic: **frame 4787** (cursor 154,81; botón izquierdo).
- Último evento: **frame 4837**.
- El replay conserva las respuestas de entrada que ve el juego, no los eventos
  crudos de SDL (ver `notes/0038`).

### 3.4. Log del port

En la grabación, `kult.log` muestra:

```text
[2026-10-22 14:15:14] titulo: clic
[2026-10-22 14:15:15] idioma: E
[2026-10-22 14:15:15] game_loop: inicio
[2026-10-22 14:16:44] reinicio: reset_clear -> 0x10158
[2026-10-22 14:16:51] fin: frames=4838 visible=0x98000 fb_nonzero=63236 exit=0
```

La reproducción posterior arranca desde título/idioma y entra en `game_loop`;
el `kult.log` conservado no contiene un `fin:` de esa reproducción. No confundir
la reproducción incompleta con el fallo: la evidencia clave es que la
grabación de endgame acaba en `reset_clear -> 0x10158`.

`kult.log` es append: las líneas de 13:34–14:08 son runs anteriores, no la
partida de endgame. La run relevante es la de 14:15–14:17. `CLEAR.BIN` no es el
save: el arranque lo reescribe; el estado de sala que importa es `SCENA.BIN`.
Se copian ambos solo para reproducir el entorno.

## 4. Build actual y herramientas

Build de referencia (T-088):

```text
kult.exe               154122ef03b7de69d52e0ac3df95f4138133e68dc5c1b8310d5259ebd58e49d2
kult-headless.exe      70c603bf06488a1f3c077d2f8733b511ea7925c036e70bde6a9d6678894d5696
kult-diag.exe          da7afa3b811d551a462afc6c9605f1d1e0be33ee5bc190cdacdc17b6b1857dd5
kult-diag-headless.exe d889843442ab1a0d72025bf038b1bfa12bcccaef41ccfe8a3fe8b01883f10a4f
ZIP                    76c5ee01d59e7c15cf3f25d5d186d80a08bf5f97a69503ca9d6e58558a8f337f
```

Herramientas disponibles en este entorno:

- Hatari 2.5.0 compilado:
  `/tmp/opencode/hatari-2.5.0/build/src/hatari`
- TOS: `/app/kult-port-legacy/tos/tos.img`
- EmuTOS: `/app/kult-port-legacy/tos/emutos-512k-1.4/etos512us.img`
- Imágenes raw del juego:
  `/app/kult-port-legacy/work/kult_a.img`, `kult_b.img`
- `Xvfb` y `xdotool` disponibles (`/usr/bin/Xvfb`, `/usr/bin/xdotool`).
- El repo legacy ya advierte que los `.stx` montan en Hatari pero no arrancan;
  con GEMDOS/autostart arranca pero el input del ST es el problema. Por eso la
  vía preferente es una **imagen raw writable** y `xdotool`, no `.stx`.
- `tools/fat12.py` del legacy es **solo lectura**; no hay `mcopy`/mtools
  instalado. Para añadir `SCENA.BIN` a una imagen mayor hará falta un escritor
  FAT12 mínimo (o una alternativa equivalente). `mkdosfs` sí está disponible.
- Hatari flags útiles: `cpu_all`, `cpu_disasm`, `cpu_regs`, `os_all`,
  `gemdos`, `fdc`; se pueden combinar en `--trace`.

## 5. Lo que ya sabemos del código (sin conclusiones precipitadas)

### 5.1. Port

- `reset_clear` (`src/fileio.c:236`) reproduce el estado de `0x1682a`: carga
  `CLEAR.BIN`, `$1ad0f=0xff`, `pal_next_byte`, `$1acd8=2`; **no** ejecuta el
  `bra 0x10158` del original.
- El `bra 0x10158` se emula en el hook de salida de instrumentación:
  `platform/cpuwork.c:596` llama a `kult_request_restart()`.
- `kult_request_restart()` hace `longjmp` (`boot/boot.c:62`) y el bucle de
  `kult_boot` entra en `boot_enter_game()` (`boot/boot.c:73`), que reproduce
  `0x10158`: espera clic, modo 7, comando `0xc001` (Maestro/objeto), hints,
  cursor y `game_loop`.
- `prepare_room` (`src/loop.c:1242`) termina llamando a `reset_clear`.
- El camino de título real (`PRES.PI1` / `DRAP.PI1` / menú / música) está solo
  en `kult_boot`, la secuencia de arranque en frío (`boot/boot.c:117` y
  siguientes). **No existe una rutina de “volver al título” desde el juego.**

### 5.2. Original (análisis estático inicial)

- `0x1682a` (`reset_clear`) termina con `bra.w 0x10158`; no vuelve al llamante.
- `0x10158` fija modo 7, limpia `$1ad0d`, ejecuta `0xc001` (Maestro/objeto),
  pinta hints y entra en `game_loop`. **No es el título**: es el inicio de una
  partida nueva.
- `0x101a8` (`l_0101a8`) llama a `0x106f2` (lista C), reinicia pila/crt0, carga
  lista A y salta a `0x100ec`; tampoco pasa por `PRES/DRAP`.
- Barrido estático del PRG:
  - no aparece un `jsr`/`jmp` absoluto directo a `0x10000`, `0x101a8` ni
    `0x106f2`;
  - los punteros a `PRES` (`0x1a427`) y `DRAP` (`0x1a430`) solo aparecen en
    `_start` (`0x10042` y `0x1006c`);
  - `0x101a8` solo aparece como constante en `hint_table_relocate`
    (`0x16f90`) y no como destino de salto.
- Por tanto, **no está demostrado** que el original vuelva al título mediante
  `0x106f2`/`0x101a8`. T-022 (`0x106f2`) sigue abierta, pero no debe usarse como
  explicación sin evidencia.

### 5.3. Hipótesis a contrastar

1. **El final original usa una ruta que el port no modela** (otro comando,
   salto indirecto, reinicio o recarga de título).
2. **El original también usa `reset_clear`**, pero el estado/paleta que deja el
   final es distinto; el port se queda con la paleta/escena final.
3. **El comando final se despacha mal** y acaba en `prepare_room`/`reset_clear`
   en el port, mientras el original hace otra cosa.

No elegir una hipótesis antes de tracear el original.

## 6. Plan de análisis para la próxima sesión

### Paso 0 — Verificar evidencia

```sh
cd /app/kult-port-pc-compilation/build-win/kult-0.1.0-win64/trazas-endgame-20261022-125714
sha256sum -c SHA256SUMS
```

No sobrescribir `save-endgame`; trabajar siempre sobre copias en `/tmp/opencode/`.

### Paso 1 — Ejecutar el original en Hatari

Objetivo: ver qué hace el original tras la escena final.

Consideraciones:

- El juego usa rutas `A:`; un GEMDOS HD en `C:` puede no servir.
- `kult_a.img` tiene solo **40.960 B libres** y `SCENA.BIN` ocupa **74.524 B**:
  no cabe en la imagen A original.
- Opciones:
  1. Crear una imagen raw FAT12 mayor (p. ej. 720 KB) con todos los ficheros de
     `disk_a` + `SCENA.BIN`/`CLEAR.BIN`, y usar la B original o una copia.
  2. Probar primero GEMDOS HD y comprobar si el juego resuelve `A:`; si no,
     descartarlo.
  3. Fallback: usar el **oráculo Musashi** del repo core, que permite
     `stage_files` y ejecutar el binario original sin las limitaciones de
     disquete. Es más preciso para traza de PC.
- No modificar las imágenes originales de `/app/kult-port-legacy/work/`; copiar
  a `/tmp/opencode/`.

Con Hatari:

- `--tos`/EmuTOS, `--harddrive` o `--disk-a`, `--protect-floppy` según el caso.
- `--log-file`, `--trace-file`.
- `--trace cpu_all`/`cpu_disasm` si el volumen es asumible; mejor breakpoints.
- Breakpoints con `:trace` en:
  - `0x10000` (`_start`)
  - `0x101a8` (reinicio suave)
  - `0x106f2` (lista C)
  - `0x1682a` (`reset_clear`)
  - `0x10158` (inicio de partida)
- En cada breakpoint, usar `savebin` para volcar RAM (1 MiB o rangos
  relevantes), `cpureg`, `profile save` y `screenshot` si hace falta.
- `statesave` antes/después del final para conservar estados.
- `history` para los últimos PCs.
- Input: `--cmd-fifo` soporta `keypress`, `doubleclick`, `rightdown/rightup`;
  para mover el ratón y hacer clic izquierdo, usar `xdotool` bajo `Xvfb`.
- No hace falta AVI.

Resultado esperado: saber si el original llega a `0x10000`/título, a
`0x1682a`/reinicio, o a otra ruta, y con qué estado de paleta/pantalla.

### Paso 2 — Trazar el port

- Restaurar `save-endgame` con la copia de la carpeta de evidencia.
- Ejecutar **`kult-diag.exe`** con el replay, no `kult.exe`, para tener trazas:

```bat
set KULT_REPLAY_PLAY=%~dp0kult-replay-endgame.bin
set KULT_MARKS=%~dp0kult-marcas-endgame.log
set KULT_RINGLOG=%~dp0kult-ring-endgame.log
set KULT_INPUTLOG=%~dp0kult-input-endgame.log
set KULT_DBG_RUN=1
set KULT_DBG_CMDS=1
set KULT_LOG=%~dp0kult-endgame.log
"%~dp0kult-diag.exe"
```

Ajustar nombres/rutas si se ejecuta desde otra carpeta. `kult-diag.exe` y
`kult-diag-headless.exe` están en el paquete.

- Comparar el final del log con `reset_clear`, `prepare_room`, `the_command`,
  `A6`, `run_command`, `load_room`/`prepare_room`.
- Guardar todo en `trazas-endgame-<fecha>/` con `SHA256SUMS`.

### Paso 3 — Comparar y localizar la divergencia

Buscar el **primer** punto donde original y port difieren:

- PC/función ejecutada;
- comando/script (`the_command`, `A6`, puntero de plantilla);
- llamada a `prepare_room` o `reset_clear`;
- estado de paleta y `FOND.PI1`;
- buffers de pantalla y `CLEAR.BIN`.

No basta con “se ve corrupto”: hay que identificar la instrucción/comando que
cambia el flujo.

### Paso 4 — Entregable (sin fix)

Informe en `notes/0039` o nota nueva:

- comportamiento esperado del original;
- ruta real del port;
- primera divergencia;
- causa probable;
- evidencias y hashes;
- recomendación de fix, **sin implementarlo**.

## 7. Trampas y falsos atajos

- **No confundir T-022/`0x106f2` con el final**: no hay referencia directa
  estática y `0x101a8` no lleva al título; no cerrar T-022 por este bug sin
  evidencia.
- **`reset_clear` no es “volver al título”**: es el reinicio de partida que ya
  se validó para muerte (T-076).
- **No usar `save-t088`** para esta prueba; el save correcto es `save-endgame`.
- **No usar `kult-replay-record.bat`/`play.bat` antiguos**: restauran T-088.
- **No modificar los saves originales**; copiar y verificar hashes.
- **No hacer AVI**: `savebin`/`memdump`/`statesave`/`profile`/breakpoints dan
  más información y menos ruido.
- **No implementar el fix** hasta que el usuario valide el informe de
  divergencia.

## 8. Primeros comandos de la próxima sesión

```sh
cd /app/kult-port-pc-compilation
git status --short
git log --oneline -3
cat notes/0039-handoff-endgame-titulo.md
(cd build-win/kult-0.1.0-win64/trazas-endgame-20261022-125714 && sha256sum -c SHA256SUMS)
```

Después: montar la referencia en Hatari y ejecutar el replay de endgame en
`kult-diag.exe`. No tocar `src/` ni `boot/` hasta tener el informe.

## 9. Referencias

- `notes/0038-replay-input-t088.md` — formato y uso del replay.
- `notes/0022-reinicio-tras-muerte.md` — `reset_clear`/`0x10158` y T-076.
- `notes/0010-t020-menu-fiel.md` y `notes/0011-plan-hitos-m7-ingesta-n3.md` —
  `0x106f2`/T-022 y por qué estaba abierto.
- `docs/VERIFICACION.md` — checklist de T-061.
- `docs/BUILD.md` — variables de diagnóstico (`KULT_MARKS`, `KULT_RINGLOG`,
  `KULT_INPUTLOG`, `KULT_DBG_RUN`, `KULT_DBG_CMDS`).
- `src/fileio.c`, `src/loop.c`, `platform/cpuwork.c`, `boot/boot.c` — camino de
  `reset_clear` y arranque.
- `build-win/kult-0.1.0-win64/trazas-endgame-20261022-125714/` — evidencia
  preservada.
