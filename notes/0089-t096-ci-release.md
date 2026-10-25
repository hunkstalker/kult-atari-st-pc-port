# 0089 — T-096: CI y release automática

Fecha: 2026-10-25. Estado: **workflow implementado**; primera ejecución
pendiente de publicar el commit/tag.

## Workflow

`.github/workflows/release.yml`:

- Se ejecuta en `push` a `main`, en tags `v*` y manualmente.
- Job `test`: instala Python, ejecuta `make test-port` y `make test-tools`.
  No ejecuta la suite diferencial del core porque necesita `KULT.PRG`, que no
  está en el repositorio.
- Job `windows`: instala MinGW/CMake, usa `make deps-win`, compila con
  `make package-win`, genera el ZIP y su `.sha256`, y lo sube como artefacto.
- Job `linux`: Ubuntu 22.04, `make deps-linux`, TGZ autosuficiente y AppImage;
  sube ambos con sus checksums (T-135).
- Job `release`: solo en tags `v*`; descarga los artefactos y crea la GitHub
  Release con `docs/RELEASE_NOTES_1.0.0.md`, ZIP, TGZ, AppImage y checksums.

## Hashes

El ZIP validado en T-094 es:

`5ed0e019ee90588b08030e5fce2b5cbfd82b1edce98daf0e079c1fb05a77965a`

La GitHub Release se genera con el ZIP construido por CI. Puede no ser
byte-idéntico al ZIP local (timestamps del empaquetado), así que la release
adjunta su propio `kult-1.0.0-win64.zip.sha256`.

## Publicación

Falta hacer force-push del commit de release y del tag `v1.0.0` (el commit local
incluye README, workflow y paquetes Linux). No hay credenciales en este entorno.
Detalle de Linux: `notes/0091`.
