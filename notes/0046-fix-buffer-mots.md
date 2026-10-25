# 0046 — Fix del buffer de MOTS

**Fecha:** 2026-10-25

## Causa

El banco `MOTS` (overlay A) se cargaba en `0x41b76`, con solo 1.078 B hasta
`PUZZL` (`0x41fac`). Los packs de español (1.151 B) y catalán (1.116 B) podían
superar ese hueco; `PUZZL` pisaba las últimas entradas. En español, La Horca
pedía el índice `0x7f` (`OSCURIDAD`), que quedaba vacío y bloqueaba
`measure_word`.

## Fix

El port mueve el banco `MOTS` a la zona libre entre los bancos ECHE y las
pantallas:

- `0x89000`–`0x90000`;
- se redirigen la entrada de la tabla A (`0x1a3de`) y el inmediato de la rutina
  de overlay A (`0x10a46`);
- `overlay_a_draw` usa `platform_overlay_a_bank()`.

Así no se toca ningún texto y el banco conserva todo su contenido.

## Verificación

- `make test`: 518 core + HAL + 1 port OK.
- `make package-win` correcto.
- `language.data`: `1728b48c…`
- ZIP: `10c17549…`
