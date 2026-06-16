#include <systemc.h>
#include "basic_adder.cpp"

int sc_main(int argc, char* argv[]) {
    // signals
    sc_signal<sc_uint<4>> num1, num2;
    sc_signal<sc_uint<5>> sum;

    // instantiate module
    Adder adder_inst("ADDER_INST");
        // connect ports
        adder_inst.num1(num1);
        adder_inst.num2(num2);
        adder_inst.sum(sum);

    // Test #1
    num1.write(5);
    num2.write(10);
    sc_start(10, SC_NS);

    // Test #2
    num1.write(7);
    num2.write(15);
    sc_start(10, SC_NS);

    return 0;
}
