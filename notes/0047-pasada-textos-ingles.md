# 0047 — Pasada de textos ingleses sin traducir

**Fecha:** 2026-10-25

## Resultado

Se compararon las entradas del pack español con el banco inglés. Las únicas
frases planas en inglés que no eran diálogo alienígena eran:

- `DIAL 240`: `HORROR! I ORDER YOU TO DIE!`
- `DIAL 241`: `STOP LIVING, OFFA. DEILOS INSISTS! ZORQ DEMANDS!`

Traducciones aplicadas:

- español 240: `HORROR! TE ORDENO QUE MUERAS!`
- español 241: `DEJA DE VIVIR, OFFA. DEILOS INSISTE! ZORQ LO EXIGE!`
- catalán 240: `HORROR! ET MANO MORIR!`

El resto de coincidencias con el inglés son nombres propios, códigos de control,
interjecciones o diálogo alienígena deliberado. T-121–T-123 tradujeron después el bloque deformado del personaje de
`STA KLAROS TA VIEN` (`notes/0074`–`notes/0076`).

## Verificación

- `make test`: 518 core + HAL + 1 port OK.
- `make package-win` correcto.
- `language.data`: `cc8981ae…`
- ZIP: `0fe3118a…`
