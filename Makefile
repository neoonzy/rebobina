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
ARM_OBJS = build/arm/registry.o build/arm/ui.o build/arm/manager.o build/arm/probe.o

.PHONY: all demo test sim-ui web-sim check-modules audit-platform arm-objects plugin-demo test-plugins clean

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

build/rebobina-plugin-host: src/host/plugin_loader.c $(CORE) include/rebobina/module.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ src/host/plugin_loader.c $(CORE) -ldl

build/plugins/hello.so: modules/hello/plugin.c include/rebobina/module.h
	@mkdir -p build/plugins
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

build/plugins/invalid-abi.so: tests/fixtures/invalid_abi_plugin.c include/rebobina/module.h
	@mkdir -p build/plugins
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

build/plugins/no-export.so: tests/fixtures/no_export_plugin.c
	@mkdir -p build/plugins
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

test-plugins: build/rebobina-plugin-host build/plugins/hello.so build/plugins/invalid-abi.so build/plugins/no-export.so
	./build/rebobina-plugin-host ./build/plugins/hello.so
	@set +e; ./build/rebobina-plugin-host ./build/plugins/invalid-abi.so; rc=$$?; \
	  test "$$rc" -eq 5 || { echo "falha: esperado retorno 5, obtido $$rc"; exit 1; }
	@set +e; ./build/rebobina-plugin-host ./build/plugins/no-export.so; rc=$$?; \
	  test "$$rc" -eq 4 || { echo "falha: esperado retorno 4, obtido $$rc"; exit 1; }
	@echo "OK: plugin host independente e falhas rejeitadas"

plugin-demo: build/rebobina-plugin-host build/plugins/hello.so
	./build/rebobina-plugin-host ./build/plugins/hello.so

build/test-ui-stress: tests/test_ui_stress.c src/core/ui.c include/rebobina/ui.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_ui_stress.c src/core/ui.c

build/test-manager: tests/test_manager.c src/core/manager.c $(CORE) include/rebobina/manager.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_manager.c src/core/manager.c $(CORE)

build/rebobina-sim-bridge: src/host/simulator_bridge.c src/core/ui.c src/core/manager.c $(CORE) include/rebobina/ui.h include/rebobina/manager.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ src/host/simulator_bridge.c src/core/ui.c src/core/manager.c $(CORE)

build/arm/manager.o: src/core/manager.c include/rebobina/manager.h include/rebobina/module.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_CFLAGS) -c -o $@ $<

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

web-sim: build/rebobina-sim-bridge
	python3 tools/simulator_server.py --port 8765

check-modules:
	python3 tools/modules.py check

audit-platform:
	python3 tools/platform_audit.py check

test: build/test-registry build/test-ui build/test-ui-stress build/test-manager build/rebobina-sim-bridge test-plugins
	./build/test-registry
	./build/test-ui
	./build/test-ui-stress
	./build/test-manager
	python3 -m unittest discover -s tests -p 'test_*.py' -v

clean:
	rm -rf build
