# test/ — pruebas de herramientas del port

La suite diferencial del core **no vive aquí**: está en
`lib/kult-core/test/` (518 pruebas) y se ejecuta con:

```sh
make test-core
```

Las pruebas de la HAL, la ingesta y los packs `S`/`C` tampoco viven aquí:
están en `platform/test/test_platform.c` y se ejecutan con `make test-port`.

En esta carpeta quedan solo las pruebas de herramientas propias del port. Ahora
mismo:

- `test_ringreport.py` — prueba de `tools/ringreport.py` con una traza `R`
  sintética.

Se ejecutan con `make test-tools` (y `make test` las incluye). No duplicar aquí
tests que ya estén en el core o en `platform/test/`.
