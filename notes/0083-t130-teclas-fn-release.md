# 0083 — T-130: TECLAS_FN.txt en la release

Fecha: 2026-10-25. Estado: **cerrada**. T-131 lo renombra a
`Instrucciones - Atajos de teclado.txt`. Siguiente: T-094 sobre el ZIP final.

## Cambio

Se añade `TECLAS_FN.txt` junto a `kult.exe` en el paquete de release y en
cualquier artefacto que use el `install()` de CMake (incluido el futuro CI).

Contenido:

- F1 fullscreen/ventana.
- F2 ciclo de escala.
- F5 dos pasos: mostrar ranura y rotar al repetir.
- F6 guardar; F7 cargar.
- F9/F10/F11 volumen, silencio y volumen.
- F12 diagnóstico opcional.
- F3/F4/F8 sin función; ESC cierra.

`LEEME.txt` enlaza a `TECLAS_FN.txt`.

## Empaquetado

- `CMakeLists.txt`: `install(FILES ... assets/TECLAS_FN.txt ...)`.
- ZIP de Windows: **13 entradas**, con `TECLAS_FN.txt` en la raíz.
- El TGZ de Linux y el futuro CI heredan el mismo `install()`.

## Formato

`assets/language.data` no cambia: 78.511 B,
SHA256 `38ff7f705bb51a06abf02b0e9fcd4cc7b8cebf6409933bcc611a12a4ed342f1f`.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `2316f64a81192a83184c571560188010710a4aa0509153763e68afb495b6bf5a`
(13 entradas, `saves/` vacía).

## Verificación

`make test` y `make package-win` OK. `unzip -l` confirma
`kult-1.0.0-win64/TECLAS_FN.txt`.

## Pendiente

T-094: validación visual de ES/CA, textos, tamaños de caja y menús en Windows.
