# VERIFICACIÓN

## Core (heredado, no se toca)
- **499 pruebas diferenciales** contra el oráculo (Musashi): dan la misma entrada
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
  `A_CURSOR_X/Y` se actualizan.
- **Tiempo/tick:** el contador de VBL avanza; las esperas retornan; el número de
  pasos por segundo es el esperado (simulación a cadencia fija).
- **Audio:** comparación del **stream de registros PSG** del reproductor de música
  (`0x3a872`) con el **oráculo** (ledger del kit, 23 eventos en una escena
  controlada), y **RMS/energía** de la emulación PSG/YM2149. Además: periodo/ISR
  del **Timer A** del MFP y el bombeo en modo tiempo real. Todo en `make test-port`
  (`test_music_player_golden`, `test_mfp_timer_a`, `test_audio_pump`,
  `test_audio_rms`, `test_integration_music`).

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

## Checklist de validación (M6, la ejecuta el usuario)
Requiere la copia de datos del usuario (recursos de `disk_a`+`disk_b`) y, para
ventana/audio, SDL3 con display y tarjeta de sonido. Ver `docs/LIMITACIONES.md`.

1. **Automático:** `make test` (499 verde) y `make test-port` (verde).
2. **Arranque headless:** `KULT_DATA_DIR=<datos> KULT_FRAMES=400 ./build/kult`
   → llega al bucle, `fb_nonzero>0`, y `audio:` con `rms/peak/samples/isr`.
3. **Ventana + entrada:** `make port-sdl` y
   `KULT_DATA_DIR=<datos> ./build/kult-sdl` (la ventana se abre por defecto;
   `KULT_WINDOW=0` fuerza headless)
   - **Pantalla inicial = título (PRES) visible**: debe verse el **título del
     juego con sus colores** (logo KULT/creditos) y sonar la música, esperando
     **tecla o clic**. El port aplica la paleta de `PRES.PI1` antes de la espera
     (ver `notes/0014`); **no** debe verse negra. **Validar**:
     (a) el título se ve desde el principio (no negro);
     (b) una **tecla o un clic** pasa a la pantalla de **idioma (DRAP)** con el
         menú D/E/F;
     (c) los colores del título y del menú son correctos.
     **Fallo** = título negro, no reacciona a tecla/ratón, o nunca aparece el
     menú. **Estado: ✅ validado por el usuario** (título visible desde el inicio,
     2026-10-19).
   - elige idioma → **no pide disco 2** (el port usa los dos discos juntos) y
     entra en la partida; se ve y se controla (ratón/teclado), **con los colores
     correctos** (sin ruido; Physbase `0x98000`, ver `notes/0014`).
4. **Audio:** con entrada real, en el título/menú debe sonar la **música de intro**
   (el port bombea el Timer A durante las esperas); en partida, voz/efectos.
   `KULT_AUDIO=0` la apaga; `KULT_MUSIC_GAIN` ajusta la música.
5. **Guardado/partida:** comprobar el guardado/recarga del juego (servicios de
   fichero reales) según el uso del original.
6. **Morir → reiniciar sala:** al morir, el juego debe reiniciar la sala (usa
   `0x1842c`→`reset_clear`, ya en el core; **no** depende de `0x106f2`).
7. Anotar cualquier fallo en `RETOMAR.md`/`docs/LIMITACIONES.md`.

### Windows (paquete T-071)
El paquete está en `build-win/kult-0.1.0-win64.zip` (generado con
`make package-win`). **No lleva assets.** Para validar:

1. Descomprimir el ZIP (p. ej. en `C:\kult\`).
2. Copiar **el contenido de `disk_a` + `disk_b`** (juntos y **extraídos**, no
   las imágenes `.STX`) dentro de `data/` (debe quedar `data\KULT.PRG`).
   *(La ingesta automática de `.STX` es T-072; aún no está en este paquete.)*
3. Doble clic en `kult.exe` (no abre consola; el icono es la K de Kult).
4. Repetir los puntos 3–6 de arriba: **primera pantalla = título visible** (no
   negro; espera **tecla o clic**) → pantalla de idioma D/E/F, juego con
   **colores correctos**, guardado, audio, morir → reiniciar sala. **Controles**:
   el **clic izquierdo** interactúa y **salta** las transiciones/reveal (como se
   decidió para el port); el botón derecho no tiene función. **Ritmo**: el port va a
   50 VBL/s; si una animación va acelerada **sin clicar**, es **T-073** (trabajo
   de CPU no modelado), no un fallo de esta validación.
5. **Si algo falla**: mirar **`kult.log`** (junto al `.exe`) y/o ejecutar
   **`kult-headless.exe`** desde una consola (imprime el diagnóstico).
   Códigos: `0` ok, `2` PRG/datos ausentes, `3` fallo de arranque, `4` fallo de
   ventana. En GUI, los errores fatales salen también en un **MessageBox**.
6. Requisito: **Windows 10 o superior** (UCRT).

## Regla
Toda corrección o feature nueva deja su prueba. Si algo es inobservable, se
documenta como *intencional* con su justificación (igual que en el core).
