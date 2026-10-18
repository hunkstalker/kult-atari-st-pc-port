# Kult: The Temple of Flying Saucers — Compilación PC (Atari ST + Amiga)

Port nativo a PC (**Windows y Linux**) de **Kult** (Exxos/ERE, 1989) que **une
las versiones Atari ST y Amiga**, con mejoras y **todos los idiomas**.

No parte de cero: parte del **C verificado** del Atari ST
(`/app/kult-atari-st-c`, 415 funciones, 499 pruebas diferenciales) y añade un
**backend de PC** (SDL3) para que ese mismo C corra en Windows/Linux.

> **Proyecto en fase inicial (M0).** Este repositorio nace **documentado**; el
> código todavía no se ha importado. El estado vivo está en **`PROYECTO.md`** y
> las tareas en **`TODO.md`**; el punto de retorno entre sesiones, en
> **`RETOMAR.md`**. Aquí no se duplica: `README` solo orienta.

## Documentación
- **`PROYECTO.md`** — contexto maestro, arquitectura y estado.
- **`TODO.md`** — tareas pendientes / en curso / completadas.
- **`RETOMAR.md`** — handoff de sesión (qué retomar y cómo).
- **`AGENTS.md`** — reglas y arranque para agentes/colaboradores.
- **`docs/INDICE.md`** — índice de toda la documentación.
- **`notes/`** — notas de sesión (histórico).

## Objetivo (resumen)
- Juego nativo en C, **sin emulador** (Musashi solo se usa como oráculo en las
  pruebas del core).
- **Dos experiencias**: Atari ST y Amiga; perfil libre (arte/audio/ritmos).
- **Todos los idiomas**.
- Backend **SDL3** (ventana, entrada, audio, GPU) + renderizador planar→RGB.

## Origen y repos
- **`/app/kult-atari-st-decomp`** — decomp ASM del ST, byte a byte (spec, no se toca).
- **`/app/kult-atari-st-c`** — C verificado del ST (**base de este repo**).
- **`/app/kult-port-pc-compilation`** — este repo (la compilación).
- **`/app/kult-port-legacy`** — predecesor de modding (solo referencia).

Detalle en `PROYECTO.md`.

## Legal
El juego sigue bajo copyright (Piko Interactive). **No** se distribuyen assets ni
binarios; el usuario aporta su copia. Todo lo derivado del juego queda
**gitignored**.
