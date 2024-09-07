module MUL_test (
  input clock, reset,

  input flush,

  output in_ready,
  input in_valid,
  input [1:0] in_sign,
  input [31:0] in_a, in_b,

  input out_ready,
  output out_valid,
  output [63:0] out_prod
);

  typedef enum logic {
    ST_IDLE,
    ST_HOLD
  } state_t;
  wire st_idle = state == ST_IDLE;
  wire st_hold = state == ST_HOLD;

  state_t state, state_next;
  reg [63:0] prod, prod_next;

  always @(posedge clock) begin
    if (reset) begin
      state <= ST_IDLE;
    end else begin
      state <= state_next;
      prod <= prod_next;
    end
  end

  assign in_ready = st_idle | (out_ready & out_valid) | flush;

  always_comb begin
    state_next = state;
    prod_next = prod;

    if (in_valid & in_valid) begin
      state_next = ST_HOLD;
      prod_next = $signed({in_sign[1] & in_a[31], in_a})
                * $signed({in_sign[0] & in_b[31], in_b});
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
  assign out_prod = prod;

endmodule
