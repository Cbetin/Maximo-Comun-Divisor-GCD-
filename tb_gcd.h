// Banco de pruebas de SystemC: 30 casos dirigidos de 8 bits.
// El testbench real del lab. 1 (tb_gcd_r.v) ejecuta 36 casos:
// 16 dirigidos y 20 pseudoaleatorios; las listas no son identicas.
#ifndef TB_GCD_H
#define TB_GCD_H

#include "gcd_top.h"

SC_MODULE(tb_gcd) {
    sc_in<bool>                 clk;
    sc_out<bool>                rst;
    sc_out<bool>                start;
    sc_out<sc_uint<GCD_WIDTH> > x_in;
    sc_out<sc_uint<GCD_WIDTH> > y_in;
    sc_in<sc_uint<GCD_WIDTH> >  gcd;
    sc_in<bool>                 done;

    int pruebas;
    int errores;

    // Ejecuta un caso: aplica a/b, genera un pulso start,
    // espera done y comprueba el resultado esperado.
    // El valor esperado se pasa explicitamente (no se calcula aqui).
    void probar(unsigned a, unsigned b, unsigned esperado) {
        pruebas++;

        // Preparar entradas antes del siguiente flanco positivo.
        wait(clk.negedge_event());
        x_in.write(a);
        y_in.write(b);
        start.write(true);

        // start dura un ciclo.
        wait(clk.negedge_event());
        start.write(false);

        // Se espera a que el controlador llegue a FIN. El tope
        // de 520 ciclos cubre el caso 255,1 para GCD_WIDTH=8;
        // si se aumenta el ancho, hay que ampliar este limite.
        // Esperar done (el limite detecta un bloqueo).
        int ciclos = 0;
        while (!done.read() && ciclos < 520) {
            wait(clk.posedge_event());
            wait(1, SC_NS);
            ciclos++;
        }

        if (!done.read()) {
            std::cout << "ERROR: timeout para X=" << a << ", Y=" << b << std::endl;
            sc_stop();
            wait();
        }

        std::cout << "Caso " << pruebas << ": gcd(" << a << "," << b << ") = "
                  << gcd.read().to_uint() << "; esperado = " << esperado
                  << "; ciclos = " << ciclos << std::endl;
        if (gcd.read().to_uint() != esperado) {
            errores++;
            std::cout << "  ERROR en el caso " << pruebas << std::endl;
        }

        // El estado FIN conserva done alto y mantiene el resultado
        // mientras no llegue otra solicitud.
        // Dos ciclos extra para ver done y gcd estables.
        wait(clk.posedge_event());
        wait(clk.posedge_event());
    }

    void estimulos() {
        rst.write(true);
        start.write(false);
        x_in.write(0);
        y_in.write(0);
        pruebas = 0;
        errores = 0;

        // El reset es sincrono y necesita al menos un flanco
        // positivo antes del primer start.
        // Reset durante dos flancos.
        wait(clk.negedge_event());
        wait(clk.negedge_event());
        rst.write(false);

        // Por convenio de esta practica, gcd(0,0)=0.
        // Casos especiales: cero e igualdad.
        probar(  0,   0,   0); //  1
        probar(  0,  18,  18); //  2
        probar( 24,   0,  24); //  3
        probar( 18,  18,  18); //  4
        probar(  1,   1,   1); //  5
        probar(255, 255, 255); //  6

        // Casos normales.
        probar( 24,  18,   6); //  7
        probar( 18,  24,   6); //  8
        probar( 48,  18,   6); //  9
        probar( 18,  48,   6); // 10
        probar( 12,   8,   4); // 11
        probar(  8,  12,   4); // 12
        probar( 21,  14,   7); // 13
        probar( 14,  21,   7); // 14
        probar( 36,  60,  12); // 15
        probar( 60,  36,  12); // 16
        probar( 17,  31,   1); // 17
        probar( 31,  17,   1); // 18
        probar( 25,  15,   5); // 19
        probar( 15,  25,   5); // 20
        probar( 27,   9,   9); // 21
        probar(  9,  27,   9); // 22
        probar( 84,  30,   6); // 23
        probar( 30,  84,   6); // 24
        probar( 64,  40,   8); // 25
        probar( 40,  64,   8); // 26

        // Valores grandes.
        probar(255,   1,   1); // 27
        probar(  1, 255,   1); // 28
        probar(128, 127,   1); // 29
        probar(192, 128,  64); // 30

        if (errores == 0)
            std::cout << "PASS: " << pruebas << " casos correctos" << std::endl;
        else
            std::cout << "FAIL: " << errores << " errores en " << pruebas
                      << " casos" << std::endl;

        wait(clk.posedge_event());
        sc_stop();
    }

    SC_CTOR(tb_gcd) : pruebas(0), errores(0) {
        SC_THREAD(estimulos);
    }
};

#endif
