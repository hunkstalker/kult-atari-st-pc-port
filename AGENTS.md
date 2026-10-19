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

## Reglas de trabajo
1. **Reproducir/medir antes de tocar.** Una hipótesis, un cambio.
2. **`src/` (el juego) no se modifica.** Lo específico de máquina va en
   `platform/`; el arranque/tick en `boot/`; el driver en `app/`.
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
