# 0071 — T-118: catalán diferencial desde ERBE

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Criterio

Se compararon las 1.051 entradas catalanas con el nuevo español ERBE +
correcciones. Se conservó la traducción catalana validada allí donde el sentido
seguía siendo equivalente y se actualizaron las entradas con cambios claros de
sentido, término o nombre propio.

Entradas actualizadas: **93**

- `VERBC.BIN`: 9
- `MOTSC.BIN`: 34
- `DESCC.BIN`: 24
- `DIALC.BIN`: 26

Se conservaron las correcciones catalanas de T-093, los nombres propios
(`DEILOS`, `ZORQ`, `QRIICH`, `HARSSK`, `NORMAJEEN`, `SCI FI`, `OFFA`, `DIVO`) y
la ortografía ASCII sin tildes ni `Ç`.

## Ejemplos

- `VERB 104`: `DAGA` → `GANIVET` (ERBE: CUCHILLO).
- `VERB 117`: `ULLS SOLARS` → `HIPERVISIO` (ERBE: HIPERVISION).
- `MOTS 51`: `CENDRA` → `ASH`.
- `MOTS 92`: `CAP DE GRIPAU` → `DESGUAS` (ERBE: DESAGÜE).
- `DESC 73`: `UN ESTRANY SIMBOL INVERTIT` → `UN CURIOS HOMENET AL REVES`.
- `DESC 261`: actualizado al mecanismo mortal del techo de ERBE.
- `DIAL 284`: `AQUEST DEFORME HA DE MORIR` → `QUE S'HI PUDREIXI, AQUEST`.
- `DIAL 327`: actualizado al complot protozorquista de ERBE.
- `DIAL 329`: conserva `SCI FI` y sigue el texto ERBE del Rezzo.

## Formato

- `VERBC.BIN`: 1.479 → 1.467 B.
- `MOTSC.BIN`: 1.116 → 1.095 B.
- `DESCC.BIN`: 9.935 → 9.939 B.
- `DIALC.BIN`: 9.509 → 9.544 B.
- `assets/language.data`: 77.887 B,
  SHA256 `ebdf22b6323d90c44bb843bad968333e5a0dbe82e5e2e8318c971ea57cd6cbd6`.

## Verificación

`make test`: 519 core + 36 funciones / 255 `CHECK` HAL + diag + **5
herramientas**. `test_catalan_differential` cubre recuentos, términos y textos
actualizados en fuente y contenedor.

`make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `3592ff9196a0c98460487b4612831203d9cb3f521548f03995120c71dd9644de`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
