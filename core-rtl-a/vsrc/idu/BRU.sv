module BRU (
  input [31:0] src1, src2,
  input [2:0] funct,

  output jump_en
);
  wire [31:0] a = src1;
  wire [31:0] b = ~src2;

  wire cf;
  wire [31:0] e;

  assign {cf, e} = a + b + 1;

  wire sf = e[31];
  wire of = (a[31] == b[31]) & (sf ^ a[31]);

  wire lts = sf ^ of;
  wire ltu = ~cf;
  wire eq = ~(|(src1 ^ src2));

  wire t = funct[2] ? (
    funct[1] ? ltu : lts
  ) : eq;

  assign jump_en = t ^ funct[0];

endmodule
