# Idiomas y `language.data`

## Estado

La release incluye `data/language.data` con:

- pantalla de selección `DRAP.PI1` de cinco banderas;
- pack español;
- pack catalán inicial.

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

La release incluye el pack español y un pack catalán inicial. Ambos se cargan
desde `language.data`.

## Limitación de la fuente catalana

La fuente original es de 4×6 y solo tiene mayúsculas. El pack catalán se ha
plegado a caracteres existentes:

- sin tildes ni diéresis;
- `Ç` → `C`;
- `·` → `.`;
- `NY` para el sonido de `Ñ`, si apareciera.

Por eso el catalán es legible pero **no está ortográficamente revisado**. Queda
pendiente una revisión lingüística y, si se quiere, una fuente con `À Ç È Ï Ò`
y el punto volat.

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
