# 0042 — Licencias y textos públicos de la release

**Fecha:** 2026-10-24
**Alcance:** documentación, licencias y empaquetado. Sin cambios de lógica.

## Objetivo

Dejar el port listo para publicación con un texto público uniforme —**port a PC
con español y catalán**— y con las licencias necesarias dentro del paquete.

## Cambios

- Añadido `LICENSE` MIT para el código, herramientas, documentación y packs de
  idioma del port.
- Añadido `THIRD_PARTY_NOTICES.md`:
  - SDL3, licencia zlib;
  - `lib/kult-core`, MIT;
  - `recreate_kit`, GPL-2.0.
- Añadido `LICENSES/` con `SDL3.txt` y `GPL-2.0.txt`.
- `CMakeLists.txt` instala `LICENSE`, `THIRD_PARTY_NOTICES.md` y `LICENSES/` en
  el paquete.
- El paquete deja de incluir scripts de desarrollo y material auxiliar; queda
  limitado a `kult.exe`, `SDL3.dll`, `LEEME.txt`, licencias, `data/` y `saves/`.
- Unificados los textos públicos a “español” y “catalán”.
- Retiradas las referencias a repositorios y materiales ajenos al port en la
  documentación, notas, comentarios y herramientas.
- `README`, `LEEME`, `RELEASE`, `DATA_FORMAT`, `LANGUAGE_PACKS`, `LIMITACIONES`,
  `PROYECTO`, `PLAN` y `BUILD` actualizados.
- `language.data` no se ha modificado.

## Verificación

- `make test`: **518 core + HAL + 1 port**, todo en verde.
- `make package-win` correcto.
- ZIP: `build-win/kult-0.1.0-win64.zip`
- SHA256:
  `af48e8b421a8dce307806a5b2d61deccdfa49c65708515b2fc81342afdbbb57e`
- Contenido comprobado: 11 entradas, sin `kult-headless.exe` ni scripts de
  desarrollo.

## Siguiente

- Vincular `origin` y publicar `main`.
- Revisión lingüística del catalán.
- Pruebas automatizadas de ingesta y packs de idioma.
