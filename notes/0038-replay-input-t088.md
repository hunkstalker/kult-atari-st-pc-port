# 0038 — T-088: replay de entrada para reproducir el crash

> **RESUELTO Y VALIDADO EN WINDOWS.** T-088 queda cerrado: el crash al atacar a
> un personaje (`object_info_card`, `0x16dec`) se reprodujo con replay, se
> localizó la causa y se corrigió. Validación final del usuario en Windows y
> replay nativo sin SIGSEGV. Detalle del cierre: §11. La captura final de T-087
> está en **`notes/0037`** §8.

## 1. Qué problema resuelve

El juego entrega un objeto inicial distinto en cada partida, así que un replay
basado en reejecutar el juego desde el principio no es determinista. Tampoco
`Kult.exe` normal graba inputs.

La solución es grabar **las respuestas que ve el juego en cada sondeo de
entrada**: clics, posición del cursor y teclas consumidas por `Crawio`/`Cnecin`.
El replay reproduce esos valores en el mismo orden de sondeos, sin depender del
tiempo real ni de SDL.

## 2. Formato

Fichero binario `kult-replay.bin`:

```text
cabecera: "KULTREP1" + version 1 + flags
evento (16 B):
  type  1 = IKBD (boton, skip, cursor x/y)
        2 = Crawio (tecla o 0)
        3 = Cnecin (tecla)
  button, skip, reserved
  x, y            (uint16)
  key             (uint32)
  frame           (uint32, solo diagnóstico)
```

Cada evento se escribe y se hace `fflush` inmediatamente, para conservar el
final del replay aunque el proceso muera con el crash.

## 3. Uso

Grabar:

```text
assets/kult-replay-record.bat
  -> KULT_REPLAY_REC=%~dp0kult-replay.bin
  -> ejecuta kult.exe
```

Reproducir:

```text
assets/kult-replay-play.bat
  -> KULT_REPLAY_PLAY=%~dp0kult-replay.bin
  -> ejecuta kult.exe
```

También se puede usar directamente:

```bat
set KULT_REPLAY_REC=%~dp0kult-replay.bin
kult.exe

set KULT_REPLAY_PLAY=%~dp0kult-replay.bin
kult.exe
```

Para un replay **reutilizable** (cualquier save, no solo T-088/endgame), usar
`assets/kult-replay-generic.bat` (copia en el paquete):

```bat
kult-replay-generic.bat record <carpeta-save> [replay.bin]
kult-replay-generic.bat play   <carpeta-save> [replay.bin]
```

Copia `SCENA.BIN`/`CLEAR.BIN` de `<carpeta-save>` a `data\` si existen, define
`KULT_REPLAY_REC`/`KULT_REPLAY_PLAY` y lanza `kult.exe`. Es la vía recomendada
para capturar incidencias nuevas (p. ej. T-090).

Durante el replay se ignoran los eventos reales de ratón/teclado de SDL. El
cursor se restaura desde el campo `x/y` de cada paquete IKBD.

## 4. Save de la partida

El save de sala es:

```text
data/SCENA.BIN   74.524 B = 0x291c + 2×0x7d00
```

`CLEAR.BIN` no sirve como save porque el arranque lo reescribe. La copia de
seguridad usada para T-088 está en:

```text
build-win/kult-0.1.0-win64/save-t088/SCENA.BIN
build-win/kult-0.1.0-win64/save-t088/CLEAR.BIN
build-win/kult-0.1.0-win64/save-t088/SHA256SUMS
```

## 5. Prueba

`make test-port` incluye `test_replay_input`:

1. graba un IKBD con cursor (12,34) y botón izquierdo;
2. graba `Crawio='A'` y `Cnecin='B'`;
3. reproduce el fichero y comprueba botón, cursor y ambas teclas.

## 6. Flujo seguido

1. El usuario cargó `SCENA.BIN`, ejecutó `kult-replay-record.bat` y reprodujo
   el crash atacando al personaje.
2. Se analizó `kult-replay.bin`/`kult.log` y, con el replay, se comparó el C
   contra el ASM (`0x16dec`, `0x1285a`, `0x18e12`, `0x12950`, `0x129c8`,
   `0x129d6`, `0x120dc`, `0x124da`, `0x12b04`).
3. Se corrigió la causa raíz, se añadió regresión y se actualizaron
   `TODO.md`/`RETOMAR.md`.

## 7. Ejecutables del build de replay

```text
kult.exe               339b6c54e3952eecbf016025cfbbd9c376d04ba9e743e08c4be86a976f2c5a9c
kult-headless.exe      e6e6a53d6b57af6f27a7a88156736b2964a72c4a3d2a3f88d80f6fdd1492f228
kult-diag.exe          68da39721ec5bca01293997a80fc1e485c7cd171330f2525fcc12f738e9e66fd
kult-diag-headless.exe b5a407c6507e707d751b2ad14fba2883458293df554492928a92aaedc38392cf
ZIP                    0e40972eb35102a4e927710c201953b75ab6c4d92d1fc8461a6362025d53b47c
```

Los bats `kult-replay-record.bat` y `kult-replay-play.bat` restauran
`save-t088/SCENA.BIN` en `data/SCENA.BIN` antes de arrancar, para que la
grabación y la reproducción usen el mismo estado. Estos ejecutables son los del
build que grabó el replay; el build final con el fix está en §10.

## 8. Run grabada y reproducción (2026-10-22 00:40 local)

El usuario grabó el crash. El replay funciona: reproducido en Linux nativo con
`build/kult` termina en **SIGSEGV** (rc=139), igual que desaparece la ventana en
Windows.

Evidencia gdb nativa:

```text
#0 wr16
#1 elem_bitmap_build (src/element.c:186)
#2 elem_list_build (src/element.c:219)
#3 object_info_card (src/loop.c:494)
#4 byte_handler (b=24, script=131625)
```

En el momento del fallo:

```text
stream = 0x281e7, idx de patron = 9
tabla 0x1a252, entrada 9 (0x1a2ac): width=1, height=0
scratch 0x1a2ad = 210  (object_info_card lo pisa con el indice de lista)
resultado: inner = width-3 = 0xfffe, row = height-3 = 0xc9
dest llega a 0x101fd0 (> 1 MiB) y el host hace SIGSEGV
```

Artefactos preservados en:

```text
build-win/kult-0.1.0-win64/trazas-t088-crash-20261021-224446/
  kult-replay.bin
  kult-record.log
  SCENA.BIN
  replay-native.log
  README.txt
  SHA256SUMS
```

`kult-replay.bin` SHA256:
`d9b86974c4032494cb1e03a652bb97e6eaac8df71174ec733722d003fcca622d`.

## 9. Causa raíz y fix

La causa no es el patrón inválido en sí: el port no avanzaba `p` al final del
bloque de texto de 3 bytes. En ASM, `0x129d6` termina con `addq.l #$3,a5`; en C
`text_block_build()` recorría el bloque pero `object_info_card` seguía apuntando
a su primer byte. El bucle exterior releía `68 d2 00` como entradas de ficha,
`0xd2` se convertía en índice de lista 210 y acababa en el patrón inválido 9.

Fix en `src/loop.c`:

```c
if (img[p] == 0xff) {
    p = (p + 1) & 0xffffffffu;
    text_block_build(img, p, (p + 3) & 0xffffffffu);
    p = (p + 3) & 0xffffffffu;   /* 0x129d6 deja A5 al final del bloque */
}
```

Es el comportamiento exacto del ASM; no se limita `elem_bitmap_build` ni se
condiciona al personaje/save. Se añadió `test_fun_16dec_text_block` en
`test/test_loop.py`.

Validación: `make test` **517**, `make test-port` OK. El replay nativo ya no
hace SIGSEGV: consume todo `kult-replay.bin`, escribe `replay: fin del fichero`
y sigue esperando entrada. Log de la reproducción corregida:
`trazas-t088-crash-20261021-224446/replay-fixed.log`.

## 10. Build final del fix

```text
kult.exe               154122ef03b7de69d52e0ac3df95f4138133e68dc5c1b8310d5259ebd58e49d2
kult-headless.exe      70c603bf06488a1f3c077d2f8733b511ea7925c036e70bde6a9d6678894d5696
kult-diag.exe          da7afa3b811d551a462afc6c9605f1d1e0be33ee5bc190cdacdc17b6b1857dd5
kult-diag-headless.exe d889843442ab1a0d72025bf038b1bfa12bcccaef41ccfe8a3fe8b01883f10a4f
ZIP                    76c5ee01d59e7c15cf3f25d5d186d80a08bf5f97a69503ca9d6e58558a8f337f
```


## 11. Validación en Windows y cierre

El usuario confirmó en Windows que el error está resuelto: al atacar al
personaje ya no desaparece la ventana y la partida continúa. La validación se
hizo con el replay preservado (`kult-replay-play.bat`), el mismo que reproducía
el SIGSEGV antes del fix.

Con esto T-088 queda cerrado:

- causa raíz: falta de avance de `p` tras `text_block_build` (§9);
- fix: `p += 3` en `object_info_card`;
- pruebas: `test_fun_16dec_text_block`, `make test` **517**, `test-port` OK;
- replay nativo: sin SIGSEGV, consume el fichero completo;
- validación visual: **OK en Windows** por el usuario.
