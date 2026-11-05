CLANG ?= clang
OBJCOPY ?= llvm-objcopy
PYTHON ?= python3
ARM_FLAGS = --target=arm-none-eabi -mcpu=arm946e-s -marm -Os \
    -ffreestanding -fno-builtin -fno-stack-protector \
    -fno-unwind-tables -fno-asynchronous-unwind-tables \
    -Wall -Wextra -Werror -Iboot

.PHONY: all test clean
all: dist/SE.BIN

build/entry.o: boot/entry.S
	@mkdir -p build
	$(CLANG) $(ARM_FLAGS) -c -o $@ $<

build/probe.o: boot/probe.c
	@mkdir -p build
	$(CLANG) $(ARM_FLAGS) -std=c11 -c -o $@ $<

build/rebobina-probe.elf: build/entry.o build/probe.o boot/payload.ld
	$(CLANG) $(ARM_FLAGS) -fuse-ld=lld -nostdlib -Wl,--gc-sections \
	    -Wl,-Map,build/rebobina-probe.map -Wl,-T,boot/payload.ld \
	    -o $@ build/entry.o build/probe.o

dist/SE.BIN: build/rebobina-probe.elf tools/validate_probe.py
	@mkdir -p dist
	$(OBJCOPY) -O binary $< $@
	$(PYTHON) tools/validate_probe.py --bin $@ --elf build/rebobina-probe.elf --manifest dist/probe-manifest.json

test: dist/SE.BIN
	$(PYTHON) -m unittest discover -s tests -p 'test_*.py' -v

clean:
	rm -rf build dist/SE.BIN dist/probe-manifest.json dist/card-staging dist/staging-manifest.json
