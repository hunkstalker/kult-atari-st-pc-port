# 0023 — Ritmo de la muerte: costes medidos de sangre y viaje espacial

> Punto 1 de los fallos de la validación. Las animaciones de sangre y viaje
> espacial iban aceleradas porque sus funciones tenían **coste 0** en el modelo
> de CPU (T-073b). Se corrige con costes **medidos** del oráculo, sin tocar
> `src/` y sin gates manuales.

## 1. Evidencia en el log

Run nueva (F12 5400; reinicio en 5398, ya sin bucle):

- **Sangre**: `181f0` (`room_reset`) + `13d0e` (`reveal_line`) + `16230`
  (`pal_next_word`) en **5273–5278**.
- **Viaje espacial**: `18334`/`1834a` (`icon_scale`/`icon_scale_loop`) +
  `18316` (`icon_list_build`) desde **5278 hasta ~5393**.
- **Estrella final**: `159e2`/`15abe`/`15bcc`/`15cd8`/`15da6` en
  5379/5384/5389 (delay 20) — ver punto 2, aún pendiente.
- **Reinicio**: `1682a` en 5398 → `c001` en 5399. El fix de `notes/0022`
  funciona.

## 2. Medición del oráculo

Con el harness y los escenarios diferenciales ya existentes:

- `room_reset` (`0x181f0`): 596.918 ciclos/llamada en el escenario de test,
  pero **no es una buena unidad de cobro**: el ASM llama a `reveal_line`
  (`bsr 0x13d0e`) **dentro de dos bucles `dbf` (16 líneas × pasadas)**. Cobrar
  todo al entrar en `room_reset` solo añade un retardo previo; no reparte el
  trabajo entre las líneas y la animación sigue pareciendo instantánea.
- `reveal_line` (`0x13d0e`) sí es la función que dibuja cada línea. Ajuste
  ponderado con el oráculo usando seis rectas sueltas **y** el conjunto de
  `room_reset` (304 líneas, 1392 pasos, 596.918 ciclos):
  ciclos ≈ **1063 + 198 × pasos**, con `pasos = max(|dx|,|dy|)+1`.
  (Las rectas sueltas de 17/29/49 pasos dan 5.142/6.694/10.142; el ajuste
  conjunto evita sobrecargar las líneas cortas de la sangre.)
- `icon_scale` (`0x18334`) completo: 18.461.836 ciclos, de los que 101 son
  esperas de VBL (~16.160.000). CPU restante: 2.301.836 ciclos →
  **22.790 ciclos/pasada** de `icon_scale_loop` (`0x1834a`). Esta sí es la
  unidad correcta: una pasada por llamada y `wait_vbl` al final.
  (Incluye `icon_list_build`; no se cobra aparte.)

## 3. Fix

1. En `tools/cpuwork/funcost_st_reference.txt`:
   ```text
   1834a 2301836 101
   ```
   (`181f0` se deja a 0; su trabajo se cobra en `reveal_line`.)

2. En `platform/cpuwork.c`, caso especial al entrar en `reveal_line`:
   lee `(0x1a516,0x1a51a)->(0x1a518,0x1a51c)` y cobra
   `1063 + 198 * (max(|dx|,|dy|)+1)` ciclos. Así el coste va **por línea**,
   intercalado con el dibujo, igual que el original.

Regenerado con:

```sh
make cmake-native
python3 tools/gen_cpuwork_costs.py
```

Resultado: `platform/cpuwork_costs.inc` tiene **80** funciones con coste;
`icon_scale_loop` cobra 22.790/pasada y `reveal_line` cobra la fórmula medida
(la tabla la deja a 0; el caso especial es el que cobra).

No se toca `src/`, no se añaden retardos manuales ni gates. El cobro va por
`platform_cpu_charge_cycles` (mismo reloj que T-073b).

## 4. Verificación

- `make test`: **499**.
- `make test-port`: OK.
- `make port`, `make port-sdl`, CMake nativo/win y ZIP: OK.
- Smoke headless y SDL dummy: OK.
- Prueba directa con un `main` aparte que llama a `room_reset` con la misma
  mesa del test: con CPU on **84 ms / 4 frames**; con `KULT_CPU=0`
  **0,08 ms / 0 frames**. El coste se cobra y se reparte entre las 304 líneas.
- Ejecutables:
  - `kult.exe` md5 `b7edc93660ea7caadd87191f8ffeee7a`
  - `kult-headless.exe` md5 `7faac3d3351c24c46f825b4096557130`
  - ZIP md5 `97d567898127ab8290d4155113c312d1`

### 4.1 Validación en Windows (run nueva, añadida al log)

Run con el exe `b7edc936…` (`# RUN` en la línea 43706 del log; F12
2298/2344/2422/2459/2482/2533/2635):

- **Sangre**: el usuario la ve **perfecta**.
- **Viaje espacial**: `18334` una vez en VBL **2373**; `1834a` entra en
  **2373–2473**, exactamente **101 entradas**, y la estrella empieza en
  **2474**. Coincide con el ASM: `d5 = 0x64` + `dbf` = 101 pasadas y
  `wait_vbl` (`0x13594`) al final de cada `icon_scale_loop`. Duración:
  **101 VBL ≈ 2,02 s**. El coste de 22.790 ciclos/pasada (~2,85 ms) cabe
  dentro del VBL de 20 ms, así que no añade VBLs; es CPU dentro de cada
  pasada, como en el original.
- **Reinicio**: `1682a` en **2624**, seguido de `c001` y la escena
  Maestro/objeto (`159bc`, 2625–2684). Correcto.

Por tanto el punto 1 queda validado. La run anterior (F12 5278–5398) queda
como evidencia del reinicio; la nueva es la que valida sangre y espacio.

## 5. Nota sobre el primer intento

El primer intento cobraba `room_reset` al entrar (596.918 ciclos). No servía:
el dibujo de las 16 líneas seguía siendo instantáneo tras el retardo previo.
Se ha retirado ese cobro y se ha movido al **leaf** (`reveal_line`), que es
donde el ASM gasta el tiempo y donde el retardo se reparte entre líneas.

Si tras esto la sangre sigue algo rápida/lenta, el siguiente paso es medir la
escena concreta de muerte con el oráculo (capturando su estado), no añadir un
retardo a ojo.

## 6. Referencias

- `notes/0015` (T-073b), `notes/0022` (reinicio).
- `tools/cpuwork/funcost_st_reference.txt`, `platform/cpuwork_costs.inc`.
- Log: `build-win/kult-0.1.0-win64/kult-marcas.log` (F12 5400; muerte
  5273–5398).
