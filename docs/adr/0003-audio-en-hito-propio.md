# ADR 0003 — Audio en hito propio

- **Estado:** aceptada
- **Fecha:** sesión 1

## Contexto
El audio exige emular el **MFP Timer A** (para disparar las ISR C) y el
**PSG/YM2149**. Es trabajo independiente del arranque/render.

## Decisión
Empezar con **audio stub (no-op)** y abordar el audio en un hito propio (M4).

## Consecuencias
- El juego puede correr y validarse antes de tocar audio.
- El motor musical (ya en C) no se toca; se añade el chip y la salida.
