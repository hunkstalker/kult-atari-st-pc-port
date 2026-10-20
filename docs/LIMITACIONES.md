# LIMITACIONES — estado de M6

Lista de limitaciones conocidas de la compilación PC, para el cierre de **M6**
(pulido y validación). El objetivo de M6 es **jugable de principio a fin**; la
**validación final** (jugar y escuchar) la hace el usuario con su copia de datos.

## Intencionales (no son fallos)
- **Tiempo de disquete modelado (T-079).** El FDC no se emula, pero
  `platform_disk_delay()` estima la lectura de disco del ST (~2 s por 32 KB,
  ver `notes/0025`/`notes/0026`); se aplica a **cada Fread**, así que la
  estrella final se mantiene mientras carga `FOND.PI1` y también hay pausas en
  salas y reinicio. **Excepción:** la carga inicial no espera
  (`platform_disk_timing(0)` desde PRES/DRAP hasta después de ECHE), para que el
  título no tarde, la canción empiece desde el principio y no haya ~20 s de
  pausa tras elegir idioma.
- **El FDC no se emula.** La comprobación de disco `0x19316` se **omite**; se
  siembra el patrón FDC en `0xd8e8` para que el modo de escena (`0x14496`) no
  espere (T-023).
- **`os_super` es un no-op** (en PC no hay modo supervisor).
- **Menú de idioma fiel** (ADR 0004): se mantiene el del original (clic de título
  y tecla D/E/F), no un menú nativo.
- **Cambio de disco omitido.** El original pide "INSERT DISK 2" (`0x918e`) antes
  de cargar los recursos del disco B (`FOND`/`PUZZL`/`BRUIT`). El port sirve
  ambos discos desde el mismo directorio, así que el aviso (y su reintento con
  clic) se **omite**; el arranque continúa directo.
- **Primera pantalla = título (PRES) visible.** El port aplica la paleta del PI1
  cargado (`PRES.PI1`) antes de la espera, así que el **título se ve con sus
  colores** desde el principio y espera **tecla o clic**. En el `_start` original
  el primer `Setpalette` llega después de la espera y el ST hereda la paleta del
  escritorio; el HAL arranca a cero, de ahí el negro anterior. Ahora no debe
  verse negro: si lo estuviera o no avanzara, sería un fallo (criterio en
  `docs/VERIFICACION.md`).
- **Physbase = 0x98000** (decisión del HAL, no un fallo): con el valor del kit
  (`0x80000`) las pantallas (`0x78000/0x80000`) y su paleta (`0x77fe0`) caían
  dentro de los datos del juego (ECHE `0x64c8e..~0x88b84` + heap) → ruido y
  colores mal. Es la misma corrección que usó el predecesor. `src/` no se toca;
  el differential usa el `os.h` del kit.
- **Audio aproximado.** El YM2149 es un modelo mínimo (no ciclo-exacto): 3 ondas
  cuadradas + ruido + envolvente + volúmenes logarítmicos. La **escucha final** la
  cierra el usuario. Ajustes: `KULT_AUDIO=0` (apaga), `KULT_MUSIC_GAIN` (nivel de
  la música, 25 % por defecto).
- **Música de intro solo en las esperas.** En `headless` el port salta el título y
  el menú de idioma, así que `music_isr=0`; en ventana con entrada real la música
  suena mientras el título espera una tecla.
- **Escucha/ventana requieren SDL3 y un equipo con display+audio.** En un
  contenedor sin `/dev/snd`/`DISPLAY` no suena (y el ejecutable cae a headless).

## Pendientes / futuros
- **M5 — filtros CRT** (`SDL_GPU` + shaders): **extra opcional, no prioritario**,
  al final (ver `notes/0009`). Sin filtros hoy (`SDL_Renderer` + escalado entero).
- **Reinicio `0x106f2` (`l_0101a8`)**: **abierto/pendiente** (no portado). Es un
  "reinicio suave" de `_start`; el análisis estático lo ve **inobservable** (solo
  aparece como constante base de relocalización, sin llamante) pero **se deja
  abierto** por si se necesita (`notes/0010`, `notes/0011`). **No confundir** con
  el reinicio tras morir (`0x1842c`→`reset_clear`): el `bra 0x10158` que faltaba
  en el C nativo ya se emula en `boot/` (`notes/0022`).
- **M7 — build/empaquetado**: **build hecho** (T-070): CMake nativo y cross
  MinGW; el ZIP/TGZ interino (T-071) lleva `kult.exe`, `kult-headless.exe`,
  `SDL3.dll` y `data/` vacía (**hay que copiar ahí los ficheros del juego**). El
  `.exe` no se ha ejecutado en el entorno de desarrollo (sin wine): lo valida el
  usuario (**T-061**). Requiere **Windows 10+** (UCRT).
- **Ingesta de discos pendiente (T-072)**: este paquete **no lee** `.STX/.MSA/.ST`;
  `data/` debe contener los ficheros **extraídos**. Es la siguiente tarea tras
  T-061 (no se le debe pedir al usuario un `.PRG` como solución final). El
  contenedor `kult.data` puede ir después con N3.
- **Windows: sin consola.** `kult.exe` es subsistema GUI (sin ventana de
  consola); los errores van a **`kult.log`** (junto al exe) y a un **MessageBox**.
  `kult-headless.exe` (incluido en el ZIP) muestra el diagnóstico por consola.
- **Amiga**: **A1** (investigación/censo/decomp) es **prerrequisito duro de N3**;
  **A2** (implementación) va después de N3.
- **N3 — motor nativo + assets nativos**: hito futuro (ADR + rama) para que el
  runtime no dependa del binario original; formatos nativos (PNG/WAV/OGG/UTF-8 +
  binario propio documentado). Ver `notes/0011`.
- **Guardado/partida completa**: los servicios de fichero son reales (Fcreate/
  Fwrite); **validado por el usuario**.
- **Ritmo / aceleración (T-073)**: implementado el **modelo de CPU medido**
  (T-073a `cpu_delay` + T-073b coste por función con `-finstrument-functions` y
  tabla del oráculo). El port cobra ciclos del original a cada función y los
  duerme cediendo VBLs. **T-073c ✅**: el `dbf` inline de `0x16dec` (retrato de
  C) ya se cobra con `cpu_delay_cycles` (ver `notes/0019`). **T-074 ✅ (HAL)**:
  izquierdo/teclado = acción sin skip; derecho = acción + skip; presentación por
  VBL para ver las animaciones de `wait_vbl` (ver `notes/0020`). **T-077 ✅**:
  `reveal_line` (`13d0e`, coste por línea) e `icon_scale_loop` (`1834a`) ya
  tienen coste medido (ver `notes/0023`). Queda **calibración fina por escenas** con
  `KULT_CPU_HZ`/`KULT_ST_CONTEND`; `KULT_CPU=0` lo apaga. Los casos **no**
  medidos en la referencia (p.ej. `palette_cycle`, cuyo `dbra 0xffff` se omitió
  en el core) pueden seguir rápidos.

## Cómo validar (resumen; detalle en `docs/VERIFICACION.md`)
1. `make test` (**511**) y `make test-port` en verde.
2. `make port-sdl` y `KULT_WINDOW=1 KULT_DATA_DIR=<datos> ./build/kult-sdl`:
   comprobar título (clic), menú D/E/F, intro/partida, efectos y música.
3. `make port` headless para arranque/estabilidad (`frames`, `fb_nonzero`,
   `audio: rms/peak/...`).
