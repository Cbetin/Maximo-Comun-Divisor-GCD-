`timescale 1ns/1ps

// Testbench de gcd_top. Simular este modulo (tb_gcd) junto con desing.v.
// Cada llamada a probar(x,y) carga un par, espera done y compara con gcd_ref.
module tb_gcd;
    parameter WIDTH = 8;

    // Entradas y salidas del circuito.
    reg                  clk   = 0;
    reg                  rst   = 1;
    reg                  start = 0;
    reg  [WIDTH-1:0]     x_in  = 0;
    reg  [WIDTH-1:0]     y_in  = 0;
    wire [WIDTH-1:0]     gcd;
    wire                 done;

    // Agregar estas senales a Wave ayuda a seguir cada prueba.
    reg  [WIDTH-1:0] esperado    = 0;
    integer          caso_actual = 0;
    reg              aleatorio   = 0; // 0: caso escrito a mano; 1: aleatorio.
    reg              coincide    = 0; // Se actualiza cuando sube done.
    integer          pruebas     = 0;
    integer          errores     = 0;
    reg              fin         = 0;

    integer i;
    integer semilla = 12345; // Misma secuencia aleatoria en cada simulacion.
    reg [WIDTH-1:0] ale_x, ale_y;

    gcd_top #(.WIDTH(WIDTH)) dut (
        .clk(clk), .rst(rst), .start(start),
        .x_in(x_in), .y_in(y_in), .gcd(gcd), .done(done)
    );

    // Alias para ver facilmente el interior del circuito en Wave.
    // Estado: 0=ESPERA, 1=COMPARAR, 2=RESTAR_X, 3=RESTAR_Y, 4=FIN.
    wire [WIDTH-1:0] X, Y, S;
    wire [2:0] estado;
    assign X      = dut.u_datapath.x_reg;
    assign Y      = dut.u_datapath.y_reg;
    assign S      = dut.u_datapath.alu_y;
    assign estado = dut.u_control.state;

    // Reloj: un periodo completo dura 10 ns.
    always #5 clk = ~clk;

    // Respuesta esperada. Este modelo usa modulo; el DUT usa restas.
    // Se usa solo en simulacion, no forma parte del hardware sintetizado.
    function [WIDTH-1:0] gcd_ref;
        input [WIDTH-1:0] a, b;
        reg [WIDTH-1:0] p, q, resto;
        begin
            p = a;
            q = b;
            while (q != 0) begin
                resto = p % q;
                p = q;
                q = resto;
            end
            gcd_ref = p; // Incluye gcd(0,0)=0 por convenio.
        end
    endfunction

    // Aplica una pareja de entradas, espera el resultado y lo comprueba.
    task probar;
        input [WIDTH-1:0] x, y;
        begin
            @(negedge clk);
            x_in        = x;
            y_in        = y;
            esperado    = gcd_ref(x, y);
            caso_actual = pruebas + 1;
            coincide    = 0;
            start       = 1;

            // start abarca exactamente un flanco positivo de clk.
            @(negedge clk);
            start = 0;

            wait (done == 1); // Resultado valido cuando done sube.
            #1;               // Esperar actualizacion de registros del DUT.

            coincide = (gcd === esperado);
            pruebas = pruebas + 1;
            if (coincide)
                $display("OK caso %0d: gcd(%0d,%0d)=%0d",
                         caso_actual, x, y, gcd);
            else begin
                $display("ERROR caso %0d: gcd(%0d,%0d)=%0d; esperado=%0d",
                         caso_actual, x, y, gcd, esperado);
                errores = errores + 1;
            end
        end
    endtask

    initial begin
        // rst sincrono: estos dos ciclos reinician control y registros.
        repeat (2) @(negedge clk);
        rst = 0;

        // Casos dirigidos: facil seguir las restas de 12 y 18 en Wave.
        probar(  0,   0);
        probar(  0,   7);
        probar(  9,   0);
        probar(  5,   5);
        probar(  1,   1);
        probar(255, 255);
        probar( 12,  18);
        probar( 18,  12);
        probar( 48,  36);
        probar( 17,  13);
        probar( 64,  32);
        probar(221, 170);
        probar(240, 180);
        probar( 97,  89);
        probar(255,   1); // Recorrido largo: muchas restas.
        probar(  1, 255);

        // 20 pares pseudoaleatorios: 16 dirigidos + 20 = 36 en total.
        aleatorio = 1;
        for (i = 0; i < 20; i = i + 1) begin
            // $random devuelve 32 bits; se conservan los WIDTH bits bajos.
            ale_x = $random(semilla);
            ale_y = $random(semilla);
            probar(ale_x, ale_y);
        end

        fin = 1;
        $display("------------------------------------");
        $display("Pruebas: %0d   Errores: %0d", pruebas, errores);
        if (errores == 0) $display("TODAS LAS PRUEBAS PASARON");
        else              $display("HAY FALLAS");
        $stop; // En Questa deja la simulacion detenida para mirar Wave.
    end

    // Proteccion global por si un error hace que done nunca se active.
    initial begin
        #5_000_000; // 5 ms con timescale 1ns/1ps.
        if (!fin) begin
            $display("TIMEOUT: el circuito no termino algun caso");
            $stop;
        end
    end
endmodule
