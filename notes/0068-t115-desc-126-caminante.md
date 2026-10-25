# 0068 — T-115: DESC 126 más fiel al inglés

Fecha: 2026-10-25. Estado: **cerrada**. Sustituida por T-116 (base ERBE).
Siguiente: T-094 sobre el ZIP final.

## Texto

Original inglés (oficial):

```text
THE
PASSERBY SAVES SOME, THEN MAKES HIS CHOICE
KNOWN TO THE
SLIDING ONE'S HEAD.
```

Antes (ES/CA): “guarda algo” / “guarda alguna cosa”, que interpretaba mal
“saves some”.

Nuevo español (equilibrado, sin tildes por la fuente 4×6):

```text
EL CAMINANTE SALVA A ALGUNOS E INDICA SU ELECCION AL REPTIL.
```

Nuevo catalán:

```text
EL CAMINANT SALVA ALGUNS I INDICA LA SEVA ELECCIO AL REPTIL.
```

Se mantiene “caminante/caminant” en vez de “transeúnte” para conservar el tono,
se usa plural (“algunos/alguns”) y se conserva la adaptación “reptil” de
“the sliding one”.

## Implementación

- `assets/lang/es/DESCS.BIN`: entrada 126 reemplazada, banco 10.250 → 10.237 B.
- `assets/lang/ca/DESCC.BIN`: entrada 126 reemplazada, banco 9.953 → 9.934 B.
- `assets/language.data` regenerado (77.715 B,
  SHA256 `d485252a724a808d37e83ae58f775c8daac5f69ccc83ac7aab794ca904c50b21`).
- Nueva regresión `test_desc_126_passerby` en `test/test_language_banks.py`
  (fuente y contenedor, ES/CA).

## Verificación

- `make test`: 519 core + 36 funciones / 255 `CHECK` HAL + diagnóstico +
  **3 herramientas**.
- `make package-win` OK; `data/language.data` del ZIP coincide con el generado.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `1384cec5817c48fed1bba2a52e907aa56cfe089e28978987e6c9925a62657792`
(12 entradas, `saves/` vacía).

## Pendiente

T-094 sobre el ZIP final.
