/* Problems:
   1. One problem I faced was that I wasn't using sc_signal in testbench which was giving an error.
      I tried to rename the signals because they were the same name as the in/out ports in the module, but that didn't work.
      I then went through the examples codes in asic-world and saw that it used sc_signal in the testbench.
      I searched more about this and found out that signals need to be explicitly created using sc_signal.

   2. When I was using sc_start the same was as in asic-world it was giving me an error, when I searched about it, I found out that it was updated in the new verions and we also need to pass the time unit to it. */

#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE(Adder) {
    // input/output ports
    sc_in<sc_uint<4>> num1, num2;
    sc_out<sc_uint<5>> sum;

    // signals/local variables
    sc_uint<4> num1_s, num2_s;
    sc_uint<5> sum_s;

    void addAndPrint() {
        // read input values
        num1_s = num1.read();
        num2_s = num2.read();
        cout << "Num1: " << num1_s << endl;
        cout << "Num2: " << num2_s << endl;

        // sum numbers
        sum_s = num1_s + num2_s;
        cout << "Sum: " << sum_s << endl << endl;

        // write output value
        sum.write(sum_s);
    }

    SC_CTOR(Adder) {
        SC_METHOD(addAndPrint);
        // call method when num1 and num2 change
        sensitive << num1 << num2;
    }
};

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