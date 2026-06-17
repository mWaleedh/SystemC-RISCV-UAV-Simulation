/* Problems:

*/

#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(clocked_counter) {
    // input/output ports
    sc_in<bool> clock_i;
    sc_in<bool> reset_i;
    sc_out<sc_uint<4>> counter_o;

    // local variables
    sc_uint<4> count_s;

    void increaseCounter() {
        // update counter value depending on value of reset
        if (reset_i.read() == 1) {
            count_s = 0;
        }
        else {
            count_s++;
        }

        // write value of result to output port
        counter_o.write(count_s);

        // print value of counter
        cout << "Value at " << sc_time_stamp() << ": " << counter_o.read() << endl;
    }

    SC_CTOR(clocked_counter) {
        count_s = 0;

        SC_METHOD(increaseCounter);
        // add clock to sensitivity list of method (function is called every rising edge)
        sensitive << clock_i.pos();
    }
};

int sc_main(int argc, char* argv[]) {
    // signals
    sc_signal<bool> reset_s;
    sc_signal<sc_uint<4>> count_s;
    sc_clock clock_s("counter_clock");  // clock object

    // map signals to input/output ports
    clocked_counter clocked_counter_inst("clocked_counter_inst");
        clocked_counter_inst.clock_i(clock_s);
        clocked_counter_inst.reset_i(reset_s);
        clocked_counter_inst.counter_o(count_s);
    
    // first test reset pin
    cout << "Reset Pin = 1" << endl;
    cout << "-------------" << endl;
    reset_s = 1;
    sc_start(5, SC_NS);    // run clock for 5 ns (5 cycles)

    // now test clock
    cout << "\nReset Pin = 0" << endl;    
    cout << "-------------" << endl;
    reset_s = 0;
    sc_start(10, SC_NS);    // run clock for 10 ns (10 cycles)

    return 0;
}