# Kult reconstruction. The shared candidate/oracle/test rules live in the kit.
KIT  := tools/recreate_kit
GAME := kult
# The toolchain here is gcc (Alpine); the kit defaults to clang.
CC   := gcc
include $(KIT)/kit.mk

# Regenera include/addrs.h desde work/names.txt (unica fuente de los globales).
.PHONY: addrs
addrs:
	$(PY) tools/gen_addrs.py --write

# Regenera docs/mapa/ (fichas por modulo + mapa) desde src/, names.txt y test/.
.PHONY: mapa
mapa:
	$(PY) tools/gen_mapa.py

# Mide la deuda de legibilidad por modulo (docs/LEGIBILIDAD.md).
.PHONY: regscan
regscan:
	$(PY) tools/regscan.py

# Core con la API publica restringida por version-script (build on-target).
# El build diferencial (build/libkult.so) NO usa esto: necesita todos los simbolos.
CORE     := build/libkultcore.so
CORE_SRC := $(wildcard src/*.c) $(wildcard $(KIT)/src/*.c)
$(CORE): $(CORE_SRC) $(wildcard include/*.h) kult_core.map
	@mkdir -p build
	$(CC) $(CFLAGS) -shared -Wl,--version-script=kult_core.map $(CORE_SRC) -o $(CORE)
.PHONY: core
core: $(CORE)
test: $(CORE)

# ---------------------------------------------------------------------------
# Port PC (M1): core + boot/ + platform/ + app/, HAL headless (sin SDL todavia).
# Este build NO usa el modelo del kit: `-Iplatform/include` sombrea os.h/hw.h/
# psg.h/sched.h con la HAL de PC. El diferencial (`make test`) sigue intacto.
PORT_SRC    := $(wildcard src/*.c) $(wildcard platform/*.c) $(wildcard boot/*.c) $(wildcard app/*.c)
PORT_INC    := -Iplatform -Iplatform/include -Iboot -Iinclude -I$(KIT)/include
PORT_CFLAGS := -std=c11 -O2 -Wall -Wextra $(PORT_INC)
PORT_BIN    := build/kult

.PHONY: port
port: $(PORT_BIN)
$(PORT_BIN): $(PORT_SRC) $(wildcard platform/include/*.h) $(wildcard platform/*.h) \
             $(wildcard include/*.h) $(wildcard boot/*.h) $(wildcard app/*.h)
	@mkdir -p build
	$(CC) $(PORT_CFLAGS) $(PORT_SRC) -o $(PORT_BIN)

# Pruebas funcionales de la HAL (sin oráculo): ficheros, vídeo, entrada, reloj.
PLATFORM_SRC  := $(wildcard platform/*.c)
PLATFORM_TEST := build/test_platform
.PHONY: test-port
test-port: $(PLATFORM_TEST)
	$(PLATFORM_TEST)
$(PLATFORM_TEST): platform/test/test_platform.c $(PLATFORM_SRC) \
                  $(wildcard platform/include/*.h) $(wildcard platform/*.h) $(wildcard include/*.h)
	@mkdir -p build
	$(CC) $(PORT_CFLAGS) platform/test/test_platform.c $(PLATFORM_SRC) -o $@
