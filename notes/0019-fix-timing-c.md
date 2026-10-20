# 0019 — T-073c: timing del retrato de C (`dbf` inline de `0x16dec`)

> Cierra T-073c. Complementa a `notes/0017-retomar-maestro.md` (handoff) y
> `notes/0018-plan-c.md` (plan anti-fallo). El fix se aplicó con permiso
> explícito del usuario para tocar `src/`, **solo como anotación de timing**.

## 1. Resultado

- La causa de la aceleración de C era el bucle `dbf` inline de
  `object_info_card` (`0x16dec`), que el core ejecutaba como C normal y **no**
  declaraba como espera de CPU.
- Fix: `src/loop.c` usa `cpu_delay_cycles()`; en el diferencial es **no-op** y
  en el port cobra ciclos por el mismo reloj de T-073b.
- Cadencia: de ~3 VBL/frame a ~**7,6 VBL/frame** (3 + 4,6 VBL del `dbf`).
  A/B no usan `0x16dec`; el input no se toca.
- `make test`: **499** en el momento del fix (actual: **511**); `make test-port`: OK (incluye `test_cpu_charge_cycles`;
  la guardia de input actual es `test_click_skip`, ver `notes/0020`).
- Exe estándar: `build-win/kult-0.1.0-win64/kult.exe`
  md5 **`fc4108e473a9d44c722245345a44fa32`**.
- Validación visual: confirmada por el usuario; commit `3235aa6`.

## 2. Síntoma y modelo

- **A**: 3 retratos + descripciones, pixel-reveal, quietos, sin diálogo.
- **B**: 4 retratos + descripciones, lift-reveal, quietos, sin diálogo; aquí
  están los 4 bancos `15ac8`/`0x36`.
- **C**: 1 retrato del Maestro + lift-reveal + diálogo; único diálogo y único
  retrato que se mueve, después de B.

C se identificó en los logs por la firma:

`object_info_wait` (`0x189aa`) → `object_info_card` (`0x16dec`) →
`walk_entry` (`0x12950`) → `elem_list_build` (`0x129c8`) →
`elem_bitmap_build` (`0x1296c`) → `elem_blit`/`elem_blit_rev`
(`0x12ad6`/`0x12b04`) → `hint_select_on` (`0x120dc`) → `blit_rect_vbl`
(`0x124da`) → `blit_rect` (`0x12510`).

`124da` y `12510` aparecen siempre juntos porque `blit_rect_vbl` llama a
`blit_rect`; **no** era un problema de “variante sin espera”. C sí espera VBL.
No aparece `15ac8` (eso es B) ni `15da6`/`S` en C.

## 3. Medición

Logs usados:

- `temp/kult-marcas-master.log`: F12 en VBL 620; C en **601–624**.
- `temp/kult-marcas-banks.log`: C en **555–579**.

Los 1219–1249 / 1513–1543 de las notas anteriores son de otra run; la firma de
funciones es idéntica.

Antes del fix, en master: 8 blits `124da` en VBL 603, 606, 609, 612, 616, 618,
621, 624 → deltas 3,3,3,4,2,3,3 ≈ **3 VBL/frame**; la llamada a `16dec` ocupa
601–624 (~24 VBL). En banks: entradas en 559, 562, 565, 568, 571, 574, 577
(~3 VBL).

## 4. Causa raíz

El guion de C (`0x19708` y `0x1970c`) es `18 01 01 0a`:

- opcode `0x18` = `object_info_card`,
- índice `0x01`,
- argumento `0x0a`, que se guarda en `0x1aa6e`.

ASM `0x16e8e–0x16ec8`:

```asm
clr.l  d1
move.w $1aa6e.l,d1
beq.w  ...
cmpi.b #$25,-1(a5)
bne.w  ...
swap   d1
lsr.l  #4,d1          ; D1 = wait << 12 = 0xA000 = 40960
loop:
subq.l #1,d1          ; 8 ciclos (68000)
bne.w  loop           ; 10 ciclos tomado
```

Con `wait=0x000a`: **40960 iteraciones × 18 ciclos = 737280 ciclos**.
A 8 MHz son **~92 ms ≈ 4,6 VBL por entrada**. El bucle es de registros, sin
accesos a memoria, así que el cálculo es determinista.

El C era correcto en lógica: hacía las mismas iteraciones y no tocaba estado
observable. Lo que faltaba era declarar ese tiempo. El diferencial compara
imagen/estado, no ciclos, por eso los 499 no lo detectaban. Es exactamente la
categoría de T-073a/b: bucles que solo consumen CPU y deben pasar por el modelo
de reloj. Ya se hacía en `src/anim.c` (`0x15da6`) y `src/reveal.c`
(`0x12e2e`/`0x12f28`); faltaba esta anotación.

## 5. Fix aplicado

### `src/loop.c` (`object_info_card`)

Antes:

```c
wait = ((wait << 16) | (wait >> 16)) & 0xffffffffu;
wait = (wait >> 4) & 0xffffffffu;
do {
    wait = (wait - 1) & 0xffffffffu;
} while (wait != 0);
```

Ahora:

```c
/* 0x16ec2-0x16ec8: swap + lsr.l #4 deja D1 = wait<<12; el
 * bucle original es `subq.l #1,d1` (8 ciclos) + `bne.w` (10),
 * 18 ciclos por iteracion. En el diferencial es no-op. */
uint32_t loops = ((wait << 16) | (wait >> 16)) >> 4;
cpu_delay_cycles((uint64_t)loops * 18u);
```

La rama `0x25` no se toca: sigue usando `hint_color()`/`screen_clean()`, que ya
tienen sus esperas.

### Hooks de port

- `include/kult_port.h`: `cpu_delay_cycles()` es **no-op** en el diferencial.
- `platform/include/kult_port.h`: enlaza con
  `platform_cpu_charge_cycles()` (`platform/time.c`), el mismo reloj que usa
  T-073b. Respeta `KULT_CPU=0`, `KULT_CPU_HZ` y `KULT_ST_CONTEND`.
- No se usa `KULT_FRAME_GATE`, ni `15da6`, ni cambios de input.

### Prueba

`platform/test/test_platform.c`: `test_cpu_charge_cycles()`.

- Sin tiempo real no avanza frames.
- Con tiempo real, 737280 ciclos (~92 ms a 8 MHz) cede **≥4 VBLs**.

## 6. Verificación

- `make test`: **499 passed**.
- `make test-port`: OK (la guardia de input es `test_click_skip`, ver
  `notes/0020`).
- `make port`, `make cmake-native`, `make cmake-win`, `make package-win`: OK.
- Artefactos:
  - `kult.exe` md5 `fc4108e473a9d44c722245345a44fa32`
  - `kult-headless.exe` md5 `e37f6592286f1929c04efc46d9051eef`
  - ZIP md5 `8c926644eeb259737047ba4b49d6c2a7`
- La run automática (`KULT_AUTOINPUT` + `KULT_F12_AT`) no llega a C; la
  validación visual la confirmó el usuario (commit `3235aa6`). El fix no es un
  ajuste a ojo: sale del ASM y del reloj de CPU ya usado por el resto del port.

## 7. Referencias

- `notes/0015-ritmo-cpu.md` — modelo de CPU (T-073a/b).
- `notes/0017-retomar-maestro.md` — handoff A/B/C.
- `notes/0018-plan-c.md` — protocolo anti-fallo.
- `src/loop.c` (`object_info_card`), `src/sprite.c` (`blit_rect_vbl`),
  `platform/time.c` (`platform_cpu_charge_cycles`).
