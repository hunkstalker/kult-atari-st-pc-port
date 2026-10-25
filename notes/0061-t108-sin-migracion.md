# 0061 — T-108: retirada de la migración de `partida-N.bin`

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP nuevo.

## Motivo

T-107 añadió migración automática desde `partida-N.bin` a `slot-N.save`, pero
el puerto aún no se ha publicado: no existen partidas externas que conservar.
El usuario ya migró las suyas al arrancar, así que el código de compatibilidad
solo añadía complejidad y pruebas sin utilidad.

## Cambio

- `platform/slots.c` usa únicamente `saves/slot-0.save`…`slot-9.save`.
- Eliminados el escaneo/renombrado de `partida-N.bin` en `platform_slots_init()`
  y el fallback de carga.
- El formato `KSLOTP01` no cambia y `kult.ini` sigue igual.
- Se retiraron las pruebas de migración.

## Verificación

`make test`: **519 core + 34 funciones / 218 `CHECK` HAL + diag + 2
herramientas**. `make port-sdl`, `make cmake-native`, `make cmake-win` y
`make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `862b20b257b7f79d28c72cfa71ac1822aa92e56d937e59260e19bee43d9ececd`
(11 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual/funcional del ZIP en Windows.
