module MemArb (
  input clock, reset,

  axi_if.in ifu_r,
  axi_if.in lsu_r,
  axi_if.in mmu_r,

  axi_if.out ram_r
);

  typedef enum logic [1:0] {
    ST_IDLE,
    ST_IFU,
    ST_LSU,
    ST_MMU
  } state_t;
  wire st_idle = state == ST_IDLE;
  wire st_ifu = state == ST_IFU;
  wire st_lsu = state == ST_LSU;
  wire st_mmu = state == ST_MMU;

  state_t state, state_next;
  always @(posedge clock) begin
    if (reset) begin
      state <= ST_IDLE;
    end else begin
      state <= state_next;
    end
  end

  wire rhs = ram_r.rready & ram_r.rvalid & ram_r.rlast;

  always_comb begin
    state_next = state;
    if (st_idle) begin
      if (mmu_r.arvalid) begin
        state_next = ST_MMU;
      end else if (ifu_r.arvalid) begin
        state_next = ST_IFU;
      end else if (lsu_r.arvalid) begin
        state_next = ST_LSU;
      end
    end else begin
      if (rhs) begin
        state_next = ST_IDLE;
      end
    end
  end

  always_comb begin
    if (st_ifu) begin
      ram_r.arvalid = ifu_r.arvalid;
      ram_r.araddr = ifu_r.araddr;
      ram_r.arid = ifu_r.arid;
      ram_r.arlen = ifu_r.arlen;
      ram_r.arsize = ifu_r.arsize;
      ram_r.arburst = ifu_r.arburst;
      ram_r.rready = ifu_r.rready;
    end else if (st_lsu) begin
      ram_r.arvalid = lsu_r.arvalid;
      ram_r.araddr = lsu_r.araddr;
      ram_r.arid = lsu_r.arid;
      ram_r.arlen = lsu_r.arlen;
      ram_r.arsize = lsu_r.arsize;
      ram_r.arburst = lsu_r.arburst;
      ram_r.rready = lsu_r.rready;
    end else if (st_mmu) begin
      ram_r.arvalid = mmu_r.arvalid;
      ram_r.araddr = mmu_r.araddr;
      ram_r.arid = mmu_r.arid;
      ram_r.arlen = mmu_r.arlen;
      ram_r.arsize = mmu_r.arsize;
      ram_r.arburst = mmu_r.arburst;
      ram_r.rready = mmu_r.rready;
    end else begin
      ram_r.arvalid = 0;
      ram_r.araddr = 0;
      ram_r.arid = 0;
      ram_r.arlen = 0;
      ram_r.arsize = 0;
      ram_r.arburst = 0;
      ram_r.rready = 0;
    end
  end

  always_comb begin
    if (st_ifu) begin
      ifu_r.arready = ram_r.arready;
      ifu_r.rvalid = ram_r.rvalid;
      ifu_r.rresp = ram_r.rresp;
      ifu_r.rdata = ram_r.rdata;
      ifu_r.rlast = ram_r.rlast;
      ifu_r.rid = ram_r.rid;
    end else begin
      ifu_r.arready = 0;
      ifu_r.rvalid = 0;
      ifu_r.rresp = 0;
      ifu_r.rdata = 0;
      ifu_r.rlast = 0;
      ifu_r.rid = 0;
    end

    if (st_lsu) begin
      lsu_r.arready = ram_r.arready;
      lsu_r.rvalid = ram_r.rvalid;
      lsu_r.rresp = ram_r.rresp;
      lsu_r.rdata = ram_r.rdata;
      lsu_r.rlast = ram_r.rlast;
      lsu_r.rid = ram_r.rid;
    end else begin
      lsu_r.arready = 0;
      lsu_r.rvalid = 0;
      lsu_r.rresp = 0;
      lsu_r.rdata = 0;
      lsu_r.rlast = 0;
      lsu_r.rid = 0;
    end

    if (st_mmu) begin
      mmu_r.arready = ram_r.arready;
      mmu_r.rvalid = ram_r.rvalid;
      mmu_r.rresp = ram_r.rresp;
      mmu_r.rdata = ram_r.rdata;
      mmu_r.rlast = ram_r.rlast;
      mmu_r.rid = ram_r.rid;
    end else begin
      mmu_r.arready = 0;
      mmu_r.rvalid = 0;
      mmu_r.rresp = 0;
      mmu_r.rdata = 0;
      mmu_r.rlast = 0;
      mmu_r.rid = 0;
    end
  end

endmodule
