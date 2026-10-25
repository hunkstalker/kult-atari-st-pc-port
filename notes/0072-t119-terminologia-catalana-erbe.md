# 0072 — T-119: terminología catalana alineada con ERBE

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Motivo

La migración diferencial de T-118 conservó la traducción catalana validada, pero
dejó términos antiguos que ya no coincidían con el español ERBE. El usuario
detectó, por ejemplo, `CALZE` donde ERBE usa `VASO` y el catalán actual debe usar
`GOT` (`OMPLIR GOT`). También se unificaron otros términos visibles.

Se mantiene la base catalana revisada en T-093; esta pasada corrige únicamente
las divergencias terminológicas respecto a ERBE.

## Cambios

- `CALZE`/`CUBILETE` → `GOT`; `GOTS` en plural.
- `DAGA` → `GANIVET`; `GANIVETA` del sacrificio → `GANIVET`.
- `TRAGO` → `GLOP`; `TRAGOS` → `GLOPS`.
- `FESOL` → `BEINA` (ERBE: `VAINA`).
- `TELA` (araña) → `TERANYINA`.
- `FRASCO` → `FLASQUET`.
- `COMERCIANT`/`COMERCIAR` → `CANVIADOR` (`GUARDIA CANVIADOR`).
- `NETWERK`/`REZZO` en texto real → `XARXA TUNER` (ERBE: `RED TUNER`).
  Las líneas alienígenas en inglés de `DIALC` 184, 238 y 239 conservan
  `NETWERK` como parte del alienígena deliberado.

Entradas modificadas:

- `VERBC.BIN`: 39, 111, 114, 153, 155.
- `DESCC.BIN`: 50, 175, 196, 273, 292, 339, 349, 350, 356, 363, 377, 378, 404.
- `MOTSC.BIN`: 114.
- `DIALC.BIN`: 13, 14, 15, 24, 174, 178, 182, 186, 193, 329, 330, 332.

`DESCC` 292 y 404 se alinean además con el texto ERBE que habla de la nave de
Qriich y de la `RED TUNER`.

## Formato

- `VERBC.BIN`: 1.467 → 1.471 B, SHA256 `9689c5d0…`.
- `MOTSC.BIN`: 1.095 → 1.091 B, SHA256 `440ce9e9…`.
- `DESCC.BIN`: 9.939 → 9.934 B, SHA256 `b00bf162…`.
- `DIALC.BIN`: 9.544 → 9.558 B, SHA256 `5c9c74b9…`.
- `assets/language.data`: 77.896 B,
  SHA256 `0d9582d961f23bc0214b605387404f03348d3abcf2bfb9a43273f3d7be9a0ad9`.

## Verificación

`test_catalan_differential` comprueba recuentos, entradas concretas y ausencia de
`CALZE`, `DAGA`, `TRAGO`, `FESOL`, `TELA` y `REZZO` en los cuatro bancos catalanes.
También verifica `XARXA TUNER` en `DESCC 404` y el contenedor `language.data`.

`make test`: 519 core + 36 funciones / 255 `CHECK` HAL + diag + 5 herramientas.
`make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `3d8cf7d6d0320479f7fe4812f0eaaa4165ff4f612e5b0b1cfae27ef28a322ac1`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
