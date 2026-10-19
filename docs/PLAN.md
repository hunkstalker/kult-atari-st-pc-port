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
**Estado:** en curso. **T-060 ✅** (menú de idioma **fiel**, ADR 0004, validado en
T-020). **T-061 🚧**: checklist de validación en `docs/VERIFICACION.md` y
limitaciones en `docs/LIMITACIONES.md`; **la ejecución la hace el usuario** (jugar
y escuchar con su copia de datos).

## A — Workstream Amiga
**Objetivo:** poder ofrecer también la experiencia **Amiga**.
**Contenido:** censo por rutina Amiga↔ST → decomp Amiga (spec) → core Amiga
(lógica compartida + capa Amiga) → arte/audio/textos.
**Aceptación:** modo Amiga verificado contra el binario Amiga.

## M7 — Empaquetado/release
**Objetivo:** distribuir sin assets (el usuario aporta su copia).
**Contenido:** builds Windows/Linux; ingesta de datos; paquete.
**Aceptación:** doble clic + copia del usuario → funciona.

## M5 — Filtros CRT (nota: extra opcional, no prioritario)
**Nota:** fuera del camino crítico. La prioridad es **terminar el juego** (M6
validación → M7 empaquetado → Amiga); esto es una **mejora de presentación** que
se desarrollará **en algún momento**, tras validar. Se conserva el id **M5/T-050**
como referencia en el plan.
**Objetivo:** escalado con shaders.
**Contenido:** `SDL_GPU` + shaders (CRT/scanlines/…); `SDL_Renderer` se mantiene
como base (ADR 0002).
**Aceptación:** filtros conmutables; perfil sin filtro intacto.
