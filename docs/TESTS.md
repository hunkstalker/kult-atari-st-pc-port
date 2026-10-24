# TESTS — inventario y política

## Inventario actual

| conjunto | ubicación | cantidad | uso |
|---|---|---:|---|
| Tests diferenciales del core | `lib/kult-core/test/` | 518 funciones en 44 `test_*.py` | canónicos del core |
| Tests propios del port | `test/` | 1 función en `test_ringreport.py` | ringreport |
| Tests del kit | `lib/kult-core/tools/recreate_kit/test/` | 339 funciones en 42 ficheros | validación del kit, no de `make test` |
| Tests funcionales HAL | `platform/test/test_platform.c` | 22 funciones / 97 `CHECK` | backend PC |
| Tests de convenciones/mapa/visibilidad | dentro del core | incluidos en los 518 | legibilidad y sincronización |

`make test` ejecuta:
- `make test-core` → 518 tests del subtree;
- `make test-port` → binario C de la HAL;
- la suite de `test/` → `test_ringreport`.

`make test-core`, `make test-port` y `make test-tools` se pueden lanzar por separado.
Los tests del kit no forman parte de ninguna de estas suites.

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
   - `test/` queda solo con pruebas específicas del port.
   - `test_ringreport.py` se mantiene.
   - Las futuras pruebas de ingesta/`kult.data`/`language.data` van aquí.
3. **Kit**
   - Los 339 tests del kit se quedan en el subtree como tests del vendor.
   - Se ejecutan aparte, no en `make test`, y no se empaquetan.
4. **HAL**
   - `platform/test/test_platform.c` se mantiene.
   - `make test-port` sigue siendo su entrada.
5. **Release**
   - Ningún test se incluye en el ZIP/TGZ.
   - Los tests que requieren `KULT.PRG` son opcionales para quien clone el repo.

## Estado de la limpieza

Hecho:

- Añadido `make test-core`.
- Eliminada la copia de los 518 tests del core en `test/`.
- `test/` reducido a `test_ringreport.py` + README.
- `make test` compone core + HAL + tests propios.

Sin hacer:

- Añadir tests nuevos de ingesta y contenedores; irán en `test/`.
- Ejecutar los 339 tests del kit en CI; de momento siguen siendo un target
  separado del vendor.
