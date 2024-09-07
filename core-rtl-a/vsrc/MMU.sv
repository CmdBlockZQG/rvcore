module MMU (
  input clock, reset,

  input flush,

  input [31:0] csr_satp,

  axi_if.out mem_r,

  input ifu_valid,
  input [31:0] ifu_vaddr,
  output reg ifu_hit,
  output [31:0] ifu_paddr,

  input lsu_valid,
  input [31:0] lsu_vaddr,
  output reg lsu_hit,
  output [31:0] lsu_paddr
);

  wire [19:0] ifu_vpn = ifu_vaddr[31:12];
  reg [19:0] ifu_ppn;
  assign ifu_paddr = {ifu_ppn, ifu_vaddr[11:0]};

  wire [19:0] lsu_vpn = lsu_vaddr[31:12];
  reg [19:0] lsu_ppn;
  assign lsu_paddr = {lsu_ppn, lsu_vaddr[11:0]};

  // -------------------- TLB --------------------
  parameter integer LINE_W = 4;
  localparam integer LINE_N = 1 << LINE_W;
  reg line_valid [LINE_N];
  reg [19:0] line_vpn [LINE_N];
  reg [19:0] line_ppn [LINE_N];

  reg [LINE_W-1:0] line_rand; // 随机替换
  reg [LINE_W-1:0] line_evit;
  always @(posedge clock) begin
    line_rand <= line_rand + 1;
  end
  always_comb begin
    line_evit = line_rand;
    for (integer i = 0; i < LINE_N; i = i + 1) begin
      if (~line_valid[i]) begin
        line_evit = i[LINE_W-1:0];
      end
    end
  end

  always_comb begin
    if (csr_satp[31]) begin // Sv32
      ifu_hit = 0;
      ifu_ppn = 0;
      lsu_hit = 0;
      lsu_ppn = 0;
      for (integer i = 0; i < LINE_N; i = i + 1) begin
        if (line_valid[i]) begin
          if (line_vpn[i] == ifu_vpn) begin
            ifu_hit = 1;
            ifu_ppn = line_ppn[i];
          end
          if (line_vpn[i] == lsu_vpn) begin
            lsu_hit = 1;
            lsu_ppn = line_ppn[i];
          end
        end
      end
    end else begin // off
      ifu_hit = 1;
      ifu_ppn = ifu_vpn;
      lsu_hit = 1;
      lsu_ppn = lsu_vpn;
    end
  end

  always @(posedge clock) if (~reset) begin
    if (st_resp & mem_r.rvalid & ~ptw_lv) begin
      line_valid[line_evit] <= 1;
      line_vpn[line_evit] <= ptw_dev ? lsu_vpn : ifu_vpn;
      line_ppn[line_evit] <= mem_r.rdata[29:10];
    end

    if (flush) begin
      /*verilator unroll_full*/
      for (integer i = 0; i < LINE_N; i = i + 1) begin
        line_valid[i] <= 0;
      end
    end
  end

  // -------------------- page table walk --------------------
  typedef enum logic [2:0] {
    ST_IDLE = 3'b001,
    ST_REQ  = 3'b010,
    ST_RESP = 3'b100
  } state_t;
  wire st_idle = state[0];
  wire st_req  = state[1];
  wire st_resp = state[2];
  state_t state, state_next;

  reg ptw_dev, ptw_dev_next;
  reg ptw_lv, ptw_lv_next;
  reg [31:0] ptw_raddr, ptw_raddr_next;

  always @(posedge clock)
  if (reset) begin
    state <= ST_IDLE;
  end else begin
    state <= state_next;
    ptw_dev <= ptw_dev_next;
    ptw_lv <= ptw_lv_next;
    ptw_raddr <= ptw_raddr_next;
  end

  always_comb begin
    state_next = state;
    ptw_dev_next = ptw_dev;
    ptw_lv_next = ptw_lv;
    ptw_raddr_next = ptw_raddr;

    case (1'b1)
      state[0]: begin // ST_IDLE
        if (lsu_valid & ~lsu_hit) begin
          state_next = ST_REQ;
          ptw_dev_next = 1; // LSU
          ptw_lv_next = 1;
          ptw_raddr_next = {csr_satp[19:0], lsu_vpn[19:10], 2'b0};
        end else if (ifu_valid & ~ifu_hit) begin
          state_next = ST_REQ;
          ptw_dev_next = 0; // IFU
          ptw_lv_next = 1;
          ptw_raddr_next = {csr_satp[19:0], ifu_vpn[19:10], 2'b0};
        end
      end
      state[1]: begin // ST_REQ
        if (mem_r.arready) begin
          state_next = ST_RESP;
        end
      end
      state[2]: begin // ST_RESP
        if (mem_r.rvalid) begin
          if (ptw_lv) begin
            state_next = ST_REQ;
            ptw_lv_next = 0;
            ptw_raddr_next = {mem_r.rdata[29:10], ptw_dev ? lsu_vpn[9:0] : ifu_vpn[9:0], 2'b0};
          end else begin
            state_next = ST_IDLE;
          end
        end
      end
      default: ;
    endcase
  end

  assign mem_r.arvalid = st_req;
  assign mem_r.araddr = ptw_raddr;
  assign mem_r.arid = 0;
  assign mem_r.arsize = 3'b010;
  assign mem_r.arburst = 0;
  assign mem_r.rready = st_resp;

endmodule
