# 0044 — Diagnóstico de texto y bloqueos con F12

**Fecha:** 2026-10-25

## Qué registra

Con `KULT_MARKS` activo:

- `T`: cada línea de texto dibujada, con VBL, banco lógico, índice, ancho de
  caja, inicio/fin de cadena, cursor y bytes crudos.
- `X`: instantánea de la última línea de texto, también en los volcados
  automáticos y F12.
- `F`: últimas ~150 VBL de funciones instrumentadas.
- `M f12`: marca manual.
- `M auto`: volcado automático cada 150 VBL con `KULT_AUTOLOG=1`.

## Cómo reproducir

Desde `build-win/`:

```bat
kult-diagnostico.bat
```

El batch define:

```bat
set KULT_MARKS=%~dp0kult-marcas.log
set KULT_AUTOLOG=1
```

- **T-097**: cuando aparezca el texto descuadrado, pulsar **F12**.
- **T-098**: entrar en “La Horca”; si se bloquea, cerrar la ventana. El último
  bloque `M auto` del log contiene los ~3 s anteriores.

Enviar `build-win/kult-marcas.log` (y `kult.log` si existe).
