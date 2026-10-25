# 0087 — T-134: make deps-win

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Script

`tools/fetch_sdl3.sh`:

- Fija SDL3 **3.4.10**.
- Descarga `SDL3-devel-3.4.10-mingw.zip` y `SDL3-3.4.10-win32-x64.zip` desde
  GitHub.
- Verifica SHA-256 antes de descomprimir.
- Es idempotente; soporta `--check` y `--force`.
- Comprueba `SDL3Config.cmake`, `libSDL3.dll.a` y `SDL3.dll`.
- Permite `THIRD_PARTY_DIR` para pruebas en una carpeta temporal.

`make deps-win` es explícito; CMake no descarga dependencias.

## Documentación

- `third_party/README.md` versionado con versión, URLs y hashes.
- `.gitignore`: `third_party/*` con excepción para el README.
- `docs/BUILD.md` usa `make deps-win`; los comandos manuales quedan como
  fallback en el README de `third_party/`.

## Verificación

- `sh -n tools/fetch_sdl3.sh`.
- `make deps-win` con las dependencias actuales: no descarga.
- Prueba en `/tmp` con los ZIP actuales: extrae y valida.
- `sh tools/fetch_sdl3.sh --check` valida sin descargar.

El ZIP de release no cambia: `5ed0e019…`.
