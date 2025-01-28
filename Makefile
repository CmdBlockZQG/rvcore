BIN = build/testbench
ARGS ?= --log=build

#CORE = emu
CORE = rtl-b

override ARGS += --core=$(CORE)

clean:
	xmake clean
	cd core-$(CORE) && make clean

cdb:
	xmake project -k compile_commands

$(CORE):
	cd core-$(CORE) && make
	xmake build core-$(CORE)

build: $(CORE)
	xmake build testbench

run:
	$(BIN) $(ARGS) $(IMG)

.DEFAULT_GOAL = build
.PHONY: clean cdb $(CORE) build run
