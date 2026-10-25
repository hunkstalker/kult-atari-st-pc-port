# 0082 — T-129: F5 en dos pasos

Fecha: 2026-10-25. Estado: **cerrada**. T-130 añade TECLAS_FN.txt.
Siguiente: T-094 sobre el ZIP final.

## Comportamiento

- Primer F5: muestra `RANURA N` con la ranura actual **sin cambiarla**.
- Segundo F5 mientras ese aviso sigue visible: rota a la siguiente ranura,
  actualiza `kult.ini` y reinicia el aviso.
- Si el aviso ya caducó o se sustituyó por otro OSD, el siguiente F5 vuelve a
  mostrar la ranura actual sin rotar.

## Implementación

- `platform/slots.c`: nueva `platform_slots_f5()`.
- `platform/platform.h`: declaración.
- `platform/video_sdl.c`: F5 llama a `platform_slots_f5()`.
- `platform_slots_select_next()` se mantiene como rotación inmediata interna.

## Verificación

`make test`: 519 core + 36 funciones / 261 `CHECK` HAL + diag + 5 herramientas.
Se añaden comprobaciones de primer F5, segundo F5, aviso caducado y OSD ajeno.

`make port-sdl`, `make cmake-native`, `make cmake-win` y `make package-win` OK.

## Formato

`assets/language.data` no cambia: 78.511 B,
SHA256 `38ff7f705bb51a06abf02b0e9fcd4cc7b8cebf6409933bcc611a12a4ed342f1f`.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `2a127b579caad642bd9a393e991d475008bbe1a2ca2c914393c5a19fecf633c9`
(12 entradas, `saves/` vacía).

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
