# ADR 0004 — Menú de idioma fiel al original

- **Estado:** aceptada
- **Fecha:** sesión 1

## Contexto
La selección de idioma vive en `_start` (0x10000): espera una tecla **D/E/F** y
escribe la letra en las fichas de bancos. Las pantallas son assets.

## Decisión
Mantener la **espera de tecla fiel** al portar `_start`; valorar un menú nativo
solo como mejora posterior.

## Consecuencias
- Menos código y más fiel al original.
- Un menú nativo queda como mejora opcional (documentada).
