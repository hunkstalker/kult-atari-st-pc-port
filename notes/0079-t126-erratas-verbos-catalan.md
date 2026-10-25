# 0079 — T-126: erratas de verbos catalanes

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## LLENCAR / LLANCAR

`LLENCAR` no es una forma catalana válida. El verbo correcto es **LLANÇAR**
(plegado a `LLANCAR` en la fuente ASCII). El problema era que el banco usaba la
misma forma para dos acciones distintas:

- **81** `LANZAR SOBRE...` → **`LLANÇAR SOBRE`** (`LLANCAR SOBRE`).
- **164** `TIRAR` (*throw away*) → **`TIRAR`**.
- **60** `TIRAR` (*pull*, inglés `PULL`, alemán `ZIEHEN`) → **`ESTIRAR`**.

Así desaparece `LLENCAR` del anillo.

## Otras erratas corregidas

- **1**: `PERTINENCIES` → `PERTINENCES`.
- **13**: `MOURE` → `DESPLACAR` (estaba intercambiado con 17).
- **17**: `DESPLACAR` → `MOURE`.
- **52**: `SACSEJAR` duplicado → `ENCAIXAR LA MA`.
- **71**: `DONAR` → `OFERIR` (intercambiado con 83).
- **83**: `OFERIR` → `DONAR`.
- **145**: `LLIGANYA` → `LLANÇAR LLIGANYA`.

No se tocan las dudas de 6 (`TEMPS`), 87 (`BESAR`), 92 (`BAIXAR`), 144
(`PODER PSI`) y 173 (`DEMANAR INFORMACIO`) sin confirmación.

## Formato

- `VERBC.BIN`: 1.481 B, SHA256 `0e4fad29…`.
- `assets/language.data`: 77.916 B,
  SHA256 `99a1816e1172ddb958e4547ba516c921535d10e60e5b8781c7add351c4cde175`.

## Verificación

`test_catalan_differential` comprueba los diez verbos corregidos y que no quedan
`LLENCAR` ni `LLANCAR` sueltos. `make test`: 519 core + 36 funciones / 255
`CHECK` HAL + diag + 5 herramientas. `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `466ae50bf68256a538f2392b7d4858bd6939426e1b0ef3ebb0d97ce43d5260a2`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
