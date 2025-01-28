BIN = build/testbench
ARGS ?= --log=build

# CORE = emu
CORE = rtl-b

override ARGS += --core=$(CORE)

build:
	xmake

clean:
	xmake clean

rtl-b:
	cd core-rtl-b && make

run: $(CORE)
	$(BIN) $(ARGS) $(IMG)

cdb:
	xmake project -k compile_commands

default: build

.PHONY: default build clean rtl-b run cdb
