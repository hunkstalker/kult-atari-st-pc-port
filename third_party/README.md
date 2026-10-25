# Dependencias de build

Los ZIP y las carpetas extraídas de SDL3 no se versionan. Para obtenerlos:

```sh
make deps-win
```

El script `tools/fetch_sdl3.sh` fija **SDL3 3.4.10**, descarga los dos ZIP,
verifica su SHA-256 y los descomprime de forma idempotente.

Ficheros esperados:

- `SDL3-devel-3.4.10-mingw.zip`
  - SHA-256: `5b2bc8589974391e7b3fa4d1bca6c4e09657254afeea57bb4dbe239932188695`
  - Contenido: `SDL3-3.4.10/x86_64-w64-mingw32/`
- `SDL3-3.4.10-win32-x64.zip`
  - SHA-256: `86e779617c6c2d4e20716e14a5bd9b1210595d62b88a05fed1ccd7fd53c902f9`
  - Contenido: `SDL3-3.4.10-win32-x64/SDL3.dll`

Uso:

```sh
make deps-win              # descarga/verifica/extrae
sh tools/fetch_sdl3.sh --check
sh tools/fetch_sdl3.sh --force
```

En Linux nativo no se usa esta carpeta: instala SDL3 del sistema y CMake lo
encuentra con `pkg-config`.
