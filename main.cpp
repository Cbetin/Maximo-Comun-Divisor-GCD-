// Punto de entrada de la simulacion: conecta el circuito (DUT)
// y el banco de pruebas, genera reloj y guarda formas de onda VCD.
#include "gcd_top.h"
#include "tb_gcd.h"

int sc_main(int argc, char* argv[]) {
    sc_set_time_resolution(1, SC_PS);

    // Un ciclo de reloj dura 10 ns, igual que en tb_gcd_r.v.
    sc_clock                        clk("clk", 10, SC_NS);   // periodo 10 ns
    sc_signal<bool>                 rst, start, done;
    sc_signal<sc_uint<GCD_WIDTH> >  x_in, y_in, gcd;

    gcd_top dut("dut");
    dut.clk(clk);  dut.rst(rst);  dut.start(start);
    dut.x_in(x_in); dut.y_in(y_in);
    dut.gcd(gcd);  dut.done(done);

    tb_gcd tb("tb");
    tb.clk(clk);  tb.rst(rst);  tb.start(start);
    tb.x_in(x_in); tb.y_in(y_in);
    tb.gcd(gcd);  tb.done(done);

    // El archivo gcd.vcd se abre con GTKWave. Ademas de las
    // entradas y salidas, se registran X, Y, S y el estado
    // para observar cada iteracion del algoritmo.
    // Trazas VCD: entradas/salidas y senales internas (X, Y, S, estado).
    sc_trace_file* tf = sc_create_vcd_trace_file("gcd");
    sc_trace(tf, clk,   "clk");
    sc_trace(tf, rst,   "rst");
    sc_trace(tf, start, "start");
    sc_trace(tf, x_in,  "x_in");
    sc_trace(tf, y_in,  "y_in");
    sc_trace(tf, gcd,   "gcd");
    sc_trace(tf, done,  "done");
    sc_trace(tf, dut.u_datapath.x_reg,  "X");
    sc_trace(tf, dut.u_datapath.y_reg,  "Y");
    sc_trace(tf, dut.u_datapath.alu_y,  "S");
    sc_trace(tf, dut.u_control.state,   "estado");
    sc_trace(tf, dut.load,    "load");
    sc_trace(tf, dut.write_x, "write_x");
    sc_trace(tf, dut.write_y, "write_y");
    sc_trace(tf, dut.swap_ab, "swap_ab");

    sc_start();   // termina cuando el testbench llama sc_stop()

    sc_close_vcd_trace_file(tf);
    // Codigo de salida distinto de cero si fallo alguna comprobacion.
    return (tb.errores == 0) ? 0 : 1;
}
