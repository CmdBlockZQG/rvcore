BIN = build/testbench
ARGS ?= --log=build/rvcore-log.txt --batch

# override ARGS += --core=emu
override ARGS += --core=rtl-a

build:
	@xmake

clean:
	@xmake clean

run:
	$(BIN) $(ARGS) $(IMG)

default: build

.PHONY: default build clean run
