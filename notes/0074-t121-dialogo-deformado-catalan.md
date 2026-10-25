# 0074 — T-121: diálogo deformado traducido al catalán

Fecha: 2026-10-25. Estado: **cerrada**. T-122 rehace estas líneas en clave
catalana. Siguiente: T-094 sobre el ZIP final.

## Localización

La frase `STA KLAROS TA VIEN` está en `DIALS.BIN` 162. En catalán seguía en
inglés deformado:

`SIT YOU ATION EVOLV ING NORMA LEE.`

El mismo personaje (el bloque de diálogo deformado de los tuners) tenía más
líneas sin traducir en `DIALC.BIN`. Se traducen al catalán las nueve entradas de
ese bloque:

- 143: `TUNER? SI... / DESACTIVAT.`
- 147: `TUNER? SI... / RECUPERAT. EN BONA CONDICIO.`
- 155: `TUNER? SI... / CONDICIO INCERTA.`
- 156: `PRECISIO. ESTAT / INQUIETANT.`
- 157: `PORTA / EQUIVOCADA.`
- 162: `ESTA CLAR QUE / ESTA BE.`
- 164: `AQUESTS ESTAN / CLASSIFICATS COM A DEFECTUOSOS. / DESACTIVEU-LOS ARA.`
- 165: `NOMES ES MOR / UNA VEGADA.`
- 166: `SITUACIO / CONFUSA.`

Se conservan los saltos de línea y los códigos de control del banco.

## Descripción 45

Se ajusta la traducción catalana a la petición del usuario:

`UN ASPIRANT COM TU, AMB PRESSA PER ACONSEGUIR ELS CINC CRANIS!`

## Formato

- `DESCC.BIN`: 9.925 B, SHA256 `4abe3268…`.
- `DIALC.BIN`: 9.559 B, SHA256 `42df322e…`.
- `MOTSC.BIN`: 1.092 B, SHA256 `0c7d6419…`.
- `VERBC.BIN`: 1.469 B, SHA256 `a2cf1ba8…`.
- `assets/language.data`: 77.887 B,
  SHA256 `3c554a1de00e4a5a04ce928ea44e52cb00adae6552ffbb6ec1a699151c49badf`.

## Verificación

`test_catalan_differential` comprueba las nueve líneas y la descripción 45 en
fuente y contenedor. `make test`: 519 core + 36 funciones / 255 `CHECK` HAL +
diag + 5 herramientas. `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `79450f59b2c2bfc6b477403c89380709bebbd1f20481a6170ede39d2521849f3`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
