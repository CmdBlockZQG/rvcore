module GPR (
  input clock, reset,

  input wen, // 写入使能
  input [4:0] waddr, // 写入地址
  input [31:0] wdata, // 写入数据

  input [4:0] raddr1, // 读出地址
  output [31:0] rdata1, // 读出数据
  input [4:0] raddr2, // 读出地址
  output [31:0] rdata2 // 读出数据
);

  parameter NR_REG = 32;

  // -------------------- WRITE --------------------
  reg [31:0] r [1:NR_REG-1]/*verilator public*/;

  always @(posedge clock) begin
    if (wen & (|waddr)) begin
      r[waddr] <= wdata;
    end
  end

  // -------------------- READ --------------------
  assign rdata1 = (|raddr1) ? r[raddr1] : 0;
  assign rdata2 = (|raddr2) ? r[raddr2] : 0;
endmodule
