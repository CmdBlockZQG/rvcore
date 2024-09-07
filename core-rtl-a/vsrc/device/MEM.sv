`ifndef SYNTHESIS
module MEM (
  input clock, reset,

  axi_if.in in
);

  always @(posedge clock) if (reset) begin // 复位
    in.arready <= 1;
    in.rvalid <= 0;
    in.rlast <= 0;

    in.awready <= 1;
    in.wready <= 1;
    in.bvalid <= 0;

    waddr_valid_reg <= 0;
    wdata_valid_reg <= 0;
  end

  assign in.rresp = 2'b00;

  reg [31:0] araddr;
  reg [7:0] arlen;
  reg [2:0] arsize;
  reg [1:0] arburst;
  reg [7:0] burst_cnt;

  reg [2:0] len_w;
  always_comb begin
    case (arlen)
      8'h01: len_w = 3'h1;
      8'h03: len_w = 3'h2;
      8'h07: len_w = 3'h3;
      8'h0f: len_w = 3'h4;
      default: len_w = 3'h0;
    endcase
  end
  wire [31:0] wrap_mask = (32'h1 << (arsize + len_w)) - 32'h1;
  wire [31:0] araddr_incr_next = araddr + (32'h1 << arsize);
  reg [31:0] araddr_next;
  always_comb begin
    case (arburst)
      2'b01: araddr_next = araddr_incr_next; // incr
      2'b10: araddr_next = (araddr & ~wrap_mask) | (araddr_incr_next & wrap_mask);
      default: araddr_next = araddr; // fixed
    endcase
  end

  always @(posedge clock) if (~reset) begin
    if (in.arvalid & in.arready) begin
      in.arready <= 0; // idle -> access

      araddr <= in.araddr;
      arlen <= in.arlen;
      arsize <= in.arsize;
      arburst <= in.arburst;
      burst_cnt <= in.arlen;

      in.rvalid <= 1;
      in.rdata <= pmem_read(in.araddr);
      in.rlast <= (in.arlen == 8'b0);
    end

    if (~in.arready & in.rvalid & in.rready) begin
      if (in.rlast) begin // burst的最后一拍
        in.arready <= 1;
        in.rvalid <= 0;
        in.rlast <= 0;
      end else begin
        burst_cnt <= burst_cnt - 1;
        araddr <= araddr_next;
        in.rdata <= pmem_read(araddr_next);
        in.rlast <= (burst_cnt == 8'h1);
      end
    end
  end

  wire waddr_handshake = in.awready & in.awvalid;
  reg [31:0] waddr_reg;
  wire [31:0] waddr = waddr_handshake ? in.awaddr : waddr_reg;
  reg waddr_valid_reg;
  wire waddr_valid = waddr_handshake | waddr_valid_reg;

  wire wdata_handshake = in.wready & in.wvalid;
  reg [31:0] wdata_reg;
  reg [3:0] wmask_reg;
  wire [31:0] wdata = wdata_handshake ? in.wdata : wdata_reg;
  wire [3:0] wmask = wdata_handshake ? in.wstrb : wmask_reg;
  reg wdata_valid_reg;
  wire wdata_valid = wdata_handshake | wdata_valid_reg;

  wire write_en = waddr_valid & wdata_valid;

  always @(posedge clock) if (~reset) begin
    if (waddr_handshake) begin
      waddr_reg <= in.awaddr;
      if (~write_en) begin
        waddr_valid_reg <= 1;
        in.awready <= 0;
      end
    end
    if (wdata_handshake) begin
      wmask_reg <= in.wstrb;
      wdata_reg <= in.wdata;
      if (~write_en) begin
        wdata_valid_reg <= 1;
        in.wready <= 0;
      end
    end

    if (write_en) begin
      pmem_write({waddr[31:2], 2'b00}, wdata[31:0], {4'b0, wmask[3:0]});

      in.bresp <= 2'b00;
      in.bvalid <= 1;
      in.awready <= 1;
      in.wready <= 1;

      waddr_valid_reg <= 0;
      wdata_valid_reg <= 0;
    end

    if (in.bvalid & in.bready) in.bvalid <= 0;
  end
endmodule
`endif
