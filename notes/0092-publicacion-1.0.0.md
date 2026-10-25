# 0092 — T-092/T-096/T-135: release 1.0.0 publicada

Fecha: 2026-10-25. Estado: **cerrada**.

## Publicación

- Repositorio: <https://github.com/hunkstalker/kult-atari-st-pc-port>
- `main` y tag `v1.0.0` apuntan al commit
  `8eb079afb9b1da5ab2d873e8c83f613001e7d2ea`.
- Release pública: <https://github.com/hunkstalker/kult-atari-st-pc-port/releases/tag/v1.0.0>
- Run del tag: <https://github.com/hunkstalker/kult-atari-st-pc-port/actions/runs/40237580337>
  (`build and release`, conclusión `success`).
- Run de `main`: <https://github.com/hunkstalker/kult-atari-st-pc-port/actions/runs/40237575710>
  (`build and release`, conclusión `success`).

## Artefactos

Los tres artefactos se descargaron y su SHA-256 coincide con el `.sha256`
publicado:

| artefacto | bytes | SHA-256 |
|---|---:|---|
| `kult-1.0.0-win64.zip` | 1.619.821 | `685491849e7e8f8948561236d2f84f2c488bafae183ef34053a809452965b77d` |
| `kult-1.0.0-linux-x86_64.tar.gz` | 1.734.090 | `a2a78c00bbb6e9ad96811359553e39c6d85e4016810eace0867d850346aa7c68` |
| `kult-1.0.0-x86_64.AppImage` | 2.542.072 | `9442791ca91fe4f502b7fe6e738a9f57d161dff440ce6d0a9f062ba43ece79d5` |

El ZIP de Windows validado localmente en T-094 era
`5ed0e019ee90588b08030e5fce2b5cbfd82b1edce98daf0e079c1fb05a77965a`.
El ZIP de CI no es byte-idéntico (metadatos/timestamps); el checksum adjunto a
la release es la referencia.

## Cierre de tareas

- **T-092**: publicada la release, con `main`, tag, GitHub Release y tres
  paquetes.
- **T-096**: primera ejecución del CI correcta en `main` y en `v1.0.0`.
- **T-135**: CI generó TGZ Linux y AppImage; `make deps-linux` compila SDL3 con
  `SDL_X11_XSCRNSAVER=OFF` y `SDL_X11_XTEST=OFF`.
- **T-094**: se mantiene cerrada (validación visual del ZIP Windows local).

## Nota documental

El cuerpo de la GitHub Release se generó desde
`docs/RELEASE_NOTES_1.0.0.md` antes de este cierre y conserva el hash local de
T-094 (`5ed0e0…`). Los `.sha256` adjuntos son los correctos
(`685491…` para Windows). Si se edita el cuerpo de la release, actualizar esa
referencia.
