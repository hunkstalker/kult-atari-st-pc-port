# 0085 — T-132: Kult Settings.txt

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Cambio

El archivo de atajos se renombra de:

`Instrucciones - Atajos de teclado.txt`

a:

`Kult Settings.txt`

El contenido no cambia: F1, F2, F5, F6, F7, F9, F10, F11 y ESC. F12 sigue
fuera de la release normal y solo actúa con `KULT_MARKS`.

## Empaquetado

- `CMakeLists.txt` instala `Kult Settings.txt` junto a `kult.exe`.
- `LEEME.txt` y la documentación apuntan al nuevo nombre.
- El ZIP mantiene 13 entradas.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `5ed0e019ee90588b08030e5fce2b5cbfd82b1edce98daf0e079c1fb05a77965a`
(13 entradas, `saves/` vacía).

## Verificación

`make test` y `make package-win` OK. `unzip -l` confirma `Kult Settings.txt`.

## Pendiente

T-094 sobre el ZIP final.
