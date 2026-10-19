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
visible** (paleta del PI1 antes de la espera). Pendiente: **T-061** (validación en
Windows) → **T-072** (ingesta `.STX`) y **T-073** (ritmo: trabajo de CPU no
modelado) → **M7-c** → **A1/N3/A2** (Amiga + motor/assets nativos) → **M5**
(opcional). Sin bloqueantes. Ver `notes/0011`–`notes/0014`.

## T-073c — reglas duras (pantalla C del Maestro)
- **Input invariante**: el **clic izquierdo** interactúa y salta
  transiciones/reveal; el **derecho no tiene función**. **No** cambiar el mapeo
  de botones, no crear copias de `src/input.c`, no tocar `wait_frame` ni el
  estado de botones de SDL. No usar modelos de botones de otros repositorios.
- **Modelo A/B/C**: A = 3 retratos + descripciones (pixel-reveal, quietos);
  B = 4 retratos + descripciones (lift-reveal, quietos, 4 bancos
  `15ac8`/`0x36`); C = 1 retrato del Maestro + lift + **diálogo** (único
  diálogo y único retrato que se mueve), después de B.
- C está en los logs: **1219–1249 / 1513–1543**; ~8 frames/16 blits a 1–3
  VBL/frame; **sin** `15ac8`/`15da6`.
- **No** usar `KULT_FRAME_GATE` ni `15da6` como fix de C sin evidencia.
- Medir antes de tocar; una hipótesis, un cambio; si no mejora, revertir.
- Handoff: `notes/0017-retomar-maestro.md`; plan detallado:
  `notes/0018-plan-c.md`. Prueba de guardia:
  `test_left_click_skips` en `make test-port`.

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
