# 0091 — T-135: Linux TGZ + AppImage

Fecha: 2026-10-25. Estado: **implementado y probado localmente**; CI pendiente
de ejecución.

## Paquete Linux

El TGZ incluye:

- `kult` ejecutable.
- `lib/libSDL3.so.0` con RUNPATH `$ORIGIN/lib`.
- `data/language.data`, `data/` y `saves/`.
- `LEEME.txt`, `Kult Settings.txt` y `.png`.
- licencias.

El usuario aporta `Kult.STX` y `Kult (B).STX`; el primer arranque genera
`data/kult.data`.

Prueba local: extraer el TGZ, ejecutar sin STX con `SDL_VIDEODRIVER=dummy` y
comprobar que falla por datos (código 2), no por SDL3. `ldd` resuelve
`libSDL3.so.0` desde `./lib`.

## AppImage

`tools/build_appimage.sh` crea un AppDir desde el staging de CMake:

- `usr/bin/kult`
- `usr/lib/libSDL3.so.0`
- `usr/share/kult/{language.data,LEEME.txt,settings,licencias}`
- `AppRun`, `.desktop` e icono.

`AppRun` usa `KULT_BASE_DIR` para que `data/` y `saves/` queden junto al
AppImage, no dentro del montaje de solo lectura. También copia
`language.data` y `LEEME.txt` al directorio externo.

Prueba local: AppImage ejecutado sin STX; falla por datos con código 2 y crea
`data/language.data` y `saves/` junto al AppImage.

## Dependencias

- `tools/fetch_sdl3_linux.sh`: SDL3 3.4.10 desde fuente, SHA-256
  `12b34280…`, compilación e instalación en `third_party/sdl3-linux`.
  Se compila con `SDL_X11_XSCRNSAVER=OFF` y `SDL_X11_XTEST=OFF`: el port no
  usa screensaver ni inyección XTest, así el build no depende de `libXss` ni de
  `libXtst`.
- `tools/fetch_appimagetool.sh`: appimagetool con SHA-256 `95cbe7cc…`.
- `make deps-linux` ejecuta ambos.

## CI

El job `linux` usa Ubuntu 22.04, `make deps-linux`, CMake con
`KULT_SDL3_LIB`, CPack TGZ, instalación staging y AppImage. Sube TGZ, AppImage
y sus `.sha256`.

## Artefactos locales

- TGZ: `e414a3ee8b613545e327b5eacccc99cb75a6c4ebdbedb82c73e58188a1f81d38`
- AppImage: `17091e9481c1f23a7d1d468024e73d065fb7079a654af6b51b9a4daa83167c6e`

Los hashes de CI pueden variar por timestamps del empaquetado; la release
adjunta los `.sha256` generados en CI.
