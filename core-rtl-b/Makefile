TOP_NAME = Top

VERILATOR = verilator
VERILATOR_FLAGS = -cc --build --trace --timescale "1ns/1ns" --no-timing --autoflush \
									-O3 --x-assign fast --x-initial fast --noassert \
									-CFLAGS -O3 -CFLAGS -fPIC

OBJ_DIR = ./verilated_obj
VSRC = ./Top.sv

.DEFAULT_GOAL = default
default:
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		--top-module $(TOP_NAME) \
		$(VSRC) --Mdir $(OBJ_DIR)
