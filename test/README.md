# test/ — pruebas propias del port

La suite diferencial del core **no vive aquí**: está en
`lib/kult-core/test/` (518 pruebas) y se ejecuta con:

```sh
make test-core
```

En esta carpeta quedan solo las pruebas específicas del port. Ahora mismo:

- `test_ringreport.py` — prueba de `tools/ringreport.py` con una traza `R`
  sintética.

Las futuras pruebas de ingesta `.STX`, `kult.data`, `language.data` y del
formato portable irán aquí. No duplicar aquí tests que ya estén en el core.
