# ADR 0005 — Arranque/tick en `boot/`, fuera de `src/`

- **Estado:** aceptada
- **Fecha:** sesión 1

## Contexto
`_start` (0x10000) y el tick `0x160c2` no existen en el core; son piezas nuevas.
Podrían añadirse a `src/` o vivir aparte.

## Decisión
Ponerlas en **`boot/`**, dejando **`src/` intacto**.

## Consecuencias
- El core verificado no se contamina; los 499 tests siguen siendo la referencia.
- Las piezas nuevas se prueban funcionalmente (y diferencialmente donde se pueda).
