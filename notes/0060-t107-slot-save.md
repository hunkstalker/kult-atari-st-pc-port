# 0060 — T-107: ranuras como `slot-N.save`

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP nuevo.

## Cambio

Las ranuras F5/F6/F7 pasan de `saves/partida-N.bin` a
`saves/slot-0.save`…`saves/slot-9.save`. El formato interno no cambia:
sigue siendo `KSLOTP01` (cabecera de 64 B + 1 MiB + paleta + base visible).

## Compatibilidad

- Al arrancar, `platform_slots_init()` busca `partida-N.bin` y lo renombra a
  `slot-N.save` si el nuevo fichero no existe.
- Si la migración de arranque no ocurrió, `platform_slots_load()` acepta el
  nombre antiguo, lo carga y lo renombra.
- Si ambos existen, gana `slot-N.save`; el antiguo se deja intacto.
- `kult.ini` sigue usando `save_slot = 0..9`.

## Verificación

`make test`: **519 core + 34 funciones / 223 `CHECK` HAL + diag + 2
herramientas**.

Casos añadidos en `test_slots`:

- el guardado crea `slot-4.save`;
- el arranque migra `partida-4.bin` → `slot-4.save`;
- la carga directa acepta y migra el nombre antiguo;
- los ficheros antiguos se limpian en las pruebas.

`make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win`
OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `1589185f0c705859c30d49bdfe45bdb68c1ee45fa37cacc04069d34e1b62cf06`
(11 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual/funcional del ZIP en Windows, incluyendo que una
partida anterior `partida-N.bin` aparezca como `slot-N.save` tras arrancar.
