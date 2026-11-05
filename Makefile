CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic -Iinclude

CORE = src/core/registry.c
DEMO = src/host/main.c modules/hello/hello.c $(CORE)
TEST = tests/test_registry.c $(CORE)

.PHONY: all demo test check-modules clean

all: build/rebobina-host

build/rebobina-host: $(DEMO) include/rebobina/module.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(DEMO)

build/test-registry: $(TEST) include/rebobina/module.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(TEST)

demo: build/rebobina-host
	./build/rebobina-host

check-modules:
	python3 tools/modules.py check

test: build/test-registry
	./build/test-registry
	python3 -m unittest discover -s tests -p 'test_*.py' -v

clean:
	rm -rf build
