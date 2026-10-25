# T-101 — «DIE» es el dado, no «morir» (ES/CA)

## Problema

En la lista de verbos, `VERB 116` (`DIE`) se mostraba como **MORIR** en español y
en catalán. El original francés es `DE` y el inglés `DIE` funcionan aquí como
**sustantivo**: el objeto de juego «dado», no el verbo «morir». La traducción se
hizo sin contexto y arrastró el verbo.

No se tocan los `DIAL` con «die» verbal (`HORROR! I ORDER YOU TO DIE!`, etc.),
que están bien como «morir». `DESC 36` y `DESC 39` ya usaban «DADO»/«DAU».

## Evidencia

- `oficial.tsv` / `revision.tsv`: `VERB 116 DE DIE DADO`.
- `work/erbe/VERBS.BIN`, entrada 116: `e4 24 2f` = **DADO** (edición Erbe).
- Packs actuales antes del fix: `2d 2f 32 29 32` = **MORIR** en `VERBS.BIN` y
  `VERBC.BIN`.
- `DESCC.BIN` 36/39 ya contienen `EL DAU` / `UN PETIT DAU`, lo que confirma la
  forma catalana.

## Cambio

Se sustituye solo la entrada 116 de cada pack:

| pack | antes | después | bytes |
|---|---|---|---|
| `assets/lang/es/VERBS.BIN` | MORIR | **DADO** | `e4 24 2f` (copiados de Erbe) |
| `assets/lang/ca/VERBC.BIN` | MORIR | **DAU** | `e4 35` |

El resto de los bancos queda intacto. `assets/language.data` se regenera con
`python3 tools/build_language_data.py`.

## Verificación

- Nueva regresión `test/test_language_banks.py`: decodifica la entrada 116 en
  los BIN y en `language.data` y exige `DADO`/`DAU` (y descarta `MORIR`).
- `make test`: 519 core + HAL (30 funciones/163 `CHECK`) + diagnóstico off/on +
  2 tests de herramientas.
- `make package-win` OK.

## Artefactos

- `assets/language.data`: 77.747 B,
  SHA256 `e69cae5a6ac0832415f9a4e4086609fb89b6577bcb6fced6cb998c82e608629b`.
- ZIP: `build-win/kult-1.0.0-win64.zip`, SHA256
  `aba7eca812cdd49c334c46658d3a52d645fb9395a9ea5f59f0d65936b0b1643c`.
- T-094 debe validar este ZIP, no el de T-022.

## Nota de fuente

El `es.json` del repo legacy de referencia sigue diciendo `MORIR`; no se toca
(regla 5: repos de referencia en solo lectura). Los packs canónicos del port ya
están corregidos y la regresión evita que vuelva a colarse.
