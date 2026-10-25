# VERIFICACIÓN

## Core (heredado, no se toca)
- **519 pruebas diferenciales** contra el oráculo (Musashi): dan la misma entrada
  al original (emulado) y al C, y comparan memoria + registros + ledger.
- Son la **red de seguridad** de que el porte **no cambia la lógica**. Deben
  seguir verdes al importar el core (M0/T-002).
- Comandos: `make test`, `make test PYTEST_ARGS=-n0`, `make regscan` (`ids_reg=0`),
  `make core`.

## Backend (nuevo)
Pruebas propias de la HAL, sin oráculo:
- **Ficheros:** mapeo `A:…` → `data/`; lectura de recursos reales (fixtures).
- **Vídeo/render:** conversión **planar→RGBA** contra una imagen **golden**;
  paleta de 16 words; escalado entero (sin interpolación inesperada).
- **Entrada:** mapeo teclado/ratón → `os_crawio`/`os_cnecin` y paquete IKBD;
  `A_CURSOR_X/Y` se actualizan. Incluye replay de entrada
  (`KULT_REPLAY_REC`/`KULT_REPLAY_PLAY`, `test_replay_input`): graba cursor,
  clics y teclas y los reproduce en orden.
- **Tiempo/tick:** el contador de VBL avanza; las esperas retornan; el número de
  pasos por segundo es el esperado (simulación a cadencia fija).
- **Audio:** comparación del **stream de registros PSG** del reproductor de música
  (`0x3a872`) con el **oráculo** (ledger del kit, 23 eventos en una escena
  controlada), y **RMS/energía** de la emulación PSG/YM2149. Además: periodo/ISR
  del **Timer A** del MFP y el bombeo en modo tiempo real. Todo en `make test-port`
  (`test_music_player_golden`, `test_mfp_timer_a`, `test_audio_pump`,
  `test_audio_rms`, `test_integration_music`).
- **Ingesta y packs:** STX/Pasti y FAT12 sintéticos, validación de `KULT.PRG`,
  contenedor atómico, pipeline condicional con los STX reales y selección/carga
  de `S`/`C` (`platform/test/test_platform.c`, T-095).
- **Diagnóstico:** `make test-diag` comprueba en dos procesos que sin variables
  no se crean logs y que con ellas aparecen las trazas y el volcado F12
  (`platform/test/test_diag.c`, T-100). También forma parte de `make test-port`.

## Integración (hitos)
- **M1:** el arranque headless avanza N frames y el framebuffer es coherente.
- **M2:** las esperas no cuelgan (el tick avanza el contador).
- **M3:** primera ventana jugable y controlable, sin artefactos.
- **M4:** la intro/música suena (RMS/energía como comprobación básica).
- **M6:** el menú de idioma fiel (drenado + D/E/F) tiene prueba en `make test-port`
  (`test_boot_language`); el resto es la **validación del usuario** (abajo).
- **M7 (build):** `make cmake-native` (Linux) y cross MinGW (Windows) compilan;
  CPack genera ZIP/TGZ. En Windows, `kult.exe` sale con icono y depende solo de
  `SDL3.dll` (incluida) y UCRT (Windows 10+).

## Checklist de validación (release 1.0.0, la ejecuta el usuario)
Requiere la copia de datos del usuario (recursos de `disk_a`+`disk_b`) y, para
ventana/audio, SDL3 con display y tarjeta de sonido. Ver `docs/LIMITACIONES.md`.

**Leyenda:** ✅ validado; ⬜ pendiente.

**Estado actual (candidato 1.0.0; T-061 ✅ cerrada y validada en Windows,
2026-10-23). T-094 no está cerrada: repetir esta checklist sobre el ZIP de T-119
(2026-10-25), incluyendo F1/F2 (con Estirar a pantalla), F5/F6/F7, `slot-N.save`,
el OSD con la fuente del juego y la terminología catalana de T-119 (`GOT`,
`XARXA TUNER`).**

- ✅ Título, menú de idioma y colores.
- ✅ Partida y controles (izquierdo/teclado = acción; derecho = acción + skip).
- ✅ Retrato del Maestro y A/B/C (T-073c).
- ✅ Cursor del juego y cursor de Windows oculto (T-075/T-080).
- ✅ Sangre, viaje espacial y espera de la estrella (T-077/T-079).
- ✅ Glitch gráfico del reveal (T-081).
- ✅ Guardado y carga de partida.
- ✅ Muerte → reinicio (T-076), incluida la secuencia de animaciones.
- ✅ Audio completo: música, voces, efectos y **puertas** (apertura/tope/cierre) validados.
- ✅ Combate/PSI: el enemigo fuerte muere tras el PSI, el anillo no se reabre y no queda el recuadro negro (T-087, `notes/0037`).
- ✅ Atacar/ficha de personaje sin crash: el replay reproduce la ficha y la partida continúa (T-088, `notes/0038`).
- ✅ Pasada visual actual sin glitches gráficos: el antiguo “glitch aislado” se retira; era un resto del punto 4, ya cubierto por T-081.
- ✅ Partida completa en Windows: jugada de principio a fin (T-089 incluido) y validada por el usuario.
- ✅ Final del juego: el original **no vuelve al título**; tras la última secuencia entra en el bucle infinito de `attract` (`0x18c80`) y muestra la animación de estrellas. El port retornaba y seguía el guion, dejando escena/paleta corruptas. Fix **T-089** (`attract_once`/`attract_loop`/`attract`) **validado por el usuario en Windows (2026-10-22)**. Detalle: `notes/0040-t089-final-attract-loop.md`.
- ✅ **T-090 cerrado**: SOLAR EYES era `palette_cycle` (`0x16310`); el C omitía los dos `dbra 0xffff` por pasada. Fix de timing en `lib/kult-core/src/video.c` con `cpu_delay(0x10000u)`; oráculo 7.879.396 ciclos (~985 ms) y port 12→60 VBL. Replay, logs y medición en `trazas-t090-20261023/`; detalle en `notes/0041`. El hueco de audio de Hatari queda documentado como artefacto de timing del original, **fuera del alcance de T-090**; la calibración de CPU/ritmo es **T-073 (feature opcional)**. T-061 quedó cerrada con la validación de Windows (2026-10-23).

**Cierre de T-061:** validada la versión de Windows (2026-10-23). Criterio:
el C sigue los mismos caminos y comportamiento que el ASM **a nivel de
experiencia del jugador**; no se exige timing ciclo-exacto. El hueco de audio
de SOLAR EYES es un artefacto de timing del original y T-073 queda como feature
opcional no bloqueante. **M6 cerrada.** Ingesta `.STX`/`kult.data`: ✅ implementada.

**“Ritmo global”** no es una prueba aparte: significa que cada escena y
transición mantenga la cadencia del original. Las escenas medidas y el audio
están validados.

1. ✅ **Automático:** `make test` (**519 core + HAL + diagnóstico + herramientas**) y `make test-port`/`make test-diag` (verde).
2. ✅ **Arranque headless:** `KULT_FRAMES=400 ./build/kult`
   → llega al bucle, `fb_nonzero>0`, y `audio:` con `rms/peak/samples/isr`.
3. ✅ **Ventana + entrada:** `make port-sdl` y `./build/kult-sdl`
   (la ventana se abre por defecto; `KULT_WINDOW=0` fuerza headless)
   - **Pantalla inicial = título (PRES) visible**: debe verse el **título del
     juego con sus colores** (logo KULT/creditos) y sonar la música, esperando
     **tecla o clic**. El port aplica la paleta de `PRES.PI1` antes de la espera
     (ver `notes/0014`); **no** debe verse negra. **Validar**:
     (a) el título se ve desde el principio (no negro);
     (b) una **tecla o un clic** pasa a la pantalla de **idioma (DRAP)** con el
         menú D/E/F/S/C;
     (c) los colores del título y del menú son correctos.
     **Fallo** = título negro, no reacciona a tecla/ratón, o nunca aparece el
     menú. **Estado: ✅ validado por el usuario** (título visible desde el inicio,
     2026-10-19).
   - ✅ elige idioma → **no pide disco 2** (el port usa los dos discos juntos) y
     entra en la partida; se ve y se controla (ratón/teclado), **con los colores
     correctos** (sin ruido; Physbase `0x98000`, ver `notes/0014`).
   - ✅ **Retrato A/B/C (T-073c)**: A = 3 retratos pixel + descripciones;
     B = 4 retratos lift + descripciones; C = Maestro + lift + diálogo.
   - ✅ **Glitch del reveal (T-081)**: el aborto de `elem_command` ya no deja
     artefactos.
   - ✅ **Sangre, viaje espacial y estrella (T-077/T-079)**: ritmo y espera de
     `FOND.PI1` validados.
4. ✅ **Audio:** música, voces, efectos y **puertas** (abrir, llegar al tope y
   cerrar) validados. `KULT_AUDIO=0` lo apaga; `KULT_MUSIC_GAIN` ajusta la
   música.
5. ✅ **Guardado/partida:** validado por el usuario.
6. ✅ **Morir → reiniciar:** validado por el usuario; al morir, tras la animación vuelve el
   **Maestro/objeto** del inicio del bucle (`reset_clear` → `bra 0x10158`,
   emulado en `boot/`; ver `notes/0022`; **no** depende de `0x106f2`). Antes,
   la **sangre**, el **viaje espacial** y la **espera de la estrella** deben
   durar como el original (costes medidos en `notes/0023`/`notes/0025`).
7. ✅ **Esperas de disco (T-079):** el título y toda la carga inicial (PRES/DRAP,
   listas A/B, FOND, ECHE) deben aparecer **sin** espera, y la canción empezar
   desde el principio; después sí debe haber pausas breves al entrar en salas
   (SCENA) y en la estrella, como el original. **Estrella final
   (punto 2):** tras morir, debe quedar visible ~2 s mientras se carga
   `FOND.PI1` (ver `notes/0025`/`notes/0026`), y después reiniciarse.
   **Fallo** = hay pantalla negra inicial, la canción empieza avanzada, no hay
   pausas donde el original las tiene, o no hay reinicio.
8. Anotar cualquier fallo en `RETOMAR.md`/`docs/LIMITACIONES.md`.

### Windows (paquete de release)
El paquete está en `build-win/kult-1.0.0-win64.zip` (generado con
`make package-win`). **No lleva assets del juego**; sí incluye
`Kult Settings.png` junto al ejecutable. Para validar:

1. Descomprimir el ZIP en una carpeta escribible (no Program Files).
2. Copiar `Kult.STX` y `Kult (B).STX` en `data/` o junto a `kult.exe`.
   El primer arranque genera `data/kult.data`; después se pueden borrar los STX.
3. Comprobar que `Kult Settings.png` está junto a `kult.exe`; después, doble clic
   en `kult.exe` (no abre consola; el icono es la K de Kult).
4. Repetir los puntos 3–6 de arriba: **primera pantalla = título visible** (no
   negro; espera **tecla o clic**) → pantalla de idioma D/E/F/S/C, juego con
   **colores correctos**, guardado ✅, morir → reiniciar ✅, **audio ✅**
   (incluidas puertas).
   **Controles**:
   **F1** alterna pantalla completa/ventana y **F2** cicla la escala
   (Estirar a pantalla, ×1–×6, Auto); comprobar que el ratón sigue apuntando bien,
   que Estirar a pantalla llena la salida y que el OSD se ve discreto. Al
   arrancar debe usarse Estirar a pantalla si `kult.ini` no indica otra escala.
   **F5** cicla ranuras 0–9, **F6** guarda y **F7** carga; probar una ranura
   vacía/corrupta (error visible), reiniciar y comprobar que se recuerda la
   ranura. El aviso debe verse con la **fuente 4×6 del juego**, no con una
   fuente externa. El savestate no rebobina la música.
   **F9/F11** bajan/suben el volumen 10% y **F10** silencia/restaura; el OSD
   debe mostrar `VOLUMEN N` o `SILENCIO` y el audio debe responder. Cambiar
   escala, pantalla, volumen o silencio y reiniciar: deben conservarse en
   `kult.ini`.
   **izquierdo / teclado** = acción normal, **sin** skip y con animación visible;
   **derecho** = acción + skip (puertas y A/B/C). **Clic mantenido ✅ (T-084)**:
   una pulsación mantenida = **un** clic; el anillo de acciones se abre una vez y
   queda abierto para elegir (ver `notes/0032`). **Retrato/intro ✅ (T-085)**: la
   animación `portrait_intro` dura ~2,2 s (ver `notes/0033`). **Selección de
   acción ✅ (T-086)**: al elegir una acción se ven los tres destellos del icono
   (ver `notes/0034`). **Cursor**: se queda en los
   bordes sin reaparecer por el lado contrario (máximo IKBD 304×184; ver
   `notes/0021`) y **no debe verse el cursor de Windows** dentro de la ventana,
   solo el del juego (`notes/0027`). **Ritmo**: el port va a
   50 VBL/s; si una animación va acelerada **sin clicar**, es **T-073** (trabajo
   de CPU no modelado; **feature opcional**), no un fallo de esta validación. En la intro: **A** son
   3 retratos con descripciones, **B** son 4 retratos con descripciones y **C**
   es el retrato del Maestro con **diálogo** (el único); solo C se mueve.
   **T-073c ✅ validado**: el `dbf` inline de `0x16dec` ya se cobra con
   `cpu_delay_cycles` (ver `notes/0019`). **T-074 ✅ (HAL)**: botones y
   presentación por VBL (ver `notes/0020`). **T-079 ✅**: espera de la estrella.
   **T-080 ✅**: cursor del sistema oculto. **T-081 ✅**: glitch del reveal
   corregido y validado. Si aún se notara acelerado, calibrar
   `KULT_CPU_HZ`/`KULT_ST_CONTEND`, no parchear la animación.
5. **Si algo falla**: mirar **`kult.log`** junto al `.exe`, con timestamp.
   Códigos: `0` ok, `2` PRG/datos ausentes, `3` fallo de arranque, `4` fallo de
   ventana, `5` carpeta del juego no escribible. Sin diálogos normales; solo si
   no se pudo escribir el log aparece uno de emergencia.
6. Requisito: **Windows 10 o superior** (UCRT).

## Regla
Toda corrección o feature nueva deja su prueba. Si algo es inobservable, se
documenta como *intencional* con su justificación (igual que en el core).
