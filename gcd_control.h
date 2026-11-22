// Unidad de control equivalente a gcd_control en Verilog (lab. 1).
// Coordina la carga, la comparacion, una resta por iteracion y done.
#ifndef GCD_CONTROL_H
#define GCD_CONTROL_H

#include <systemc.h>

SC_MODULE(gcd_control) {
    sc_in<bool>  clk;
    sc_in<bool>  rst;
    sc_in<bool>  start;
    sc_in<bool>  x_zero;
    sc_in<bool>  y_zero;
    sc_in<bool>  equal_xy;
    sc_in<bool>  x_ge_y;
    sc_out<bool> load;
    sc_out<bool> write_x;
    sc_out<bool> write_y;
    sc_out<bool> swap_ab;
    sc_out<bool> done;

    // Se conservan los numeros de estado del RTL para facilitar
    // la comparacion de las formas de onda entre Verilog y SystemC.
    enum { ESPERA = 0, COMPARAR = 1, RESTAR_X = 2, RESTAR_Y = 3, FIN = 4 };

    sc_signal<sc_uint<3> > state, next_state;

    // Registro de estado (reset sincrono).
    void registro_estado() {
        if (rst.read()) state.write(ESPERA);
        else            state.write(next_state.read());
    }

    // Logica de siguiente estado y salidas (combinacional).
    void logica() {
        sc_uint<3> ns = state.read();
        bool l = false, wx = false, wy = false, sw = false, d = false;

        switch (state.read().to_uint()) {
            case ESPERA:
                // En este ciclo se habilita la carga de x_in/y_in.
                if (start.read()) { l = true; ns = COMPARAR; }
                break;

            case COMPARAR:
                // Al ser cero uno de los registros o X=Y, ya hay resultado.
                if (x_zero.read() || y_zero.read() || equal_xy.read()) ns = FIN;
                // cout de X-Y selecciona que registro se reduce.
                else if (x_ge_y.read())                                ns = RESTAR_X;
                else                                                   ns = RESTAR_Y;
                break;

            case RESTAR_X:                    // X <- X - Y
                wx = true; ns = COMPARAR;
                break;

            case RESTAR_Y:                    // Y <- Y - X
                sw = true; wy = true; ns = COMPARAR;
                break;

            case FIN:
                // done permanece alto hasta aceptar un nuevo start
                // o hasta que rst fuerce el estado ESPERA.
                d = true;
                if (start.read()) { l = true; ns = COMPARAR; }
                break;

            default:
                ns = ESPERA;
                break;
        }

        next_state.write(ns);
        load.write(l);
        write_x.write(wx);
        write_y.write(wy);
        swap_ab.write(sw);
        done.write(d);
    }

    SC_CTOR(gcd_control) {
        SC_METHOD(registro_estado);
        sensitive << clk.pos();
        dont_initialize();

        // Al cambiar una entrada o el estado se recalculan
        // las salidas combinacionales y el siguiente estado.
        SC_METHOD(logica);
        sensitive << state << start << x_zero << y_zero << equal_xy << x_ge_y;
    }
};

#endif
