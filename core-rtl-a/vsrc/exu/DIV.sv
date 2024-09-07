module DIV (
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

  parameter ST_HOLD = 6'b111111;
  parameter ST_IDLE = 6'd32;

  reg [5:0] state, state_next;
  wire st_hold = state == ST_HOLD;
  wire st_idle = state == ST_IDLE;

  reg [63:0] a, a_next;
  reg [31:0] b, b_next;
  reg qs, qs_next;
  reg rs, rs_next;
  reg [31:0] q, q_next;

  wire [32:0] rem = a[63:31] - {1'b0, b};

  always @(posedge clock) begin
    if (reset) begin
      state <= ST_IDLE;
    end else begin
      state <= state_next;
      a <= a_next;
      b <= b_next;
      qs <= qs_next;
      rs <= rs_next;
      q <= q_next;
    end
  end

  wire in_as = in_a[31] & in_sign;
  wire in_bs = in_b[31] & in_sign;

  always_comb begin
    state_next = state;
    a_next = a;
    b_next = b;
    qs_next = qs;
    rs_next = rs;
    q_next = q;

    if (in_ready & in_valid) begin
      state_next = 6'd31;
      a_next = {32'b0, ({32{in_as}} ^ in_a) + {31'b0, in_as}};
      b_next = ({32{in_bs}} ^ in_b) + {31'b0, in_bs};
      qs_next = (in_a[31] ^ in_b[31]) & in_sign;
      rs_next = in_a[31] & in_sign;
    end

    if (st_idle) begin
      if (in_valid) ;
    end else if (st_hold) begin
      if (flush | out_ready) begin
        if (in_valid) begin
          ;
        end else begin
          state_next = ST_IDLE;
        end
      end
    end else begin
      if (flush) begin
        if (in_valid) begin
          ;
        end else begin
          state_next = ST_IDLE;
        end
      end else begin
        state_next = state - 1;
        a_next = rem[32] ? {a[62:0], 1'b0} : {rem[31:0], a[30:0], 1'b0};
        q_next = {q[30:0], ~rem[32]};
      end
    end
  end

  assign in_ready = st_idle | flush | (out_ready & st_hold);
  assign out_valid = st_hold & ~flush;

  assign out_quot = ({32{qs}} ^ q) + {31'b0, qs};
  assign out_rem  = ({32{rs}} ^ a[63:32]) + {31'b0, rs};

endmodule
