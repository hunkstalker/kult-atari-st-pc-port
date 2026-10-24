# Kult reconstruction. The shared candidate/oracle/test rules live in the kit.
KIT  := lib/kult-core/tools/recreate_kit
GAME := kult
# The toolchain here is gcc (Alpine); the kit defaults to clang.
CC   := gcc
include $(KIT)/kit.mk

# El core vive en el subtree; el kit sigue usando SRC/CFLAGS.
SRC := $(wildcard lib/kult-core/src/*.c) $(wildcard $(KIT)/src/*.c)
CFLAGS += -Ilib/kult-core/include

# Regenera lib/kult-core/include/addrs.h desde lib/kult-core/symbols/names.txt.
.PHONY: addrs
addrs:
	$(PY) lib/kult-core/tools/gen_addrs.py --write

# Regenera lib/kult-core/docs/mapa/ (fichas por modulo + mapa) desde su src/, names.txt y test/.
.PHONY: mapa
mapa:
	$(PY) lib/kult-core/tools/gen_mapa.py

# Mide la deuda de legibilidad por modulo (docs/LEGIBILIDAD.md).
.PHONY: regscan
regscan:
	$(PY) lib/kult-core/tools/regscan.py

# Core con la API publica restringida por version-script (build on-target).
# El build diferencial (build/libkult.so) NO usa esto: necesita todos los simbolos.
CORE     := build/libkultcore.so
CORE_SRC := $(wildcard lib/kult-core/src/*.c) $(wildcard $(KIT)/src/*.c)
$(CORE): $(CORE_SRC) $(wildcard lib/kult-core/include/*.h) lib/kult-core/kult_core.map
	@mkdir -p build
	$(CC) $(CFLAGS) -shared -Wl,--version-script=lib/kult-core/kult_core.map $(CORE_SRC) -o $(CORE)
.PHONY: core
core: $(CORE)
test: $(CORE)

# ---------------------------------------------------------------------------
# Port PC: core + boot/ + platform/ + app/. Este build NO usa el modelo del kit:
# `-Iplatform/include` sombrea os.h/hw.h/psg.h/sched.h con la HAL de PC. El
# diferencial (`make test`) sigue intacto.
#   make port       -> build/kult      headless (PPM), sin SDL
#   make port-sdl   -> build/kult-sdl  con ventana SDL3 (-DHAVE_SDL3)
#   make test-port  -> pruebas de la HAL (sin oráculo)
PLATFORM_SRC := $(filter-out platform/video_sdl.c platform/audio_sdl.c platform/dispatch_trace.c,$(wildcard platform/*.c))
PORT_CORE_SRC := $(wildcard lib/kult-core/src/*.c)
PORT_SRC     := $(PORT_CORE_SRC) $(PLATFORM_SRC) $(wildcard boot/*.c) $(wildcard app/*.c)
PORT_INC     := -Iplatform -Iplatform/include -Iboot -Ilib/kult-core/include -Iassets -I$(KIT)/include
PORT_CFLAGS  := -std=c11 -O2 -Wall -Wextra -DKULT_VERSION=\"dev\" $(PORT_INC)
PORT_HDRS    := $(wildcard platform/include/*.h) $(wildcard platform/*.h) \
                $(wildcard lib/kult-core/include/*.h) $(wildcard boot/*.h) $(wildcard app/*.h) \
                platform/cpuwork_costs.inc
PORT_BIN     := build/kult

# T-073b: el core se compila con -finstrument-functions para cobrar el coste
# medido por función (platform/cpuwork.c). Solo en el build del port; el
# diferencial (make test) y test-port no se instrumentan.
PORT_OBJDIR  := build/port-obj
PORT_CORE_O  := $(patsubst lib/kult-core/src/%.c,$(PORT_OBJDIR)/lib/kult-core/src/%.o,$(PORT_CORE_SRC))
PORT_OBJS    := $(patsubst %.c,$(PORT_OBJDIR)/%.o,$(filter-out lib/kult-core/src/%,$(PORT_SRC)))

.PHONY: port
port: $(PORT_BIN)
$(PORT_OBJDIR)/lib/kult-core/src/%.o: lib/kult-core/src/%.c $(PORT_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) -finstrument-functions -c $< -o $@
$(PORT_OBJDIR)/%.o: %.c $(PORT_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) -c $< -o $@
$(PORT_BIN): $(PORT_CORE_O) $(PORT_OBJS)
	@mkdir -p build
	$(CC) $^ -o $@

# Ventana SDL3 (M3). Requiere `sdl3` (pkg-config).
SDL3_CFLAGS := $(shell pkg-config --cflags sdl3)
SDL3_LIBS   := $(shell pkg-config --libs sdl3)
PORT_SDL_BIN := build/kult-sdl
SDL_OBJDIR  := build/port-sdl-obj
SDL_CORE_O  := $(patsubst lib/kult-core/src/%.c,$(SDL_OBJDIR)/lib/kult-core/src/%.o,$(PORT_CORE_SRC))
SDL_OBJS    := $(patsubst %.c,$(SDL_OBJDIR)/%.o,$(filter-out lib/kult-core/src/%,$(PORT_SRC)))
SDL_EXTRA   := platform/video_sdl.c platform/audio_sdl.c
SDL_EXTRA_O := $(patsubst %.c,$(SDL_OBJDIR)/%.o,$(SDL_EXTRA))
.PHONY: port-sdl
port-sdl: $(PORT_SDL_BIN)
$(SDL_OBJDIR)/lib/kult-core/src/%.o: lib/kult-core/src/%.c $(PORT_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) -DHAVE_SDL3 $(SDL3_CFLAGS) -finstrument-functions -c $< -o $@
$(SDL_OBJDIR)/%.o: %.c $(PORT_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) -DHAVE_SDL3 $(SDL3_CFLAGS) -c $< -o $@
$(PORT_SDL_BIN): $(SDL_CORE_O) $(SDL_OBJS) $(SDL_EXTRA_O)
	@mkdir -p build
	$(CC) $^ $(SDL3_LIBS) -o $@

# Pruebas funcionales de la HAL (sin oráculo): ficheros, vídeo, entrada, reloj.
# Sin instrumentación ni platform/cpuwork.c (la tabla referencia todo el core).
PLATFORM_TEST_SRC := $(filter-out platform/cpuwork.c platform/dispatch_trace.c,$(PLATFORM_SRC))
PLATFORM_TEST := build/test_platform
.PHONY: test-port
test-port: $(PLATFORM_TEST)
	$(PLATFORM_TEST)
$(PLATFORM_TEST): platform/test/test_platform.c boot/vbl.c boot/prg.c boot/music_player.c boot/language.c lib/kult-core/src/music.c lib/kult-core/src/input.c $(PLATFORM_TEST_SRC) \
                  $(wildcard platform/include/*.h) $(wildcard platform/*.h) $(wildcard lib/kult-core/include/*.h)
	@mkdir -p build
	$(CC) $(PORT_CFLAGS) -DKULT_NO_CPUWORK platform/test/test_platform.c boot/vbl.c boot/prg.c boot/music_player.c boot/language.c lib/kult-core/src/music.c lib/kult-core/src/input.c $(PLATFORM_TEST_SRC) -o $@

# ---------------------------------------------------------------------------
# M7/T-070: build CMake (nativo Linux y cross Windows MinGW). Son envoltorios
# de conveniencia; el Makefile de arriba sigue siendo el del core/diferencial.
CMAKE      ?= cmake
BUILD_NAT  := build-cmake
BUILD_WIN  := build-win
BUILD_WIN_DIAG := build-win-diag
SDL3_MINGW := $(CURDIR)/third_party/SDL3-3.4.10/x86_64-w64-mingw32

.PHONY: cmake-native
cmake-native:
	$(CMAKE) -S . -B $(BUILD_NAT) -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(BUILD_NAT) -j

.PHONY: cmake-win
cmake-win:
	$(CMAKE) -S . -B $(BUILD_WIN) -DCMAKE_BUILD_TYPE=Release \
	  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
	  -DCMAKE_PREFIX_PATH=$(SDL3_MINGW)
	$(CMAKE) --build $(BUILD_WIN) -j

.PHONY: cmake-win-diag
cmake-win-diag:
	$(CMAKE) -S . -B $(BUILD_WIN_DIAG) -DCMAKE_BUILD_TYPE=Release \
	  -DKULT_DIAG_TRACE=ON \
	  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
	  -DCMAKE_PREFIX_PATH=$(SDL3_MINGW)
	$(CMAKE) --build $(BUILD_WIN_DIAG) -j

.PHONY: package-native
package-native: cmake-native
	cpack --config $(BUILD_NAT)/CPackConfig.cmake -G TGZ -B $(BUILD_NAT)

.PHONY: package-win
package-win: cmake-win
	cpack --config $(BUILD_WIN)/CPackConfig.cmake -G ZIP -B $(BUILD_WIN)
