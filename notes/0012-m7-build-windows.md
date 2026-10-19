# Nota 0012 — M7 build (T-070) y empaquetado interino (T-071)

- **Sesión 12.** Build **Windows** (`kult.exe`) para que el usuario valide
  (T-061) y empaquetado interino (ZIP/TGZ). `src/` **intacto**; todo el cambio
  vive en `platform/`, `app/`, `CMakeLists.txt`, `cmake/`, `assets/` y `tools/`.
- Verificación aquí: `make test` **499 verde**, `make test-port` verde,
  `make port`/`make port-sdl` y build nativo CMake; **cross Windows OK** (link,
  recurso de icono, dependencias). **Sin wine** en el contenedor: el `.exe` no se
  ha ejecutado aquí → lo valida el usuario (T-061).

## T-070 — build Windows/Linux (CMake)

### Portabilidad (M7)
- **`platform/time.c`**: reloj/espera con `#ifdef _WIN32`
  (`QueryPerformanceCounter` + `Sleep`); POSIX sigue con
  `clock_gettime`/`nanosleep`. Sin SDL (el headless no enlaza SDL).
- **`platform/fs.c`**:
  - separador de `KULT_DATA_DIR`: **`;` en Windows**, `:` en POSIX
    (`DATA_SEP`);
  - directorio por defecto: **`data/` junto al ejecutable**
    (`GetModuleFileNameA` / `readlink(/proc/self/exe)`; si no se puede resolver,
    `data/` del directorio actual). Antes era `data/` relativo al cwd;
  - nuevo **`platform_data_resolve(name, out, n)`** (público en `platform.h`)
    para localizar un fichero en los directorios de datos.
- **`app/main.c`**:
  - `KULT_PRG` ya no tiene por defecto `work/disk_a/KULT.PRG`: si no está
    definido se busca **`KULT.PRG` en los directorios de datos** (el paquete
    Windows lo lleva en `data/`); la ruta del oráculo queda como último recurso;
  - `platform_init()` se hace antes de cargar el PRG (los dirs de datos se
    resuelven en `platform_fs_init`); el PRG se carga justo después;
  - volcado PPM: en **ventana no se vuelca por defecto** (`KULT_DUMP` lo fuerza);
    en headless sigue siendo `out/frame.ppm`;
  - firma `int main(int argc, char **argv)` (SDL3 en Windows lo exige) e
    `#include <SDL3/SDL_main.h>` bajo `HAVE_SDL3`.

### CMake (raíz) y toolchain
- **`CMakeLists.txt`**: enlaza `src/ + boot/ + platform/ + app/` con
  `-Iplatform/include` **el primero** (sombrea `os.h`/`hw.h`/`psg.h`/`sched.h`).
  Targets: **`kult`** (headless) y **`kult-sdl`** (SDL3, `HAVE_SDL3`).
  - En **Windows**: `kult-sdl` → **`kult.exe`** (subsistema GUI, icono) y
    `kult` → `kult-headless.exe` (consola); `find_package(SDL3 CONFIG)`.
  - **El Makefile se mantiene** para el core/diferencial y para
    `make port`/`port-sdl`/`test-port`; además hay envoltorios
    `make cmake-native`, `cmake-win`, `package-native`, `package-win`.
- **`cmake/mingw-w64.cmake`**: cross x86_64 (gcc + windres).
- **SDL3 3.4.10 MinGW** en `third_party/` (**gitignored**):
  `SDL3-devel-3.4.10-mingw.zip` (headers, `libSDL3.dll.a`, CMake config, DLL) y
  `SDL3-3.4.10-win32-x64.zip` (el `SDL3.dll` que va al paquete).

### Icono
- `assets/kult_icon.ico` (ya copiado en la sesión 11) + **`assets/kult_icon.rc`**
  (`101 ICON "kult_icon.ico"`, `windres` con `-Iassets` y `OBJECT_DEPENDS`).
- **`assets/kult_icon_rgba.h`**: 48x48 RGBA embebido para
  `SDL_SetWindowIcon` (`platform/video_sdl.c`); lo genera
  **`tools/make_icon_rgba.py`** (parser del `.ico`, sin dependencias) y es
  **byte-idéntico** al del predecesor (verificado).

## T-071 — empaquetado interino

- **CPack**: ZIP en Windows (`kult-0.1.0-win64.zip`: `kult.exe` + `SDL3.dll` +
  `data/` vacía + `LEEME.txt`) y TGZ en Linux (`kult` + `kult-sdl`).
  **Sin assets**: el usuario aporta su copia en `data/`.
- Salidas (gitignored): `build-win/kult-0.1.0-win64.zip` (~1,3 MB),
  `build-cmake/kult-0.1.0-linux-x86_64.tar.gz`.
- El binario de Windows depende de **SDL3.dll** y de **UCRT**
  (`api-ms-win-crt-*`, Windows 10+). No lleva DLLs extra.

## Verificación de esta sesión
- `make test` → **499 passed**; `make test-port` → OK.
- `make port`, `make port-sdl`, `make cmake-native` (→ `build-cmake/kult`,
  `kult-sdl`) sin warnings nuevos.
- Smoke headless y ventana (dummy): `KULT_DATA_DIR=/tmp/kult_data` sin
  `KULT_PRG` → resuelve `/tmp/kult_data/KULT.PRG` y el resultado (frames,
  `fb_nonzero`, audio) coincide entre el binario del Makefile y el de CMake.
- Cross: `kult.exe` (GUI) y `kult-headless.exe` enlazan; `.rsrc` con el icono
  (~170 KB); dependencias: `SDL3.dll`, `KERNEL32.dll`, `api-ms-win-crt-*`.
- `cpack`: ZIP y TGZ generados con el contenido esperado.

## Pendiente
- **T-061**: validación del usuario en Windows (arranque, partida, guardado,
  audio, **morir → reiniciar sala**). Checklist en `docs/VERIFICACION.md`.
- **M7-c** (empaquetado completo + ingesta `kult.data`) va de la mano de N3.
