# Kult: The Temple of Flying Saucers — Port a PC 1.0.0

Primera release pública del port nativo a PC de **Kult: The Temple of Flying
Saucers** (Exxos/ERE, 1989), con **español y catalán**.

El port ejecuta el C verificado del Atari ST sobre un backend SDL3, sin emulador.
No incluye el juego original: el usuario aporta su propia copia legal.

## Destacado

- **Port nativo** para Windows y Linux, sin emulador en tiempo de ejecución.
- **Español y catalán** integrados en `data/language.data`, además de D/E/F.
- **Selector de idioma** con el `DRAP.PI1` de cinco banderas.
- **Ingesta inicial desde `.STX`**: detección de los discos A y B por SHA-256,
  extracción Pasti/FAT12, validación de `KULT.PRG` y generación atómica de
  `data/kult.data`.
- **Release portable**: `data/`, `saves/` y `kult.log` junto al ejecutable, sin
  appdata ni rutas alternativas.
- Errores solo en `kult.log`; diálogo de emergencia únicamente si el log no se
  puede escribir.
- Guardado y carga, audio, entrada, combate, final del juego y bucle de
  `attract` incluidos.
- Paquete limpio: sin `kult-headless.exe`, sin `DIAGNOSTICO.txt` y sin scripts de
  desarrollo.

## Paquete

ZIP portable de Windows x64 con `kult.exe`, `SDL3.dll`, `LEEME.txt`, `LICENSE`,
avisos de terceros, `data/language.data` y `saves/`.

SHA256 del ZIP:
`a11a6948b0a3a6658c89719469f56d47df3be761b4ba0928888639ca25c137bc`

Las instrucciones de uso están en `LEEME.txt` y el `README.md`; la licencia del
port está en `LICENSE` y las licencias de terceros en `THIRD_PARTY_NOTICES.md`.
