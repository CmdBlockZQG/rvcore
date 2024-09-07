// ALU funct 常量
// 括号中为sw为0/1时的运算
parameter ALU_ADD = 3'b000; // 加减（加法/减法）
parameter ALU_SHL = 3'b001; // 左移
parameter ALU_LTS = 3'b010; // 有符号小于
parameter ALU_LTU = 3'b011; // 无符号小于
parameter ALU_XOR = 3'b100; // 异或
parameter ALU_SHR = 3'b101; // 右移（逻辑/算数）
parameter ALU_OR  = 3'b110; // 或
parameter ALU_AND = 3'b111; // 与
