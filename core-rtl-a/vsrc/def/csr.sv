// CSR寄存器地址常量
localparam logic [11:0] CSR_MSTATUS  = 12'h300;
localparam logic [11:0] CSR_MTVEC    = 12'h305;
localparam logic [11:0] CSR_MEPC     = 12'h341;
localparam logic [11:0] CSR_MCAUSE   = 12'h342;
localparam logic [11:0] CSR_SATP     = 12'h180;
localparam logic [11:0] CSR_MSCRATCH = 12'h340;

localparam logic [11:0] CSR_MVENDORID = 12'hf11;
localparam logic [11:0] CSR_MARCHID   = 12'hf12;

localparam logic [2:0] CSRF_RW  = 3'b001;
localparam logic [2:0] CSRF_RS  = 3'b010;
localparam logic [2:0] CSRF_RC  = 3'b011;
localparam logic [2:0] CSRF_RWI = 3'b101;
localparam logic [2:0] CSRF_RSI = 3'b110;
localparam logic [2:0] CSRF_RCI = 3'b111;
