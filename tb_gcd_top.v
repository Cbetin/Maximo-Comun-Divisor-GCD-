`timescale 1ns/1ps

// Banco de pruebas sencillo para desing.v. Simular tb_gcd_top como top.
// La practica pide al menos 30 pares de entrada; abajo hay exactamente 30.
module tb_gcd_top;
    reg clk, rst, start;
    reg  [7:0] x_in, y_in;
    wire [7:0] gcd;
    wire done;

    integer pruebas, errores;

    // Circuito que estamos probando (DUT = Design Under Test).
    gcd_top dut (
        .clk(clk), .rst(rst), .start(start),
        .x_in(x_in), .y_in(y_in), .gcd(gcd), .done(done)
    );

    // Estos nombres aparecen directamente en la ventana Wave si anades
    // las senales del testbench. Son copias de senales internas del circuito:
    // X y Y cambian con las restas; S es la salida actual de la ALU.
    // estado: 0=ESPERA, 1=COMPARAR, 2=RESTAR_X, 3=RESTAR_Y, 4=FIN.
    wire [7:0] X, Y, S;
    wire [2:0] estado;
    assign X = dut.u_datapath.x_reg;
    assign Y = dut.u_datapath.y_reg;
    assign S = dut.u_datapath.alu_y;
    assign estado = dut.u_control.state;

    // Reloj: cambia cada 5 ns, asi que su periodo completo es 10 ns.
    initial clk = 1'b0;
    always #5 clk = ~clk;

    // Esta tarea ejecuta UN caso de prueba. Recibe las entradas y la
    // respuesta que esperamos, por ejemplo probar(24, 18, 6).
    task probar;
        input [7:0] a, b, esperado;
        integer ciclos;
        begin
            pruebas = pruebas + 1;

            // Preparar las entradas antes del siguiente flanco positivo.
            @(negedge clk);
            x_in  = a;
            y_in  = b;
            start = 1'b1;

            // El circuito copia a y b en el flanco positivo intermedio.
            // Bajamos start en el siguiente flanco negativo: pulso de 1 ciclo.
            @(negedge clk);
            start = 1'b0;

            // Esperar a que termine. El limite detecta un posible bloqueo.
            // #1 permite ver los nuevos valores escritos en el flanco.
            ciclos = 0;
            while ((done !== 1'b1) && (ciclos < 520)) begin
                @(posedge clk);
                #1;
                ciclos = ciclos + 1;
            end

            if (done !== 1'b1) begin
                $display("ERROR: timeout para X=%0d, Y=%0d", a, b);
                $stop;
            end

            // Mostrar cada resultado en la consola y compararlo con la guia
            // de casos que escribimos mas abajo. !== detecta tambien X/Z.
            $display("Caso %0d: gcd(%0d,%0d) = %0d; esperado = %0d; ciclos = %0d",
                     pruebas, a, b, gcd, esperado, ciclos);
            if (gcd !== esperado) begin
                errores = errores + 1;
                $display("  ERROR en el caso %0d", pruebas);
            end

            // Dos ciclos extra para distinguir el resultado de un pulso.
            // En Wave deben permanecer altos done y el valor de gcd.
            repeat (2) @(posedge clk);
        end
    endtask

    initial begin
        rst = 1'b1;
        start = 1'b0;
        x_in = 8'd0;
        y_in = 8'd0;
        pruebas = 0;
        errores = 0;

        // Mantener reset durante dos flancos positivos del reloj.
        repeat (2) @(negedge clk);
        rst = 1'b0;

        // Casos especiales: cero e igualdad.
        probar(  0,   0,   0); //  1
        probar(  0,  18,  18); //  2
        probar( 24,   0,  24); //  3
        probar( 18,  18,  18); //  4
        probar(  1,   1,   1); //  5
        probar(255, 255, 255); //  6

        // Casos normales: observar X e Y disminuir en la ventana Wave.
        probar( 24,  18,   6); //  7
        probar( 18,  24,   6); //  8: mismos numeros en orden inverso
        probar( 48,  18,   6); //  9
        probar( 18,  48,   6); // 10
        probar( 12,   8,   4); // 11
        probar(  8,  12,   4); // 12
        probar( 21,  14,   7); // 13
        probar( 14,  21,   7); // 14
        probar( 36,  60,  12); // 15
        probar( 60,  36,  12); // 16
        probar( 17,  31,   1); // 17: coprimos
        probar( 31,  17,   1); // 18
        probar( 25,  15,   5); // 19
        probar( 15,  25,   5); // 20
        probar( 27,   9,   9); // 21
        probar(  9,  27,   9); // 22
        probar( 84,  30,   6); // 23
        probar( 30,  84,   6); // 24
        probar( 64,  40,   8); // 25
        probar( 40,  64,   8); // 26

        // Valores grandes: ejercitan el acarreo y recorridos largos.
        probar(255,   1,   1); // 27: muchas restas
        probar(  1, 255,   1); // 28
        probar(128, 127,   1); // 29
        probar(192, 128,  64); // 30

        if (errores == 0) begin
            $display("PASS: %0d casos correctos", pruebas);
            $finish;
        end else begin
            $display("FAIL: %0d errores en %0d casos", errores, pruebas);
            $stop;
        end
    end
endmodule
