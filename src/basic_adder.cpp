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