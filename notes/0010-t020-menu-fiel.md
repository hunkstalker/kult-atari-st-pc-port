# Nota 0010 — T-020/T-022/T-023 y preparación de M6

- **Sesión 10.** Cierre del `_start` fiel (T-020), parte de T-022 y T-023; y
  preparación de la validación (M6/T-060/T-061).

## T-020 — menú de idioma fiel
- **Antes:** `boot/boot.c` forzaba `A_MODE_FLAG = 0x87` para que `prompt_wait`
  saliera por `Cnecin` en vez de esperar el clic del título, y no drenaba teclas.
- **Ahora (fiel a `_start`):**
  - `prompt_wait` espera un **clic real** (bit 2 de `A_INPUT_BUTTONS`); en headless
    lo libera el clic alterno de la HAL y con entrada real el botón izquierdo.
  - `boot/language.c` (`kult_boot_language`): **drena** las teclas pendientes
    (Cconis 0x0B + Cnecin 0x08; el port usa `Crawio(0xff)` como equivalente no
    bloqueante) y luego espera (Cnecin bloqueante) una tecla **D/E/F** (a
    mayúsculas). `kult_boot_apply_language` escribe la letra en las 4 entradas de
    la lista A (`0x1a3aa+4/12/20/2e`).
  - `l_010158`: se espera a **soltar** el clic (`do { wait_frame } while (bit 2)`).
  - `KULT_SKIP_DRAP` se conserva solo como escape de bring-up (no fiel).
- Prueba: `test_boot_language` en `make test-port` (drenado, minúscula→mayúscula,
  teclas inválidas ignoradas, escritura en las 4 entradas).

## T-022 — `0x135ba` y `0x106f2`
- `0x135ba` = XBIOS `Setscreen(-1,-1,0)`: se llama `os_setscreen(0xffffffff,
  0xffffffff, 0)` en el punto exacto de `_start` (la HAL lo trata como no-op: no
  cambia la base).
- `0x106f2` = `l_0101a8` (reinicio suave): carga la **lista C** (`0x1a2c8`),
  reinicia el estado de pantalla (`crt0`) y recarga la lista A, volviendo al bucle
  de carga. **Diferido**: solo se alcanza por un **puntero relocalizado**
  (`+0x101a8`, ver `0x16f90`/`0x106e0`) y no es observable en el flujo actual. Se
  conectará si la partida lo ejerce (documentado en `docs/LIMITACIONES.md`).

## T-023 — FDC omitido
- La comprobación `0x19316` no se reconstruye; se **siembra** el patrón FDC en
  `0xd8e8` (bloques de 0x18, 0x46 veces) para que el modo de escena `0x14496` no
  espere. La espera de `0x1947e` pasa inmediatamente (queda 0).

## M6 — preparación de la validación
- **T-060** ✅: se mantiene el menú de idioma **fiel** (ADR 0004), validado en T-020.
- **T-061** 🚧: checklist y comandos en `docs/VERIFICACION.md`; limitaciones
  conocidas en `docs/LIMITACIONES.md`. La ejecución (jugar y escuchar) la hace el
  usuario con su copia de datos, display y audio.

## Verificación
- `make test` 499 verde; `make test-port` verde (incluye `test_boot_language`).
- `make port` headless: arranca, atraviesa `pres/drap/menu → lang → lista_a →
  fond → eche → clear/music → l_010158 → c001 → hint_icons → game_loop`.
