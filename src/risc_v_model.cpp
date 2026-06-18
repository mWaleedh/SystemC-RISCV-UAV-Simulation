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
    sc_out<bool> write_en_o;
    sc_out<bool> read_en_o;
    sc_out<sc_uint<WIDTH>> addr_bus_o;
    sc_out<sc_uint<WIDTH>> data_bus_o;

    // local variables
    sc_uint<WIDTH> pc;
    sc_uint<WIDTH> cur_inst;
    sc_uint<WIDTH> registers[WIDTH];

    // IF: Instruction Fetch
    // ------------------------------
    void fetch() {

    }

    // ID: Instruction Decode
    // ------------------------------
    void decode() {

    }

    // EX: Instruction Execute
    // ------------------------------
    void execute() {

    }

    // MEM: Memory/Peripheral Access
    // ------------------------------
    void memoryAccess() {

    }

    // WB: Write Back
    // ------------------------------
    void writeBack() {

    }

    // Main Thread
    // ------------------------------
    void mainThread() {
        // reset/initial state logic
        pc = 0;
        cur_inst = 0;
        for (int i = 0; i < WIDTH; i++) {
            registers[i] = 0;
        }

        addr_bus_o.write(0);
        data_bus_o.write(0);

        // wait marking end of reset part
        wait();

        // main loop
        while (true) {
            // reset flags to default before executing each instruction
            write_en_o.write(false);
            read_en_o.write(false);

        }
    }

    SC_CTOR(risc_v_model) {
        SC_CTHREAD(mainThread, clk_i);
        reset_signal_is(rst_i, true);
    }
};

// Test Bench
// ------------------------------
int sc_main(int argc, char* argv[]) {

    return 0;
}