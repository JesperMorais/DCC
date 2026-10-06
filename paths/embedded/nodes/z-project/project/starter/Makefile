# Thin wrappers around west. They need the Zephyr workspace from README.md:
# the venv with west activated and ZEPHYR_BASE pointing at its zephyr/ folder.
# Verified with Zephyr v4.2.0.
#
#   make build            build the app into build/app
#   make run              build and run it (Ctrl-C stops it)
#   make test             build and run every milestone's tests
#   make test-m1          ... only milestone 1 (m2, m3, m4, m5 likewise)
#   make test-m2 TEST=m2_driver::test_ramp_steps_and_wraps
#                         ... only that test (suite::test, * works as a wildcard)
#   make twister          run the test suite through Twister instead
#   make clean            delete every build directory
#
# BOARD defaults to native_sim (a 32-bit Linux executable, needs gcc-multilib).
# Use "make BOARD=native_sim/native/64 ..." for a 64-bit one, and "make clean"
# when you switch.

WEST ?= west
BOARD ?= native_sim
# native_sim builds with the host gcc, no Zephyr SDK needed.
ZEPHYR_TOOLCHAIN_VARIANT ?= host
export ZEPHYR_TOOLCHAIN_VARIANT
# Optional: run only these tests, e.g. TEST=m2_driver::* (see "make test-m2").
TEST ?=
RUN_ARGS := $(if $(TEST),-test=$(TEST))
MILESTONES := m1 m2 m3 m4 m5
TEST_TARGETS := $(addprefix test-,$(MILESTONES))

.PHONY: build run test $(TEST_TARGETS) twister clean check-env

check-env:
	@command -v $(WEST) >/dev/null 2>&1 || { echo "west not found: activate the Zephyr venv first (see README.md)"; exit 1; }
	@test -n "$$ZEPHYR_BASE" || { echo "ZEPHYR_BASE is not set: export ZEPHYR_BASE=~/zephyrproject/zephyr (see README.md)"; exit 1; }

build: check-env
	$(WEST) build -b $(BOARD) -d build/app .

run: build
	$(WEST) build -d build/app -t run

test: check-env
	$(WEST) build -b $(BOARD) -d build/tests-all tests -- -DMILESTONE=all
	build/tests-all/zephyr/zephyr.exe $(RUN_ARGS)

$(TEST_TARGETS): test-%: check-env
	$(WEST) build -b $(BOARD) -d build/tests-$* tests -- -DMILESTONE=$*
	build/tests-$*/zephyr/zephyr.exe $(RUN_ARGS)

twister: check-env
	$(WEST) twister -T tests -p $(BOARD) -O build/twister

clean:
	rm -rf build twister-out*
