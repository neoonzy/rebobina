CLANG ?= clang
OBJCOPY ?= llvm-objcopy
PYTHON ?= python3
FLAGS = --target=arm-none-eabi -mcpu=arm946e-s -marm -Os -std=c11 \
 -ffreestanding -fno-builtin -fno-common -fno-stack-protector \
 -fno-unwind-tables -fno-asynchronous-unwind-tables -Wall -Wextra -Werror
.PHONY: all test clean
all: dist/SE.BIN
build/entry.o: boot/entry.S
	@mkdir -p build
	$(CLANG) $(FLAGS) -c -o $@ $<
build/probe.o: boot/probe.c
	@mkdir -p build
	$(CLANG) $(FLAGS) -c -o $@ $<
build/diagnostic.elf: build/entry.o build/probe.o boot/payload.ld
	$(CLANG) $(FLAGS) -fuse-ld=lld -nostdlib -Wl,--gc-sections -Wl,-Map,build/diagnostic.map -Wl,-T,boot/payload.ld -o $@ build/entry.o build/probe.o
dist/SE.BIN: build/diagnostic.elf tools/validate_probe.py
	@mkdir -p dist
	$(OBJCOPY) -O binary $< $@
	$(PYTHON) tools/validate_probe.py --bin $@ --elf build/diagnostic.elf --manifest dist/manifest.json
test: dist/SE.BIN
	$(CLANG) -std=c11 -O1 -Wall -Wextra -Werror -DRB_PROBE_HOST_TEST -o build/test_probe tests/test_probe.c boot/probe.c
	./build/test_probe
	$(PYTHON) -m unittest discover -s tests -p 'test_*.py' -v
clean:
	rm -rf build dist/SE.BIN dist/manifest.json
