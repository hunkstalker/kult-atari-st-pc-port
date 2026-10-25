# AGENTS.md — relevo de la compilación PC

Documento de arranque para un agente/colaborador que retoma. Empieza por
**`RETOMAR.md`** (estado exacto y siguiente tarea); contexto maestro en
**`PROYECTO.md`**; tareas en **`TODO.md`**.

## Qué es este repo
Port nativo a **Windows/Linux** de **Kult** (Atari ST, con español y catalán), a
partir del **C verificado** de `kult-atari-st-c`. El juego no habla con el Atari:
llama a **19 servicios**; el port implementa esos servicios para PC (SDL3). Ver
`docs/PORTING.md`.

## Estado
El port es jugable de principio a fin en Windows y Linux. **M0–M7 funcionalmente cerradas**:
incluye ingesta de los discos originales, selector de idioma, packs español y
catalán, guardado/carga, audio, final del juego, paquete portable y licencias.
El catalán está validado por el usuario. **T-022 está cerrada (2026-10-25):**
`0x106f2` (lista C) está portado en el core con prueba diferencial (519 verdes)
y `l_0101a8` reconstruido en `boot/` como ruta latente verificada
(`KULT_SOFT_RESTART_AT`). T-102–T-135 añaden fullscreen nativo, F1/F2 (Estirar a pantalla), las
ranuras F5/F6/F7 (`slot-N.save`), el OSD con la fuente original del juego y la
terminología catalana T-119/T-132 (`GOT`, `XARXA TUNER`, `DAGA`, `FESOL`,
`COMERCIANT`, `Ç`) y el diálogo deformado en clave fonética catalana; el ZIP
final está regenerado en T-135 con Linux. La release **1.0.0** está publicada
como `v1.0.0` (commit `8eb079a`) con ZIP Windows, TGZ Linux y AppImage; el CI de
`.github/workflows/release.yml` terminó en `success` (T-092/T-096).
Las incidencias de texto T-097 (cajas de objeto) y T-098 (La Horca) están
corregidas y **validadas por el usuario (2026-10-25)**. Los tests de ingesta y
packs (T-095) están integrados en `make test`. Los diagnósticos opcionales están
separados en `platform/diag.c` (T-100; guía `docs/DIAGNOSTICO.md`). T-101
corrigió «DIE» (objeto) a **DADO**/**DAU** en los packs y dejó regresión de
textos. T-106 usa la fuente original del juego para el OSD, T-107/T-108 dejan
las ranuras en `slot-N.save` sin migración y T-109/T-110 añaden Estirar a
pantalla. T-073 es opcional. Sin bloqueantes de código. Siguiente hito: A1 (investigación Amiga). Detalle:
`notes/0053`–`notes/0092`. Ver `notes/0011`–`notes/0014`, `notes/0041`,
`notes/0042`, `notes/0049`–`notes/0092`.

## T-073c — reglas duras (pantalla C del Maestro)
- **Input (modelo original)**: **izquierdo / teclado** = acción normal, **sin**
  skip; **derecho** = acción + skip (A/B/C y puertas). El IKBD se interroga con
  `0x0D` y responde con **eventos desde la última interrogación**: una pulsación
  mantenida produce **un solo clic** (no repetir el nivel). La HAL entrega el
  flanco de bajada en el bit0 del IKBD y **pisa `$1a570`** al salir de
  `wait_frame` con el flanco del derecho. `lib/kult-core/src/input.c` no se toca. No usar
  modelos de botones de otros repositorios. T-084: `notes/0032`.
- **Presentación**: `platform_tick` presenta en **cada VBL** (arranque y bucle);
  el frame hook ya no presenta. Así se ven las animaciones que corren en
  `wait_vbl` dentro de un comando (puertas, wipes).
- **Cursor**: la HAL emula `Ikbdws` `0x09` (el juego fija el ratón absoluto a
  **304×184** para el sprite 16×16). No recortar a ojo a 319×199: `lib/kult-core/src/` no se
  toca; ver `notes/0021-cursor-e-ikbd.md`. En SDL se oculta el cursor del
  sistema (`SDL_HideCursor`) para que solo se vea el del juego; ver
  `notes/0027-cursor-sistema-sdl.md`.
- **Reinicio tras morir**: el original salta de `reset_clear` (`0x1682a`) a
  `0x10158`; el port lo emula en `boot/` con `longjmp` desde el hook de salida de
  `reset_clear` y `boot_enter_game()`. No es `0x106f2`; ver
  `notes/0022-reinicio-tras-muerte.md`.
- **Ritmo de muerte**: la sangre se cobra por línea en `reveal_line`
  (`13d0e`, fórmula medida 1063 + 198 × pasos) y el viaje espacial en
  `icon_scale_loop` (`1834a`, 22.790/pasada); `181f0` no se cobra al entrar.
  No se ajusta a mano. Ver `notes/0023`.
- **Estrella final**: la espera tras la animación es la lectura de `FOND.PI1`
  (`0x13568`, 0x7d22 B). La HAL modela el disquete ST (~2 s) en
  `platform_disk_delay`; se aplica a cada Fread, así que da ritmo a salas,
  reinicio y estrella. **La carga inicial no espera** (`platform_disk_timing(0)`
  desde PRES/DRAP hasta después de ECHE), para no perder el inicio de la canción
  ni esperar ~20 s tras el idioma. Ver `notes/0025` y `notes/0026`.
- **Retornos dobles**: `elem_command` (`0x128a4`) puede hacer
  `addq.l #4,a7; rts` (aborta a su llamante). El port lo modela con
  `elem_command_ex`/`elem_command_wrap_ex` y los 12 llamantes; no parchear solo
  el reveal. Validado visualmente por el usuario. Ver
  `notes/0028-retornos-dobles-128a4.md` (T-081, resuelto).
- **Modelo A/B/C**: A = 3 retratos + descripciones (pixel-reveal, quietos);
  B = 4 retratos + descripciones (lift-reveal, quietos, 4 bancos
  `15ac8`/`0x36`); C = 1 retrato del Maestro + lift + **diálogo** (único
  diálogo y único retrato que se mueve), después de B.
- C está en los logs: **1219–1249 / 1513–1543**; ~8 frames/16 blits a 1–3
  VBL/frame; **sin** `15ac8`/`15da6`.
- **No** usar `KULT_FRAME_GATE` ni `15da6` como fix de C sin evidencia.
- Medir antes de tocar; una hipótesis, un cambio; si no mejora, revertir.
- Handoff: `notes/0017-retomar-maestro.md`; plan detallado:
  `notes/0018-plan-c.md`. Input/presentación: `notes/0020-input-hal-y-presentacion.md`.
  Prueba de guardia: `test_click_skip` en `make test-port`.

## Reglas de trabajo
1. **Reproducir/medir antes de tocar.** Una hipótesis, un cambio.
2. **El core en `lib/kult-core/` no cambia de lógica.** El delta del port está en
   `docs/CORE_DELTA.md`; lo específico de máquina va en
   `platform/`; el arranque/tick en `boot/`; el driver en `app/`. El **timing**
   se anota con `cpu_delay`/`cpu_delay_cycles` (no-op en diferencial), como en
   `anim.c`, `reveal.c` y `loop.c`; cualquier otro cambio en `lib/kult-core/src/` requiere
   permiso explícito.
3. **Los 519 tests diferenciales del core deben seguir verdes.** Se ejecutan
   con `make test-core`; son la red de seguridad de que no cambiamos la lógica.
4. **Toda corrección/feature deja prueba** (diferencial si aplica; funcional si es
   del backend).
5. **No tocar** los repos de referencia (solo lectura).
6. **Documentar lo que se toca**: `TODO.md`, `RETOMAR.md`, `notes/`.
7. Todo en **español**.

## Orden de lectura
`RETOMAR.md` → `PROYECTO.md` → `TODO.md` → `docs/INDICE.md` → el doc del área.

## Estructura
```
lib/kult-core/      core C importado de kult-atari-st-c (subtree)
                    src/, include/, symbols/, test/ y tools/
boot/       _start (0x10000), tick 0x160c2      [nuevo]
platform/   HAL PC (os_*/hw_*/psg_*/sched_*) + render + audio;
            diag.c para diagnóstico opcional, cpuwork.c solo timing  [nuevo]
app/        main + driver + presentación         [nuevo]
assets/     icono (kult_icon.ico)                [nuevo]
docs/  adr/  notes/
```

## Decisiones cerradas
SDL3; `SDL_Renderer` base y `SDL_GPU` para CRT; audio en hito propio; menú de
idioma fiel; `boot/` fuera de `lib/kult-core/src/`; headless primero. Ver `docs/adr/`.
