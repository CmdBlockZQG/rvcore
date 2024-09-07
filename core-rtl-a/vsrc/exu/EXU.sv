module EXU (
  input clock, reset,

  // 冲刷信号
  input flush,

  // EXU将要写入但还没写入的寄存器
  output [4:0] exu_rd,
  output [31:0] exu_rd_val,

  // LSU-MMU
  output mmu_valid,
  output [31:0] mmu_vaddr,
  input mmu_hit,
  input [31:0] mmu_paddr,

  // 访存AXI接口
  axi_if.out mem_r,
  axi_if.out mem_w,

  // 上游IDU输入
  output in_ready,
  input in_valid,
  input [31:0] in_pc,
  input [31:0] in_dnpc,
  input [31:0] in_val_a,
  input [31:0] in_val_b,
  input [31:0] in_val_c,
  input [ 2:0] in_alu_funct,
  input        in_alu_sw,
  input        in_mul,
  input [ 4:0] in_rd,
  input        in_rd_src,
  input [ 3:0] in_ls,
  input        in_zicsr,
  input        in_csr_src,
  input        in_exc,
  input        in_ret,
  input        in_fencei,

  // 下游WBU输出
  input out_ready,
  output out_valid,
  output [31:0] out_pc,
  output [31:0] out_dnpc,
  output [4:0] out_gpr_waddr,
  output [31:0] out_gpr_wdata,
  output out_zicsr,
  output [11:0] out_csr_waddr,
  output [31:0] out_csr_wdata,
  output out_exc,
  output out_ret,
  output out_fencei

  `ifndef SYNTHESIS
    ,
    input [31:0] in_inst,
    output [31:0] out_inst
  `endif
);

  reg valid;
  reg [31:0] pc;
  reg [31:0] dnpc;
  reg [31:0] val_a, val_b, val_c;
  reg [ 2:0] alu_funct;
  reg        alu_sw;
  reg        mul;
  reg [ 4:0] rd;
  reg        rd_src;
  reg [ 3:0] ls;
  reg        zicsr;
  reg        csr_src;
  reg        exc;
  reg        ret;
  reg        fencei;

  `ifndef SYNTHESIS
    reg [31:0] inst;
    assign out_inst = inst;
  `endif

  always @(posedge clock)
  if (reset) begin
    valid <= 0;
  end else begin
    if (flush) begin
      valid <= 0;
    end if (in_ready & in_valid) begin
      valid <= 1;
      pc <= in_pc;
      dnpc <= in_dnpc;
      val_a <= in_val_a;
      val_b <= in_val_b;
      val_c <= in_val_c;
      alu_funct <= in_alu_funct;
      alu_sw <= in_alu_sw;
      mul <= in_mul;
      rd <= in_rd;
      rd_src <= in_rd_src;
      ls <= in_ls;
      zicsr <= in_zicsr;
      csr_src <= in_csr_src;
      exc <= in_exc;
      ret <= in_ret;
      fencei <= in_fencei;
      `ifndef SYNTHESIS
        inst <= in_inst;
      `endif
    end else if (out_ready & out_valid) begin
      valid <= 0;
    end
  end

  assign in_ready = ~valid | (out_ready & out_valid);
  assign out_valid = ~flush & valid & exec_out_valid;

  assign out_pc = pc;
  assign out_dnpc = dnpc;

  // -------------------- EXEC --------------------
  reg exec_en_r, exec_en_r_next;

  always @(posedge clock)
  if (reset) begin
    exec_en_r <= 0;
  end else begin
    exec_en_r <= exec_en_r_next;
  end

  always_comb begin
    exec_en_r_next = exec_en_r;

    if (flush) exec_en_r_next = 0;
    else if (in_ready & in_valid) exec_en_r_next = 1;
    else if (exec_en_r) begin
      if (lsu_en) begin
        if (lsu_in_ready) exec_en_r_next = 0;
      end else if (mul_en) begin
        if (mul_in_ready) exec_en_r_next = 0;
      end else if (div_en) begin
        if (div_in_ready) exec_en_r_next = 0;
      end
    end
  end

  wire exec_en = ~flush & exec_en_r;

  reg exec_out_valid;
  always_comb begin
    if (exec_en_r) begin
      exec_out_valid = ~(|ls | mul);
    end else begin
      exec_out_valid =
        (lsu_en & lsu_out_valid) |
        (mul_en & mul_out_valid) |
        (div_en & div_out_valid) ;
    end
  end

  // -------------------- ALU --------------------
  wire [31:0] alu_val;
  ALU ALU (
    .alu_a(val_a), .alu_b(val_b),
    .funct(alu_funct), .sw(alu_sw),
    .val(alu_val)
  );

  // -------------------- LSU --------------------
  wire lsu_en = |ls;
  wire lsu_in_ready, lsu_out_valid;
  wire [31:0] lsu_val;
  LSU LSU (
    .clock(clock), .reset(reset),
    .mem_r(mem_r), .mem_w(mem_w),
    .mmu_valid(mmu_valid), .mmu_vaddr(mmu_vaddr),
    .mmu_hit(mmu_hit), .mmu_paddr(mmu_paddr),
    .in_ready(lsu_in_ready), .in_valid(exec_en & lsu_en),
    .ls(ls), .alu_val(alu_val), .val_c(val_c),
    .out_ready(out_ready), .out_valid(lsu_out_valid),
    .out_rdata(lsu_val)
  );

  // -------------------- MUL --------------------
  wire mul_en = mul & ~alu_funct[2];
  wire [1:0] mul_sign = {^alu_funct[1:0], ~alu_funct[1] & alu_funct[0]};
  wire mul_in_ready, mul_out_valid;
  wire [63:0] mul_prod;
  wire [31:0] mul_val = (|alu_funct[1:0]) ? mul_prod[63:32] : mul_prod[31:0];
  // MUL_radix_4 MUL(
  MUL_test MUL (
    .clock(clock), .reset(reset), .flush(0),
    .in_ready(mul_in_ready), .in_valid(exec_en & mul_en),
    .in_sign(mul_sign), .in_a(val_a), .in_b(val_b),
    .out_ready(out_ready), .out_valid(mul_out_valid),
    .out_prod(mul_prod)
  );

  // -------------------- DIV --------------------
  wire div_en = mul & alu_funct[2];
  wire div_sign = ~alu_funct[0];
  wire div_in_ready, div_out_valid;
  wire [31:0] div_quot, div_rem;
  wire [31:0] div_val = alu_funct[1] ? div_rem : div_quot;
  // DIV DIV(
  DIV_test DIV (
    .clock(clock), .reset(reset), .flush(0),
    .in_ready(div_in_ready), .in_valid(exec_en & div_en),
    .in_sign(div_sign), .in_a(val_a), .in_b(val_b),
    .out_ready(out_ready), .out_valid(div_out_valid),
    .out_quot(div_quot), .out_rem(div_rem)
  );

  // -------------------- GPR写回 --------------------
  assign out_gpr_waddr = rd;
  assign out_gpr_wdata =
    ls[3] ? lsu_val :
    rd_src ? val_a :
    mul ? (alu_funct[2] ? div_val : mul_val) :
    alu_val;

  assign exu_rd = rd & {5{valid}};
  // assign exu_rd_val = out_gpr_wdata;
  // TEMP: 只有CSR指令的rd_src为1，反正要被冲刷掉，前递错误也没关系
  // WARN: 数据通路修改可能会导致错误
  assign exu_rd_val =
    ls[3] ? lsu_val :
    mul ? (alu_funct[2] ? div_val : mul_val) :
    alu_val;

  // -------------------- CSR写回 --------------------
  assign out_zicsr = zicsr;
  assign out_csr_waddr = val_c[11:0];
  assign out_csr_wdata = csr_src ? val_b : alu_val;

  // -------------------- sys --------------------
  assign out_exc = exc;
  assign out_ret = ret;
  assign out_fencei = fencei;

  // -------------------- 性能计数器 --------------------
`ifndef SYNTHESIS
  always @(posedge clock) if (~reset) begin
    if (out_ready & out_valid) begin
      perf_event(PERF_EXU_INST);
    end
    if (~valid) begin
      perf_event(PERF_EXU_IDLE);
    end
  end
`endif

endmodule
