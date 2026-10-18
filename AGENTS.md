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
Fase **M0**: documentación creada, **código aún no importado**. Sin bloqueantes.

## Reglas de trabajo
1. **Reproducir/medir antes de tocar.** Una hipótesis, un cambio.
2. **`src/` (el juego) no se modifica.** Lo específico de máquina va en
   `platform/`; el arranque/tick en `boot/`; el driver en `app/`.
3. **Los 499 tests diferenciales del core deben seguir verdes.** Son la red de
   seguridad de que no cambiamos la lógica.
4. **Toda corrección/feature deja prueba** (diferencial si aplica; funcional si es
   del backend).
5. **No tocar** `/app/kult-port-legacy` ni `/app/kult-atari-st-decomp`.
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
docs/  adr/  notes/
```

## Decisiones cerradas
SDL3; `SDL_Renderer` base y `SDL_GPU` para CRT; audio en hito propio; menú de
idioma fiel; `boot/` fuera de `src/`; headless primero. Ver `docs/adr/`.
