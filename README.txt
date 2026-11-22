GCD en SystemC - practica 02 (version comentada)

Archivos: alu_gcd.h, gcd_datapath.h, gcd_control.h, gcd_top.h,
tb_gcd.h, main.cpp y Makefile. El circuito conserva la logica
del laboratorio 1. Se corrigio el comentario del testbench: el
testbench SystemC ejecuta 30 casos dirigidos y el del laboratorio
1 ejecuta 16 dirigidos mas 20 pseudoaleatorios.

En Linux con SystemC instalado y GTKWave:
    make SYSTEMC_HOME=/ruta/de/instalacion
    make run SYSTEMC_HOME=/ruta/de/instalacion
    make wave SYSTEMC_HOME=/ruta/de/instalacion

Debe existir /ruta/de/instalacion/include/systemc.h y la biblioteca
libsystemc en lib/ o lib64/. La simulacion crea gcd.vcd.

PENDIENTE: programa del GCD a nivel de instrucciones en MIPS o
RISC-V, solicitado en la parte 3 del enunciado.
