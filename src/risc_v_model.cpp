#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(risc_v_model) {
    // constants
    static const int WIDTH = 32;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<sc_uint<WIDTH>> data_bus_i;

    // output ports
    sc_out<bool> write_en;
    sc_out<bool> read_en;
    sc_out<sc_uint<WIDTH>> addr_bus_o;
    sc_out<sc_uint<WIDTH>> data_bus_o;

    // local variables
    sc_uint<WIDTH> pc;
    sc_uint<WIDTH> cur_inst;
    sc_uint<WIDTH> registers[WIDTH];

    void fetch() {

    }

    void decode() {

    }

    void execute() {

    }

    void writeBack() {

    }

    void mainThread() {
        pc = 0;
        cur_inst = 0;
        for (int i = 0; i < WIDTH; i++) {
            registers[i] = 0;
        }

        write_en.write(false);
        read_en.write(false);

        addr_bus_o.write(0);
        data_bus_o.write(0);

        wait();

        while (true) {

        }
    }

    SC_CTOR(risc_v_model) {
        SC_CTHREAD(mainThread, clk_i);
        reset_signal_is(rst_i, true);
    }
};

int sc_main(int argc, char* argv[]) {

    return 0;
}