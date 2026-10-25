# 0080 — T-127: Ç catalana en la fuente

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Glifo

Se adapta el diseño del usuario a la fuente 4×6 existente. El glifo ocupa el
código **`0x0b`** (el mismo que usa el español para `Ñ`, pero la fuente se carga
por idioma):

```text
1C 10 10 10 1C 04
```

```text
XXX.
X...
X...
X...
XXX.
..X.
```

La `C` conserva el ancho de 3 píxeles del resto de mayúsculas y la cedilla ocupa
la última fila. No hace falta cambiar el renderizador ni la interlineación.

## Fuente catalana

Se añade `assets/lang/ca/CARACC.BIN` (512 B) con el glifo en `0x0b`. `boot.c` ya
cargaba `CARAC<idioma>.BIN`, así que solo se ha añadido el asset. El contenedor
`language.data` pasa a 11 entradas.

## Textos con Ç

Se ha recorrido todo el catalán y se ha sustituido `C` por `0x0b` donde la
ortografía pide cedilla:

- `VERBC`: `LLANÇAR SOBRE`, `LLANÇAR LLIGANYA`.
- `DESCC`: `FORÇA` (0, 175), `LLENÇA` (18), `LLANÇEN` (129), `ABRAÇADA` (156),
  `LLENÇAT` (173), `COMENÇEN` (213), `ABALANÇAR` (227), `COMENÇA` (368),
  `ENLLAÇ` (404).
- `MOTSC`: `RAÇA` (24), `CALÇ VIVA` (77).
- `DIALC`: `ESPERANÇA` (23, 70), `RAÇA` (227, 229, 234, 317, 324), `FORÇA`
  (279, 282), `COMENÇO` (288).

Se mantiene `FORCA` sin cedilla en `DESCC 180` («forca gran i llisa», objeto),
frente a `FORÇA` de fuerza. `PLACA` de `MOTSC 82` se mantiene: es una placa, no
una plaza.

## Formato

- `CARACC.BIN`: 512 B, SHA256 `04af8b0c…`.
- `VERBC.BIN`: 1.483 B, SHA256 `4e6a0b76…`.
- `DESCC.BIN`: 9.932 B, SHA256 `256dd1e6…`.
- `MOTSC.BIN`: 1.093 B, SHA256 `2c631c56…`.
- `DIALC.BIN`: 9.579 B, SHA256 `214bb769…`.
- `assets/language.data`: 78.513 B,
  SHA256 `1809189c19fe9aeb9595d48c8bab8831bdcad37c8faf0bb4910c1f9ce2cfbb68`.

## Verificación

`test_catalan_differential` comprueba el glifo, `CARACC.BIN` en el contenedor y
las palabras con `[0b]`. `make test`: 519 core + 36 funciones / 256 `CHECK` HAL +
diag + 5 herramientas. `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `1c1e8f6f80668e4b9dca2ddd3c78ba805d82005f13ed663e3e5e9d395fcbaf15`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
