module IFU (
  input clock, reset,

  axi_if.out mem_r,

  input jump_flush,
  input [31:0] jump_dnpc,
  input cs_flush,
  input [31:0] cs_dnpc,
  input flush_icache,

  output mmu_valid,
  output [31:0] mmu_vaddr,
  input mmu_hit,
  input [31:0] mmu_paddr,

  input out_ready,
  output out_valid,
  output reg [31:0] out_pc,
  output reg [31:0] out_inst
);

  // -------------------- ICache --------------------
  reg [31:0] fetch_pc;
  wire hit;
  wire [31:0] cache_inst;
  ICache ICache (
    .clock(clock), .reset(reset),
    .flush_icache(flush_icache),
    .addr(fetch_pc), .hit(hit), .inst(cache_inst),
    .mmu_valid(mmu_valid), .mmu_vaddr(mmu_vaddr),
    .mmu_hit(mmu_hit), .mmu_paddr(mmu_paddr),
    .mem_r(mem_r)
  );

  // -------------------- 静态分支预测 --------------------
  wire [31:0] imm_b =
    {{20{cache_inst[31]}}, cache_inst[7], cache_inst[30:25], cache_inst[11:8], 1'b0};
  wire [31:0] imm_j =
    {{12{cache_inst[31]}}, cache_inst[19:12], cache_inst[20], cache_inst[30:21], 1'b0};
  reg [31:0] pc_incr;
  always_comb begin
    case (cache_inst[6:2])
      5'b11000: pc_incr = cache_inst[31] ? imm_b : 32'h4;
      5'b11011: pc_incr = imm_j;
      5'b11100: pc_incr = 0; // sys指令卡住取指
      default : pc_incr = 32'h4;
    endcase
  end
  wire [31:0] fetch_pc_pred = fetch_pc + pc_incr;

  // -------------------- flush & dnpc --------------------
  // input
  wire flush = jump_flush | cs_flush;
  wire [31:0] dnpc = cs_flush ? cs_dnpc : jump_dnpc;
  // reg
  reg flush_r;
  wire flush_w = flush | flush_r;
  wire flush_r_next = flush_w & ~hit;
  reg [31:0] dnpc_r;
  wire [31:0] dnpc_r_next = (flush & ~hit) ? dnpc : dnpc_r;

  // -------------------- output --------------------
  assign out_valid = out_valid_r & ~flush;
  reg out_valid_r, out_valid_r_next;
  wire out_step_en = ~out_valid_r | out_ready;
  wire [31:0] out_pc_next   = (~flush_w & hit & out_step_en) ? fetch_pc   : out_pc;
  wire [31:0] out_inst_next = (~flush_w & hit & out_step_en) ? cache_inst : out_inst;
  always_comb begin
    out_valid_r_next = out_valid_r;
    if (flush) begin
      out_valid_r_next = 0;
    end else if (~flush_r) begin
      if (out_valid_r) begin
        if (out_ready) begin
          if (hit) out_valid_r_next = 1;
          else out_valid_r_next = 0;
        end
      end else begin
        if (hit) out_valid_r_next = 1;
      end
    end
  end

  // -------------------- fetch --------------------
  reg [31:0] fetch_pc_next;
  always_comb begin // WARN: magic timing
    fetch_pc_next = fetch_pc;
    if (flush_r) begin
      if (hit) begin
        fetch_pc_next = dnpc_r;
      end
    end else if (out_valid_r) begin
      if (out_ready) begin
        if (hit) begin
          fetch_pc_next = fetch_pc_pred;
        end
      end
    end else begin
      if (hit) begin
        fetch_pc_next = fetch_pc_pred;
      end
    end
    if (flush) begin
      if (hit) begin
        fetch_pc_next = dnpc;
      end
    end
  end

  // -------------------- 状态机 --------------------
  always @(posedge clock) begin
    if (reset) begin
      out_valid_r <= 0;
      flush_r <= 0;
      fetch_pc <= 32'h80000000;
    end else begin
      out_valid_r <= out_valid_r_next;
      out_pc <= out_pc_next;
      out_inst <= out_inst_next;
      fetch_pc <= fetch_pc_next;
      flush_r <= flush_r_next;
      dnpc_r <= dnpc_r_next;
    end
  end

  // -------------------- 性能计数器 --------------------
`ifndef SYNTHESIS
  always @(posedge clock) if (~reset) begin
    if (out_valid) begin
      perf_event(PERF_IFU_HOLD);
    end else begin
      perf_event(PERF_IFU_WAIT);
    end
    if (hit) begin
      perf_event(PERF_IFU_FETCH_HOLD);
    end else begin
      perf_event(PERF_IFU_FETCH_WAIT);
    end
    if (out_valid & out_ready) begin
      perf_event(PERF_IFU_INST);
    end
  end
`endif

endmodule
