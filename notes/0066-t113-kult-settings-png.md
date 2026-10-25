# 0066 — T-113: incluir `Kult Settings.png` en la release

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094.

## Cambio

- La imagen `assets/Kult Settings.png` (96.889 B) se versiona en el repo.
- `CMakeLists.txt` la instala en la raíz del paquete, **junto a `kult.exe`**:
  `install(FILES ".../assets/Kult Settings.png" DESTINATION .)`.
- El paquete Windows pasa de 11 a **12 entradas**.
- Aplica también al `install()` usado por el futuro CI/paquete Linux; no es un
  asset del juego.

## Verificación

`make cmake-win` y `make package-win` OK. `unzip -l` confirma:

```text
kult-1.0.0-win64/Kult Settings.png
kult-1.0.0-win64/kult.exe
...
```

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `16c004ba2fba7d1486b0038812a2ac590ee35cf328806a5dc7728f47d6bdefef`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual/funcional del ZIP en Windows, incluyendo que la imagen
aparezca junto al `.exe`.
