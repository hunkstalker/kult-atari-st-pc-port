# Índice de documentación

Punto de entrada a los documentos de `docs/`. Para el estado y las tareas, ver
`PROYECTO.md`, `TODO.md` y `RETOMAR.md`.

| documento | para qué |
|---|---|
| [`PLAN.md`](PLAN.md) | plan por fases/hitos con objetivos y criterios |
| [`ARQUITECTURA.md`](ARQUITECTURA.md) | capas (core/boot/platform/app), contrato, flujo |
| [`PORTING.md`](PORTING.md) | **frontera de port**: los 19 servicios y sus equivalentes PC; piezas que faltan |
| [`VERIFICACION.md`](VERIFICACION.md) | pruebas del core y del backend |
| [`BUILD.md`](BUILD.md) | compilación (CMake + SDL3), Windows/Linux |
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
