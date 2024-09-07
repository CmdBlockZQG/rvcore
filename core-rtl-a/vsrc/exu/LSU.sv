module LSU (
  input clock, reset,

  axi_if.out mem_r,
  axi_if.out mem_w,

  output mmu_valid,
  output [31:0] mmu_vaddr,
  input mmu_hit,
  input [31:0] mmu_paddr,

  output in_ready,
  input in_valid,
  input [3:0] ls,
  input [31:0] alu_val,
  input [31:0] val_c,

  input out_ready,
  output out_valid,
  output [31:0] out_rdata
);

  typedef enum logic [3:0] {
    ST_IDLE,
    ST_HOLD,

    ST_MMU,

    ST_LOAD_REQ,
    ST_LOAD_RESP,

    ST_STORE_REQ,
    ST_STORE_ADDR,
    ST_STORE_DATA,
    ST_STORE_RESP
  } state_t;

  state_t state, state_next;
  wire st_idle = state == ST_IDLE;
  wire st_hold = state == ST_HOLD;
  wire st_mmu  = state == ST_MMU;

  reg [31:0] addr, addr_next;
  reg [31:0] load_val, load_val_next;

  always @(posedge clock) begin
    if (reset) begin
      state <= ST_IDLE;
    end else begin
      state <= state_next;
      addr <= addr_next;
      load_val <= load_val_next;
    end
  end

  assign in_ready = st_idle | (st_hold & out_ready);

  always_comb begin
    state_next = state;
    addr_next = addr;
    load_val_next = load_val;

    if (in_valid & in_ready) begin
      addr_next = alu_val;
      state_next = ST_MMU;
    end

    case (state)
      ST_IDLE: begin
        if (in_valid) ; // input
      end
      ST_HOLD: begin
        if (out_ready) begin
          if (in_valid) begin
            ; // input
          end else begin
            state_next = ST_IDLE;
          end
        end
      end

      ST_MMU: begin
        if (mmu_hit) begin
          addr_next = mmu_paddr;
          if (ls[3]) begin
            state_next = ST_LOAD_REQ;
          end else begin
            state_next = ST_STORE_REQ;
          end
        end
      end

      ST_LOAD_REQ: begin
        if (mem_r.arready) begin
          state_next = ST_LOAD_RESP;
        end
      end
      ST_LOAD_RESP: begin
        if (mem_r.rvalid) begin
          load_val_next = mem_rdata;
          state_next = ST_HOLD;
        end
      end

      ST_STORE_REQ: begin
        if (mem_w.awready & mem_w.wready) begin
          state_next = ST_STORE_RESP;
        end else if (mem_w.awready) begin
          state_next = ST_STORE_DATA;
        end else if (mem_w.wready) begin
          state_next = ST_STORE_ADDR;
        end
      end
      ST_STORE_ADDR: begin
        if (mem_w.awready) begin
          state_next = ST_STORE_RESP;
        end
      end
      ST_STORE_DATA: begin
        if (mem_w.wready) begin
          state_next = ST_STORE_RESP;
        end
      end
      ST_STORE_RESP: begin
        if (mem_w.bvalid) begin
          state_next = ST_HOLD;
        end
      end

      default: ;
    endcase
  end

  assign mmu_valid = st_mmu;
  assign mmu_vaddr = addr;

  assign out_valid = st_hold;

  // -------------------- LOAD --------------------
  assign mem_r.arvalid = state == ST_LOAD_REQ;
  assign mem_r.araddr = addr;
  assign mem_r.arid = 4'b0;
  assign mem_r.arlen = 8'b0;
  assign mem_r.arsize = {1'b0, ls[1:0]};
  assign mem_r.arburst = 2'b0;
  assign mem_r.rready = state == ST_LOAD_RESP;

  wire [31:0] mem_rdata_raw = mem_r.rdata >> {addr[1:0], 3'b0};
  reg [31:0] mem_rdata;
  always_comb begin
    case (ls[1:0])
      2'b00: mem_rdata = {{24{mem_rdata_raw[7]  & ls[2]}}, mem_rdata_raw[7:0] };
      2'b01: mem_rdata = {{16{mem_rdata_raw[15] & ls[2]}}, mem_rdata_raw[15:0]};
      default: mem_rdata = mem_rdata_raw[31:0];
    endcase
  end

  assign out_rdata = load_val;

  // -------------------- STORE --------------------
  wire st_store_req = state == ST_STORE_REQ;
  wire st_store_addr = state == ST_STORE_ADDR;
  wire st_store_data = state == ST_STORE_DATA;

  assign mem_w.awvalid = st_store_req | st_store_addr;
  assign mem_w.awaddr = addr;
  assign mem_w.awid = 4'b0;
  assign mem_w.awlen = 8'b0;
  assign mem_w.awsize = {1'b0, ls[1:0]};
  assign mem_w.awburst = 2'b0;

  wire [31:0] mem_wdata = val_c << {addr[1:0], 3'b0};
  reg [3:0] mem_wstrb_raw;
  always_comb begin
    case (ls[1:0])
      2'b00: mem_wstrb_raw = 4'b0001;
      2'b01: mem_wstrb_raw = 4'b0011;
      2'b10: mem_wstrb_raw = 4'b1111;
      default: mem_wstrb_raw = 4'b0000;
    endcase
  end
  wire [3:0] mem_wstrb = mem_wstrb_raw << addr[1:0];

  assign mem_w.wvalid = st_store_req | st_store_data;
  assign mem_w.wdata = mem_wdata;
  assign mem_w.wstrb = mem_wstrb;
  assign mem_w.wlast = 1'b1;

  assign mem_w.bready = state == ST_STORE_RESP;

  // -------------------- 性能计数器 --------------------
`ifndef SYNTHESIS
  always @(posedge clock) if (~reset) begin
    case (state)
      ST_LOAD_REQ, ST_LOAD_RESP:
               perf_event(PERF_LSU_MEMR);
      ST_STORE_REQ, ST_STORE_ADDR, ST_STORE_DATA, ST_STORE_RESP:
               perf_event(PERF_LSU_MEMW);
      default: ;
    endcase
    if (mem_r.rready & mem_r.rvalid) begin
      event_mem_read(mem_r.araddr, {29'b0, mem_r.arsize}, mem_rdata);
    end
    if (mem_w.awready & mem_w.awvalid) begin
      event_mem_write(mem_w.awaddr, {29'b0, mem_w.awsize}, val_c);
    end
  end
`endif

endmodule
