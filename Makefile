# NOP
all:

# initial setup of build folder
setup:
	./scripts/setup.bash

# check that the library is only manipulating memory through the macro
mcheck:
	./scripts/check_mems.bash

# test code coverage
coverage: test
	./scripts/coverage.bash

# download jetbrains mono
font:
	./scripts/get_jbm.bash

build:
	meson compile -C build

# --no-fork is required, X11 errors, unsure exactly why
# tests are almost instant anyway
test: build
	./scripts/clean_coverage.bash
	cd build && ./run_tests --no-fork

.PHONY: setup mcheck coverage font test build
