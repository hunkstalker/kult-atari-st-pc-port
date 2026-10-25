# 0058 — T-105: documentación y ZIP con F1/F2 + ranuras

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: **T-094** sobre este ZIP.

## Cambios incluidos

- **T-102**: fullscreen sin bordes a resolución nativa; F1 fullscreen/ventana;
  F2 escala Auto/×1–×6 con viewport entero y ratón corregido.
- **T-103**: savestates `KSLOTP01` (1 MiB + paleta + base visible) en
  `saves/partida-0.bin`…`partida-9.bin`; `kult.ini` recuerda la ranura.
- **T-104**: F5 cicla ranuras, F6 guarda y F7 carga en el tope del bucle; OSD
  discreto y avisos de error.

## Verificación

- `make test`: **519 tests del core + 33 funciones / 209 `CHECK` de HAL +
  diagnóstico off/on + 2 herramientas**.
- `make port`, `make port-sdl`, `make cmake-native`, `make cmake-win` y
  `make package-win` OK.
- ZIP: `build-win/kult-1.0.0-win64.zip`
  SHA256 `66bd1af5cb1deafff569fd527a48b7c923ef9ab4f7914a099faa33d43e5951c4`
- El ZIP contiene 11 entradas, sin tests, sin `kult-headless.exe` y sin
  `DIAGNOSTICO.txt`; `saves/` va vacía.

## Pendiente

**T-094**: validación visual/funcional sobre este ZIP, incluyendo F1/F2,
ratón en ventana/fullscreen, F5/F6/F7, ranura recordada, errores de ranura
vacía/corrupta y reinicio de la aplicación. Después T-092 (publicar) y T-096
(CI).

## Limitaciones

- El savestate no guarda el estado del MFP/PSG: la música no rebobina.
- F6/F7 solo actúan durante la partida; en el título avisan `SOLO EN PARTIDA`.
