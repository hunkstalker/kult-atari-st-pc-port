# 0088 — T-094: cierre de validación y release 1.0.0

Fecha: 2026-10-25. Estado: **T-094 cerrada**. Release 1.0.0 preparada y
etiquetada localmente; publicación remota pendiente de credenciales.

## Validación

El usuario valida el ZIP de Windows de T-134 y autoriza el cierre de T-094.

Cubre la checklist de `docs/VERIFICACION.md`: título/menú de idioma, partida y
controles, F1/F2, Estirar a pantalla, ratón, OSD, F5/F6/F7 con `slot-N.save`,
F9/F10/F11, textos ES/CA con `Ç` y terminología T-119/T-134, y el diálogo
deformado en clave fonética.

## Artefacto

- `build-win/kult-1.0.0-win64.zip`
- SHA256 `5ed0e019ee90588b08030e5fce2b5cbfd82b1edce98daf0e079c1fb05a77965a`
- 13 entradas; `saves/` vacía.
- `Kult Settings.txt` y `Kult Settings.png` incluidos.

## Release

- Commit de cierre de T-094.
- Tag local: `v1.0.0`.
- `docs/RELEASE_NOTES_1.0.0.md` es la nota pública.
- El remoto `origin` está vacío y el entorno no tiene credenciales
  (`GITHUB_TOKEN`/`gh`), así que **no se ha hecho push ni GitHub Release**.
  T-092 queda pendiente de publicar `main`, el tag y el ZIP.
- T-096 (CI) sigue pendiente.

## Pendiente

1. Publicar `main` y `v1.0.0` en GitHub.
2. Crear la GitHub Release 1.0.0 y adjuntar el ZIP.
3. Añadir CI (T-096).
