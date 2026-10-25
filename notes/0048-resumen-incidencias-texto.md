# 0048 — Resumen detallado de las incidencias de texto

**Fecha:** 2026-10-25

## Evidencias

- Run española con freeze en La Horca:
  `build-win/trazas-lang-20261025-es/` (`kult-marcas-es.log`, `kult-es.log`).
- Run catalana con F12 en la entrega de objeto:
  `build-win/trazas-lang-20261025-ca/` (`kult-marcas-ca.log`, `kult-ca.log`).
- Diagnóstico: `KULT_MARKS` + `KULT_AUTOLOG=1`; cada línea de texto deja una
  entrada `T`, F12 deja una instantánea `X` y el anillo de funciones `F`.

## T-097 — cajas de objeto en catalán

Los nombres de objeto de `DESCC.BIN` habían perdido el separador previo a los
códigos de control `[04]`:

- `DESC 374`: faltaba el salto de línea tras `CORDA`;
- `DESC 375`: faltaba el espacio tras `LLANTERNA`;
- `DESC 376`: faltaba el espacio tras `MOSCA`;
- `DESC 377`: faltaba el espacio tras `CALZE`;
- `DESC 378`: faltaba el espacio tras `DAGA`.

El motor calcula el ancho de la caja a partir de esas líneas; al faltar el
separador, el bloque de control cambiaba el ancho y descolocaba el marco.

Fix: restaurados los espacios/saltos exactos, sin cambiar el texto ni los
códigos. Commit del contenido: `c3a18e5`.

## T-098 — bloqueo de La Horca en español

El log español muestra:

- VBL 1002: `T ... ovl_a idx=007f ... str=41fd6 end=41fd5 raw=`;
- `end` anterior a `str`, texto vacío;
- después, el frame queda en 1050 y solo alternan `1910e` (audio) y `10946`
  (`measure_word`).

Causa: el buffer de `MOTS` era de 1.078 B (`0x41b76`–`0x41fac`). `MOTSS.BIN`
ocupa 1.151 B y `MOTSC.BIN` 1.116 B. `PUZZL` se carga después y pisaba la cola
de `MOTS`. En español, el índice usado en La Horca (`0x7f`) correspondía a
`OSCURIDAD`, en el tramo pisado; en catalán, la entrada equivalente
(`OSCURITAT`) quedaba antes del solape y por eso no se reproducía el bloqueo.

Fix: el banco `MOTS` se carga en la zona libre `0x89000`–`0x90000`, y se
redirigen la tabla A (`0x1a3de`) y el inmediato de la rutina de overlay A
(`0x10a46`). No se cambia ningún texto. Commit: `73876c7`.

## T-099 — textos ingleses sin traducir

Se comparó el pack español con el banco inglés. Las únicas frases planas que
quedaban sin traducir eran:

- `DIAL 240`: `HORROR! I ORDER YOU TO DIE!`
  - español: `HORROR! TE ORDENO QUE MUERAS!`
- `DIAL 241`: `STOP LIVING, OFFA. DEILOS INSISTS! ZORQ DEMANDS!`
  - español: `DEJA DE VIVIR, OFFA. DEILOS INSISTE! ZORQ LO EXIGE!`
- catalán 240: `HORROR! ET MANO MORIR!`

El resto de coincidencias son nombres propios, códigos, interjecciones o
diálogo alienígena deliberado. Commit del contenido: `c3a18e5`.

## Detalle de la instrumentación

- `T <vbl> <tag> idx=... w=... str=... end=... cx=... cy=... raw=...`:
  cada línea de texto dibujada.
- `X`: última línea de texto, incluida en F12 y en `M auto`.
- `F`: últimas ~150 VBL de funciones instrumentadas.
- `M auto`: volcado automático cada 150 VBL con `KULT_AUTOLOG=1`.
- `build-win/kult-diagnostico.bat` activa `KULT_MARKS` y `KULT_AUTOLOG`.

Commit de instrumentación: `506da67`.

## Estado final

- `make test`: 518 core + HAL + 1 port OK.
- `make package-win` correcto.
- `language.data`: `cc8981ae…`
- ZIP: `0fe3118a…`
- **Validado por el usuario (2026-10-25):** T-097 (cajas de objeto catalanas) y
  T-098 (La Horca en español). Pendientes: T-094, T-095 y publicación.
