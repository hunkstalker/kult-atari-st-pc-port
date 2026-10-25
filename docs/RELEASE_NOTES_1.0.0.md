# Kult: The Temple of Flying Saucers — Port a PC 1.0.0

Primera release pública del port nativo a PC de **Kult: The Temple of Flying
Saucers** (Exxos/ERE, 1989), con **español y catalán**.

Fecha: 2026-10-25. Tag: `v1.0.0`.

El port ejecuta el C verificado del Atari ST sobre un backend SDL3, sin emulador.
No incluye el juego original: el usuario aporta su propia copia legal.

## Destacado

- **Port nativo** para Windows y Linux, sin emulador en tiempo de ejecución.
- **Español y catalán** integrados en `data/language.data`, además de D/E/F.
  El español parte de la traducción ERBE con correcciones; el catalán conserva
  su revisión, la migración diferencial, la terminología ERBE (`GOT`,
  `XARXA TUNER`, etc.), la traducción del diálogo deformado y la `Ç` con glifo
  propio.
- **Selector de idioma** con el `DRAP.PI1` de cinco banderas.
- **Ingesta inicial desde `.STX`**: detección de los discos A y B por SHA-256,
  extracción Pasti/FAT12, validación de `KULT.PRG` y generación atómica de
  `data/kult.data`.
- **Release portable**: `data/`, `saves/` y `kult.log` junto al ejecutable, sin
  appdata ni rutas alternativas.
- **Pantalla y atajos**: arranque en fullscreen sin bordes a resolución nativa y
  **Estirar a pantalla** por defecto; **F1** alterna ventana/fullscreen; **F2**
  cicla **Estirar a pantalla → ×1 → … → ×6 → Auto**. La configuración de las
  teclas F se guarda en `kult.ini`.
- **Ranuras de guardado**: **F5** muestra la ranura actual; si se repite mientras se ve el aviso, cicla 0–9. **F6** guarda y **F7** carga
  savestates completos en `saves/slot-N.save`; la ranura activa se recuerda en
  `kult.ini`. Los avisos usan la fuente original del juego.
- **Volumen**: **F9/F11** bajan/suben el volumen 10% y **F10** silencia/restaura,
  con OSD del estado (`VOLUMEN N` / `SILENCIO`).
- Errores solo en `kult.log`; diálogo de emergencia únicamente si el log no se
  puede escribir.
- Guardado y carga, audio, entrada, combate, final del juego y bucle de
  `attract` incluidos.
- Paquete limpio: sin `kult-headless.exe`, sin `DIAGNOSTICO.txt` y sin scripts de
  desarrollo.

## Paquete

La release incluye:

- Windows x64: `kult-1.0.0-win64.zip` (`kult.exe`, `SDL3.dll`, datos y ajustes).
- Linux x86_64: `kult-1.0.0-linux-x86_64.tar.gz` (`kult`, `lib/libSDL3.so.0`,
  datos, `LEEME_LINUX.txt` y ajustes).
- Linux AppImage: `kult-1.0.0-x86_64.AppImage`.

El usuario solo aporta `Kult.STX` y `Kult (B).STX`. El ZIP Windows validado en
T-094 tiene SHA256
`5ed0e019ee90588b08030e5fce2b5cbfd82b1edce98daf0e079c1fb05a77965a`;
la GitHub Release adjunta los `.sha256` generados en CI. Los artefactos de CI
pueden no ser byte-idénticos por los timestamps del empaquetado.

Las instrucciones de uso están en `LEEME.txt`, `LEEME_LINUX.txt`,
`Kult Settings.txt` y el `README.md`; la licencia del
port está en `LICENSE` y las licencias de terceros en `THIRD_PARTY_NOTICES.md`.
