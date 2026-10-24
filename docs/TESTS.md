# TESTS — inventario y política

## Inventario actual

| conjunto | ubicación | cantidad | uso |
|---|---|---:|---|
| Tests diferenciales del core | `lib/kult-core/test/` | 518 funciones en 44 `test_*.py` | canónicos del core |
| Copia de tests en el port | `test/` | 519 funciones en 45 `test_*.py` | incluye los 518 del core + `test_ringreport` |
| Tests del kit | `lib/kult-core/tools/recreate_kit/test/` | 339 funciones en 42 ficheros | validación del kit, no de `make test` |
| Tests funcionales HAL | `platform/test/test_platform.c` | 22 funciones / 97 `CHECK` | backend PC |
| Tests de convenciones/mapa/visibilidad | dentro del core | incluidos en los 518 | legibilidad y sincronización |

`make test` en el port ejecuta la copia de `test/` (519).
`make test-port` ejecuta el binario C de la HAL.
Los tests del kit no forman parte de ninguna de las dos suites anteriores.

## ¿Es normal conservarlos?

Sí. Los tests son parte del valor del proyecto: fijan el comportamiento
diferencial, las convenciones y las garantías del port. No deben borrarse para
“limpiar” el código.

Pero **no deben estar duplicados ni empaquetarse en la release**.

## Política recomendada

1. **Core**
   - Los 518 tests viven solo en `lib/kult-core/test/`.
   - Son la fuente canónica y se ejecutan contra el core del subtree.
2. **Port**
   - `test/` debe quedarse solo con pruebas específicas del port.
   - Conservar `test_ringreport.py` y añadir aquí las futuras pruebas de
     ingesta/`kult.data`.
   - Eliminar la copia de los 518 tests del core.
3. **Kit**
   - Los 339 tests del kit se quedan en el subtree como tests del vendor.
   - Se ejecutan aparte, no en `make test`, y no se empaquetan.
4. **HAL**
   - `platform/test/test_platform.c` se mantiene.
   - `make test-port` sigue siendo su entrada.
5. **Release**
   - Ningún test se incluye en el ZIP/TGZ.
   - Los tests que requieren `KULT.PRG` son opcionales para quien clone el repo.

## Limpieza pendiente

- Añadir un target `test-core` que ejecute la suite canónica del subtree.
- Reducir `test/` a pruebas propias del port.
- Mantener `make test` documentado como suite integrada mientras dure la
  transición, y después sustituirlo por `test-core` + `test-port`.
- No borrar tests del kit: son del vendor y tienen su propio `make test`.
