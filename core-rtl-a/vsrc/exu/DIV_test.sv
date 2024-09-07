module DIV_test (
  input clock, reset,

  input flush,

  output in_ready,
  input in_valid,
  input in_sign,
  input [31:0] in_a, in_b,

  input out_ready,
  output out_valid,
  output [31:0] out_quot,
  output [31:0] out_rem
);

  wire as = in_a[31] & in_sign;
  wire bs = in_b[31] & in_sign;

  wire [31:0] a = ({32{as}} ^ in_a) + {31'b0, as};
  wire [31:0] b = ({32{bs}} ^ in_b) + {31'b0, bs};

  wire qs = (in_a[31] ^ in_b[31]) & in_sign;
  wire rs = in_a[31] & in_sign;

  wire [31:0] q = a / b;
  wire [31:0] r = a - (q * b);

  typedef enum logic {
    ST_IDLE,
    ST_HOLD
  } state_t;
  wire st_idle = state == ST_IDLE;
  wire st_hold = state == ST_HOLD;

  state_t state, state_next;
  reg [31:0] quot, quot_next;
  reg [31:0] rem, rem_next;

  always @(posedge clock) begin
    if (reset) begin
      state <= ST_IDLE;
    end else begin
      state <= state_next;
      quot <= quot_next;
      rem <= rem_next;
    end
  end

  assign in_ready = st_idle | (out_ready & out_valid) | flush;

  always_comb begin
    state_next = state;
    quot_next = quot;
    rem_next = rem;

    if (in_valid & in_valid) begin
      state_next = ST_HOLD;
      quot_next = ({32{qs}} ^ q) + {31'b0, qs};
      rem_next = ({32{rs}} ^ r) + {31'b0, rs};
    end

    if (st_hold) begin
      if ((out_valid & out_ready) | flush) begin
        if (~in_valid) begin
          state_next = ST_IDLE;
        end
      end
    end
  end

  assign out_valid = st_hold & ~flush;
  assign out_quot = quot;
  assign out_rem = rem;

endmodule
