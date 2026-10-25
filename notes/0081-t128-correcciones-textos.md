# 0081 — T-128: correcciones de textos catalanes

Fecha: 2026-10-25. Estado: **cerrada**. T-129 añade F5 en dos pasos.
Siguiente: T-094 sobre el ZIP final.

## Cambios

- `DESCC 69`: `S'OBRI LA BARRIGA DE L'ESTATUA.` → **`S'OBRE EL VENTRE DE L'ESTATUA.`**
- `DESCC 174`: `ATRAVIESES AMB FACILITAT...` → **`TRAVESSES AMB FACILITAT...`**
- `DESCC 292`: `LA NAVE DE QRIICH...` → **`LA NAU DE QRIICH...`**
- `DIALC 117`: `S'ATREVE A TORNAR` → **`S'ATREVEIX A TORNAR`**
- `DIALC 127`: `LA BARRIGA DE DEILOS RETUMBA!` → **`EL VENTRE DE DEILOS RUGUEIX!`**
- `DIALC 130`: `T'ATREUS A EMBRUTAR...` → **`T'ATREVEIXES A EMBRUTAR...`**, partido en tres líneas.
- `DIALC 134`: `T'ATREUS A MIRAR...` → **`T'ATREVEIXES A MIRAR...`**
- `DIALC 176`: `NO INTENTA INTERCEPTAR-NOS!` → **`NO VOL ATRAPAR-NOS!`**

`DIALC 130` queda:

```text
VIL PROFANADOR,
T'ATREVEIXES A EMBRUTAR
LA CAMBRA DE L'EXPIACIO!
```

No quedan `BARRIGA`, `ATRAVIESES`, `T'ATREUS`, `S'ATREVE A` ni `NAVE` en los
bancos catalanes.

## Formato

- `DESCC.BIN`: 9.929 B, SHA256 `6368a713…`.
- `DIALC.BIN`: 9.580 B, SHA256 `27cefae6…`.
- `MOTSC.BIN`: 1.093 B, SHA256 `2c631c56…`.
- `VERBC.BIN`: 1.483 B, SHA256 `4e6a0b76…`.
- `CARACC.BIN`: 512 B, SHA256 `04af8b0c…`.
- `assets/language.data`: 78.511 B,
  SHA256 `38ff7f705bb51a06abf02b0e9fcd4cc7b8cebf6409933bcc611a12a4ed342f1f`.

## Verificación

`test_catalan_differential` cubre las ocho entradas y la ausencia de las formas
antiguas. `make test`: 519 core + 36 funciones / 256 `CHECK` HAL + diag + 5
herramientas. `make package-win` OK.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `435df4be54c371883f0e988a9957ea3a888b9ff7008b8ed364a2ed5122ef5fed`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
