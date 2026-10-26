# 0096 — Merge de T-138 y preparación de la release 1.2.0

Fecha: 2026-10-26. Estado: **T-138 merged y validado en Windows/Linux/macOS;
release 1.2.0 publicada (tag `v1.2.0`)**.

## Hecho

- `feature/cursor-refresh` se actualizó con `main` (`d54dc28`) y se mezcló en
  `main` con `--no-ff` (`dbca6d3`).
- T-138 (cursor a refresco nativo) queda en `main` y está validado en
  Windows/Linux/macOS; no forma parte de 1.1.0, que ya está publicada.
- T-139 (render y simulación en hilos) sigue descartada.
- Versión de trabajo actualizada a **1.2.0**: `Makefile`, `CMakeLists.txt`,
  `tools/build_macos.sh` y `.github/workflows/release.yml`.
- Notas públicas creadas en `docs/RELEASE_NOTES_1.2.0.md`, sin referencias a
  testing, CI ni builds locales.
- Documentación de release actualizada a 1.2.0 en README, RELEASE, BUILD,
  VERIFICACION, TESTS, LIMITACIONES y LEEME_LINUX.

## Release 1.2.0

Assets publicados:

- Windows: `kult-1.2.0-win64.zip`.
- Linux: `kult-1.2.0-linux-x86_64.tar.gz` y `kult-1.2.0-x86_64.AppImage`.
- macOS: `kult-1.2.0-macos-universal.zip` (contiene solo el DMG) y
  `.zip.sha256` como descarga aparte.
- Sin DMG suelto en la release.

## Pendiente

- QA visual del DMG de 1.1.0 en Mac Intel (T-136).
- 1.1.0 y 1.2.0 quedan publicadas; la siguiente release no está definida.
