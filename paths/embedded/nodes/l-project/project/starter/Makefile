# sensord: every .c file in src/ is part of the daemon. Add files freely.
# `make test-m2 T=reconnect` runs only the m2 tests whose name contains "reconnect".
CC      ?= cc
CFLAGS  ?= -O2 -g
override CFLAGS += -std=gnu17 -Wall -Wextra -Werror -D_GNU_SOURCE

SRC     := $(wildcard src/*.c)
HDR     := $(wildcard src/*.h)
TESTSRC := $(wildcard tests/*.c)
TESTHDR := $(wildcard tests/*.h)
T       ?=

.PHONY: build test test-m1 test-m2 test-m3 test-m4 test-m5 clean

build: sensord

sensord: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $@ $(SRC)

tests/run: $(TESTSRC) $(TESTHDR)
	$(CC) $(CFLAGS) -o $@ $(TESTSRC)

test: sensord tests/run
	./tests/run $(T)

test-m1 test-m2 test-m3 test-m4 test-m5: test-%: sensord tests/run
	./tests/run $* $(T)

clean:
	rm -f sensord tests/run
