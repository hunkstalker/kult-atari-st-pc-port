# 0097 — Fix del pump de presentación y release 1.2.1

Fecha: 2026-10-26. Estado: **QA Windows hecha; tag local `v1.2.1`; pendiente push/CI. Bugs gráficos conocidos para parches posteriores.**

## Síntoma

En 1.2.0 el pixel-reveal inicial y las transiciones de aparición/desaparición
de retratos iban más lentos. El log mostraba `present: 155–165 Hz`, por encima
del máximo documentado de ~120 Hz.

## Causa

- `cpu_charge_us()` y `platform_disk_delay()` dormían con `sleep_with_render()`.
- Ese pump presentaba antes de dormir; con VSync, un `cpu_delay` de ~1 ms podía
  costar ~6 ms. Repetido por píxel/frame, alargaba reveal y retratos.
- El pump tampoco tenía un cap real por tiempo entre presentaciones.

## Fix

- El pump no se ejecuta durante `cpu_delay`/`disk_delay`; esos bucles siguen
  cediendo VBLs y presentando a 50 Hz.
- La cadencia del pump se limita a ~120 Hz con un intervalo en `time.c`; no se
  usa un cap global de presentación.
- `KULT_RENDER_PUMP=0` desactiva el pump; `60`/`120` ajustan su frecuencia.
- Tests actualizados: `cpu_delay`, `disk_delay` y `cpu_charge_cycles` no deben
  llamar al pump.

## Validación Windows

- `baseline`: retratos lentos (el fallo de 1.2.0 se reproduce).
- `cpufix` y `timefix`: retratos bien; audio bien.
- `capfix` y `bothfix`: audio mal en el intro.
- `timefix` con `KULT_RENDER_PUMP=0`: mismo resultado bueno.
- Conclusión: el cap global de presentación de `video_sdl.c` era el causante del
  ruido; se descartó. El fix que queda es solo `time.c`.

## Medición de reveals por runtime (1.0.0 / 1.2.0 / 1.2.1)

Medición hecha con el core instrumentado (`-finstrument-functions`) en builds
SDL de `v1.0.0` (`8eb079a`), `v1.2.0` (`c72051d`) y 1.2.1 (`9b198ae`). El
profiler temporal registra entrada/salida de cada función con VBL y reloj
monotónico (`platform_now_ns`) en `KULT_PROFILE_LOG`; no usa capturas ni
píxeles. Mismo `kult.data`, `KULT_AUTOINPUT=1` y VSync de 60 Hz (Xvfb);
`KULT_PROFILE_US=1000`. Las capturas de `temp/` solo sirven como corroboración
visual, no para los tiempos.

Funciones medidas, por orden de aparición:

- **A**: tres llamadas a `reveal_pixel` (`12f28`), una por retrato.
- **B**: las cuatro primeras llamadas a `anim_bank` (`15ac8`), los cuatro
  bancos de retrato. El lift de pantalla (`copy_rows_a`) es aparte.
- **C**: la llamada a `object_info_card` (`16dec`) que dibuja el retrato del
  Maestro. No hay `15ac8`/`15da6` en C.

| Retrato | v1.0.0 | v1.2.1 | v1.2.0 |
|---|---:|---:|---:|
| A1 | 852,96 ms / 42 VBL | 867,31 ms / 43 | 12.683,86 ms / 634 |
| A2 | 800,17 ms / 40 VBL | 806,80 ms / 40 | 11.817,10 ms / 591 |
| A3 | 799,63 ms / 40 VBL | 804,90 ms / 41 | 12.167,14 ms / 609 |
| B1 | 10,38 ms / 1 VBL | 10,20 ms / 1 | 116,63 ms / 6 |
| B2 | 8,20 ms / 0 VBL | 8,31 ms / 0 | 66,59 ms / 3 |
| B3 | 8,40 ms / 1 VBL | 8,23 ms / 1 | 68,74 ms / 3 |
| B4 | 8,28 ms / 0 VBL | 8,28 ms / 0 | 64,45 ms / 4 |
| C1 | 1.103,24 ms / 55 VBL | 1.103,04 ms / 55 | 2.900,05 ms / 145 |

- Suma A: 2.452,76 / 2.479,01 / 36.668,10 ms.
- Suma B (4 bancos): 35,26 / 35,02 / 316,41 ms.
- C: 1.103,24 / 1.103,04 / 2.900,05 ms.
- Inicio del primer reveal A → inicio de C: 18,06 / 18,32 / 65,60 s;
  incluye esperas y clics, no solo los reveals.
- El lift de pantalla de B (`copy_rows_a`) queda en ~1,05 s en las tres
  versiones y no se usa para el tiempo por retrato.
- Variabilidad (5 ejecuciones, log directo de `reveal_pixel`, sin el profiler
  genérico): A1 853,21 ± 1,98 vs 859,19 ± 2,75 ms; A2 796,39 ± 1,80 vs
  801,47 ± 2,87 ms; A3 797,88 ± 1,04 vs 798,27 ± 2,89 ms (1.0.0 vs 1.2.1).
  Con `KULT_RENDER_PUMP=0` en 1.2.1: 856,21 / 798,34 / 799,86 ms.
- La diferencia 1.0.0 → 1.2.1 en A es +0,05–0,7 % (≈0,4–6 ms), menor que un
  VBL; es ruido de alineación de VBL más el coste del pump, no un cambio del
  reveal. El profiler genérico daba +1,6–1,9 % porque añade instrumentación a
  todas las funciones.
- B/C en 5 ejecuciones: B ~8–10 ms en ambas; C 1.106,5 ± 6,9 vs
  1.116,7 ± 10,7 ms (con un outlier de 1.136 ms); diferencia no significativa.
- 1.2.0 multiplica A por ~14–15×, B por ~8–11× y C por ~2,6×.
- Las cifras absolutas de 1.2.0 están infladas por el VSync de 60 Hz de Xvfb;
  en 165 Hz serán menores, pero la comparación relativa se mantiene.

## Limpieza y desacoplo

- La medición de reveals se hizo en **copias temporales** del core con
  instrumentación (`KULT_PROFILE_LOG` y un log directo de `reveal_pixel`); no
  se añadió ningún test ni instrumentación al repo. El binario de release no
  se usa como entrada de tests.
- Los tests existentes (`make test`, `make test-port`, `pytest test/`) siguen
  desacoplados: compilan fuentes y no dependen de `build-win/` ni de un ZIP.
  Los diagnósticos siguen en `platform/diag.c`, activados por entorno.
- Se eliminaron solo los artefactos de test: variantes de 1.2.0, trazas,
  capturas, `.pytest_cache/`, `__pycache__/`,
  `temp/mediciones-1.2.1/`, `oracle/build/` y las builds instrumentadas
  temporales. También se eliminó la rama local obsoleta
  `wip/1.2.1-pump-fix` (`f9d96de`), que contenía el capfix erróneo descartado;
  la solución correcta es `9b198ae` en `main`.
- Se conserva el build oficial 1.2.1 en `build-win/`:
  `kult.exe`, `kult-headless.exe`, `libkultport.a` y
  `kult-1.2.1-win64.zip` (recompilado desde `9b198ae`; el ZIP anterior se
  borró y su hash no es reproducible).
- Se restauraron las carpetas locales 1.0.0/1.2.0 desde `/app/Kult PC` a
  `build-win/`, con `data/`, `saves/` y sus ZIP oficiales. Las copias de
  `saves/` contienen `CLEAR.BIN`; no se han encontrado `slot-*.save` de
  usuario en el sistema.
- Para una QA aislada se puede generar otro ZIP fuera:
  `make BUILD_WIN=/tmp/opencode/kult-1.2.1-test package-win`.

## QA final (hecha)

- El usuario probó el ZIP de 1.2.1 en Windows a 165 Hz: arranque, idioma,
  reveals A/B/C y uso general. No se reportaron incidencias de timing.
- Quedan **bugs gráficos conocidos**, que se abordarán en parches posteriores
  (T-142); no bloquean la release 1.2.1.
- La batería de timing ya estaba medida; esta QA valida el artefacto y la
  percepción en el entorno real.

## T-136 (para que no se confunda con 1.2.1)

- T-136 es la **QA visual del DMG de macOS Universal 2 en un Mac Intel**,
  pendiente desde la release 1.1.0. Apple Silicon está validado por CI; macOS
  va firmado en ad-hoc y sin notarizar.
- No es un bloqueante del fix de timing de 1.2.1; si no hay Mac Intel, queda
  como incidencia conocida en la release.

## Release

- Versión 1.2.1 en workflow, CMake, Makefile y `build_macos.sh`.
- Notas públicas: `docs/RELEASE_NOTES_1.2.1.md`.
- Notas públicas reescritas para que el Release de GitHub abra con la
  corrección principal (pixel-reveal y retratos lentos), separando la base
  heredada de 1.2.0.
- Paquetes 1.2.1: los genera el CI al hacer push del tag `v1.2.1`, con
  Windows, Linux y macOS (DMG dentro de ZIP + `.zip.sha256`).
- Estado: **QA OK; tag local `v1.2.1` sobre el commit de documentación que
  sigue a `9b198ae`; pendiente push/CI**. Los bugs gráficos conocidos se
  tratan en T-142.
