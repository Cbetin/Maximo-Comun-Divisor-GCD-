// Modulo superior equivalente a gcd_top del laboratorio 1.
// Une las banderas que produce el datapath con el controlador,
// y lleva las ordenes del controlador de vuelta al datapath.
#ifndef GCD_TOP_H
#define GCD_TOP_H

#include "gcd_datapath.h"
#include "gcd_control.h"

SC_MODULE(gcd_top) {
    sc_in<bool>                 clk;
    sc_in<bool>                 rst;
    sc_in<bool>                 start;
    sc_in<sc_uint<GCD_WIDTH> >  x_in;
    sc_in<sc_uint<GCD_WIDTH> >  y_in;
    sc_out<sc_uint<GCD_WIDTH> > gcd;
    sc_out<bool>                done;

    // Ordenes hacia el datapath: cargar entradas, escribir X/Y
    // y cambiar el orden de los operandos de la ALU.
    sc_signal<bool> load, write_x, write_y, swap_ab;
    // Banderas desde el datapath: detectan cero, igualdad
    // y el orden relativo necesario para elegir la resta.
    sc_signal<bool> x_zero, y_zero, equal_xy, x_ge_y;

    gcd_datapath u_datapath;
    gcd_control  u_control;

    SC_CTOR(gcd_top) : u_datapath("u_datapath"), u_control("u_control") {
        u_datapath.clk(clk);
        u_datapath.rst(rst);
        u_datapath.x_in(x_in);
        u_datapath.y_in(y_in);
        u_datapath.load(load);
        u_datapath.write_x(write_x);
        u_datapath.write_y(write_y);
        u_datapath.swap_ab(swap_ab);
        u_datapath.x_zero(x_zero);
        u_datapath.y_zero(y_zero);
        u_datapath.equal_xy(equal_xy);
        u_datapath.x_ge_y(x_ge_y);
        u_datapath.gcd(gcd);

        u_control.clk(clk);
        u_control.rst(rst);
        u_control.start(start);
        u_control.x_zero(x_zero);
        u_control.y_zero(y_zero);
        u_control.equal_xy(equal_xy);
        u_control.x_ge_y(x_ge_y);
        u_control.load(load);
        u_control.write_x(write_x);
        u_control.write_y(write_y);
        u_control.swap_ab(swap_ab);
        u_control.done(done);
    }
};

#endif
