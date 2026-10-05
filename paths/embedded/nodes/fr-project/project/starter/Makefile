# The plant controller on the real FreeRTOS kernel, POSIX port.
#
#   make build      fetch the kernel (first time only) and compile everything
#   make run        build and run the default scenario
#   make test       build and run every milestone's tests
#   make test-m1    ... just milestone 1 (also test-m2, test-m3, test-m4)
#   make clean      remove build/ (third_party/ stays)

FREERTOS_TAG := V11.3.1
KERNEL       := third_party/FreeRTOS-Kernel
PORT         := $(KERNEL)/portable/ThirdParty/GCC/Posix

CC      := gcc
WARN    := -Wall -Wextra -Werror
CFLAGS  := -std=gnu17 -O1 -g -pthread -MMD -MP
INCS    := -Isrc -I$(KERNEL)/include -I$(PORT) -I$(PORT)/utils
LDFLAGS := -pthread

KERNEL_SRCS := $(KERNEL)/tasks.c $(KERNEL)/queue.c $(KERNEL)/list.c $(KERNEL)/timers.c \
               $(KERNEL)/portable/MemMang/heap_4.c $(PORT)/port.c $(PORT)/utils/wait_for_event.c
KERNEL_OBJS := $(patsubst $(KERNEL)/%.c,build/kernel/%.o,$(KERNEL_SRCS))

APP_OBJS  := $(patsubst src/%.c,build/app/%.o,$(wildcard src/*.c))
TEST_OBJS := $(patsubst tests/%.c,build/tests/%.o,$(wildcard tests/*.c))

.PHONY: build run test test-m1 test-m2 test-m3 test-m4 clean

build: build/plant build/run-tests

run: build
	./build/plant

test: build
	@./build/run-tests

test-m%: build
	@./build/run-tests m$*

$(KERNEL)/tasks.c:
	git -c advice.detachedHead=false clone --quiet --depth 1 --branch $(FREERTOS_TAG) \
		https://github.com/FreeRTOS/FreeRTOS-Kernel.git $(KERNEL)

# The kernel is third-party code: compiled without -Werror.
build/kernel/%.o: $(KERNEL)/%.c src/FreeRTOSConfig.h | $(KERNEL)/tasks.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

build/app/%.o: src/%.c | $(KERNEL)/tasks.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARN) $(INCS) -c $< -o $@

build/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARN) -c $< -o $@

build/plant: $(APP_OBJS) $(KERNEL_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

build/run-tests: $(TEST_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf build

-include $(APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d)
