# 0076 — T-123: clave fonética catalana creativa

Fecha: 2026-10-25. Estado: **cerrada**. T-124 ajusta `DIALC 162`.
Siguiente: T-094 sobre el ZIP final.

## Criterio

El texto no se traduce de forma plana: se cortan palabras, se cambian letras y
sílabas por otras que suenan igual, y se parte la frase como en el original.
Las referencias son las versiones inglesa, francesa y alemana del mismo bloque:

- inglés: `SIT YOU ATION EVOLV ING NORMA LEE.`;
- francés: `S'EKLAIR ET S'EST NETT`;
- alemán: `AHLE\nSIN BUH TA.`;
- español: `STA KLAROS TA VIEN`.

Para `DIAL 162` se usa la propuesta del usuario:
`LASIT UAC IOESDES EBOLUPA AM NORMA YTAT`.

## Líneas finales

- 143: `TU NER? SAI FAI... / DES BRAN XAT.`
- 147: `TU NER? SAI FAI... / REKUPE RAT. IN GUT / ESTAT.`
- 155: `TU NER? SAI FAI... / LOKA LITZAT. / AI TA UN IN SERT.`
- 156: `PRE SISI. / AI TA UN KIT TANT.`
- 157: `PORTA / RONGA.`
- 162: `LASIT UAC IOESDES / EBOLUPA AM NORMA YTAT`
- 164: `AKETS SON KLASI FIKATS / DEFEK TUOSOS. / DES BRAN XEU LOS ARA.`
- 165: `NO MES SA MOR / UNA BE GADA.`
- 166: `LA SITU ACIO / IN KLE RA.`

`DESCC 45` se mantiene:
`UN ASPIRANT COM TU, AMB PRESSA PER ACONSEGUIR ELS CINC CRANIS!`.

## Formato

- `DIALC.BIN`: 9.575 B, SHA256 `29aed3d5…`.
- `DESCC.BIN`: 9.925 B, SHA256 `4abe3268…`.
- `MOTSC.BIN`: 1.092 B, SHA256 `0c7d6419…`.
- `VERBC.BIN`: 1.469 B, SHA256 `a2cf1ba8…`.
- `assets/language.data`: 77.903 B,
  SHA256 `923dba182ac1bf1e4126be2e374fdc9e0c438cc2bdabfe5f642fdc57edebe656`.

## Verificación

`test_catalan_differential` comprueba las nueve líneas en fuente y contenedor.
`make test`: 519 core + 36 funciones / 255 `CHECK` HAL + diag + 5 herramientas.
`make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `229db6476db6870d0496000ae8a1e1e72b58de94a496e525e5b223132b5b8f37`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
