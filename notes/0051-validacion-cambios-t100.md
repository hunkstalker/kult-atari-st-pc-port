# 0051 — Validación funcional de los cambios T-095/T-100

Fecha: 2026-10-25. Estado: **validación funcional del usuario**. No cierra T-094.

## Alcance

El usuario confirma que los últimos cambios siguen funcionando bien. Esta es una
validación funcional sobre el estado reciente del port, **no la validación
visual final de la release Windows (T-094)**, que sigue pendiente.

No debe usarse como autorización para publicar ni como sustituto del checklist
de T-094: arranque desde el ZIP en carpeta limpia, con los STX, y revisión de
DRAP, menú D/E/F/S/C, título, textos español/catalán, fuentes, cajas de objeto,
La Horca, guardado/carga, audio y menús.

## Contexto validado

- T-095: tests de ingesta (STX/Pasti/FAT12), `KULT.PRG`, `kult.data` atómico y
  packs `S`/`C`.
- T-100: diagnósticos separados en `platform/diag.c`, buffer F12 perezoso,
  `make test-diag` y guía `docs/DIAGNOSTICO.md`.

## Release

El ZIP de `build-win/` es regenerable y su SHA256 cambia con cada empaquetado;
el hash definitivo se fijará al cerrar T-094/T-092. Antes de publicar hay que
completar la validación visual final y regenerar/verificar el artefacto.
