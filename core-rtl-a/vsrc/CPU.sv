module CPU (
  input clock, reset,

  // axi_if.in io_in,
  axi_if.out io_out
);

  wire [31:0] gpr_rdata1, gpr_rdata2;
  GPR gpr (
    .clock(clock), .reset(reset),
    .raddr1(gpr_raddr1), .rdata1(gpr_rdata1),
    .raddr2(gpr_raddr2), .rdata2(gpr_rdata2),
    .wen(gpr_wen), .waddr(gpr_waddr), .wdata(gpr_wdata)
  );

  wire ifu_out_valid;
  wire [31:0] ifu_out_pc;
  wire [31:0] ifu_out_inst;
  axi_if ifu_mem_r();
  wire ifu_mmu_valid;
  wire [31:0] ifu_mmu_vaddr;
  IFU ifu (
    .clock(clock), .reset(reset),

    .mem_r(ifu_mem_r),

    .jump_flush(jump_flush), .jump_dnpc(jump_dnpc),
    .cs_flush(cs_flush), .cs_dnpc(cs_dnpc),
    .flush_icache(flush_icache),

    .mmu_valid(ifu_mmu_valid), .mmu_vaddr(ifu_mmu_vaddr),
    .mmu_hit(ifu_mmu_hit), .mmu_paddr(ifu_mmu_paddr),

    .out_ready(idu_in_ready),
    .out_valid(ifu_out_valid),
    .out_pc(ifu_out_pc),
    .out_inst(ifu_out_inst)
  );

  wire [4:0] gpr_raddr1, gpr_raddr2;
  wire [11:0] csr_raddr;
  wire jump_flush;
  wire [31:0] jump_dnpc;
  wire idu_in_ready;
  wire idu_out_valid;
  wire [31:0] idu_out_pc;
  wire [31:0] idu_out_dnpc;
  wire [31:0] idu_out_val_a;
  wire [31:0] idu_out_val_b;
  wire [31:0] idu_out_val_c;
  wire        idu_out_alu_src;
  wire [2:0]  idu_out_alu_funct;
  wire        idu_out_alu_sw;
  wire        idu_out_mul;
  wire [4:0]  idu_out_rd;
  wire        idu_out_rd_src;
  wire [3:0]  idu_out_ls;
  wire idu_out_zicsr;
  wire idu_out_csr_src;
  wire idu_out_exc;
  wire idu_out_ret;
  wire idu_out_fencei;
  `ifndef SYNTHESIS
    wire [31:0] idu_out_inst;
  `endif
  IDU idu (
    .clock(clock), .reset(reset),

    .flush(cs_flush),

    .rs1(gpr_raddr1), .src1(gpr_rdata1),
    .rs2(gpr_raddr2), .src2(gpr_rdata2),

    .csr_raddr(csr_raddr), .csr_rdata(csr_rdata),

    .exu_rd(exu_rd), .exu_rd_val(exu_rd_val),

    .jump_flush(jump_flush), .jump_dnpc(jump_dnpc),

    .in_ready(idu_in_ready),
    .in_valid(ifu_out_valid),
    .in_pc(ifu_out_pc),
    .in_inst(ifu_out_inst),

    .out_ready(exu_in_ready),
    .out_valid(idu_out_valid),
    .out_pc(idu_out_pc),
    .out_dnpc(idu_out_dnpc),
    .out_val_a(idu_out_val_a),
    .out_val_b(idu_out_val_b),
    .out_val_c(idu_out_val_c),
    .out_alu_funct(idu_out_alu_funct),
    .out_alu_sw(idu_out_alu_sw),
    .out_mul(idu_out_mul),
    .out_rd(idu_out_rd),
    .out_rd_src(idu_out_rd_src),
    .out_ls(idu_out_ls),
    .out_zicsr(idu_out_zicsr),
    .out_csr_src(idu_out_csr_src),
    .out_exc(idu_out_exc),
    .out_ret(idu_out_ret),
    .out_fencei(idu_out_fencei)
    `ifndef SYNTHESIS
      ,
      .out_inst(idu_out_inst)
    `endif
  );

  wire [4:0] exu_rd;
  wire [31:0] exu_rd_val;
  wire lsu_mmu_valid;
  wire [31:0] lsu_mmu_vaddr;
  axi_if lsu_mem_r();
  wire exu_in_ready;
  wire exu_out_valid;
  wire [31:0] exu_out_pc;
  wire [31:0] exu_out_dnpc;
  wire [4:0] exu_out_gpr_waddr;
  wire [31:0] exu_out_gpr_wdata;
  wire exu_out_zicsr;
  wire [11:0] exu_out_csr_waddr;
  wire [31:0] exu_out_csr_wdata;
  wire exu_out_exc;
  wire exu_out_ret;
  wire exu_out_fencei;
  `ifndef SYNTHESIS
    wire [31:0] exu_out_inst;
  `endif
  EXU exu (
    .clock(clock), .reset(reset),

    .flush(cs_flush),

    .exu_rd(exu_rd), .exu_rd_val(exu_rd_val),

    .mmu_valid(lsu_mmu_valid), .mmu_vaddr(lsu_mmu_vaddr),
    .mmu_hit(lsu_mmu_hit), .mmu_paddr(lsu_mmu_paddr),

    .mem_r(lsu_mem_r),
    .mem_w(io_out),

    .in_ready(exu_in_ready),
    .in_valid(idu_out_valid),
    .in_pc(idu_out_pc),
    .in_dnpc(idu_out_dnpc),
    .in_val_a(idu_out_val_a),
    .in_val_b(idu_out_val_b),
    .in_val_c(idu_out_val_c),
    .in_alu_funct(idu_out_alu_funct),
    .in_alu_sw(idu_out_alu_sw),
    .in_mul(idu_out_mul),
    .in_rd(idu_out_rd),
    .in_rd_src(idu_out_rd_src),
    .in_ls(idu_out_ls),
    .in_zicsr(idu_out_zicsr),
    .in_csr_src(idu_out_csr_src),
    .in_exc(idu_out_exc),
    .in_ret(idu_out_ret),
    .in_fencei(idu_out_fencei),

    .out_ready(wbu_in_ready),
    .out_valid(exu_out_valid),
    .out_pc(exu_out_pc),
    .out_dnpc(exu_out_dnpc),
    .out_gpr_waddr(exu_out_gpr_waddr),
    .out_gpr_wdata(exu_out_gpr_wdata),
    .out_zicsr(exu_out_zicsr),
    .out_csr_waddr(exu_out_csr_waddr),
    .out_csr_wdata(exu_out_csr_wdata),
    .out_exc(exu_out_exc),
    .out_ret(exu_out_ret),
    .out_fencei(exu_out_fencei)
    `ifndef SYNTHESIS
      ,
      .in_inst(idu_out_inst),
      .out_inst(exu_out_inst)
    `endif
  );

  wire gpr_wen;
  wire [4:0] gpr_waddr;
  wire [31:0] gpr_wdata;
  wire [31:0] csr_rdata;
  wire cs_flush;
  wire [31:0] cs_dnpc;
  wire flush_icache;
  wire [31:0] csr_satp;
  wire flush_tlb;
  wire clint_mtip_clear;
  wire wbu_in_ready;
  WBU wbu(
    .clock(clock), .reset(reset),

    .gpr_wen(gpr_wen), .gpr_waddr(gpr_waddr), .gpr_wdata(gpr_wdata),
    .csr_raddr(csr_raddr), .csr_rdata(csr_rdata),

    .cs_flush(cs_flush), .cs_dnpc(cs_dnpc),
    .flush_icache(flush_icache),

    .csr_satp(csr_satp), .flush_tlb(flush_tlb),

    .clint_mtip(0), .clint_mtip_clear(clint_mtip_clear),

    .in_ready(wbu_in_ready),
    .in_valid(exu_out_valid),
    .in_pc(exu_out_pc),
    .in_dnpc(exu_out_dnpc),
    .in_gpr_waddr(exu_out_gpr_waddr),
    .in_gpr_wdata(exu_out_gpr_wdata),
    .in_zicsr(exu_out_zicsr),
    .in_csr_waddr(exu_out_csr_waddr),
    .in_csr_wdata(exu_out_csr_wdata),
    .in_exc(exu_out_exc),
    .in_ret(exu_out_ret),
    .in_fencei(exu_out_fencei)

    `ifndef SYNTHESIS
      ,
      .in_inst(exu_out_inst)
    `endif
  );

  axi_if mmu_mem_r();
  wire ifu_mmu_hit;
  wire [31:0] ifu_mmu_paddr;
  wire lsu_mmu_hit;
  wire [31:0] lsu_mmu_paddr;
  MMU mmu(
    .clock(clock), .reset(reset),

    .flush(flush_tlb),

    .csr_satp(csr_satp),

    .mem_r(mmu_mem_r),

    .ifu_valid(ifu_mmu_valid), .ifu_vaddr(ifu_mmu_vaddr),
    .ifu_hit(ifu_mmu_hit), .ifu_paddr(ifu_mmu_paddr),

    .lsu_valid(lsu_mmu_valid), .lsu_vaddr(lsu_mmu_vaddr),
    .lsu_hit(lsu_mmu_hit), .lsu_paddr(lsu_mmu_paddr)
  );

  axi_if mem_r();
  MemArb mem_arb (
    .clock(clock), .reset(reset),
    .ifu_r(ifu_mem_r), .lsu_r(lsu_mem_r), .mmu_r(mmu_mem_r),
    .ram_r(io_out)
  );

endmodule
