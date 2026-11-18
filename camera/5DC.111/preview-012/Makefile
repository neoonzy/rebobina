CLANG ?= clang
OBJCOPY ?= llvm-objcopy
PYTHON ?= python3
ARMFLAGS = --target=arm-none-eabi -mcpu=arm946e-s -marm -Os -std=c11 \
 -ffreestanding -fno-builtin -fno-common -fno-zero-initialized-in-bss \
 -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables \
 -Wall -Wextra -Werror -Iinclude
.PHONY: all test clean
all: dist/SE.BIN
build/entry.o: boot/entry.S
	@mkdir -p build
	$(CLANG) $(ARMFLAGS) -c -o $@ $<
build/hal_5dc.o: boot/hal_5dc.c include/rb_hal.h
	@mkdir -p build
	$(CLANG) $(ARMFLAGS) -c -o $@ $<
build/rb_core.o: core/rb_core.c include/rb_hal.h
	@mkdir -p build
	$(CLANG) $(ARMFLAGS) -c -o $@ $<
build/rebobina.elf: build/entry.o build/hal_5dc.o build/rb_core.o boot/payload.ld
	$(CLANG) $(ARMFLAGS) -fuse-ld=lld -nostdlib \
	    -Wl,--gc-sections -Wl,-Map,build/rebobina.map -Wl,-T,boot/payload.ld \
	    -o $@ build/entry.o build/hal_5dc.o build/rb_core.o
dist/SE.BIN: build/rebobina.elf tools/validate_probe.py
	@mkdir -p dist
	$(OBJCOPY) -O binary $< $@
	$(PYTHON) tools/validate_probe.py --bin $@ --elf build/rebobina.elf --manifest dist/manifest.json
test: dist/SE.BIN
	$(CLANG) -std=c11 -Wall -Wextra -Werror -Iinclude -o build/test_core tests/test_core.c core/rb_core.c
	./build/test_core
	$(PYTHON) -m unittest discover -s tests -p 'test_*.py' -v
clean:
	rm -rf build dist/SE.BIN dist/manifest.json
