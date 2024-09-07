module MUL_booth (
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

  reg [5:0] state, state_next;
  wire st_idle = state == 0;
  wire st_hold = state == 6'd33;

  reg [32:0] a, a_next;
  reg [32:0] b, b_next;

  wire [65:0] s = {a, 33'b0};
  wire [65:0] ms = {~a + 1, 33'b0};

  reg [65:0] prod, prod_next;
  wire [65:0] prod_next_uns = prod + (
    (b[state] ^ b[state-1]) ? (
      b[state] ? ms : s
    ) : 0
  );

  always @(posedge clock) begin
    if (reset) begin
      state <= 0;
    end else begin
      state <= state_next;
      prod <= prod_next;
      a <= a_next;
      b <= b_next;
    end
  end

  wire [32:0] in_sa = {in_sign[1] & in_a[31], in_a};
  wire [65:0] in_ms = {~in_sa + 1, 33'b0};

  always_comb begin
    state_next = state;
    prod_next = prod;
    a_next = a;
    b_next = b;

    if (in_ready & in_valid) begin
      state_next = 6'd1;
      prod_next = in_b[0] ? {in_ms[65], in_ms[65:1]} : 0;
      a_next = in_sa;
      b_next = {in_sign[0] & in_b[31], in_b};
    end

    if (st_idle) begin
      if (in_valid) ;
    end else if (st_hold) begin
      if (flush | out_ready) begin
        if (in_valid) begin
          ; // input
        end else begin
          state_next = 0;
        end
      end
    end else begin
      if (flush) begin
        if (in_valid) begin
          ; // input
        end else begin
          state_next = 0;
        end
      end else begin
        state_next = state + 1;
        prod_next = {prod_next_uns[65], prod_next_uns[65:1]};
      end
    end
  end

  assign in_ready = st_idle | flush | (out_ready & st_hold);
  assign out_valid = st_hold & ~flush;
  assign out_prod = prod[63:0];

endmodule
