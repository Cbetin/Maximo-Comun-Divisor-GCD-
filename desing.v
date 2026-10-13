// GCD por restas sucesivas. Operandos sin signo; WIDTH >= 1.
// rst es sincrono y activo en alto. start debe durar un ciclo de clk.

// ALU (ver figura): f[2] invierte B y entra como acarreo inicial.
module alu_gcd #(parameter WIDTH = 8) (
    input  wire [WIDTH-1:0] a,
    input  wire [WIDTH-1:0] b,
    input  wire [2:0]       f,
    output reg  [WIDTH-1:0] y,
    output wire             cout
);
    wire [WIDTH-1:0] bb;
    wire [WIDTH:0]   suma_ext;

    assign bb       = f[2] ? ~b : b;
    assign suma_ext = {1'b0, a} + {1'b0, bb} + f[2];
    assign cout     = suma_ext[WIDTH];

    always @(*) begin
        case (f[1:0])
            2'b00: y = a & bb;
            2'b01: y = a | bb;
            2'b10: y = suma_ext[WIDTH-1:0];
            // Figura: extension con ceros del bit S[WIDTH-1].
            2'b11: y = {{(WIDTH-1){1'b0}}, suma_ext[WIDTH-1]};
            default: y = {WIDTH{1'b0}};
        endcase
    end
endmodule

// Registros, seleccion de operandos, ALU y banderas.
module gcd_datapath #(parameter WIDTH = 8) (
    input  wire             clk,
    input  wire             rst,
    input  wire [WIDTH-1:0] x_in,
    input  wire [WIDTH-1:0] y_in,
    input  wire             load,
    input  wire             write_x,
    input  wire             write_y,
    input  wire             swap_ab,
    output wire             x_zero,
    output wire             y_zero,
    output wire             equal_xy,
    output wire             x_ge_y,
    output wire [WIDTH-1:0] gcd
);
    reg [WIDTH-1:0] x_reg;
    reg [WIDTH-1:0] y_reg;
    wire [WIDTH-1:0] alu_a;
    wire [WIDTH-1:0] alu_b;
    wire [WIDTH-1:0] alu_y;
    wire             alu_cout;

    assign alu_a = swap_ab ? y_reg : x_reg;
    assign alu_b = swap_ab ? x_reg : y_reg;

    // f=110: A + ~B + 1 = A-B. En COMPARAR, A=X y B=Y.
    alu_gcd #(.WIDTH(WIDTH)) u_alu (
        .a(alu_a), .b(alu_b), .f(3'b110),
        .y(alu_y), .cout(alu_cout)
    );

    assign x_zero  = (x_reg == {WIDTH{1'b0}});
    assign y_zero  = (y_reg == {WIDTH{1'b0}});
    // Solo se usa en COMPARAR, donde alu_y = X-Y.
    assign equal_xy = (alu_y == {WIDTH{1'b0}});
    // En una resta sin signo, cout=1 significa A>=B.
    assign x_ge_y = alu_cout;

    // Cuando X=0, el resultado es Y; en los demas casos es X.
    // gcd solo se considera valido cuando done=1.
    assign gcd = x_zero ? y_reg : x_reg;

    always @(posedge clk) begin
        if (rst) begin
            x_reg <= {WIDTH{1'b0}};
            y_reg <= {WIDTH{1'b0}};
        end else if (load) begin
            x_reg <= x_in;
            y_reg <= y_in;
        end else begin
            if (write_x) x_reg <= alu_y;
            if (write_y) y_reg <= alu_y;
        end
    end
endmodule

// Maquina de estados: ordena cargas y escrituras.
module gcd_control (
    input  wire clk,
    input  wire rst,
    input  wire start,
    input  wire x_zero,
    input  wire y_zero,
    input  wire equal_xy,
    input  wire x_ge_y,
    output reg  load,
    output reg  write_x,
    output reg  write_y,
    output reg  swap_ab,
    output reg  done
);
    localparam [2:0] ESPERA   = 3'd0,
                     COMPARAR = 3'd1,
                     RESTAR_X = 3'd2,
                     RESTAR_Y = 3'd3,
                     FIN      = 3'd4;

    reg [2:0] state, next_state;

    always @(posedge clk) begin
        if (rst) state <= ESPERA;
        else     state <= next_state;
    end

    always @(*) begin
        // Valores por defecto: no cambiar registros.
        next_state = state;
        load       = 1'b0;
        write_x    = 1'b0;
        write_y    = 1'b0;
        swap_ab    = 1'b0;
        done       = 1'b0;

        case (state)
            ESPERA: begin
                if (start) begin
                    load       = 1'b1;
                    next_state = COMPARAR;
                end
            end

            COMPARAR: begin
                // La ALU calcula X-Y; aqui solo se observan sus banderas.
                if (x_zero || y_zero || equal_xy)
                    next_state = FIN;
                else if (x_ge_y)
                    next_state = RESTAR_X;
                else
                    next_state = RESTAR_Y;
            end

            RESTAR_X: begin
                // swap_ab=0: A=X, B=Y; X <- X-Y.
                write_x    = 1'b1;
                next_state = COMPARAR;
            end

            RESTAR_Y: begin
                // swap_ab=1: A=Y, B=X; Y <- Y-X.
                swap_ab    = 1'b1;
                write_y    = 1'b1;
                next_state = COMPARAR;
            end

            FIN: begin
                done = 1'b1;
                if (start) begin
                    load       = 1'b1;
                    next_state = COMPARAR;
                end
            end

            default: next_state = ESPERA;
        endcase
    end
endmodule

// Interfaz publica: une control y camino de datos.
module gcd_top #(parameter WIDTH = 8) (
    input  wire             clk,
    input  wire             rst,
    input  wire             start,
    input  wire [WIDTH-1:0] x_in,
    input  wire [WIDTH-1:0] y_in,
    output wire [WIDTH-1:0] gcd,
    output wire             done
);
    wire load, write_x, write_y, swap_ab;
    wire x_zero, y_zero, equal_xy, x_ge_y;

    gcd_datapath #(.WIDTH(WIDTH)) u_datapath (
        .clk(clk), .rst(rst), .x_in(x_in), .y_in(y_in),
        .load(load), .write_x(write_x), .write_y(write_y),
        .swap_ab(swap_ab), .x_zero(x_zero), .y_zero(y_zero),
        .equal_xy(equal_xy), .x_ge_y(x_ge_y), .gcd(gcd)
    );

    gcd_control u_control (
        .clk(clk), .rst(rst), .start(start),
        .x_zero(x_zero), .y_zero(y_zero),
        .equal_xy(equal_xy), .x_ge_y(x_ge_y),
        .load(load), .write_x(write_x), .write_y(write_y),
        .swap_ab(swap_ab), .done(done)
    );
endmodule
