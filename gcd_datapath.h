// Camino de datos equivalente a gcd_datapath en Verilog (lab. 1).
// Guarda X/Y y reutiliza una unica ALU para X-Y o Y-X.
#ifndef GCD_DATAPATH_H
#define GCD_DATAPATH_H

#include "alu_gcd.h"

SC_MODULE(gcd_datapath) {
    sc_in<bool>                 clk;
    sc_in<bool>                 rst;
    sc_in<sc_uint<GCD_WIDTH> >  x_in;
    sc_in<sc_uint<GCD_WIDTH> >  y_in;
    sc_in<bool>                 load;
    sc_in<bool>                 write_x;
    sc_in<bool>                 write_y;
    sc_in<bool>                 swap_ab;
    sc_out<bool>                x_zero;
    sc_out<bool>                y_zero;
    sc_out<bool>                equal_xy;
    sc_out<bool>                x_ge_y;
    sc_out<sc_uint<GCD_WIDTH> > gcd;

    // Senales internas (publicas para poder trazarlas en el VCD).
    // x_reg/y_reg son los operandos de trabajo; alu_y equivale a S.
    sc_signal<sc_uint<GCD_WIDTH> > x_reg, y_reg;
    sc_signal<sc_uint<GCD_WIDTH> > alu_a, alu_b, alu_y;
    sc_signal<sc_uint<3> >         alu_f;
    sc_signal<bool>                alu_cout;

    alu_gcd u_alu;

    // En COMPARAR y RESTAR_X: A=X, B=Y.
    // En RESTAR_Y: swap_ab=1, A=Y, B=X.
    // alu_a = swap ? Y : X ; alu_b = swap ? X : Y
    void mux_operandos() {
        if (swap_ab.read()) {
            alu_a.write(y_reg.read());
            alu_b.write(x_reg.read());
        } else {
            alu_a.write(x_reg.read());
            alu_b.write(y_reg.read());
        }
    }

    // Banderas y salida gcd (combinacional).
    void banderas() {
        bool xz = (x_reg.read() == 0);
        bool yz = (y_reg.read() == 0);
        x_zero.write(xz);
        y_zero.write(yz);
        equal_xy.write(alu_y.read() == 0);   // solo valido en COMPARAR (X-Y)
        x_ge_y.write(alu_cout.read());       // cout=1 en resta sin signo => A>=B
        // El valor de gcd solo se considera valido cuando done=1.
        // Con X=0 se devuelve Y; de lo contrario, X.
        gcd.write(xz ? y_reg.read() : x_reg.read());
    }

    // Registros X, Y: reset sincrono, carga y escritura desde la ALU.
    void registros() {
        // Prioridad: reset > carga inicial > actualizacion por ALU.
        if (rst.read()) {
            x_reg.write(0);
            y_reg.write(0);
        } else if (load.read()) {
            x_reg.write(x_in.read());
            y_reg.write(y_in.read());
        } else {
            if (write_x.read()) x_reg.write(alu_y.read());
            if (write_y.read()) y_reg.write(alu_y.read());
        }
    }

    SC_CTOR(gcd_datapath) : u_alu("u_alu") {
        // La ALU permanece en modo resta durante toda la simulacion.
        alu_f.write(6);   // f = 110: A + ~B + 1 = A - B

        u_alu.a(alu_a);
        u_alu.b(alu_b);
        u_alu.f(alu_f);
        u_alu.y(alu_y);
        u_alu.cout(alu_cout);

        SC_METHOD(mux_operandos);
        sensitive << swap_ab << x_reg << y_reg;

        SC_METHOD(banderas);
        sensitive << x_reg << y_reg << alu_y << alu_cout;

        // Los registros solo se actualizan en el flanco ascendente.
        SC_METHOD(registros);
        sensitive << clk.pos();
        dont_initialize();
    }
};

#endif
