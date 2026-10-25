# 0045 — Cajas de texto de objetos en catalán

**Fecha:** 2026-10-25

## Causa

Los textos de objeto de `DESCC.BIN` habían perdido el espacio o el salto de línea
que separa el nombre del bloque de control `[04]`:

- `UNA\nCORDA[04]...` en vez de `UNA\nCORDA\n[04]...`;
- `UNA\nLLANTERNA[04]...` en vez de `UNA\nLLANTERNA [04]...`;
- `UNA\nMOSCA[04]...`, `UN\nCALZE[04]...`, `UNA\nDAGA[04]...`.

Eso cambiaba el ancho calculado y descolocaba la caja.

## Fix

Restaurados los espacios/saltos de línea en `DESC` 374–378, manteniendo los
códigos `[04]` y el texto catalán.

## Verificación

- **Validado por el usuario (2026-10-25):** los espacios extra desaparecieron y las cajas de objeto catalanas quedan correctas.
- `make test`: 518 core + HAL + 1 port OK.
- `make package-win` correcto.
- `language.data`: `1728b48c…`
- ZIP: `f81c6cbe…`
