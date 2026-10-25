# 0078 — T-125: revisión completa del diálogo en clave

Fecha: 2026-10-25. Estado: **cerrada**. T-126 corrige los verbos catalanes.
Siguiente: T-094 sobre el ZIP final.

## Alcance

Se revisaron las 337 entradas de `DIALC.BIN` comparándolas con los bancos
inglés, francés, alemán y español. Se tradujeron al catalán fonético **75
entradas** que seguían en inglés o en clave inglesa.

Se mantuvieron sin cambios las 15 entradas que son solo nombres, sonidos,
interjecciones o control:

`44, 203, 217, 221, 277, 289, 296, 301, 302, 307, 312, 313, 314, 315, 336`.

También se conservan los nombres propios y los códigos `[xx]`.

## Criterio

Se cortan palabras, se cambian letras y sílabas por otras que suenan igual y se
parte la frase como en el original. Referencias usadas: inglés, francés y
alemán, además del español.

Ejemplos:

- `162`: `LASIT UAC IOESDES / EMBOLUPA AM NORMA LYTAT`.
- `11`: `JO SOK EL MASTEL / DELS ESKORPIONS. / KIN FAM KIN TINK!`.
- `42`: `RAY VUN EN KONTAKTE / AMB DOS NOUS!`.
- `80`: `KE! UN TU NER / LLARDOS!`.
- `184`: `LA XARXA TU NER / DONA GRA SIES PER / LOKA LITSAR LOS, RAY VEN.`.
- `238`: `LA XARXA TU NER / A DE MANAR, / NO EL KRICH.`.
- `242`: `LA SITU ASIO / EBOLU SIONA NORMAL MEN.`.
- `294`: `OLA LAVEN. / HEH HEH. JO TAMBE!`.
- `309`: `TU NER RAY VUN / EN POSI SIO / FABO RABLE.`.
- `311`: `SITU ASIO KRITI KA / PER AL TU NER / RAY VUN.`.

## Formato

- `DIALC.BIN`: 9.576 B, SHA256 `9c293a0a…`.
- `DESCC.BIN`: 9.925 B, SHA256 `4abe3268…`.
- `MOTSC.BIN`: 1.092 B, SHA256 `0c7d6419…`.
- `VERBC.BIN`: 1.469 B, SHA256 `a2cf1ba8…`.
- `assets/language.data`: 77.904 B,
  SHA256 `2e83f042e5e1aeb1c38f3ad205c287d2e3a164f8437686e59b0466f7b07f1ee8`.

## Verificación

`test_catalan_differential` cubre una muestra amplia de las nuevas líneas y
comprueba que las 75 entradas traducidas no conservan las claves inglesas
conocidas. `make test`: 519 core + 36 funciones / 255 `CHECK` HAL + diag + 5
herramientas. `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `295465c6884421d39c49945bbbb82a120e3a8de03fa1e219ff0ceb92852c3608`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
