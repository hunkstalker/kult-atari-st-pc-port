# Índice de documentación

Punto de entrada a los documentos de `docs/`. Para el estado y las tareas, ver
`PROYECTO.md`, `TODO.md` y `RETOMAR.md`.

| documento | para qué |
|---|---|
| [`PLAN.md`](PLAN.md) | plan por fases/hitos con objetivos y criterios |
| [`ARQUITECTURA.md`](ARQUITECTURA.md) | capas (core/boot/platform/app), contrato, flujo |
| [`PORTING.md`](PORTING.md) | **frontera de port**: los 19 servicios y sus equivalentes PC; piezas que faltan |
| [`VERIFICACION.md`](VERIFICACION.md) | pruebas del core y del backend |
| [`LIMITACIONES.md`](LIMITACIONES.md) | limitaciones conocidas y checklist de validación (M6) |
| [`BUILD.md`](BUILD.md) | compilación (CMake + SDL3), Windows/Linux |
| [`DIAGNOSTICO.md`](DIAGNOSTICO.md) | arquitectura y uso de logs, marcas y trazas |
| [`RELEASE.md`](RELEASE.md) | formato portable, contenedores y errores de release |
| [`RELEASE_NOTES_1.0.0.md`](RELEASE_NOTES_1.0.0.md) | notas públicas de la release 1.0.0 |
| [`REVISION_CATALAN.xlsx`](REVISION_CATALAN.xlsx) | tabla español/catalán: todas las frases y solo cambios |
| [`DATA_FORMAT.md`](DATA_FORMAT.md) | `kult.data`/`language.data` e ingesta STX |
| [`LANGUAGE_PACKS.md`](LANGUAGE_PACKS.md) | packs español/catalán y limitaciones de fuente |
| [`TESTS.md`](TESTS.md) | inventario y política de tests |
| [`CORE_DELTA.md`](CORE_DELTA.md) | core importado y cambios locales del port |
| [`adr/`](adr/README.md) | decisiones de arquitectura (ADR) |

## ADRs
| id | decisión |
|---|---|
| [0001](adr/0001-sdl3.md) | Usar SDL3 |
| [0002](adr/0002-render-sdl-renderer-y-sdl-gpu.md) | `SDL_Renderer` base y `SDL_GPU` para CRT |
| [0003](adr/0003-audio-en-hito-propio.md) | Audio en hito propio |
| [0004](adr/0004-menu-idioma-fiel.md) | Menú de idioma fiel |
| [0005](adr/0005-boot-fuera-de-src.md) | Arranque/tick en `boot/`, fuera de `src/` |
| [0006](adr/0006-headless-primero.md) | Empezar headless |

## Notas de sesión
- [`notes/0001-inicio.md`](../notes/0001-inicio.md)
- [`notes/0002-importacion-core.md`](../notes/0002-importacion-core.md)
- [`notes/0003-m1-hal-headless.md`](../notes/0003-m1-hal-headless.md)
- [`notes/0004-tick-vbl-y-bloqueo.md`](../notes/0004-tick-vbl-y-bloqueo.md)
- [`notes/0005-loader-reloc-fix.md`](../notes/0005-loader-reloc-fix.md)
- [`notes/0006-m3-ventana-sdl3.md`](../notes/0006-m3-ventana-sdl3.md)
- [`notes/0007-m3-entrada-y-ritmo.md`](../notes/0007-m3-entrada-y-ritmo.md)
- [`notes/0008-m4-audio.md`](../notes/0008-m4-audio.md)
- [`notes/0009-m5-como-extra-opcional.md`](../notes/0009-m5-como-extra-opcional.md)
- [`notes/0010-t020-menu-fiel.md`](../notes/0010-t020-menu-fiel.md)
- [`notes/0011-plan-hitos-m7-ingesta-n3.md`](../notes/0011-plan-hitos-m7-ingesta-n3.md)
- [`notes/0012-m7-build-windows.md`](../notes/0012-m7-build-windows.md)
- [`notes/0013-datos-y-log.md`](../notes/0013-datos-y-log.md)
- [`notes/0014-physbase-y-ritmo.md`](../notes/0014-physbase-y-ritmo.md)
- [`notes/0015-ritmo-cpu.md`](../notes/0015-ritmo-cpu.md)
- [`notes/0016-maestro-handoff.md`](../notes/0016-maestro-handoff.md)
- [`notes/0017-retomar-maestro.md`](../notes/0017-retomar-maestro.md)
- [`notes/0018-plan-c.md`](../notes/0018-plan-c.md)
- [`notes/0019-fix-timing-c.md`](../notes/0019-fix-timing-c.md)
- [`notes/0020-input-hal-y-presentacion.md`](../notes/0020-input-hal-y-presentacion.md)
- [`notes/0021-cursor-e-ikbd.md`](../notes/0021-cursor-e-ikbd.md)
- [`notes/0022-reinicio-tras-muerte.md`](../notes/0022-reinicio-tras-muerte.md)
- [`notes/0023-costes-sangre-espacio.md`](../notes/0023-costes-sangre-espacio.md)
- [`notes/0025-espera-carga-fond.md`](../notes/0025-espera-carga-fond.md)
- [`notes/0026-esperas-de-disco.md`](../notes/0026-esperas-de-disco.md)
- [`notes/0027-cursor-sistema-sdl.md`](../notes/0027-cursor-sistema-sdl.md)
- [`notes/0028-retornos-dobles-128a4.md`](../notes/0028-retornos-dobles-128a4.md)
- [`notes/0029-audio-puertas.md`](../notes/0029-audio-puertas.md)
- [`notes/0030-diagnostico-yv-completo.md`](../notes/0030-diagnostico-yv-completo.md)
- [`notes/0031-sesion-34-t083-tailcall.md`](../notes/0031-sesion-34-t083-tailcall.md)
- [`notes/0032-input-clic-mantenido.md`](../notes/0032-input-clic-mantenido.md)
- [`notes/0033-portrait-intro-dbf.md`](../notes/0033-portrait-intro-dbf.md)
- [`notes/0034-seleccion-anillo-dbra.md`](../notes/0034-seleccion-anillo-dbra.md)
- [`notes/0035-incidencias-tras-combate.md`](../notes/0035-incidencias-tras-combate.md)
- [`notes/0036-a7-unwind-combate.md`](../notes/0036-a7-unwind-combate.md)
- [`notes/0037-dos-caminos-unwind-a7.md`](../notes/0037-dos-caminos-unwind-a7.md)
- [`notes/0038-replay-input-t088.md`](../notes/0038-replay-input-t088.md)
- [`notes/0039-handoff-endgame-titulo.md`](../notes/0039-handoff-endgame-titulo.md)
- [`notes/0040-t089-final-attract-loop.md`](../notes/0040-t089-final-attract-loop.md)
- [`notes/0041-t090-solar-eyes-palette-cycle.md`](../notes/0041-t090-solar-eyes-palette-cycle.md)
- [`notes/0042-release-licencias-y-textos.md`](../notes/0042-release-licencias-y-textos.md)
- [`notes/0043-revision-catalan.md`](../notes/0043-revision-catalan.md)
- [`notes/0044-diagnostico-f12.md`](../notes/0044-diagnostico-f12.md)
- [`notes/0045-fix-cajas-objetos-catalan.md`](../notes/0045-fix-cajas-objetos-catalan.md)
- [`notes/0046-fix-buffer-mots.md`](../notes/0046-fix-buffer-mots.md)
- [`notes/0047-pasada-textos-ingles.md`](../notes/0047-pasada-textos-ingles.md)
- [`notes/0048-resumen-incidencias-texto.md`](../notes/0048-resumen-incidencias-texto.md)
- [`notes/0049-tests-ingesta-packs.md`](../notes/0049-tests-ingesta-packs.md)
- [`notes/0050-diagnostico-separado.md`](../notes/0050-diagnostico-separado.md)
- [`notes/0051-validacion-cambios-t100.md`](../notes/0051-validacion-cambios-t100.md)
- [`notes/0052-plan-t022.md`](../notes/0052-plan-t022.md)
- [`notes/0053-t022-reinicio-suave.md`](../notes/0053-t022-reinicio-suave.md)
- [`notes/0054-t101-die-dado-dau.md`](../notes/0054-t101-die-dado-dau.md)
- [`notes/0055-t102-pantalla-f1-f2.md`](../notes/0055-t102-pantalla-f1-f2.md)
- [`notes/0056-t103-slots.md`](../notes/0056-t103-slots.md)
- [`notes/0057-t104-atajos-f5-f6-f7.md`](../notes/0057-t104-atajos-f5-f6-f7.md)
- [`notes/0058-t105-release-f1-f2-slots.md`](../notes/0058-t105-release-f1-f2-slots.md)
- [`notes/0059-t106-osd-fuente-juego.md`](../notes/0059-t106-osd-fuente-juego.md)
- [`notes/0060-t107-slot-save.md`](../notes/0060-t107-slot-save.md)
- [`notes/0061-t108-sin-migracion.md`](../notes/0061-t108-sin-migracion.md)
- [`notes/0062-t109-rellenar.md`](../notes/0062-t109-rellenar.md)
- [`notes/0063-t110-estirar-pantalla.md`](../notes/0063-t110-estirar-pantalla.md)
- [`notes/0064-t111-osd-tiempo-real.md`](../notes/0064-t111-osd-tiempo-real.md)
- [`notes/0065-t112-volumen.md`](../notes/0065-t112-volumen.md)
- [`notes/0066-t113-kult-settings-png.md`](../notes/0066-t113-kult-settings-png.md)
- [`notes/0067-t114-estirar-y-config.md`](../notes/0067-t114-estirar-y-config.md)
- [`notes/0068-t115-desc-126-caminante.md`](../notes/0068-t115-desc-126-caminante.md)
- [`notes/0069-t116-desc-126-erbe.md`](../notes/0069-t116-desc-126-erbe.md)
- [`notes/0070-t117-es-erbe.md`](../notes/0070-t117-es-erbe.md)
- [`notes/0071-t118-catalan-diferencial.md`](../notes/0071-t118-catalan-diferencial.md)
- [`notes/0072-t119-terminologia-catalana-erbe.md`](../notes/0072-t119-terminologia-catalana-erbe.md)
- [`notes/0073-t120-preferencias-catalan.md`](../notes/0073-t120-preferencias-catalan.md)
- [`notes/0074-t121-dialogo-deformado-catalan.md`](../notes/0074-t121-dialogo-deformado-catalan.md)
- [`notes/0075-t122-dialogo-deformado-catalan-clave.md`](../notes/0075-t122-dialogo-deformado-catalan-clave.md)
- [`notes/0076-t123-clave-fonetica-catalana.md`](../notes/0076-t123-clave-fonetica-catalana.md)
- [`notes/0077-t124-ajuste-162.md`](../notes/0077-t124-ajuste-162.md)
- [`notes/0078-t125-dialogo-clave-completo.md`](../notes/0078-t125-dialogo-clave-completo.md)
