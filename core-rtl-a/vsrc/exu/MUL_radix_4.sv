module MUL_radix_4 (
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
  wire st_hold = state == 6'd34;

  reg [33:0] a, a_next;
  reg [33:0] b, b_next;

  wire [67:0] s = {a, 34'b0};
  wire [67:0] ds = {s[66:0], 1'b0};
  wire [67:0] ms = {~a + 1, 34'b0};
  wire [67:0] mds = {ms[66:0], 1'b0};

  reg [67:0] prod, prod_next;
  reg [67:0] prod_next_add;
  always_comb begin
    case ({b[state+1], b[state], b[state-1]})
      3'b001, 3'b010 : prod_next_add = s;
      3'b011         : prod_next_add = ds;
      3'b100         : prod_next_add = mds;
      3'b101, 3'b110 : prod_next_add = ms;
      default        : prod_next_add = 0;
    endcase
  end
  wire [67:0] prod_next_uns = prod + prod_next_add;

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

  wire [33:0] in_sa = {{2{in_sign[1] & in_a[31]}}, in_a};
  wire [67:0] in_s = {in_sa, 34'b0};
  wire [67:0] in_ms = {~in_sa + 1, 34'b0};

  always_comb begin
    state_next = state;
    prod_next = prod;
    a_next = a;
    b_next = b;

    if (in_ready & in_valid) begin
      state_next = 6'd2;
      case (in_b[1:0])
        2'b01: prod_next = {{2{in_s[67]}}, in_s[67:2]};
        2'b10: prod_next = {in_ms[67], in_ms[67:1]};
        2'b11: prod_next = {{2{in_ms[67]}}, in_ms[67:2]};
        default: prod_next = 0;
      endcase
      a_next = in_sa;
      b_next = {{2{in_sign[0] & in_b[31]}}, in_b};
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
        state_next = state + 6'd2;
        prod_next = {{2{prod_next_uns[67]}}, prod_next_uns[67:2]};
      end
    end
  end

  assign in_ready = st_idle | flush | (out_ready & st_hold);
  assign out_valid = st_hold & ~flush;
  assign out_prod = prod[63:0];

endmodule
