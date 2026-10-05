# uart-shell: the firmware core of a debug shell, built and tested on your PC.
#
#   make            build the shell and the tests
#   make run        type at the shell (Ctrl-C or Ctrl-D quits)
#   make test       run every milestone's tests
#   make test-m1    run one milestone's tests (m1 to m4)

CC       = gcc
CFLAGS   = -std=gnu17 -Wall -Wextra -Werror -g -O1 -Isrc
# The tests also run with AddressSanitizer and UBSan, so an out-of-bounds write
# fails loudly. If your gcc lacks them: make SANITIZE= test
SANITIZE ?= -fsanitize=address,undefined -fno-sanitize-recover=all

CORE    = src/ringbuf.c src/uart.c src/line.c src/shell.c src/commands.c src/out.c src/hal_host.c
TESTS   = tests/check.c tests/test_m1.c tests/test_m2.c tests/test_m3.c tests/test_m4.c
HEADERS = $(wildcard src/*.h) tests/check.h

.PHONY: build run test test-m1 test-m2 test-m3 test-m4 clean

build: build/uart-shell build/tests

build/uart-shell: $(CORE) src/main.c $(HEADERS)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(CORE) src/main.c

build/tests: $(CORE) $(TESTS) $(HEADERS)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SANITIZE) -Itests -o $@ $(CORE) $(TESTS)

run: build/uart-shell
	./build/uart-shell

test: build/tests
	./build/tests

test-m1 test-m2 test-m3 test-m4: build/tests
	./build/tests $(@:test-%=%)

clean:
	rm -rf build
