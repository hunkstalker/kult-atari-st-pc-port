# AGENTS.md — relevo de la compilación PC

Documento de arranque para un agente/colaborador que retoma. Empieza por
**`RETOMAR.md`** (estado exacto y siguiente tarea); contexto maestro en
**`PROYECTO.md`**; tareas en **`TODO.md`**.

## Qué es este repo
Port nativo a **Windows/Linux** de **Kult** (Atari ST + Amiga, todos los
idiomas), a partir del **C verificado** de `/app/kult-atari-st-c`. El juego no
habla con el Atari: llama a **19 servicios**; el port implementa esos servicios
para PC (SDL3). Ver `docs/PORTING.md`.

## Estado
**M0–M4 hechas**, **M6 en curso** (T-060 ✅; T-061 = validación del usuario) y
**M7 build + empaquetado interino hechos** (T-070/T-071: CMake Linux/Windows,
`kult.exe` + ZIP sin assets, `kult.log` de diagnóstico). El juego arranca con el
**menú de idioma fiel**, se ve y suena (ventana SDL3 + audio); **colores del juego
corregidos** (Physbase del HAL `0x98000`) y **primera pantalla = título
visible** (paleta del PI1 antes de la espera). **T-077/T-079 ✅** (sangre,
espacio y espera de la estrella), **T-083 ✅** (objetos en serpientes) y
**T-084 ✅** (input mantenido), **T-085 ✅** (retrato/intro) y **T-086 ✅**
(selección de acción en el anillo), validados por el usuario. Pendiente:
**T-061** (validación en Windows) → **T-072** (ingesta `.STX`) → **M7-c** →
**A1/N3/A2** (Amiga + motor/assets nativos) → **M5** (opcional). Sin
bloqueantes. Ver `notes/0011`–`notes/0014`; T-083 en `notes/0031`, T-084 en
`notes/0032`, T-085 en `notes/0033` y T-086 en `notes/0034`.

## T-073c — reglas duras (pantalla C del Maestro)
- **Input (modelo original)**: **izquierdo / teclado** = acción normal, **sin**
  skip; **derecho** = acción + skip (A/B/C y puertas). El IKBD se interroga con
  `0x0D` y responde con **eventos desde la última interrogación**: una pulsación
  mantenida produce **un solo clic** (no repetir el nivel). La HAL entrega el
  flanco de bajada en el bit0 del IKBD y **pisa `$1a570`** al salir de
  `wait_frame` con el flanco del derecho. `src/input.c` no se toca. No usar
  modelos de botones de otros repositorios. T-084: `notes/0032`.
- **Presentación**: `platform_tick` presenta en **cada VBL** (arranque y bucle);
  el frame hook ya no presenta. Así se ven las animaciones que corren en
  `wait_vbl` dentro de un comando (puertas, wipes).
- **Cursor**: la HAL emula `Ikbdws` `0x09` (el juego fija el ratón absoluto a
  **304×184** para el sprite 16×16). No recortar a ojo a 319×199: `src/` no se
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
2. **`src/` (el juego) no cambia de lógica.** Lo específico de máquina va en
   `platform/`; el arranque/tick en `boot/`; el driver en `app/`. El **timing**
   se anota con `cpu_delay`/`cpu_delay_cycles` (no-op en diferencial), como en
   `anim.c`, `reveal.c` y `loop.c`; cualquier otro cambio en `src/` requiere
   permiso explícito.
3. **Los 499 tests diferenciales del core deben seguir verdes.** Son la red de
   seguridad de que no cambiamos la lógica.
4. **Toda corrección/feature deja prueba** (diferencial si aplica; funcional si es
   del backend).
5. **No tocar** los repos de referencia (solo lectura).
6. **Documentar lo que se toca**: `TODO.md`, `RETOMAR.md`, `notes/`.
7. Todo en **español**.

## Orden de lectura
`RETOMAR.md` → `PROYECTO.md` → `TODO.md` → `docs/INDICE.md` → el doc del área.

## Estructura
```
src/        juego en C (intacto, importado de kult-atari-st-c)
include/    addrs.h / kult_api.h / …   (intacto)
test/       499 diferenciales          (intacto)
boot/       _start (0x10000), tick 0x160c2      [nuevo]
platform/   HAL PC (os_*/hw_*/psg_*/sched_*) + render + audio  [nuevo]
app/        main + driver + presentación         [nuevo]
assets/     icono (kult_icon.ico)                [nuevo]
docs/  adr/  notes/
```

## Decisiones cerradas
SDL3; `SDL_Renderer` base y `SDL_GPU` para CRT; audio en hito propio; menú de
idioma fiel; `boot/` fuera de `src/`; headless primero. Ver `docs/adr/`.
