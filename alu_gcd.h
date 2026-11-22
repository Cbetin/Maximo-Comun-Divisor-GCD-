// ALU equivalente a la del laboratorio 1.
// Para el GCD se configura f=110: invierte B, suma uno y calcula A-B.
// El resultado se recorta a GCD_WIDTH bits; cout indica ausencia de prestamo.
// f[2] invierte B y entra como acarreo inicial.
// f[1:0]: 00 AND, 01 OR, 10 suma, 11 extension con ceros del bit S[WIDTH-1].
#ifndef ALU_GCD_H
#define ALU_GCD_H

#include <systemc.h>

#ifndef GCD_WIDTH
// Ancho predeterminado: coincide con WIDTH=8 del modulo Verilog.
// Puede cambiarse al compilar con -DGCD_WIDTH=N.
#define GCD_WIDTH 8
#endif

SC_MODULE(alu_gcd) {
    sc_in<sc_uint<GCD_WIDTH> >  a;
    sc_in<sc_uint<GCD_WIDTH> >  b;
    sc_in<sc_uint<3> >          f;
    sc_out<sc_uint<GCD_WIDTH> > y;
    sc_out<bool>                cout;

    void comb() {
        // SC_METHOD se ejecuta cuando cambia A, B o f. read() toma
        // el valor actual de cada puerto en la simulacion.
        sc_uint<GCD_WIDTH>     av = a.read();
        sc_uint<GCD_WIDTH>     bv = b.read();
        sc_uint<3>             fv = f.read();
        bool                   inv = fv[2];
        sc_uint<GCD_WIDTH>     bb  = inv ? sc_uint<GCD_WIDTH>(~bv) : bv;
        sc_uint<GCD_WIDTH + 1> suma_ext = sc_uint<GCD_WIDTH + 1>(av)
                                        + sc_uint<GCD_WIDTH + 1>(bb)
                                        + (inv ? 1 : 0);
        sc_uint<GCD_WIDTH> res;

        // El bit adicional es el acarreo de salida de A + ~B + 1.
        // Para operandos sin signo: cout=1 si A>=B.
        cout.write(suma_ext[GCD_WIDTH] != 0);

        switch (fv.range(1, 0).to_uint()) {
            case 0:  res = av & bb;                           break;
            case 1:  res = av | bb;                           break;
            case 2:  res = suma_ext.range(GCD_WIDTH - 1, 0);  break;
            // f[1:0]=11: extension con ceros del bit alto de S.
            default: res = suma_ext[GCD_WIDTH - 1] ? 1 : 0;   break;
        }
        y.write(res);
    }

    SC_CTOR(alu_gcd) {
        SC_METHOD(comb);
        sensitive << a << b << f;
    }
};

#endif
