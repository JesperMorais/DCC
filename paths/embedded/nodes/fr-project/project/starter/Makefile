# The plant controller on the real FreeRTOS kernel, POSIX port.
#
#   make build      fetch the kernel (first time only) and compile everything
#   make run        build and run the default scenario
#   make test       build and run every milestone's tests
#   make test-m1    ... just milestone 1 (also test-m2, test-m3, test-m4)
#   make test-m1 T=drift   ... only the m1 checks whose name contains "drift"
#   make clean      remove build/ (third_party/ stays)

FREERTOS_TAG := V11.3.1
KERNEL       := third_party/FreeRTOS-Kernel
PORT         := $(KERNEL)/portable/ThirdParty/GCC/Posix

CC      := gcc
WARN    := -Wall -Wextra -Werror
CFLAGS  := -std=gnu17 -O1 -g -pthread -MMD -MP
INCS    := -Isrc -I$(KERNEL)/include -I$(PORT) -I$(PORT)/utils
LDFLAGS := -pthread

# hw.c checks that the e-stop ISR only calls FromISR APIs: each of these
# task-only kernel functions is linked through a __wrap_ function in hw.c.
comma     := ,
TASK_ONLY := xQueueGenericSend xQueueReceive xQueueSemaphoreTake xTaskGenericNotify xTaskGenericNotifyWait \
             ulTaskGenericNotifyTake vTaskDelay xTaskGetTickCount \
             xTimerGenericCommandFromTask vPortEnterCritical
WRAP      := $(foreach f,$(TASK_ONLY),-Wl$(comma)--wrap=$(f))

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

# A static pattern rule: a plain `test-m%:` pattern is skipped for .PHONY targets, so it would do nothing.
test-m1 test-m2 test-m3 test-m4: test-m%: build
	@./build/run-tests m$* $(if $(T),"$(T)")

# Fetches just the tagged commit. (A shallow `git clone --branch` of this tag
# prints a harmless but alarming "refs/tags/... is not a commit!" warning.)
$(KERNEL)/tasks.c:
	@echo "fetching FreeRTOS-Kernel $(FREERTOS_TAG) into $(KERNEL) (first build only)"
	@mkdir -p $(KERNEL)
	@git -C $(KERNEL) init --quiet
	@git -C $(KERNEL) fetch --quiet --depth 1 https://github.com/FreeRTOS/FreeRTOS-Kernel.git \
		refs/tags/$(FREERTOS_TAG)
	@git -C $(KERNEL) -c advice.detachedHead=false checkout --quiet FETCH_HEAD

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
	$(CC) $(LDFLAGS) $(WRAP) $^ -o $@

build/run-tests: $(TEST_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf build

-include $(APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d)
