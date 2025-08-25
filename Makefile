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

test: build
	./scripts/clean_coverage.bash
	meson test -v -C build

.PHONY: setup mcheck coverage font test build
