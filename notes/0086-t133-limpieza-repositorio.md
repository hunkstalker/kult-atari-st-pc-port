# 0086 — T-133: limpieza y política del repositorio

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-134.

## Fuentes de diseño

- `Kult Settings.af` y `Kult-Idiomas.af` pasan a `assets/design/`.
- `assets/design/README.md` documenta que son fuentes Affinity y que los PNG son
  las exportaciones de release.

## Revisión lingüística

- `docs/REVISION_CATALAN.xlsx` sale del índice y queda ignorado localmente.
- No se reescribe el historial: el archivo seguirá en commits antiguos, pero no
  en el árbol actual.
- `docs/INDICE.md` ya no lo enlaza; `RETOMAR.md` lo marca como copia local.

## Limpieza local

Se eliminaron:

- `.pytest_cache/`, `**/__pycache__/`, `lib/kult-core/build/`,
  `lib/kult-core/.pytest_cache/`, `lib/kult-core/work/`.
- `build/`, `build-cmake/`, `out/`, `temp/`.
- La carpeta extraída `build-win/kult-1.0.0-win64/`, `_CPack_Packages/`,
  `build-win/saves/`, logs y binarios de build.

Se conservaron:

- `build-win/kult-1.0.0-win64.zip`
- `build-win/trazas-lang-20261025-ca/`
- `build-win/trazas-lang-20261025-es/`
- `third_party/`, `work/`, `.venv/`.

## Política

Se añade `docs/REPOSITORIO.md`: código/tests/docs/fuentes sí; builds, datos del
juego, dependencias descargables, ZIPs y revisiones locales no.

## ZIP

No cambia: `build-win/kult-1.0.0-win64.zip`
SHA256 `5ed0e019ee90588b08030e5fce2b5cbfd82b1edce98daf0e079c1fb05a77965a`
(13 entradas).
