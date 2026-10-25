# 0043 — Revisión completa del catalán

**Fecha:** 2026-10-24
**Validación del usuario:** 2026-10-25
**Alcance:** textos catalanes de `language.data`. Sin cambios de motor.

## Revisión

Se revisaron las **1.051 entradas** de los cuatro bancos catalanes:

| banco | entradas | cambios |
|---|---:|---:|
| `VERBC.BIN` | 176 | 49 |
| `MOTSC.BIN` | 131 | 33 |
| `DESCC.BIN` | 407 | 173 |
| `DIALC.BIN` | 337 | 149 |
| **total** | **1.051** | **404** |

Criterios aplicados:

- catalán central, registro del juego y mayúsculas;
- corrección de gramática, artículos, preposiciones, tiempos verbales y
  concordancia;
- normalización de nombres propios: `DEILOS`, `ZORQ`, `QRIICH`, `HARSSK`,
  `NORMAJEEN`, `SCI FI`, `OFFA`, `DIVO`;
- `CRANEU` → `CRANI`; `CRANIS` en plural;
- `CALIZ` → `CALZE`; `SACERDOTISA` → `SACERDOTESSA`;
- `MONO` → `MICO`; `TERANYANYA` → `TERANYINA`;
- `GALLEDA` → `CUB`; `LLICANT` → `LLISCANT`; `HIRBENT` → `BULLENT`;
- `AQUESTES`/`AQUESTA` usados como “you are” → `ETS`/`ESTA` según contexto;
- correcciones manuales del usuario en el Excel: 57 entradas adicionales;
- las seis dudas de “LA TRAMPILLA” se resuelven como `PORTELLA` (son trap-door);
- revisión de las frases de las primeras pantallas y de los textos de objetos,
  combate, final y diálogos;
- los diálogos deliberadamente deformados se mantienen en su forma original.

## Formato

El pack sigue en ASCII mayúsculas por la fuente original de 4×6:

- sin tildes ni diéresis;
- `Ç` → `C`;
- `·` → `.`;
- `L·L` se conserva como `L.L`;
- se mantienen los códigos de control `[xx]` y los saltos de línea.

Por tanto, la ortografía está revisada dentro de esas limitaciones; falta la
validación visual final con la fuente del juego.

## Tamaños

| banco | anterior | revisado | original E |
|---|---:|---:|---:|
| `VERBC.BIN` | 1.466 B | 1.482 B | 1.314 B |
| `MOTSC.BIN` | 1.112 B | 1.116 B | 1.082 B |
| `DESCC.BIN` | 10.015 B | 9.948 B | 10.491 B |
| `DIALC.BIN` | 9.457 B | 9.511 B | 9.635 B |

## Verificación

- Los cuatro bancos se decodificaron de nuevo y coinciden entrada a entrada con
  el texto revisado.
- `language.data`: 10 entradas, 77.742 B,
  SHA256 `855df7c60030e8ed52d2e44a0100fbb07904b4e40a011916be71fd172d78cd27`.
- `make test`: **518 core + HAL + 1 port**, verde.
- `make package-win` correcto.
- ZIP: `build-win/kult-1.0.0-win64.zip`,
  SHA256 `a11a6948b0a3a6658c89719469f56d47df3be761b4ba0928888639ca25c137bc`.

## Validación

El usuario da por validado el idioma catalán el **2026-10-25**. Las dos
incidencias abiertas no son de traducción:

- **T-097**: texto descuadrado al recibir objetos, por ejemplo “una mosca de
  pedra”;
- **T-098**: bloqueo en la prueba “La Horca” con `kult.data` + `language.data`.

Sigue pendiente la validación visual final de la release completa (T-094) y, si
se quiere ortografía con tildes y `Ç`, una fuente catalana con esos glifos.
