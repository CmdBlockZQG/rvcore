interface axi_if;

  // read
  logic        arready;
  logic        arvalid;
  logic [31:0] araddr;
  logic [3:0]  arid;
  logic [7:0]  arlen;
  logic [2:0]  arsize;
  logic [1:0]  arburst;
  logic        rready;
  logic        rvalid;
  logic [1:0]  rresp;
  logic [31:0] rdata;
  logic        rlast;
  logic [3:0]  rid;

  // write
  logic        awvalid;
  logic        awready;
  logic [31:0] awaddr;
  logic [3:0]  awid;
  logic [7:0]  awlen;
  logic [2:0]  awsize;
  logic [1:0]  awburst;
  logic        wready;
  logic        wvalid;
  logic [31:0] wdata;
  logic [3:0]  wstrb;
  logic        wlast;
  logic        bready;
  logic        bvalid;
  logic [1:0]  bresp;
  logic [3:0]  bid;

  modport out ( // master
    // read
    input  arready,
    output arvalid,
    output araddr,
    output arid,
    output arlen,
    output arsize,
    output arburst,
    output rready,
    input  rvalid,
    input  rresp,
    input  rdata,
    input  rlast,
    input  rid,
    // write
    input  awready,
    output awvalid,
    output awaddr,
    output awid,
    output awlen,
    output awsize,
    output awburst,
    input  wready,
    output wvalid,
    output wdata,
    output wstrb,
    output wlast,
    output bready,
    input  bvalid,
    input  bresp,
    input  bid
  );

  modport in ( // slave
    // read
    output arready,
    input  arvalid,
    input  araddr,
    input  arid,
    input  arlen,
    input  arsize,
    input  arburst,
    input  rready,
    output rvalid,
    output rresp,
    output rdata,
    output rlast,
    output rid,
    // write
    output awready,
    input  awvalid,
    input  awaddr,
    input  awid,
    input  awlen,
    input  awsize,
    input  awburst,
    output wready,
    input  wvalid,
    input  wdata,
    input  wstrb,
    input  wlast,
    input  bready,
    output bvalid,
    output bresp,
    output bid
  );

endinterface
