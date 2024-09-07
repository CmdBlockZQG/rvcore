module ALU (
  input [31:0] alu_a,
  input [31:0] alu_b,

  input [2:0] funct,
  input sw,

  output reg [31:0] val
);

  // `include "def/alu.sv"

  // 是否做减法
  // 是减法需要将alu_b取反，在加法器输入进位
  wire sub = sw | (funct == ALU_LTS) | (funct == ALU_LTU);

  wire [31:0] a = alu_a;
  wire [31:0] b = alu_b ^ {32{sub}};

  reg [31:0] e;
  reg cf; // carry flag 无符号进位标记
  wire sf = e[31]; // sign flag 符号标记
  wire of = (a[31] == b[31]) & (sf ^ a[31]); // overflow flag 补码溢出

  // 算术、逻辑右移结果
  wire [4:0] shift = alu_b[4:0];
  wire signed [31:0] sra = $signed(a) >>> $signed(shift);
  wire [31:0] srl = a >> shift;
  wire [31:0] shr = sw ? sra : srl;

  always_comb begin
    cf = 0;
    case (funct)
      ALU_ADD, ALU_LTS, ALU_LTU:
        {cf, e} = a + b + {31'b0, sub}; // 减法进位输入
      ALU_SHL: e = a << shift;
      ALU_XOR: e = a ^ b;
      ALU_SHR: e = shr;
      ALU_OR : e = a | b;
      ALU_AND: e = a & b;
      default: e = 32'b0;
    endcase
  end

  always_comb begin
    case (funct)
      ALU_LTS: val = {31'b0, sf ^ of};
      ALU_LTU: val = {31'b0, ~cf};
      default: val = e;
    endcase
  end

endmodule
