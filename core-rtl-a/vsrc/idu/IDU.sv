module IDU (
  input clock, reset,

  // 冲刷信号
  input flush,

  // GPR
  output [4:0] rs1,
  input [31:0] src1,
  output [4:0] rs2,
  input [31:0] src2,

  // CSR
  output [11:0] csr_raddr,
  input [31:0] csr_rdata,

  // EXU旁路
  input [4:0] exu_rd,
  input [31:0] exu_rd_val,

  // 跳转输出
  output jump_flush,
  output [31:0] jump_dnpc,

  // 上游IFU输入
  output in_ready,
  input in_valid,
  input [31:0] in_pc,
  input [31:0] in_inst,

  // 下游EXU输出
  input out_ready,
  output out_valid,
  output     [31:0] out_pc,
  output     [31:0] out_dnpc,
  output reg [31:0] out_val_a,
  output reg [31:0] out_val_b,
  output reg [31:0] out_val_c,
  output reg [ 2:0] out_alu_funct,
  output reg        out_alu_sw,
  output            out_mul,
  output reg [ 4:0] out_rd,
  output            out_rd_src,
  output reg [ 3:0] out_ls,
  output            out_zicsr,
  output            out_csr_src,
  output            out_exc,
  output            out_ret,
  output            out_fencei

  `ifndef SYNTHESIS
    ,
    output reg [31:0] out_inst
  `endif
);

  reg valid;
  reg [31:0] pc, inst;
  reg jump_flush_en;

  always @(posedge clock)
  if (reset) begin
    valid <= 0;
  end else begin
    if (flush) begin
      valid <= 0;
    end else if (in_valid & in_ready) begin
      valid <= 1;
      pc <= in_pc;
      inst <= in_inst;
      jump_flush_en <= 1;
    end else begin
      if (out_valid & out_ready) begin
        valid <= 0;
      end else if (jump_flush) begin
        jump_flush_en <= 0;
      end
    end
  end

  assign in_ready = ~valid | (out_ready & ~gpr_raw);
  assign out_valid = valid & ~flush & ~gpr_raw;

  assign out_pc = pc;
  `ifndef SYNTHESIS
    assign out_inst = inst;
  `endif

  // -------------------- INST --------------------
  wire [4:0] opcode = inst[6:2];
  wire [2:0] funct3 = inst[14:12];
  wire [6:0] funct7 = inst[31:25];
  wire [11:0] funct12 = inst[31:20];

  // -------------------- IMM --------------------
  wire [31:0] imm_i, imm_s, imm_b, imm_u, imm_j, imm_z;
  assign imm_i = {{20{inst[31]}}, inst[31:20]};
  assign imm_s = {{20{inst[31]}}, inst[31:25], inst[11:7]};
  assign imm_b = {{20{inst[31]}}, inst[7], inst[30:25], inst[11:8], 1'b0};
  assign imm_u = {inst[31:12], 12'b0};
  assign imm_j = {{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0};
  assign imm_z = {27'b0, inst[19:15]};

  // -------------------- GPR --------------------
  assign rs1 = inst[19:15];
  assign rs2 = inst[24:20];
  wire [4:0] rd = inst[11:7];

  wire rs1_raw = (|rs1) & (rs1 == exu_rd);
  wire rs2_raw = (|rs2) & (rs2 == exu_rd);

  // reg gpr_raw;
  // always_comb begin
  //   case (opcode)
  //     OP_BRANCH : gpr_raw = rs1_raw | rs2_raw;
  //     OP_JALR   : gpr_raw = rs1_raw;
  //     default   : gpr_raw = 0;
  //   endcase
  // end
  // NOTE: 等价时序优化
  wire gpr_raw = (opcode[4:1] == 4'b1100) & (rs1_raw | (rs2_raw & ~opcode[0]));

  wire [31:0] src1_fw = ((exu_rd == rs1) & (|rs1)) ? exu_rd_val : src1;
  wire [31:0] src2_fw = ((exu_rd == rs2) & (|rs2)) ? exu_rd_val : src2;

  // -------------------- SYS --------------------
  wire sys = opcode == OP_SYS;

  // sys 时有效
  wire zicsr = |funct3;

  // sys & ~zicsr 时有效
  wire ecall  = funct12[1:0] == 2'b00;
  wire ebreak = funct12[1:0] == 2'b01;
  wire mret   = funct12[1:0] == 2'b10;

  wire [11:0] csr = inst[31:20];
  assign csr_raddr = csr;

  reg [2:0] sys_alu_funct;
  always_comb begin
    case (funct3)
      CSRF_RS, CSRF_RSI: sys_alu_funct = ALU_OR;
      CSRF_RC, CSRF_RCI: sys_alu_funct = ALU_AND;
      default          : sys_alu_funct = ALU_ADD;
    endcase
  end

  // -------------------- BRANCH --------------------
  // jump_flush
  wire br_jump_en;
  BRU BRU (
    .src1(src1), .src2(src2), .funct(funct3),
    .jump_en(br_jump_en)
  );

  // reg jump_pred_fail;
  // always_comb begin
  //   case (opcode)
  //     OP_JALR : begin
  //       jump_pred_fail = 1;
  //     end
  //     OP_BRANCH: begin
  //       jump_pred_fail = br_jump_en ^ inst[31];
  //     end
  //     default: begin
  //       jump_pred_fail = 0;
  //     end
  //   endcase
  // end
  // NOTE: 等价时序优化
  wire jump_pred_fail = (opcode[4:1] == 4'b1100) & (opcode[0] | (br_jump_en ^ inst[31]));

  assign jump_flush = valid & ~gpr_raw & jump_pred_fail & jump_flush_en;

  // jump_dnpc
  wire [31:0] dnpc_a = (opcode == OP_JALR) ? src1 : pc;
  reg [31:0] jump_dnpc_b;

  always_comb begin
    case (opcode)
      OP_JALR: jump_dnpc_b = imm_i;
      default: jump_dnpc_b = inst[31] ? 32'h4 : imm_b;
    endcase
  end

  wire [31:0] jump_dnpc_c = dnpc_a + jump_dnpc_b;

  assign jump_dnpc = {jump_dnpc_c[31:1], 1'b0};

  // out_dnpc
  reg [31:0] out_dnpc_b;
  always_comb begin
    case (opcode)
      OP_JAL    : out_dnpc_b = imm_j;
      OP_JALR   : out_dnpc_b = imm_i;
      OP_BRANCH : out_dnpc_b = br_jump_en ? imm_b : 32'h4;
      default   : out_dnpc_b = 32'h4;
    endcase
  end

  wire [31:0] out_dnpc_c = dnpc_a + out_dnpc_b;
  assign out_dnpc = {out_dnpc_c[31:1], 1'b0};

  // -------------------- 选数 --------------------
  always_comb begin
    case (opcode)
      OP_LUI                    : out_val_a = 0;
      OP_AUIPC, OP_JAL, OP_JALR : out_val_a = pc;
      OP_SYS                    : out_val_a = csr_rdata;
      default                   : out_val_a = src1_fw;
    endcase

    case (opcode)
      OP_LUI, OP_AUIPC : out_val_b = imm_u;
      // OP_JAL, OP_JALR  : out_val_b = 32'h4;
      OP_LOAD, OP_RI   : out_val_b = imm_i;
      OP_STORE         : out_val_b = imm_s;
      OP_RR            : out_val_b = src2_fw;
      OP_SYS           : out_val_b = funct3[2] ? imm_z : src1_fw;
      default          : out_val_b = 32'h4;
    endcase

    case (opcode)
      // OP_STORE : out_val_c = src2_fw;
      OP_SYS   : out_val_c = {20'h0, csr};
      default  : out_val_c = src2_fw;
    endcase
  end

  // -------------------- 控制信号 --------------------

  // alu_funct ALU模式选择
  always_comb begin
    case (opcode)
      OP_RI, OP_RR : out_alu_funct = funct3;
      OP_SYS       : out_alu_funct = sys_alu_funct;
      default      : out_alu_funct = ALU_ADD;
    endcase
  end

  // alu_sw ALU符号切换
  always_comb begin
    case (opcode)
      OP_RR   : out_alu_sw = funct7[5];
      OP_RI   : out_alu_sw = (funct3 == ALU_SHR) & funct7[5];
      OP_SYS  : out_alu_sw = &funct3[1:0];
      default : out_alu_sw = 1'b0;
    endcase
  end

  // mul 是否为乘除法指令
  // 若是，则alu_funct传递乘除法模式
  assign out_mul = (opcode == OP_RR) & funct7[0];

  // rd 目标寄存器
  // 0表示不写入寄存器
  always_comb begin
    case (opcode)
      OP_BRANCH, OP_STORE : out_rd = 5'b0;
      default             : out_rd = rd;
      // ecall, mret, ebreak, fence.i也不写入寄存器，但它们的rd字段都是0
    endcase
  end

  // rd_src 目标寄存器写入值的来源
  // 0: ALU
  // 1: val_a 用于支持zicsr指令
  // 当执行LOAD指令时（可从ls得知），该值无效，寄存器写入内存读取结果
  // 当执行乘除法指令时（可从mul得知），该值无效，寄存器写入乘除法结果
  assign out_rd_src = sys;

  // ls[3:0] 内存操作类型
  // 4'b0: 无内存操作
  // ls[3]: gpr_wen/mem_ren/~mem_wen
  // ls[2]: sext, 1表示符号拓展（只对load有意义）
  // ls[1:0]: size, 00, 01, 10, 11分别表示b, h, w, d
  always_comb begin
    case (opcode)
      OP_LOAD  : out_ls = {1'b1, ~funct3[2], funct3[1:0]};
      OP_STORE : out_ls = {2'b01, funct3[1:0]};
      default  : out_ls = 4'b0;
    endcase
  end

  // zicsr 是否写入CSR
  assign out_zicsr = sys & zicsr;

  // csr_src 写入CSR值来源
  // 0: ALU
  // 1: val_b
  assign out_csr_src = (funct3[1:0] == 2'b01);

  // exc 是否异常
  // TEMP: 目前异常只有ecall, ebreak
  assign out_exc = sys & ~zicsr & (ecall | ebreak);

  // ret 是否返回
  // TEMP: 目前仅mret
  assign out_ret = sys & ~zicsr & (mret | ebreak);

  // TEMP: 将ebreak标记为 exc & ret

  // fencei
  assign out_fencei = opcode == OP_FENCEI;

  // -------------------- 性能计数器 --------------------
`ifndef SYNTHESIS
  always @(posedge clock) if (~reset) begin
    if (~valid) begin
      perf_event(PERF_IDU_IDLE);
    end
    if (out_valid) begin
      perf_event(PERF_IDU_HOLD);
    end
    if (valid & gpr_raw & ~flush) begin
      perf_event(PERF_IDU_RAW);
    end
    if (out_ready & out_valid) begin
      perf_event(PERF_IDU_INST);
      if (opcode == OP_LOAD) perf_event(PERF_IDU_LOAD);
      if (opcode == OP_STORE) perf_event(PERF_IDU_STORE);
      if (opcode == OP_BRANCH) perf_event(PERF_IDU_BRANCH);
      if (opcode == OP_JAL) perf_event(PERF_IDU_JAL);
      if (opcode == OP_JALR) perf_event(PERF_IDU_JALR);
    end
    if (jump_flush) begin
      perf_event(PERF_BR_FLUSH);
    end
  end
`endif

endmodule
