# 0031 — Sesión 34: cierre de T-083 (tail-call de `icon_ring_loop`)

> **Estado:** resuelto y validado con `kult.exe` y `kult-diag.exe`. Este documento
> resume la sesión completa: evidencia, causa, fix, artefactos y validación.

## 1. Síntoma

Al usar un objeto en la boca de una serpiente, el port podía ejecutar una
cabecera de plantilla TEMPL como si fuera un guion:

- cuchillo / 1.ª serpiente: `Y 3336 10 2563e` (`scale_prep`, retrato 73×182).
- dado / 2.ª serpiente: `Y 5342 08 206b5` (`blit_dispatch_b`) y bucle
  `F 12b40` hasta el cierre.

El objeto es aleatorio; los dos síntomas parecían distintos, pero compartían
mecanismo.

## 2. Diagnóstico

Se añadió al build `KULT_DIAG_TRACE=ON` una traza `D` de control de la VM:

- `VMIN` / `VMOUT`: entrada/salida de `vm_run` con `A6`, `A_TEMPL_PTR`, pila.
- `RUN`: comando, script, nibble, punteros y pila al despachar.
- `WALK_RUN` / `WALK_INLINE` / `WALK_4D`: índice, `entry` y `end`.
- `JMP` / `CALL` / `RET`: destino, retorno y pila antes/después de
  `0x33/0x34/0x35`.

La run con el fix aún no aplicado mostró:

```text
D 1303 VMIN a6=1fbc0 tmpl=1fc2e
Y 1303 3d 1fc1a          (icon_ring_loop, 0x15888)
D 1590 RUN cmd=c32f ...  (nibble c)
D 1711 VMOUT a6=20726 tmpl=20727
Y 1711 34 1fc2e          (cabecera de la entrada siguiente)
```

`0x1fc2e` es la cabecera de la entrada `0xd3`; su longitud `0x34` se leía como
`call` y saltaba a `0x2563e`. En la 2.ª serpiente el mismo mecanismo llevaba a
`0x206b1` (cabecera de `0x132`) y `0x206b5` con `py=0` → índice `0xffff` →
`elem_list_build`/`walk_entry` fuera de rango → bucle `12b40`.

## 3. Causa raíz

ASM `0x15888`, rama `0x1592e`:

```asm
01592e: 588f        addq.l #4,a7   ; descarta el retorno al VM exterior
015930: 6000aa7c    bra.w $103ae   ; tail-call a run_command_keep_sp
```

No es una llamada normal: es un **retorno doble/tail-call**. El VM exterior se
abandona y el `vm_run` anidado vuelve a su llamante con su propio `A6`. El C
hacía:

```c
run_command_keep_sp(img, script);
return script;   // el VM exterior seguía vivo
```

El despacho anidado ya había pisado `A_TEMPL_PTR` (`0x20727`), así que el VM
exterior no reconocía su antiguo final (`0x1fc2e`) y ejecutaba la cabecera
siguiente.

La auditoría de `notes/0028` había detectado `0x15888` como retorno doble, pero
lo dio por modelado solo por sustituir `addq`+`bra` por el despacho; faltaba
modelar el abandono del VM exterior.

## 4. Fix

- `src/dispatch.c`: `run_command_keep_sp_a6()` devuelve el `A6` final del
  despacho; `run_command_keep_sp()` queda como wrapper compatible.
- `src/commands.c`: `icon_ring_loop` usa `run_command_keep_sp_a6()` y devuelve
  su `A6` en las dos ramas de `0x1592e` (nibble `c`/`a` y comando pendiente).
- `platform/dispatch_trace.c`: mismo despacho para el build de diagnóstico.
- `kult_core.map` / `kult_api.h`: exporta el nuevo símbolo.
- Test: `test_run_command_keep_sp_a6_nibble0`.

## 5. Validación

- `make test`: **512 passed**.
- `make test-port`: **OK**.
- Smoke nativo headless: OK.
- `kult.exe`: el usuario confirma dado en la 2.ª serpiente y mosca en la 1.ª
  sin error.
- `kult-diag.exe`: el log
  `build-win/kult-0.1.0-win64/diag-logs/post-fix-dado-mosca-2026-10-21.log`
  no contiene `0x1fc2e`, `0x206b1`, `0x2563e` ni `0x206b5`. Tras el `0x3d`
  aparece `VMOUT a6=20726 why=end_byte tmpl=20727` y siguen los comandos
  normales.
- Log pre-fix conservado en `diag-logs/pre-fix-2026-10-21.log`.

## 6. Artefactos

```
kult.exe               ae991db620516ed925afc3eda1352d25
kult-headless.exe      57e09a8414d91c1a33860b8b6e3b5fd4
kult-diag.exe          28c63ef30571be50b6309160f5febe3b
kult-diag-headless.exe e0a20681954fd89fdb7182f098979bf8
```

## 7. Pendiente de pulido

El fix es correcto pero la emulación del tail-call es implícita: devuelve el
`A6` anidado y confía en que el VM exterior pare en el siguiente chequeo de
byte/`A_TEMPL_PTR`. El refactor de cierre introduce un resultado explícito de
tail-call en la VM para que `0x3d` pueda abandonar el VM exterior sin depender
de ese chequeo.
