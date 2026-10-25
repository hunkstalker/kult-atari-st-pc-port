# Core importado y delta del port

El core C vive en `lib/kult-core/`, importado del repositorio
[kult-atari-st-c](https://github.com/hunkstalker/kult-atari-st-c) mediante
`git subtree`.

- Última importación: commit `94c0be1` del repo C (T-022; `load_resource_list_c`).
- Importación inicial: commit `beebb52` (subtree con `--squash`).
- El build del port usa `lib/kult-core/src` y `lib/kult-core/include`.
- Las herramientas y tests del core viven también en el subtree.
- No se compilan `lib/kult-core/docs`, `test` ni `tools` salvo que se invoquen
  explícitamente.

## Delta local del port

Estos ficheros del core llevan cambios propios del port:

- `lib/kult-core/src/commands.c`
- `lib/kult-core/src/dispatch.c`
- `lib/kult-core/src/handlers.c`
- `lib/kult-core/src/input.c`
- `lib/kult-core/src/loop.c`
- `lib/kult-core/src/mode.c`
- `lib/kult-core/src/text.c`
- `lib/kult-core/src/video.c`
- `lib/kult-core/include/kult_port.h`

Los cambios son:

1. Instrumentación de diagnóstico (`kult_diag_*`/`kult_text_ev`): llamadas
   acotadas en el core; la implementación vive en `platform/diag.c` y en
   el diferencial son no-op. Ver `DIAGNOSTICO.md`.
2. Espera del sample 0x1f al final de la puerta (`mode.c`), específica del
   timing del port.
3. Hooks/declaraciones no-op para que el core siga compilando sin el port.

## Actualizar desde el repo C

```sh
git subtree pull \
  --prefix=lib/kult-core \
  https://github.com/hunkstalker/kult-atari-st-c.git main \
  --squash
```

Si el pull toca alguno de los ficheros del delta, hay que resolver el conflicto
manteniendo la instrumentación y el shim del port. No se debe borrar el delta
sin revisar antes el comportamiento del port.

## Regenerar

```sh
make addrs
make mapa
make regscan
```

`make addrs` y `make mapa` escriben en `lib/kult-core/include` y
`lib/kult-core/docs/mapa`.
