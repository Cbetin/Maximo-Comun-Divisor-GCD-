# Makefile - GCD en SystemC (practica 02)
# Uso:  make        compila
#       make run    compila y ejecuta (genera gcd.vcd)
#       make wave   abre gcd.vcd en GTKWave
#       make clean
# Si SystemC esta en otra ruta:  make SYSTEMC_HOME=/ruta/a/systemc

# Cambia esta ruta o pasa SYSTEMC_HOME=... al ejecutar make.
# Se esperan include/systemc.h y lib/libsystemc.* (o lib64).
SYSTEMC_HOME ?= /mnt/work/systemc/install

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -I$(SYSTEMC_HOME)/include
LDFLAGS  = -L$(SYSTEMC_HOME)/lib -L$(SYSTEMC_HOME)/lib64 -lsystemc \
           -Wl,-rpath,$(SYSTEMC_HOME)/lib -Wl,-rpath,$(SYSTEMC_HOME)/lib64

# Si cambia cualquiera de las cabeceras, se recompila la simulacion.
HDRS = alu_gcd.h gcd_datapath.h gcd_control.h gcd_top.h tb_gcd.h
SRCS = main.cpp
TARGET = gcd_sim

$(TARGET): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Compila si hace falta y ejecuta las pruebas; produce gcd.vcd.
run: $(TARGET)
	./$(TARGET)

# Abre las ondas (requiere GTKWave con interfaz grafica).
wave: gcd.vcd
	gtkwave gcd.vcd &

gcd.vcd: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) gcd.vcd

.PHONY: run wave clean
