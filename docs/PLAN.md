# PLAN — fases y objetivos

Plan de la compilación por hitos. Estado vivo en `PROYECTO.md`/`TODO.md`.
Cada fase tiene objetivo y **criterio de aceptación**.

## Principios
- **No se reescribe el core verificado**: se reutiliza (415 funciones, 499 tests).
- **El core no conoce SDL ni hardware**: los 19 servicios se inyectan.
- **Nada a medias**: una fase se cierra con su criterio cumplido y pruebas.
- **Todo derivado del juego, gitignored**: el usuario aporta su copia.

## M0 — Repositorio y base
**Objetivo:** repo con documentación e importación del core.
**Aceptación:** `src/include/test/tools` importados; `make test` **499 verde** en
este repo; `work/disk_a/KULT.PRG` disponible para el oráculo.

## M1 — HAL mínima (headless)
**Objetivo:** ejecutar el arranque y el bucle **sin ventana**, volcando el
framebuffer a fichero; validar ficheros, entrada simulada y tick.
**Contenido:** `platform/` (19 servicios), `app/main` (init → `_start` →
`game_loop`), `os_super` no-op, audio stub.
**Aceptación:** el juego avanza N frames solo (o con entrada inyectada) y el
framebuffer resultante es coherente; sin cuelgues de esperas.

## M2 — Arranque y tick
**Objetivo:** disponer en C de las piezas del original que faltan.
**Contenido:** `boot/_start.c` (0x10000, con menú de idioma), `boot/vbl.c`
(0x160c2), `0x135ba`/`0x106f2`; FDC omitido.
**Aceptación:** el tick avanza `A_DAT_1ACD4`; las esperas retornan; el arranque
carga pantallas/recursos y llega al bucle.

## M3 — Render y entrada reales
**Objetivo:** **primera ventana jugable**.
**Contenido:** ventana SDL3 + `SDL_Renderer`; planar→RGBA con la paleta de 16
words; escalado entero; ratón/teclado → paquete IKBD; simulación a ritmo fijo +
presentación al refresco.
**Aceptación:** se ve y se controla el juego; el cursor responde; sin artefactos.

## M4 — Audio
**Objetivo:** sonido y música.
**Contenido:** modelo del **MFP Timer A** (dispara las ISR C ya existentes),
emulación **PSG/YM2149**, salida **SDL audio**.
**Aceptación:** suena la intro/música y los efectos, sin petardeo.
**Estado:** ✅ hecho (T-040/041/042; ver `notes/0008-m4-audio.md`). El reproductor
de música `0x3a872` se reconstruyó en `boot/`; la verificación objetiva compara su
ledger PSG con el oráculo y mide RMS/energía. La escucha final la valida el usuario.

## M6 — Pulido y validación
**Objetivo:** dejarlo jugable de principio a fin.
**Contenido:** menú de idioma (fiel o nativo), guardado, ajustes; validación.
**Aceptación:** partida completa jugable; lista de limitaciones documentada.
**Estado:** ✅ **M6 cerrada**. **T-060 ✅** (menú de idioma **fiel**, ADR 0004,
validado en T-020). **T-061 ✅ validada en Windows (2026-10-23)**: checklist
completo (checklist en `docs/VERIFICACION.md`, limitaciones en
`docs/LIMITACIONES.md`). Criterio: mismos caminos/comportamiento ASM↔C a nivel
de experiencia del jugador; no se exige timing ciclo-exacto. **La ejecución la
hizo el usuario** (jugar y escuchar con su copia de datos).
Incluye **morir → reiniciar** (`reset_clear` → `bra 0x10158`, emulado en `boot/`;
**no** depende de `0x106f2`). **Colores** del juego
corregidos en la HAL (`OS_SCREEN_BASE=0x98000`) y **primera pantalla = título
visible** (se aplica la paleta del PI1 antes de la espera; `notes/0014`).
**T-073 🧩 feature opcional**: calibración de ritmo real: **T-073a ✅** (`cpu_delay` real), **T-073b ✅**
(coste medido por función con `-finstrument-functions` + tabla del oráculo) y
**T-073c ✅ validado** (el `dbf` inline de `0x16dec` ya se cobra con
`cpu_delay_cycles`; validación del usuario y commit `3235aa6`; ver `notes/0019`). Queda calibración fina por escenas/casos no medidos (ISR/VBL,
contención de bus). Ver `notes/0015`. **T-088 ✅ validado en Windows**: crash de
la ficha al atacar a un personaje corregido en `object_info_card`; replay de
entrada en `notes/0038`. **T-089 ✅ validado en Windows**: el final se queda en
la animación de estrellas (`attract`/`0x18c80`), sin volver al guion ni dejar
escena/paleta corruptas; `notes/0040`. **T-090 ✅ cerrado**: SOLAR EYES (`palette_cycle`, `0x16310`) medido y
corregido con `cpu_delay(0x10000u)` en las dos fases. El hueco de audio de
Hatari queda documentado como artefacto de timing del original, fuera de
alcance. Detalle en `notes/0041` y `trazas-t090-20261023/`; la calibración
opcional es **T-073 (feature opcional)**.

## M7 — Empaquetado/release
**Objetivo:** distribuir sin assets (el usuario aporta su copia); **build Windows**
para que el usuario valide.
**Contenido:** portabilidad Windows (`platform/time.c`, `platform/fs.c`); **CMake**
+ `cmake/mingw-w64.cmake`; **SDL3 3.4.10** para MinGW (`third_party/`, gitignored);
**icono** (`assets/`); ZIP Windows + TGZ Linux; **ingesta** (`kult.data`) — esta
última va de la mano de N3.
**Aceptación:** doble clic + copia del usuario → funciona.
**Nota:** el **primer build** (validación) empaqueta con carpeta `data/` interina
(sin ingesta); el empaquetado completo (TGZ/CPack/ingesta) es M7-c.
**Estado:** **build hecho** (T-070: CMake nativo/cross MinGW + SDL3 3.4.10 +
icono + log/diagnóstico `kult.log`) y **empaquetado interino hecho** (T-071: ZIP
Windows con `kult.exe`+`kult-headless.exe`+`SDL3.dll`+`data/`+`LEEME.txt`+
`DIAGNOSTICO.txt`; TGZ Linux). El `.exe` quedó validado en el checklist de **T-061** (cerrada el 2026-10-23).
**Siguiente: T-072 — ingesta `.STX`** (el usuario solo aporta imágenes de
disquete). Detalle:
`notes/0012`, `notes/0013`.

## A1 — Investigación Amiga (prerrequisito **DURO** de N3)
**Objetivo:** entender el binario Amiga para **diseñar el modelo de datos de N3 a
dos versiones**, sin rehacer nada luego.
**Contenido:** `T-A00` censo Amiga↔ST por rutina; `T-A01` decomp ASM Amiga (spec
byte a byte). **No toca el motor.**
**Aceptación:** mapa de *lógica compartida vs específica* + diferencias de
hardware (Paula/audio, paleta/planos) y de assets.

## N3 — Motor nativo y assets nativos (hito futuro: ADR + rama)
**Objetivo:** runtime **100 % independiente** del binario original (ni `KULT.PRG`
ni blobs originales); motor propio + **assets nativos**.
**Contenido:** **ADR** que lo fija; formato de assets + cargador; conversores por
subsistema. Formatos: **PNG** (gráficos), **WAV/OGG** (audio), **UTF-8** (textos),
**formato tracker propio** (música) y **binario propio documentado** (datos de
sala/guion/tablas; **no** habrá mods). La **ingesta** pasa a ser un **conversor de
un solo uso** (tu copia → assets). Modelo preparado para **ST/Amiga**.
**Aceptación:** validación funcional por subsistema; sin dependencia del original.

## A2 — Implementación Amiga (después de N3)
**Objetivo:** ofrecer la experiencia **Amiga**.
**Contenido:** `T-A02` core Amiga (lógica compartida + capa Amiga) sobre el motor
de N3; `T-A03` arte/audio/textos Amiga.
**Aceptación:** modo Amiga verificado contra el binario Amiga.

## M5 — Filtros CRT (nota: extra opcional, no prioritario)
**Nota:** fuera del camino crítico y **al final**; independiente de A. Es una
**mejora de presentación**, no afecta a la HAL ni a la lógica. Se conserva el id
**M5/T-050**.
**Objetivo:** escalado con shaders.
**Contenido:** `SDL_GPU` + shaders (CRT/scanlines/…); `SDL_Renderer` se mantiene
como base (ADR 0002).
**Aceptación:** filtros conmutables; perfil sin filtro intacto.
