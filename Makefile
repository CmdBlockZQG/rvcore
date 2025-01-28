BIN = build/testbench
ARGS ?= --log=build --batch

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

default: build

.PHONY: default build clean run
