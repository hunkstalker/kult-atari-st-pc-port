# 0075 — T-122: diálogo deformado en clave catalana

Fecha: 2026-10-25. Estado: **cerrada**. T-123 fija la clave fonética final.
Siguiente: T-094 sobre el ZIP final.

## Referencias en otros idiomas

El texto original está escrito como una deformación fonética deliberada. Para el
mismo bloque:

- español, `DIAL 162`: `STA KLAROS TA VIEN` → «está claro, está bien»;
- inglés, `DIAL 162`: `SIT YOU ATION EVOLV ING NORMA LEE.`;
- francés, `DIAL 162`: `S'EKLAIR ET S'EST NETT`;
- alemán, `DIAL 162`: `AHLE\nSIN BUH TA.`.

Se han usado esas referencias para reconstruir el sentido y crear una clave
catalana equivalente, no una traducción plana.

## Clave catalana

Se mantiene la idea de habla deformada: caída de la `E` inicial, `K` por sonido
duro, `S` por sonido suave, supresión de letras y palabras partidas.

- 143: `TU NER? SI... / DESAKTIVAT.`
- 147: `TU NER? SI... / REKUPERAT. EN BON ESTAT.`
- 155: `TU NER? SI... / LOCALITZAT. ESTAT INKERT.`
- 156: `PRESISIO. ESTAT / INKIETANT.`
- 157: `PORTA / EKIVOKADA.`
- 162: `STA KLAR, / STA BE.`
- 164: `AKETS ESTAN / KLASIFIKATS KOM A DEFEKTUOSOS. / DESKONEKTEU-LOS ARA.`
- 165: `NOMES ES MOR / UNA VEGADA.`
- 166: `SITUASIO / KONFUSA.`

`DESCC 45` se mantiene como se cerró en T-121:
`UN ASPIRANT COM TU, AMB PRESSA PER ACONSEGUIR ELS CINC CRANIS!`.

## Formato

- `DIALC.BIN`: 9.552 B, SHA256 `751c7c3b…`.
- `DESCC.BIN`: 9.925 B, SHA256 `4abe3268…`.
- `MOTSC.BIN`: 1.092 B, SHA256 `0c7d6419…`.
- `VERBC.BIN`: 1.469 B, SHA256 `a2cf1ba8…`.
- `assets/language.data`: 77.880 B,
  SHA256 `cd4c9a04b3e13ee5822c96e9da8c79e5bbe9f2add99f34c8e93ad162f6e37e86`.

## Verificación

`test_catalan_differential` comprueba las nueve líneas con la clave catalana y
la descripción 45 en fuente y contenedor. `make test`: 519 core + 36 funciones /
255 `CHECK` HAL + diag + 5 herramientas. `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `a6a982ee7eb735ebefe7e4126b443ab6a2e4d5a3effa1e984adc38132a594d24`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
