# 0093 — macOS Universal 2 y release 1.1.0

Fecha: 2026-10-25. Estado: **implementación en curso**; build/CI de macOS
pendiente de ejecutar en GitHub.

## Decisión

- Un solo paquete macOS **Universal 2** (`arm64` + `x86_64`), no dos releases.
- Sin cuenta Apple Developer: `Kult.app` se firma en modo **ad-hoc** y no se
  notariza. Se documenta el aviso de Gatekeeper.
- La validación visual manual se hará en un Mac Intel aportado por el usuario;
  Apple Silicon queda cubierto por CI (arquitectura, firma y smoke test), no por
  una prueba visual manual.

## Cambios

- `platform/paths.c`: `platform_exe_dir` usa `_NSGetExecutablePath` en Apple.
- `platform/fs.c`: base en `~/Library/Application Support/Kult`; copia el
  `language.data` del bundle a `data/`.
- `platform/log.c`: crea la base en macOS y permite un diálogo de error visible.
- `platform/mac_dialog.c`: selector de los dos `.STX` en el primer arranque y
  diálogo de error para Finder.
- `app/main.c`: reintenta la ingesta tras seleccionar los STX; mensajes
  adaptados a macOS.
- `CMakeLists.txt`: `Kult.app`, `Info.plist`, icono `.icns`, SDL3 en
  `Contents/Frameworks`, RPATH y recursos de la app.
- `cmake/macos/Info.plist.in`, `assets/kult.icns`, `assets/LEEME_MAC.txt`.
- `tools/fetch_sdl3_macos.sh`: SDL3 universal desde fuente.
- `tools/build_macos.sh`: ZIP + DMG, firma ad-hoc o Developer ID opcional,
  checksums y notarización opcional si hay secretos.
- `Makefile`: `deps-macos`, `cmake-macos`, `package-macos`.
- CI: job `macos-15` Universal 2 y artefacto; la release incluye ZIP/DMG.
- Desinstalación documentada en README, `LEEME_MAC.txt`, RELEASE y notas:
  quitar solo `Kult.app` conserva datos; borrar Application Support elimina
  datos y partidas.

## Pendiente

- Ejecutar CI macOS y descargar el artifact.
- Prueba visual en Mac Intel: arranque, selector STX, audio, fullscreen,
  F1/F2/F5–F7, guardado y Gatekeeper.
- Smoke test de Apple Silicon en CI; no hay QA visual de Apple Silicon.
- Si aparece un problema de `-finstrument-functions` en Apple Clang, ajustar
  solo el build macOS y documentar el impacto en timing.
