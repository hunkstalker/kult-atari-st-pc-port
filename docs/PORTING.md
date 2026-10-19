# PORTING — la frontera con la máquina

El juego (en C) **no habla con el Atari**: llama a **19 servicios**. Sustituir
esas respuestas (las del **TOS/XBIOS/BIOS** y las del **hardware**) por la de un
backend de PC es todo el porte. El core **no se toca**.

## Los 19 servicios y su equivalente PC

| servicio | para qué lo usa el juego | equivalente PC |
|---|---|---|
| `os_fopen` `os_fclose` `os_fcreate` `os_fread` `os_fwrite` | ficheros (recursos, `SCENA.BIN`, `CLEAR.BIN`) | sistema de ficheros real; mapear `A:…` al directorio de datos |
| `os_fsfirst` `os_fsetdta` | búsqueda de ficheros (listas de recursos) | listado de directorio |
| `os_crawio` `os_cnecin` | teclado (esperas, prompts) | entrada SDL |
| `os_setscreen` | base de pantalla (doble buffer) | estado del renderizador |
| `os_setpalette` `os_setcolor` | paleta (16 words) | tabla de paleta del renderizador |
| `os_super` | modo supervisor (privilegios) | no-op en PC |
| `hw_write8` `hw_sub8` `hw_bclr8` | registros del **MFP** (Timer A, interrupciones) | modelo del MFP (ver Audio) |
| `psg_port_read` `psg_port_write` | chip de sonido **PSG/YM2149** | emulación del chip + audio |
| `sched_poll8` | esperas (VBL / paquete IKBD) | reloj + planificador del tick |

> Nota: el **ratón** no es un servicio; entra por el **paquete IKBD**
> (`A_IKBD_PACKET`, 0x1a44f), cuyos bytes 1–4 **son** `A_CURSOR_X` (0x1a450) y
> `A_CURSOR_Y` (0x1a452). La HAL escribe ahí la posición del ratón de PC.

## Piezas del original que faltan en C (se reconstruyen)

| pieza | qué es | nota |
|---|---|---|
| `_start` (0x10000) | arranque + **menú de idioma** D/E/F | único nombre sin definición C |
| `0x160c2` | tick de VBL: `addq.b #1,A_DAT_1ACD4` + trabajo por frame | sin él las esperas cuelgan |
| `0x135ba` | line-A + XBIOS (Setpalette/paleta) | pequeña |
| `0x106f2` | reinicio | camino de reinicio |
| `0x19316` | comprobación **FDC** | **se omite** en PC |

## Vídeo / render
- Formato del framebuffer: **planar** 4 planos, 320×200, **160 B/fila**,
  32000 B; dirección de píxel `((x&0xfff0)>>1) + y*0xa0`.
- Paleta: **16 words** en `A_PALETTE_PTR` (formato ST `0x0RGB`, 3 bits/canal).
- El renderizador: framebuffer visible (A/B según Setscreen) → RGB → escalado →
  presentación SDL3. Base `SDL_Renderer`; CRT con `SDL_GPU` (ADR 0002).

## Audio
- El juego **programa el MFP Timer A** (`hw_write8` a `$fffa13/07/19/1f`) y
  escribe el **PSG** (`psg_port_write`).
- La HAL debe: **emular el Timer A** para disparar las ISR ya reconstruidas en C
  (música: vector `$134` y traps `#3/#4`; PCM: ISR del Timer A) y **emular el
  YM2149** para producir sonido; salida por **SDL audio**.
- La **lógica** musical (notas, tracker, PCM) ya está en C; falta el **chip** y
  el **temporizador**.
- **Implementado (M4)**: `platform/mfp.c` (Timer A + despacho del vector `$134`),
  `platform/psg.c` (+ledger), `platform/audio.c` (YM2149), `platform/audio_sdl.c`
  (SDL3) y `boot/music_player.c` (reproductor `0x3a872`/envolvente `0x3a936`).

## Cursor y ritmo (fluidez)
- El cursor **se repinta una vez por tick**; la posición viene del paquete IKBD.
- Para fluidez: **desacoplar** el repintado del tick y presentarlo al refresco
  (o como overlay), manteniendo la lógica a su cadencia. Ver `docs/PLAN.md` M3.

## Lo que NO hace falta
- **Musashi** / el binario del Atari: son el **oráculo de test** del core, no
  forman parte del ejecutable.
- La **comprobación FDC**.
