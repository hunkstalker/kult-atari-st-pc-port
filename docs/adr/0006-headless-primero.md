# ADR 0006 — Empezar headless

- **Estado:** aceptada
- **Fecha:** sesión 1

## Contexto
El primer hito es validar arranque, ficheros, entrada y tick. Una ventana añade
variables (drivers, display, HiDPI) que estorban al depurar.

## Decisión
**M1 sin ventana**: volcar el framebuffer a fichero y usar entrada simulada.
La ventana SDL llega en M3.

## Consecuencias
- Depuración más rápida y reproducible.
- El render headless y el de SDL comparten la conversión planar→RGBA (golden).
