module ICache (
  input clock, reset,

  input flush_icache,

  input [31:0] addr,
  output hit,
  output [31:0] inst,

  output mmu_valid,
  output [31:0] mmu_vaddr,
  input mmu_hit,
  input [31:0] mmu_paddr,

  axi_if.out mem_r
);
  parameter OFFSET_W = 4; // 块内地址宽度，块大小=2^x字节
  parameter INDEX_W  = 2; // 组地址宽度，组数=2^x
  parameter TAG_W    = 32 - OFFSET_W - INDEX_W; // 标记字宽度

  parameter SET_N = 1 << INDEX_W; // 组数
  parameter BLOCK_W = (1 << OFFSET_W) << 3; // 块位宽
  parameter BLOCK_SZ = (1 << OFFSET_W) >> 2; // 一块中几个32位的部分

  // TEMP: 直接映射实现
  reg line_valid [SET_N];
  reg [TAG_W-1:0] line_tag [SET_N];
  reg [31:0] line_data [SET_N][BLOCK_SZ];

  wire [TAG_W-1:0] tag = addr[31:OFFSET_W+INDEX_W];
  wire [INDEX_W-1:0] index = addr[OFFSET_W+INDEX_W-1:OFFSET_W];
  wire [(OFFSET_W-2)-1:0] off = addr[OFFSET_W-1:2];
  wire [(OFFSET_W-2)-1:0] off_next = off + 1;

  assign hit = line_valid[index] & (line_tag[index] == tag);
  assign inst = line_data[index][off];

  // -------------------- 访存状态机 --------------------

  typedef enum logic [1:0] {
    ST_IDLE,
    ST_MMU,
    ST_REQ,
    ST_RESP
  } state_t;
  wire st_idle = state == ST_IDLE;
  wire st_mmu  = state == ST_MMU;
  wire st_req  = state == ST_REQ;
  wire st_resp = state == ST_RESP;

  state_t state, state_next;
  reg [31:0] paddr, paddr_next;

  always @(posedge clock) begin
    if (reset) begin
      state <= ST_IDLE;
    end else begin
      state <= state_next;
      paddr <= paddr_next;
    end
  end

  always_comb begin
    state_next = state;
    paddr_next = paddr;
    case (state)
      ST_IDLE: begin
        if (~hit) begin
          state_next = ST_MMU;
        end
      end
      ST_MMU: begin
        if (mmu_hit) begin
          state_next = ST_REQ;
          paddr_next = mmu_paddr;
        end
      end
      ST_REQ: begin
        if (mem_r.arready) begin
          state_next = ST_RESP;
        end
      end
      ST_RESP: begin
        if (mem_r.rready & mem_r.rvalid & mem_r.rlast) begin
          state_next = ST_IDLE;
        end
      end
      default: ;
    endcase
  end

  assign mmu_valid = st_mmu;
  assign mmu_vaddr = {tag, index, off_next, 2'b00};

  assign mem_r.arvalid = st_req;
  assign mem_r.araddr = paddr;
  assign mem_r.arid = 0;
  assign mem_r.arlen = BLOCK_SZ - 1;
  assign mem_r.arsize = 3'b010;
  assign mem_r.arburst = (BLOCK_SZ == 1) ? 2'b00 : 2'b10;
  assign mem_r.rready = st_resp;

  //  -------------------- 缓存更新 --------------------
  reg [(OFFSET_W-2)-1:0] off_r, off_r_next;

  always @(posedge clock) begin
    if (reset) begin
      off_r <= 0;
    end else begin
      off_r <= off_r_next;
    end
  end

  always_comb begin
    off_r_next = off_r;
    if (mem_r.arready & mem_r.arvalid) begin
      off_r_next = off_next;
    end
    if (mem_r.rready & mem_r.rvalid) begin
      off_r_next = off_r + 1;
    end
  end

  integer i;
  always @(posedge clock) begin
    if (flush_icache) begin
      /*verilator unroll_full*/
      for (i = 0; i < SET_N; i = i + 1) begin
        if (mem_r.rready & mem_r.rvalid & mem_r.rlast & (i[INDEX_W-1:0] == index)) ;
        else line_valid[i] <= 0;
      end
    end
    if (mem_r.rready & mem_r.rvalid) begin
      line_data[index][off_r] <= mem_r.rdata;
      if (mem_r.rlast) begin
        line_valid[index] <= 1;
        line_tag[index] <= tag;
      end
    end
  end

  // -------------------- 性能计数器 --------------------
`ifndef SYNTHESIS
  always @(posedge clock) if (~reset) begin
    if (~st_idle) begin
      perf_event(PERF_ICACHE_MEM);
    end
    if (mem_r.arready & mem_r.arvalid) begin
      perf_event(PERF_ICACHE_MISS);
    end
  end
`endif

endmodule
