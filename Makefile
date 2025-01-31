BIN = build/testbench
ARGS ?= --log=build

#CORE = emu
CORE = rtl-b
REF = emu

override ARGS += --core=$(CORE)
ifdef REF
override ARGS += --diff=$(REF)
endif

clean:
	xmake clean
	cd core-$(CORE) && make clean

cdb:
	xmake project -k compile_commands

$(REF):
	cd core-$(REF) && make
	xmake build core-$(REF)

$(CORE):
	cd core-$(CORE) && make
	xmake build core-$(CORE)

build: $(CORE) $(REF)
	xmake build testbench

run:
	$(BIN) $(ARGS) $(IMG)

.DEFAULT_GOAL = build
.PHONY: clean cdb $(CORE) build run
