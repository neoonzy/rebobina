CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic -Iinclude
ARM_CC ?= clang
ARM_CFLAGS = --target=arm-none-eabi -mcpu=arm946e-s -marm -std=c11 -O2 \
             -ffreestanding -fno-builtin -fno-common -nostdlib \
             -Wall -Wextra -Werror -Iinclude

CORE = src/core/registry.c
DEMO = src/host/main.c modules/hello/hello.c $(CORE)
TEST = tests/test_registry.c $(CORE)
UI_TEST = tests/test_ui.c src/core/ui.c
ARM_OBJS = build/arm/registry.o build/arm/ui.o build/arm/probe.o

.PHONY: all demo test sim-ui check-modules audit-platform arm-objects clean

all: build/rebobina-host

build/rebobina-host: $(DEMO) include/rebobina/module.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(DEMO)

build/test-registry: $(TEST) include/rebobina/module.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(TEST)

build/test-ui: $(UI_TEST) include/rebobina/ui.h include/rebobina/display.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(UI_TEST)

build/rebobina-ui-host: src/host/menu_sim.c src/core/ui.c include/rebobina/ui.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ src/host/menu_sim.c src/core/ui.c

build/arm/registry.o: src/core/registry.c include/rebobina/module.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_CFLAGS) -c -o $@ $<

build/arm/ui.o: src/core/ui.c include/rebobina/ui.h include/rebobina/display.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_CFLAGS) -c -o $@ $<

build/arm/probe.o: camera/5DC.111/probe.c include/rebobina/ui.h include/rebobina/module.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_CFLAGS) -c -o $@ $<

arm-objects: $(ARM_OBJS)
	@for f in $(ARM_OBJS); do \
	  readelf -h "$$f" | grep -Eq 'Machine:[[:space:]]+ARM' || exit 1; \
	  readelf -h "$$f" | grep -Eq 'Type:[[:space:]]+REL' || exit 1; \
	  readelf -h "$$f" | grep -Eq 'Class:[[:space:]]+ELF32' || exit 1; \
	done
	@echo "OK: objetos ARM ELF32 relocaveis (NAO sao firmware instalavel)"

demo: build/rebobina-host
	./build/rebobina-host

sim-ui: build/rebobina-ui-host
	./build/rebobina-ui-host tddsbt

check-modules:
	python3 tools/modules.py check

audit-platform:
	python3 tools/platform_audit.py check

test: build/test-registry build/test-ui
	./build/test-registry
	./build/test-ui
	python3 -m unittest discover -s tests -p 'test_*.py' -v

clean:
	rm -rf build
