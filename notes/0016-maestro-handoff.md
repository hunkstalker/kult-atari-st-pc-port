# 0016 — T-073c: retrato del Maestro — sesión 16 (handoff completo)

> Punto de entrada para retomar **sin contexto previo**. Complementa
> `RETOMAR.md` (resumen y estado) y `notes/0015-ritmo-cpu.md` (modelo de ritmo CPU). Todo en
> español. `src/` sigue **intacto**; las modificaciones de esta sesión son de
> `platform/`, `assets/`, `Makefile`, `CMakeLists.txt` y docs.

## 0. TL;DR (estado exacto)

- Objetivo: port nativo de **Kult** (Atari ST + Amiga) en
  `/app/kult-port-pc-compilation`. El juego ya arranca, se ve, suena y es
  jugable. Queda **T-073c**: el **retrato/brazos del Maestro** se ven
  acelerados respecto al original.
- **Descartado**: no es la puerta de bancos (`KULT_FRAME_GATE`). El ASM
  `0x16582`/`0x16636` demuestra que los 4 bancos (`6x37/6x35`) son **4
  objetos/partes** actualizados en una sola llamada, igual que el original. La
  puerta 10 solo espacia la construcción en el **back buffer**; el frame visible
  no cambia (el usuario lo confirmó: seguía acelerado).
- **Confirmado con trazas**:
  - Los **3 primeros retratos** usan `reveal_pixel` (`byte 0x0d` →
    `12f28`) y **no se han tocado**: VBL 22/119/211 (deltas 97/92) en la run
    nueva frente a 29/127/219 (98/92) en la anterior; `F 12f28` en los mismos
    VBL; `S 15da6` del intro = 63 en ambas.
  - El **Maestro** entra por `0x36 scene_mode_draw` (`0x14602`/`0x1460a`), cuyo
    operando de guion es **0x07** (leído en la imagen PRG en `0x1d41c`). No
    llama a los reveals de tipo 4 (`12e2e/12e08/12ea2/12fb6`); los blits
    `12ad6/12b04` (16+16 en ~60 VBL) son el dibujo de elementos/retrato, no el
    reveal.
  - Estado en el Maestro (línea `V`): `$1acc9=0x0a`, `$1acd2=0x02`,
    `$1aaf9=0x63`, `$1ad1d=0`, modo `0x07`, skip `0`, botón `0x04`.
    `anim_mode` (`0x188ce`) solo anima si `$1aaf9==$1acd2` y el byte de modo
    está en `0x3b..0x3e`; no entra. `icon_repeat` (`0x18944`) retorna si
    `$1ad1d==0`; por eso **no se llama `15da6`** (motor de elementos/habla) en el
    Maestro. El `$1acc9` se fija **después** del `scene_mode_draw` (byte
    `0x52`), y `$1aaf9` solo lo actualiza `icon_repeat` (que no corre):
    deadlock; el original debe llegar a un **segundo scene draw** o a otro
    camino tras la selección. Pendiente localizarlo.
- **Bug extra detectado**: `A_INPUT_BUTTONS` bit2 (`0x04`) estaba **pegado**
  desde VBL 2, sin clic real (click fantasma). Corregido en `platform/input.c`
  (sincroniza el bit con el paquete IKBD real) y `platform/video_sdl.c`
  (refresca `g_mouse_btn` con `SDL_GetMouseState`). **El usuario aún no ha
  probado este fix**; el exe de traza reconstruido es md5
  **`3fc2f940e11db072f1d0ce65785158ee`**.
- **Build de diagnóstico**: `platform/dispatch_trace.c` (copia port-side de
  `src/dispatch.c`) sustituye a `src/dispatch.c` en el port y escribe `Y`
  (bytes VM) y `V` (estado). **Hay que revertir esto** antes de generar el ZIP
  final. El ZIP estándar `build-win/kult-0.1.0-win64.zip` **no** se ha
  regenerado en esta sesión (sigue siendo el build no-trace anterior). La
  carpeta del usuario `build-win/kult-0.1.0-win64/` tiene el exe de traza.

## 0.1. CORRECCIÓN (sesión 18) — la sesión 17 se rechaza

- La sesión 17 cambió el mapeo de botones para que el **derecho** saltara las
  transiciones/reveal. **Eso se ha revertido**: se eliminó
  `platform/input_core.c`, el port vuelve a compilar `src/input.c` y el **clic
  izquierdo** sigue saltando transiciones/reveal. El **botón derecho queda sin
  función** (decisión explícita del usuario). No volver a proponer el modelo del
  botón derecho del legacy: `docs/RUNTIME.md` avisa de que es del oráculo y **no
  autoriza** a cambiar el port sin una petición.
- Se conserva el **fix de click pegado** (bit2 de `A_INPUT_BUTTONS` sincronizado
  con el botón real; refresco con `SDL_GetMouseState`) y el hallazgo útil de la
  sesión 17: los brazos del Maestro aparecen **después del primer prompt**; el
  primer `0x36 1d41b` es el **scene draw inicial**, no el retrato hablado.
- El exe `296ce6c689c9237097cfb5f454eaf787` y la prueba del botón derecho quedan
  **invalidados**. El build estándar sin traza debe reconstruirse con el clic
  izquierdo. La traza `Y`/`V` es diagnóstica: si se necesita, reactivarla en una
  build aparte (filtros de la sesión 16).
- **T-073c sigue abierto**: el `15da6`/segundo scene draw no está identificado.
  La próxima run debe avanzar el diálogo hasta los brazos y pulsar F12
  **después**, no en el primer prompt.

## 1. Reglas y decisiones que NO se deben romper

- `src/` **intacto** (core diferencial). Lo específico de máquina va en
  `platform/`, `boot/`, `app/`; los parches de port se hacen por sombra de
  cabeceras (`platform/include` primero) o copias port-side justificadas.
- **No tocar** `/app/kult-port-legacy` ni `/app/kult-atari-st-decomp` (solo
  lectura/referencia). Los discos/imágenes del usuario no se modifican.
- Todo en **español**; mantener `RETOMAR.md`, `TODO.md`, `PROYECTO.md` y
  `notes/` al día. **No hacer commits** salvo petición explícita.
- Decisiones cerradas: SDL3; `SDL_Renderer` base + `SDL_GPU` para CRT; audio en
  hito propio; menú de idioma fiel; `boot/` fuera de `src/`; headless primero.
- **Input (corregido en sesión 18)**: el **clic izquierdo** interactúa (bit2) y
  **salta** las transiciones/reveal (`$1a570`), como decidió el usuario; el
  **botón derecho queda sin función**. Se conserva el fix de click pegado. **No
  cambiar este mapeo**: el doc legacy `docs/RUNTIME.md` es del oráculo y avisa de
  que no autoriza cambios en el port.
- El objetivo de velocidad es el original (oráculo/Hatari); **no** ajustar
  animaciones a mano sin medida.

## 2. Entorno, datos y builds

- Proyecto: `/app/kult-port-pc-compilation`.
- Core: `/app/kult-atari-st-c` (415 funciones, **499** tests diferenciales).
- Legacy/oráculo (solo lectura): `/app/kult-port-legacy`;
  decomp ASM: `/app/kult-atari-st-decomp`.
- Datos del usuario extraídos de sus `.STX`:
  `build-win/kult-0.1.0-win64/data/` (28 ficheros; `KULT.PRG` md5
  `e4fdc6fa943afdee74bc2fbdfb19657e`; `CLEAR.BIN` original restaurado).
- **Imagen PRG**: base de carga **0x10000**; para leer un byte del guion:
  `file_offset = 0x1c + (addr - 0x10000)` (comprobado con `0x1d40f..`).
  Ejemplo: `0x1d41c` está en `KULT.PRG` y vale **0x07**.
- Builds:
  - `make test` → 499 diferenciales (verde; 35 s).
  - `make test-port` → HAL (verde: fs, log, present, cpu_delay, vídeo, entrada,
    reloj, tick VBL, loader PRG, idioma, música PSG, Timer A, RMS).
  - `make cmake-native` / `make cmake-win` → `build/` / `build-win/`.
  - `make package-win` / `make package-native` → ZIP/TGZ.
  - Makefile: `make port` (`build/kult` headless), `make port-sdl`
    (`build/kult-sdl`).
  - SDL3 3.4.10 MinGW en `third_party/` (gitignored).
- Windows: `kult.exe` = GUI (`kult-sdl`), `kult-headless.exe` = headless.
  El ZIP estándar lleva `kult.exe`, `kult-headless.exe`, `SDL3.dll`, `data/`,
  `LEEME.txt`, `DIAGNOSTICO.txt`.
- Logs del usuario:
  - `build-win/kult-0.1.0-win64/kult.log` (arranque/título/idioma/F12/FPS).
  - `build-win/kult-0.1.0-win64/kult-marcas.log` (**append**, con `# RUN`; run
    normal `Y`/`V`).
  - `build-win/kult-0.1.0-win64/kult-frame10.log` (experimento de puerta 10).
  - Copias de evidencia en `temp/` (gitignored).
- Lanzadores en la carpeta del usuario:
  - `kult-marcas.bat`: `KULT_FRAME_GATE=0`, `KULT_MARKS=...kult-marcas.log`,
    `KULT_VBL_TITLE=1`, ejecuta `kult.exe` (traza).
  - `kult-frame10.bat`: `KULT_FRAME_GATE=10` (experimento; **no** usar para la
    run normal).
- Claves/entorno relevantes: `KULT_CPU=0`, `KULT_CPU_HZ` (8 MHz),
  `KULT_ST_CONTEND`, `KULT_ANIM_GATE` (100), `KULT_REVEAL_GATE` (100),
  `KULT_FRAME_GATE` (0), `KULT_15DA6_BLIT=0`, `KULT_FPSLOG`, `KULT_MARKS`,
  `KULT_VBL_TITLE`, `KULT_WINDOW=0`, `KULT_DATA` (el binario busca `data/` junto al exe).

## 3. Cronología de esta sesión (qué se probó y qué se descartó)

1. **Punto de partida**: el Maestro cargaba 4 bancos `15ac8` (6x37/35) en ~3 VBL
   y los blits `12ad6/12b04` se veían; no había llamadas a los reveals de tipo 4
   (`12e2e/12e08/12ea2/12fb6`); `K=0` (el flag de skip `$1a570` no cambiaba).
   El usuario veía los brazos acelerados.
2. **Traza de bytes `Y`** (copia `platform/dispatch_trace.c`; exe Y-only md5
   `24729db9ad8b414c3ae6197174713d5d`): por cada byte de la VM escribe
   `Y <vbl> <byte> <script>`. Resultado: los reveals `0x0d` **sí** se ejecutan
   en los 3 primeros retratos (VBL 42/141/233 en esa run); el Maestro ejecuta
   `0x36` (`scene_mode_draw`) y luego `0x15/0x52/0x3b...`; no hay `0x0d` en el
   Maestro.
3. **Experimento `KULT_FRAME_GATE`**: se añadió/verificó la puerta en
   `platform/include/kult_port.h` (`frame_gate_delay`, llamada desde
   `anim_bank` y `anim_bank_build`). Prueba nativa:
   - gate 0: los 4 bancos en el mismo VBL;
   - gate 10: bancos a 10 VBL exactos (17/27/37/47).
   Run del usuario (`kult-frame10.log`): bancos del Maestro en VBL
   1365/1375/1385/1395, `scene_mode_draw` 1352→1448 (96 VBL), pero **el retrato
   seguía acelerado**. **Conclusión: la puerta solo retrasa la construcción en
   el back buffer; no cambia el frame visible. Descartada como fix.**
4. **ASM original** (`/app/kult-atari-st-decomp/matching/src/`):
   - `auto_016582.s` (`update_icon_list`): recorre 0x29 entradas de la tabla
     `0x1aa72`, y para cada entrada activa de `$1acd2` con bit6 limpio busca el
     primer objeto de la lista con la clase `(entry[3]&7)|0x10`, lo marca y, si
     el objeto tiene bit6, llama a `0x16636`.
   - `auto_016636.s` (`obj_list_iter`): incrementa el contador `$1a57d` **por
     objeto**, selecciona la clase, carga el banco `0x15ac8` y blitea al back
     buffer (`-0x8000`).
   - **El C `src/loop.c` coincide con el ASM**; los 4 bancos son 4
     objetos/partes, no 4 frames temporales. No es un bug de la puerta ni del
     contador.
5. **Lectura del PRG**: el guion del Maestro en `0x1d41b`:
   `36 07 15 52 3b 3b 3b 3b 3c 64 06 37 ...` (direcciones 1d41b..1d4cf; ver §4).
   El operando de `0x36` es **0x07** (modo de escena). El script no vuelve a
   ejecutar `0x36`; el bucle de texto usa `0x37/0x3c/0x53/0x29/0x18/0x03/0x2e`.
6. **ASM del modo/selección**:
   - `auto_0188ce.s` (`anim_mode`): exige `$1aaf9 == $1acd2` y byte
     `0x3b..0x3e`; si entra, pone `$1ad1d=0xff` y llama `obj_anim(0x19d24)` /
     `obj_anim(0x19d2a)`.
   - `auto_018944.s` (`icon_repeat`): **retorna si `$1ad1d==0`**; si no,
     `obj_anim(0x19d21/0x19d27)`, fija `$1aaf9=$1acd2` y llama
     `update_icon_list(0x16582)`.
   - `auto_01460a.s` (`scene_mode_apply`): llama `anim_mode(0x188ce)` **antes**
     de `scene_mode_set(0x1443e)`; después fija `$1acfe`, selecciona el objeto
     (`$1acc9`), repinta texto/iconos y llama `update_icon_list`.
   - `auto_0146d2.s` (cuerpo): hint blits; si `$1a572==0` y skip 0 →
     `0x15934` (lift/copy_rows); `copy_screen`; sonido; `elem_mode_blit`;
     paleta; `0x1857c`; `icon_repeat`; `textform_rev` si `$1acc9`;
     `draw_hint_icons`.
7. **Traza de estado `V`**: se añadió `platform_marks_state` (en
   `platform/cpuwork.c` + `platform/platform.h`) y la llamada en la copia
   `platform/dispatch_trace.c`. Formato:
   `V <vbl> <byte> <script> <acc9> <acd2> <aaf9> <ad1d> <acc6> <skip> <a572> <a0f> <cx> <cy> <btn> <cmd> <obj>`.
   Run del usuario (exe `13a81df4…`, sin el fix de entrada): ver §4.
8. **Bug de entrada**: `btn=0x04` en **todas** las `V`, desde VBL 2. Se
   corrigió:
   - `platform/input.c` `platform_ikbd_deliver`: sincroniza
     `A_INPUT_BUTTONS` bit2 con el botón del paquete IKBD (`b&1`) cada entrega
     (clear en release).
   - `platform/video_sdl.c` `platform_sdl_pump`: refresca `g_mouse_btn` con
     `SDL_GetMouseState` (`SDL_BUTTON_LMASK`) al final de cada bombeo, para que
     un `MOUSE_BUTTON_UP` perdido no deje el click pegado.
   - Rebuild de la traza Windows: md5 **`3fc2f940e11db072f1d0ce65785158ee`**;
     `make test` 499 y `make test-port` OK. **Pendiente de probar por el
     usuario**.

## 4. Evidencia concreta (para no repetir trabajo)

### 4.1. Timings de los otros reveals (respuesta a “¿has cambiado otros?”)

- Run normal nueva (`V`, gate 0): `Y 22 0d 23df5`, `Y 119 0d 23dff`,
  `Y 211 0d 23e08`; `F 12f28 22/119/211`; deltas **97/92**.
- Run normal anterior (gate 0): `Y 29 0d 23df5`, `Y 127 0d 23dff`,
  `Y 219 0d 23e08`; deltas **98/92**.
- `S 15da6` del intro: **63** líneas en ambas.
- El `KULT_FRAME_GATE=10` solo afectó a bancos (`B 15abe` y `B 15ac8`); los
  reveals (`0x0d`/`12f28`) no usan esa puerta. `KULT_REVEAL_GATE` y
  `KULT_ANIM_GATE` siguen a **100**; el bat normal vuelve a `KULT_FRAME_GATE=0`.
- **Conclusión**: no se ha cambiado la velocidad de ningún otro reveal. La
  diferencia de VBL absolutos (7 VBL) es el desfase inicial de la run; la
  cadencia relativa es idéntica.

### 4.2. `V` del Maestro (run 10, `kult-marcas.log`)

Antes del `0x36` (V 1095 `36 1d41b`):
```
acc9=00 acd2=00 aaf9=63 ad1d=00 acc6=07 skip=00 a572=00 a0f=ff
cx=0017 cy=003f btn=04 cmd=c001 obj=00000
```
Tras `0x36` (V 1154 `15 1d41d`):
```
acc9=00 acd2=02 aaf9=63 ad1d=00 acc6=07 skip=00 a572=00 a0f=00
cx=0017 cy=003f btn=04 cmd=c001 obj=00000
```
Tras `0x52` (V 1154 `52 1d420`):
```
acc9=0a acd2=02 aaf9=63 ad1d=00 acc6=07 skip=00 a572=00 a0f=00
cx=0017 cy=003f btn=04 cmd=c001 obj=1b08a
```
Interpretación:
- El `0x36` es el `scene_mode_draw`; el modo es `0x07`.
- `anim_mode` corre **antes** de cargar la escena: `acc9=0`, `acd2=0`,
  `aaf9=0x63`; no coincide (`0x63 != 0`), así que no pone `$1ad1d`.
- La escena carga `acd2=0x02`; el `0x52` posterior fija `acc9=0x0a` y el objeto
  seleccionado `0x1b08a`, pero ya es tarde para ese `anim_mode`.
- `icon_repeat` (al final del cuerpo) ve `ad1d=0` y retorna; por eso `aaf9`
  sigue `0x63` y **nunca** se llama `15da6`. Sin `15da6` no está el motor de
  elementos/habla; los 4 bancos y los blits de elementos son el camino que se
  ve.
- El botón `0x04` pegado es un bug aparte (ver §3.8); puede afectar a
  esperas/prompts, pero los reveals `0x0d` son independientes del botón y no
  cambiaron.

### 4.3. Guion del Maestro en la imagen PRG

Base de imagen **0x10000**; `file_offset = 0x1c + addr - 0x10000`.
Bytes relevantes (comprobados contra `KULT.PRG`):
```
1d40f: 3b
1d415: 3b
1d41b: 36   (scene_mode_draw)
1d41c: 07   (modo de escena; NO es 0x3b..0x3e)
1d41d: 15
1d420: 52
1d421: 3b
1d428: 3b
1d42f: 3b
1d436: 3b
1d43d: 3c
1d446: 64
1d447: 06
1d44b: 37
1d451: 3c
1d472: 3c
1d47b: 53
1d47f: 29
1d484: 3b
1d48a: 3b
1d490: 33
1d4c9: 03
1d4cb: 18
1d4cf: 2e
```
No hay `0x38` (motor de elementos) ni `0x0d`/`0x20`/`0x54` (reveals) en el
tramo del Maestro. Los primeros retratos usan el guion `0x23df5/0x23dff/0x23e08`
(`0x0d` → `12f28`).

### 4.4. Bancos del Maestro

`B` de la run V (gate 0):
```
B 1099 15ac8 16636 6 37 d7b6a50f 37373700 0
B 1099 15ac8 16636 6 35 ffb5e22f 37373700 0
B 1100 15ac8 16636 6 35 6c83aaa7 37373700 0
B 1101 15ac8 16636 6 35 ffb5e22f 37373700 0
```
Con gate 10: 1365/1375/1385/1395 (10 VBL) y el `scene_mode_draw` de 59 a
96 VBL, sin cambio visual perceptible en el retrato (el usuario seguía viéndolo
acelerado).

### 4.5. Ventana F12 del Maestro (run V, VBL ~1500–1560)

Funciones más frecuentes en `F`: `12b04` (16), `12ad6` (16), `12b40` (4665),
`124da`/`12510` (8), `12950/1296c/129c8` (8), `10b24/10f8e/11050` (3), etc.
No aparecen `15da6`/`15a90`/`15cd8` (motor de elementos) ni `12e2e/12e08/12ea2/
12fb6` (reveals de tipo 4) en el Maestro. Los `12b04/12ad6` son los blits de
elementos/retrato, no el reveal.

### 4.6. Run del experimento de puerta (referencia)

`build-win/kult-0.1.0-win64/kult-frame10.log`: M f12 1560; bancos
1365/1375/1385/1395; `scene_mode_draw` 1352→1448 (96 VBL); Y del scene draw:
`Y 1352 36 1d41b`, siguiente `Y 1448 15 1d41d`. El usuario confirmó que el
retrato **seguía acelerado**.

## 5. Estado del código y del build (LEER ANTES DE TOCAR)

### 5.1. Build de diagnóstico actual (hay que revertir)

- `platform/dispatch_trace.c` es una **copia** de `src/dispatch.c` con:
  - `#include "platform.h"`;
  - en `byte_handler`: `platform_marks_byte(...)` (línea `Y`) y
    `platform_marks_state(...)` (línea `V`).
- `Makefile`:
  - `PORT_CORE_SRC` **excluye** `src/dispatch.c`;
  - `PLATFORM_TEST_SRC` **excluye** `platform/dispatch_trace.c` (y
    `platform/cpuwork.c`) para el test-port.
- `CMakeLists.txt`: quita `src/dispatch.c` de `KULT_CORE_SRC` (el glob recoge
  `platform/dispatch_trace.c`).
- `platform/cpuwork.c` + `platform/platform.h`: función/declaración
  `platform_marks_state` (solo la usa la copia diagnóstica).
- **Para el build normal**: eliminar `platform/dispatch_trace.c`; revertir los
  filtros de `Makefile`/`CMakeLists.txt`; opcionalmente quitar
  `platform_marks_state` (o dejarla sin uso); reconstruir y **regenerar el ZIP
  estándar**.
- La carpeta del usuario tiene el exe de traza con el fix de entrada
  (`3fc2f940…`). El ZIP estándar **no** se ha regenerado y sigue con el build
  no-trace anterior.

### 5.2. Cambios de esta sesión (resumen)

- **Diagnóstico** (revertir): `platform/dispatch_trace.c`, `Y`/`V` en
  `byte_handler`, `platform_marks_state` en `platform/cpuwork.c`/`platform.h`,
  filtros de Makefile/CMake, bats `kult-marcas.bat`/`kult-frame10.bat` en
  `assets/`.
- **Fix de entrada** (permanente candidato): `platform/input.c` (sincroniza
  `A_INPUT_BUTTONS` bit2 con el IKBD) y `platform/video_sdl.c` (refresca
  `g_mouse_btn` con `SDL_GetMouseState`). Compila y `test-port` pasa.
- **Nada en `src/`**.
- Docs: este `notes/0016`, `RETOMAR.md`, `TODO.md`, `PROYECTO.md` y una nota en
  `notes/0015`.

### 5.3. Tests/builds verificados en esta sesión

- `make test` → **499 passed** (35,03 s).
- `make test-port` → **OK** (incluye entrada/IKBD y reloj).
- `make cmake-win` → OK con la traza + fix de entrada;
  `build-win/kult.exe` md5 **`3fc2f940e11db072f1d0ce65785158ee`**.
- El build nativo `build/kult`/`build/kult-sdl` no se reconstruyó tras el fix
  de entrada; `make port`/`make port-sdl` lo harán.
- La build estándar (sin traza) **no** se ha regenerado.

### 5.4. Md5s útiles de la sesión

- `24729db9ad8b414c3ae6197174713d5d` — traza Y-only.
- `13a81df4fdf767c8b89c58ccca6bc8ca` — traza Y+V sin fix de entrada (run V del
  usuario).
- `3fc2f940e11db072f1d0ce65785158ee` — traza Y+V **con fix de entrada** (actual
  en `build-win/kult-0.1.0-win64/kult.exe`).
- `KULT.PRG` `e4fdc6fa943afdee74bc2fbdfb19657e`.

## 6. Hipótesis abiertas (orden de ataque)

1. **Click fantasma (fix ya hecho, sin probar)**: `A_INPUT_BUTTONS` bit2 `0x04`
   desde VBL 2. Puede acelerar prompts/textos/esperas y dar la sensación de
   brazos rápidos. Probar el nuevo exe (`3fc2…`) con `kult-marcas.bat`, sin
   mantener el botón pulsado; comprobar en `V` que `btn=00` salvo clics reales.
   Si el retrato se normaliza, cerrar T-073c con este fix.
2. **Camino `15da6`/icono seleccionado**: el `anim_mode` no entra (modo `0x07`,
   `aaf9=0x63` vs `acd2=0x00/0x02`) y el `icon_repeat` retorna (`ad1d=0`). El
   `$1acc9` se fija **después** del `scene_mode_draw`. Hay que localizar el
   **segundo scene draw**/camino del original tras la selección (¿`0x14602` desde
   el bucle de texto/prompt/click? ¿`0x13f86`/`0x13f56`? ¿`0x16460`?).
   Comparar con el oráculo legacy (`$1aaf9/$1acd2/$1acc9/$1ad1d` con
   `KULT_ANIMLOG`; `docs/ANIMACIONES.md` §“Arnés de medida 1:1”).
   - Comprobar si el **id de escena** del Maestro debería ser `0x63` (el
     `aaf9` inicial) y el port carga `0x02` por un mapeo/offset de escena
     incorrecto (`scene_mode_set`/`scene_mode_load`).
3. **Reveal tipo 4**: el Maestro no llama `12e2e/12e08/12ea2/12fb6`; los 3
   primeros sí usan `12f28`. El legacy dice “blits + reveal, un frame por
   reveal”, pero el Maestro está **APARCADO** y el oráculo se atascaba en VBL
   1100. No añadir retardos a mano sin identificar el camino/caller original.
4. **Percepción vs 1:1**: `docs/ANIMACIONES.md` midió el retrato del Maestro a
   ~44/44 cambios/s **1:1** con clics humanos densos, y el usuario lo ve rápido;
   el momento exacto está pendiente. Si el original con los mismos clics también
   va rápido, el port es fiel; hace falta vídeo/log de entrada del original para
   decidir.

## 7. Próximos pasos concretos (para la sesión siguiente)

1. Pedir al usuario una run con el exe **`3fc2f940…`** y `kult-marcas.bat`
   (gate 0), **sin mantener el botón del ratón**; llegar al Maestro, F12, cerrar
   y enviar `kult-marcas.log`. Revisar:
   - que las `V` del Maestro tengan `btn=00` (o `04` solo en clics reales);
   - si el retrato ya no va acelerado (fix de entrada);
   - si sigue, comparar `V`/`Y`/`B` con la run 10.
2. Si sigue: localizar el **segundo `scene_mode_draw`/`0x14602`** del original
   tras la selección (`$1acc9=0x0a`) y el setter de `$1aaf9`; probar un
   experimento port-side controlado (sin tocar `src/`) que fuerce el segundo
   draw **solo** para el Maestro, y ver si aparece `15da6`/`S`. No dejarlo como
   fix si no está justificado por el ASM/oráculo.
3. Comparar el **id de escena** (`$1acd2`) y el mapeo de `scene_mode_set` con el
   original; el `aaf9=0x63` inicial es sospechoso.
4. Si se identifica el fix real: aplicarlo en `platform/` (o core si el usuario
   autoriza, pero la regla es `src/` intacto), **revertir la traza**
   (`platform/dispatch_trace.c`, Makefile, CMake, `platform_marks_state`),
   reconstruir la build normal, regenerar el ZIP estándar y actualizar
   `RETOMAR.md`/`TODO.md`/`notes/0015`.
5. No tocar `KULT_FRAME_GATE` como fix del Maestro; dejarlo en 0. La puerta solo
   afecta a la construcción de bancos en el back buffer.

## 8. Runbook rápido

```sh
cd /app/kult-port-pc-compilation
make test                 # 499
make test-port            # HAL
make cmake-win            # build-win/kult.exe (traza actual)
make cmake-native         # build/{kult,kult-sdl}
make package-win          # ZIP (¡solo cuando NO haya traza!)
make package-native
```

- Datos de prueba en Linux: `build/data/` (copia de
  `build-win/kult-0.1.0-win64/data`); el binario busca `data/` junto al exe.
- Para una prueba nativa de la puerta: `KULT_MARKS=/tmp/g10.log
  KULT_FRAME_GATE=10 ./build/kult` y mirar `B`.
- La VM/traza: `Y <vbl> <byte> <script>`;
  `V <vbl> <byte> <script> <acc9> <acd2> <aaf9> <ad1d> <acc6> <skip> <a572>
  <a0f> <cx> <cy> <btn> <cmd> <obj>`;
  `B <vbl> <func> <caller> <cols> <rows> <hash> <pal> <skip>`;
  `S <vbl> <delay> <w> <h> <reveal>`; `F <addr> <vbl>`; `M f12 <vbl> 0 0`;
  `# RUN`.

## 9. Checklist para cerrar T-073c

- [ ] Confirmar que el click fantasma está corregido (`btn=00` sin pulsar).
- [ ] Confirmar visualmente el retrato del Maestro con la build sin traza.
- [ ] Verificar que los 3 primeros reveals siguen igual (22/119/211 o el
      desfase de la run; deltas 97/92).
- [ ] Revertir `platform/dispatch_trace.c` + filtros Makefile/CMake +
      `platform_marks_state`.
- [ ] Reconstruir `make test`/`test-port`/`cmake-win`/`cmake-native`.
- [ ] Regenerar `build-win/kult-0.1.0-win64.zip` y el TGZ; quitar
      `kult-frame10.bat`/logs de diagnóstico del paquete.
- [ ] Actualizar `RETOMAR.md`, `TODO.md` (T-073c), `notes/0015` y este
      `notes/0016` con el resultado.
- [ ] No commit salvo petición del usuario.

## 10. Ficheros importantes

- `platform/dispatch_trace.c` — copia diagnóstica con `Y`/`V` (eliminar).
- `platform/cpuwork.c` — instrumentación, marcas, `platform_marks_byte/state`.
- `platform/platform.h` — declaraciones de marcas.
- `platform/input.c` — fix de `A_INPUT_BUTTONS` bit2.
- `platform/video_sdl.c` — refresco de `g_mouse_btn` con `SDL_GetMouseState`.
- `platform/include/kult_port.h` — sombra `cpu_delay`/`frame_gate_delay`
  (`KULT_FRAME_GATE`).
- `src/dispatch.c` — VM original (**no tocar**); `byte_handler`, `vm_run`.
- `src/loop.c` — `update_icon_list` 0x16582, `obj_list_iter` 0x16636,
  `anim_bank` 0x15ac8, `icon_repeat` 0x18944, `anim_mode` 0x188ce.
- `src/mode.c` — `scene_mode_apply` 0x1460a, `scene_mode_draw_wrap` 0x14602,
  `body_146d2` 0x146d2.
- `src/textblit.c` — `text_entry_blit` 0x141dc, `run_text_entry_ex` 0x14138.
- `src/reveal.c` — `reveal_region` 0x12e2e, `reveal_pixel` 0x12f28.
- `src/anim.c` — `anim_engine_entry` 0x15da6, `anim_engine` 0x159d2,
  `anim_bank_build` 0x15abe; `KULT_ANIM_GATE`.
- `src/screens.c` — `copy_rows_a` 0x15934 (lift), `copy_screen` 0x121c6.
- `boot/boot.c` — arranque, espera `l_010158` del clic, paleta/PRES, c001.
- `Makefile`, `CMakeLists.txt` — filtros de la traza (revertir).
- `assets/kult-marcas.bat`, `assets/kult-frame10.bat` — lanzadores.
- `build-win/kult-0.1.0-win64/` — carpeta del usuario (exe de traza, data,
  logs).
- `build-win/kult-0.1.0-win64.zip` — ZIP estándar (no regenerado; build
  no-trace anterior).
- `/app/kult-port-legacy/docs/ANIMACIONES.md`,
  `/app/kult-port-legacy/docs/mapa/animacion_frames.md` — referencia de tipos
  de animación, medidas 1:1 y estado APARCADO del Maestro.
- `/app/kult-atari-st-decomp/matching/src/auto_016582.s`,
  `auto_016636.s`, `auto_0188ce.s`, `auto_018944.s`, `auto_01460a.s`,
  `auto_0146d2.s` — ASM de referencia.

## 11. Sesión 17 (en curso): reproducción del camino completo — RECHAZADA (ver 0.1)

- **La línea `S` va una llamada por detrás**: `platform_marks_state`/`S` se
  emiten en `__cyg_profile_func_enter`, antes de que `15da6` fije
  `$1a49e/$1a4a0`; por eso una secuencia 5x22 se ve como `3 15` (cols/rows de la
  llamada anterior). No interpretar `S` como el tamaño de la llamada actual.
- **Confirmado**: el tramo `15da6` con `delay=0` que se veía en el Maestro es el
  **objeto que va al inventario** (5x22, x 44→73, y 85→55), no los brazos; ya
  estaba documentado en `notes/0015` y ahora está reproducido en el port con
  `KULT_15DA6`.
- **Camino con transiciones completas** (headless, `KULT_CLICK=0` +
  inyección de tecla temporal para que el core vea bit2 por Crawio sin
  `skip`): el Maestro entra en `0x36 1d41b` con `skip=00`, los 4 bancos
  `15ac8` se construyen en el mismo VBL, el scene draw dura **53 VBL**
  (`screen_copies`, 0x35 filas con `wait_vbl`) y el `0x06` posterior
  (`overlay_dispatch_2`, lift) **61 VBL**. Luego vienen el texto y los
  `prompt_wait`. El retrato que aparece en el centro (el del Maestro hablando)
  es **posterior** a ese primer prompt; hay que avanzar el diálogo para verlo.
- **Causa raíz del Maestro acelerado (sesión 17)**: el reveal/lift comprueba
  `$1a570`; el `wait_frame` del core lo fijaba con el **clic izquierdo**, el
  mismo que se usa para interactuar/avanzar, así que cada clic saltaba el
  reveal. En el original `$1a570` sigue al **botón derecho** y el izquierdo solo
  interactúa (referencia: `kult-port-legacy/docs/RUNTIME.md` §"flag de saltar
  transición").
- **Fix implementado (port-side, `src/` intacto)**:
  - `platform/input_core.c`: copia justificada de `src/input.c`; el izquierdo
    marca bit2 (interacción) y `$1a570` se fija con
    `platform_input_skip_requested()` (derecho).
  - `platform/input.c` + `platform/video_sdl.c`: se sigue el botón derecho de
    SDL (`SDL_BUTTON_RMASK`) además del izquierdo; `platform_mouse_set_buttons`
    refresca el estado al final del bombeo.
  - `Makefile`/`CMakeLists.txt`: `src/input.c` se excluye **solo del build del
    port** y entra `platform/input_core.c`; el diferencial (`make test`) sigue
    compilando `src/input.c` intacto.
  - Prueba nueva en `make test-port`: izquierdo → `skip=0`; derecho → `skip=0xff`.
- **Verificado**: `make test` 499; `make test-port` (incluye la prueba nueva);
  `make port`, `make port-sdl`, `make cmake-native`, `make cmake-win`. En la
  traza, el camino automático con clic izquierdo entra en `0x36 1d41b` con
  `skip=00` y hace el scene draw de **53 VBL** y el lift `0x06` de **61 VBL**;
  con el derecho mantenido en SDL (Xvfb) el `skip` es `ff`. El retrato del
  Maestro aparece después del primer prompt de la gruta.
- **Pendiente**: validación visual del usuario con el exe estándar
  `build-win/kult-0.1.0-win64/kult.exe` md5 **`296ce6c689c9237097cfb5f454eaf787`**
  (sin traza): el clic izquierdo ya no debe saltar el reveal; el derecho sí.
  Después: cerrar T-073c, eliminar `platform/dispatch_trace.c` y
  `platform_marks_state`, regenerar el ZIP y actualizar docs.
