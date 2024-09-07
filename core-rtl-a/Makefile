TOP_NAME = top

VERILATOR = verilator
VERILATOR_FLAGS = -cc --build --trace --timescale "1ns/1ns" --no-timing --autoflush \
									-O3 --x-assign fast --x-initial fast --noassert \
									-CFLAGS -O3 -CFLAGS -fPIC

OBJ_DIR = ./verilated_obj
VSRCS = $(shell find $(abspath ./vsrc) -name "*.sv")
INCDIR = "$(abspath ./vsrc)"

.DEFAULT_GOAL = default
default:
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		--top-module $(TOP_NAME) \
		$(VSRCS) $(addprefix -y , $(INCDIR)) \
		--Mdir $(OBJ_DIR)
