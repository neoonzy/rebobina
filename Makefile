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
ARM_OBJS = build/arm/registry.o build/arm/ui.o build/arm/manager.o build/arm/output.o build/arm/hal_sim.o build/arm/probe.o \
           build/arm/5dc_ml_input.o build/arm/5dc_ml_rom_stubs.o

.PHONY: all demo hal-demo test sim-ui web-sim check-modules audit-platform arm-objects offline-bootstrap plugin-demo test-plugins clean

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

build/test-hal: tests/test_hal_sim.c src/host/hal_sim.c include/rebobina/hal_sim.h include/rebobina/hal.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_hal_sim.c src/host/hal_sim.c

build/rebobina-hal-demo: src/host/hal_demo.c src/host/hal_sim.c include/rebobina/hal_sim.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ src/host/hal_demo.c src/host/hal_sim.c

hal-demo: build/rebobina-hal-demo
	./build/rebobina-hal-demo

build/test-output: tests/test_output.c src/core/output.c src/host/hal_sim.c include/rebobina/output.h include/rebobina/hal_sim.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_output.c src/core/output.c src/host/hal_sim.c

build/test-posix: tests/test_hal_posix.c src/host/hal_posix.c src/core/output.c include/rebobina/hal_posix.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_hal_posix.c src/host/hal_posix.c src/core/output.c

build/arm/output.o: src/core/output.c include/rebobina/output.h include/rebobina/hal.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_CFLAGS) -c -o $@ $<

build/arm/hal_sim.o: src/host/hal_sim.c include/rebobina/hal_sim.h include/rebobina/hal.h
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

build/arm/5dc_ml_input.o: camera/5DC.111/ml_compat/input.c camera/5DC.111/ml_compat/input.h include/rebobina/ui.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_CFLAGS) -c -o $@ $<

build/arm/5dc_ml_rom_stubs.o: camera/5DC.111/ml_compat/rom_stubs.S
	@mkdir -p build/arm
	$(ARM_CC) --target=arm-none-eabi -mcpu=arm946e-s -marm -x assembler-with-cpp -c -o $@ $<

# Only a linked ELF for offline inspection. It has no loader header or Canon
# firmware handoff: NEVER objcopy this into AUTOEXEC.BIN / .FIR / a CF.
ARM_BOOT_CFLAGS = $(ARM_CFLAGS) -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables
BOOT_DIR = camera/5DC.111/bootstrap
BOOT_OBJS = build/arm/5dc-stage0-entry.o build/arm/5dc-stage0.o build/arm/5dc-stage0-contract.o

build/arm/5dc-stage0-entry.o: $(BOOT_DIR)/stage0_entry.S
	@mkdir -p build/arm
	$(ARM_CC) --target=arm-none-eabi -mcpu=arm946e-s -marm -c -o $@ $<

build/arm/5dc-stage0.o: $(BOOT_DIR)/stage0.c $(BOOT_DIR)/stage0_contract.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_BOOT_CFLAGS) -I$(BOOT_DIR) -c -o $@ $<

build/arm/5dc-stage0-contract.o: $(BOOT_DIR)/stage0_contract.c $(BOOT_DIR)/stage0_contract.h
	@mkdir -p build/arm
	$(ARM_CC) $(ARM_BOOT_CFLAGS) -I$(BOOT_DIR) -c -o $@ $<

build/arm/5dc-stage0-offline.elf: $(BOOT_OBJS) build/arm/registry.o build/arm/ui.o $(BOOT_DIR)/stage0_offline.ld
	$(ARM_CC) --target=arm-none-eabi -mcpu=arm946e-s -marm -nostdlib -fuse-ld=lld \
	    -Wl,--no-undefined -Wl,--build-id=none -Wl,-T,$(BOOT_DIR)/stage0_offline.ld \
	    -o $@ $(BOOT_OBJS) build/arm/registry.o build/arm/ui.o
	python3 tools/audit_5dc_stage0.py $@

offline-bootstrap: build/arm/5dc-stage0-offline.elf
	@echo "OFFLINE ELF ONLY: NAO usar na camera ou converter para AUTOEXEC.BIN"

build/test-5dc-stage0-contract: tests/test_5dc_stage0_contract.c $(BOOT_DIR)/stage0_contract.c $(BOOT_DIR)/stage0_contract.h
	@mkdir -p build
	$(CC) $(CFLAGS) -I$(BOOT_DIR) -o $@ tests/test_5dc_stage0_contract.c $(BOOT_DIR)/stage0_contract.c

build/test-5dc-ml-input: tests/test_5dc_ml_input.c camera/5DC.111/ml_compat/input.c src/core/ui.c camera/5DC.111/ml_compat/input.h
	@mkdir -p build
	$(CC) $(CFLAGS) -Icamera/5DC.111/ml_compat -o $@ tests/test_5dc_ml_input.c camera/5DC.111/ml_compat/input.c src/core/ui.c
arm-objects: $(ARM_OBJS)
	@for f in $(ARM_OBJS); do \
	  readelf -h "$$f" | grep -Eq 'Machine:[[:space:]]+ARM' || exit 1; \
	  readelf -h "$$f" | grep -Eq 'Type:[[:space:]]+REL' || exit 1; \
	  readelf -h "$$f" | grep -Eq 'Class:[[:space:]]+ELF32' || exit 1; \
	done
	python3 tools/ml_5dc_audit.py --elf build/arm/5dc_ml_rom_stubs.o
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
	python3 tools/ml_5dc_audit.py

test: build/test-registry build/test-ui build/test-ui-stress build/test-manager build/test-hal build/test-output build/test-posix build/rebobina-sim-bridge build/test-5dc-ml-input build/test-5dc-stage0-contract test-plugins
	./build/test-registry
	./build/test-ui
	./build/test-ui-stress
	./build/test-manager
	./build/test-hal
	./build/test-output
	./build/test-posix
	./build/test-5dc-ml-input
	./build/test-5dc-stage0-contract
	python3 -m unittest discover -s tests -p 'test_*.py' -v

clean:
	rm -rf build
