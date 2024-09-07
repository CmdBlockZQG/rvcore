module top (
  input clock, reset,

  input io_interrupt,

  input         io_master_awready,
  output        io_master_awvalid,
  output [31:0] io_master_awaddr,
  output [3:0]  io_master_awid,
  output [7:0]  io_master_awlen,
  output [2:0]  io_master_awsize,
  output [1:0]  io_master_awburst,
  input         io_master_wready,
  output        io_master_wvalid,
  output [31:0] io_master_wdata,
  output [3:0]  io_master_wstrb,
  output        io_master_wlast,
  output        io_master_bready,
  input         io_master_bvalid,
  input  [1:0]  io_master_bresp,
  input  [3:0]  io_master_bid,
  input         io_master_arready,
  output        io_master_arvalid,
  output [31:0] io_master_araddr,
  output [3:0]  io_master_arid,
  output [7:0]  io_master_arlen,
  output [2:0]  io_master_arsize,
  output [1:0]  io_master_arburst,
  output        io_master_rready,
  input         io_master_rvalid,
  input  [1:0]  io_master_rresp,
  input  [31:0] io_master_rdata,
  input         io_master_rlast,
  input  [3:0]  io_master_rid,

  output        io_slave_awready,
  input         io_slave_awvalid,
  input  [31:0] io_slave_awaddr,
  input  [3:0]  io_slave_awid,
  input  [7:0]  io_slave_awlen,
  input  [2:0]  io_slave_awsize,
  input  [1:0]  io_slave_awburst,
  output        io_slave_wready,
  input         io_slave_wvalid,
  input  [31:0] io_slave_wdata,
  input  [3:0]  io_slave_wstrb,
  input         io_slave_wlast,
  input         io_slave_bready,
  output        io_slave_bvalid,
  output [1:0]  io_slave_bresp,
  output [3:0]  io_slave_bid,
  output        io_slave_arready,
  input         io_slave_arvalid,
  input  [31:0] io_slave_araddr,
  input  [3:0]  io_slave_arid,
  input  [7:0]  io_slave_arlen,
  input  [2:0]  io_slave_arsize,
  input  [1:0]  io_slave_arburst,
  input         io_slave_rready,
  output        io_slave_rvalid,
  output [1:0]  io_slave_rresp,
  output [31:0] io_slave_rdata,
  output        io_slave_rlast,
  output [3:0]  io_slave_rid
);

`ifdef SYNTHESIS

  axi_if io_master();
  // axi_if io_slave();

  // master input
  assign io_master.awready = io_master_awready;
  assign io_master.wready  = io_master_wready;
  assign io_master.bvalid  = io_master_bvalid;
  assign io_master.bresp   = io_master_bresp;
  assign io_master.bid     = io_master_bid;
  assign io_master.arready = io_master_arready;
  assign io_master.rvalid  = io_master_rvalid;
  assign io_master.rresp   = io_master_rresp;
  assign io_master.rdata   = io_master_rdata;
  assign io_master.rlast   = io_master_rlast;
  assign io_master.rid     = io_master_rid;
  // master output
  assign io_master_awvalid = io_master.awvalid;
  assign io_master_awaddr  = io_master.awaddr;
  assign io_master_awid    = io_master.awid;
  assign io_master_awlen   = io_master.awlen;
  assign io_master_awsize  = io_master.awsize;
  assign io_master_awburst = io_master.awburst;
  assign io_master_wvalid  = io_master.wvalid;
  assign io_master_wdata   = io_master.wdata;
  assign io_master_wstrb   = io_master.wstrb;
  assign io_master_wlast   = io_master.wlast;
  assign io_master_bready  = io_master.bready;
  assign io_master_arvalid = io_master.arvalid;
  assign io_master_araddr  = io_master.araddr;
  assign io_master_arid    = io_master.arid;
  assign io_master_arlen   = io_master.arlen;
  assign io_master_arsize  = io_master.arsize;
  assign io_master_arburst = io_master.arburst;
  assign io_master_rready  = io_master.rready;

  // slave output
  assign io_slave_awready = 0; // io_slave.awready;
  assign io_slave_wready  = 0; // io_slave.wready;
  assign io_slave_bvalid  = 0; // io_slave.bvalid;
  assign io_slave_bresp   = 0; // io_slave.bresp;
  assign io_slave_bid     = 0; // io_slave.bid;
  assign io_slave_arready = 0; // io_slave.arready;
  assign io_slave_rvalid  = 0; // io_slave.rvalid;
  assign io_slave_rresp   = 0; // io_slave.rresp;
  assign io_slave_rdata   = 0; // io_slave.rdata;
  assign io_slave_rlast   = 0; // io_slave.rlast;
  assign io_slave_rid     = 0; // io_slave.rid;
  // slave input
  // assign io_slave.awvalid = io_slave_awvalid;
  // assign io_slave.awaddr  = io_slave_awaddr;
  // assign io_slave.awid    = io_slave_awid;
  // assign io_slave.awlen   = io_slave_awlen;
  // assign io_slave.awsize  = io_slave_awsize;
  // assign io_slave.awburst = io_slave_awburst;
  // assign io_slave.wvalid  = io_slave_wvalid;
  // assign io_slave.wdata   = io_slave_wdata;
  // assign io_slave.wstrb   = io_slave_wstrb;
  // assign io_slave.wlast   = io_slave_wlast;
  // assign io_slave.bready  = io_slave_bready;
  // assign io_slave.arvalid = io_slave_arvalid;
  // assign io_slave.araddr  = io_slave_araddr;
  // assign io_slave.arid    = io_slave_arid;
  // assign io_slave.arlen   = io_slave_arlen;
  // assign io_slave.arsize  = io_slave_arsize;
  // assign io_slave.arburst = io_slave_arburst;
  // assign io_slave.rready  = io_slave_rready;

  CPU cpu (
    .clock(clock), .reset(reset),
    // .io_interrupt(io_interrupt),
    // .io_in(io_slave),
    .io_out(io_master)
  );

`else

  axi_if io_master();
  // axi_if io_slave();

  MEM mem (
    .clock(clock), .reset(reset),
    .in(io_master)
  );

  CPU cpu (
    .clock(clock), .reset(reset),
    // .io_interrupt(io_interrupt),
    // .io_in(io_slave),
    .io_out(io_master)
  );

`endif
endmodule
