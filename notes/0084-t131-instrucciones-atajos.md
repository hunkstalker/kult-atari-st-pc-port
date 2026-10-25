# 0084 — T-131: instrucciones de atajos y F12 en release

Fecha: 2026-10-25. Estado: **cerrada**. Siguiente: T-094 sobre el ZIP final.

## Archivo

`TECLAS_FN.txt` se renombra a:

`Instrucciones - Atajos de teclado.txt`

Solo documenta teclas útiles:

- F1 pantalla completa/ventana.
- F2 escala.
- F5 ranura en dos pasos.
- F6 guardar y F7 cargar.
- F9/F10/F11 volumen.
- ESC cerrar.

No incluye F3, F4, F8 ni F12.

## F12

F12 queda inerte en la release normal: solo actúa si `KULT_MARKS` está activo.
Sin esa variable no escribe en `kult.log` ni genera traza. En la build de
diagnóstico sigue funcionando.

## Empaquetado

- `CMakeLists.txt` instala el nuevo nombre con espacios.
- El paquete mantiene 13 entradas; cambia el nombre del TXT.
- `LEEME.txt` y la documentación apuntan al nuevo archivo.

## ZIP

`build-win/kult-1.0.0-win64.zip`
SHA256 `d45392a3eeffcb69ddf3a9d1b273e4ada1ded925c9359946e8686d9e66651ae4`
(13 entradas).

## Verificación

`make test` y `make package-win` OK. `unzip -l` confirma el nombre nuevo.

## Pendiente

T-094 sobre el ZIP final.
