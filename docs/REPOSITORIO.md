# Qué se versiona en el repositorio

El repositorio contiene solo el código y los recursos necesarios para construir,
probar y documentar el port. Los datos del juego, dependencias descargables y
artefactos de release quedan fuera.

## Se versiona

- `app/`, `boot/`, `platform/` y `lib/kult-core/`.
- `CMakeLists.txt`, `Makefile`, `cmake/`, `tools/`, `test/` y `requirements.txt`.
- `docs/`, `notes/`, `README.md`, `PROYECTO.md`, `TODO.md`, `RETOMAR.md` y `AGENTS.md`.
- `assets/lang/` y `assets/language.data`.
- Recursos de release: `assets/LEEME.txt` (Windows), `assets/LEEME_LINUX.txt`
  (Linux; se instala como `LEEME.txt`), `assets/LEEME_MAC.txt` (macOS; se
  instala como `LEEME.txt`), `assets/Kult Settings.txt`,
  `assets/Kult Settings.png`, `assets/kult_icon.ico`, `assets/kult.icns`,
  `assets/kult_icon.rc` y `cmake/macos/Info.plist.in`.
- Fuentes de diseño: `assets/design/*.af`.
- `third_party/README.md`.
- `tools/fetch_sdl3_macos.sh`, `tools/build_macos.sh` y `tools/make_icns.py`.

## No se versiona

- `build/`, `build-*/`, `out/` y cachés (`__pycache__`, `.pytest_cache`).
- ZIP/TGZ de release: se publican en GitHub Releases.
- `third_party/` descargable (`make deps-win`).
- `work/`, `data/`, `saves/`, `*.STX`, `*.PRG`, `kult.data` y `kult.ini`.
- `temp/`, logs, capturas y ficheros locales de revisión.
- `.venv/`.
- `docs/REVISION_CATALAN.xlsx`: copia local de revisión, no pública.

## Dependencias

- Linux: SDL3 del sistema (`sdl3-dev`/`libsdl3-dev`) mediante CMake/pkg-config.
- Windows cross: SDL3 3.4.10 con `make deps-win`; el script fija URL y SHA-256.
- No hay descargas automáticas al configurar CMake.
