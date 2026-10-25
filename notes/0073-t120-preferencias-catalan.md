# 0073 — T-120: preferencias de términos catalanes

Fecha: 2026-10-25. Estado: **cerrada**. T-121 traduce después el diálogo
deformado. Siguiente: T-094 sobre el ZIP final.

## Decisión del usuario

Se mantienen tres términos catalanes que T-119 había cambiado:

- **`DAGA`** en lugar de `GANIVET`.
- **`FESOL`** en lugar de `BEINA`.
- **`COMERCIANT`** en lugar de `CANVIADOR`.

El resto de T-119 se conserva: `CALZE`→`GOT`, `TRAGO`→`GLOP`,
`TELA`→`TERANYINA`, `FRASCO`→`FLASQUET` y `NETWERK`/`REZZO`→`XARXA TUNER`.

## Entradas modificadas

- `VERBC.BIN`: 104 (`DAGA`), 115 (`FESOL`), 155 (`DONAR EL FESOL PER MENJAR`).
- `DESCC.BIN`: 175, 273, 363 y 378 vuelven a `DAGA`.
- `MOTSC.BIN`: 124 vuelve a `COMERCIANT`.
- `DIALC.BIN`: 13, 14, 15, 24 y 332 vuelven a `COMERCIANT`/`COMERCIAR`.

## Líneas alienígenas

`DIALC` 184, 238 y 239 son intervenciones de alienígenas en inglés deformado.
Se dejó `NETWERK` en ellas porque forma parte del lenguaje alienígena deliberado,
no del texto catalán normal:

- 184: `THE NETWERK THANKZYOO FOR LOKATINGTHEM, RAY VENN.`
- 238: `THE NETWERK MUSSTROOL, NOT THE KREECH.`
- 239: `YOR ORDERZ AR TO GARD SAW SER TILNETWERK TEEM ARE IVES.`

Si se prefiere, pueden cambiarse también, pero perderían parte del efecto de
“lengua alienígena”.

## Formato

- `VERBC.BIN`: 1.469 B, SHA256 `a2cf1ba8…`.
- `MOTSC.BIN`: 1.092 B, SHA256 `0c7d6419…`.
- `DESCC.BIN`: 9.925 B, SHA256 `d306836c…`.
- `DIALC.BIN`: 9.575 B, SHA256 `8bb3bf29…`.
- `assets/language.data`: 77.903 B,
  SHA256 `a3e606b56ff95d17bcb5c8b89bf179bc3af7795da5672a96f96ad3ccecbb7710`.

## Verificación

`test_catalan_differential` comprueba `DAGA`, `FESOL` y `COMERCIANT`, y la
ausencia de `BEINA` y `CANVIADOR` en los cuatro bancos catalanes. Se mantiene la
regresión de `CALZE`, `TRAGO`, `TELA` y `REZZO`.

`make test`: 519 core + 36 funciones / 255 `CHECK` HAL + diag + 5 herramientas.
`make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `de37885a8b68a2c2da25fbbf977eb65749dd8fc82227c3235d236c2f5dbae483`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
