# Idiomas y `language.data`

## Estado

La release incluye `data/language.data` con:

- pantalla de selección `DRAP.PI1` de cinco banderas;
- pack español;
- pack catalán revisado.

El juego acepta `S` y `C`; si un pack falta o no está instalado, registra el
aviso y usa `E`.

## Contenido del contenedor

`language.data` es un `KULTDTA1` tipo `LANG` con estas entradas:

| entrada | uso |
|---|---|
| `DRAP.PI1` | selector de idioma |
| `VERBS.BIN` | español: verbos |
| `DESCS.BIN` | español: descripciones |
| `MOTSS.BIN` | español: nombres |
| `DIALS.BIN` | español: diálogos |
| `CARACS.BIN` | fuente española: glifo `Ñ` |
| `VERBC.BIN` | catalán: verbos |
| `DESCC.BIN` | catalán: descripciones |
| `MOTSC.BIN` | catalán: nombres |
| `DIALC.BIN` | catalán: diálogos |

La release incluye el pack español (ERBE + correcciones) y el pack catalán
revisado, migrado de forma diferencial y con la terminología ERBE unificada en
T-119/T-125. Ambos se cargan desde `language.data`.
La selección y la carga de `S`/`C` están cubiertas por
`platform/test/test_platform.c` (T-095, `notes/0049`).

## Revisión y limitación de la fuente

Se revisaron las 1.051 entradas catalanas y se corrigieron 404. El detalle está en
`notes/0043-revision-catalan.md`.

La fuente original es de 4×6 y solo tiene mayúsculas. El pack catalán se pliega a
caracteres existentes:

- sin tildes ni diéresis;
- `Ç` → `C`;
- `·` → `.`;
- `L·L` se conserva como `L.L`;
- `NY` para el sonido de `Ñ`, si apareciera.

La ortografía está revisada dentro de esas limitaciones y el pack está validado
por el usuario. T-101 corrigió `VERB 116` («DIE», el objeto) a **DADO**/**DAU**;
los usos verbales de «die» en `DIAL` se mantienen como «morir». El español usa
ERBE + correcciones (T-117); el catalán conserva la revisión T-093, la migración
diferencial T-118, el cierre terminológico T-119/T-120 y la traducción del
diálogo deformado T-121/T-125:

- `CALZE`/`CUBILETE` → `GOT` (`OMPLIR GOT`);
- `TRAGO`/`TRAGOS` → `GLOP`/`GLOPS`;
- `TELA` (araña) → `TERANYINA`;
- `FRASCO` → `FLASQUET`;
- `NETWERK`/`REZZO` real → `XARXA TUNER` (ERBE: `RED TUNER`).

Por preferencia del usuario se conservan **`DAGA`**, **`FESOL`** y
**`COMERCIANT`** (T-120). T-121 traduce al catalán las líneas deformadas del
personaje de `STA KLAROS TA VIEN` (`DIALC` 143, 147, 155, 156, 157, 162, 164,
165 y 166) en clave fonética catalana: `LASIT UAC IOESDES EMBOLUPA AM NORMA LYTAT`,
`DES BRAN XAT`, `AI TA UN KIT TANT`, `DES BRAN XEU LOS`, etc. T-125 revisa todo el diálogo en clave (75 entradas) y deja solo sonidos,
nombres, interjecciones y control sin traducir. `DESC 126` sigue la base ERBE.
T-097/T-098 quedaron cerradas; si se quiere ortografía completa con tildes y
`Ç`, hace falta una fuente catalana con esos glifos.

## Generación

Fuentes:

```text
assets/lang/DRAP.PI1
assets/lang/es/{VERBS,DESCS,MOTSS,DIALS,CARACS}.BIN
assets/lang/ca/{VERBC,DESCC,MOTSC,DIALC}.BIN
```

Contenedor:

```sh
python3 tools/build_language_data.py
```

El comando escribe `assets/language.data`.

Para un pack individual de un solo idioma también existe:

```sh
python3 tools/make_language.py --code S \
  --drap assets/lang/DRAP.PI1 \
  --lang-dir <directorio-con-VERBS...> \
  --out data/language.data
```

## Integración

- `CMake` instala `assets/language.data` en `data/language.data`.
- El loader usa `language.data` **antes** que `kult.data`.
- La fuente `CARACS.BIN` se aplica al elegir `S`.
- El catalán no necesita `CARACC.BIN` porque el texto se ha plegado a ASCII.
- `saves/` sigue separado; `language.data` es de solo lectura.

## Verificación

- `make test`.
- `make port` con `KULT_KEY=S` y `KULT_KEY=C`.
- `make package-win` y comprobar que el ZIP incluye `data/language.data`.
- Wine con `kult-headless.exe`, STX + `language.data`, y `KULT_KEY=S`.
