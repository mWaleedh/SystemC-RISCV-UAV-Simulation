#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(risc_v_model) {
    // constants
    static const int WIDTH = 32;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> irq_timer_i;
    sc_in<bool> irq_ext_i;
    sc_in<bool> irq_sw_i;
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

        write_en_o.write(false);
        read_en_o.write(false);
        addr_bus_o.write(0);
        data_bus_o.write(0);

        // wait marking end of reset part
        wait();

        // main loop
        while (true) {
            // reset flags to default before executing each instruction
            write_en_o.write(false);
            read_en_o.write(false);

            fetch();

            decode();

            execute();
            
            memoryAccess();

            writeBack();

            wait();
        }
    }

    SC_CTOR(risc_v_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};

// Test Bench
// ------------------------------
int sc_main(int argc, char* argv[]) {
    sc_clock clk_s("clk");

    sc_signal<bool> rst_s;
    sc_signal<bool> irq_timer_s;
    sc_signal<bool> irq_ext_s;
    sc_signal<bool> irq_sw_s;
    sc_signal<sc_uint<32>> data_bus_i_s;
    
    sc_signal<bool> write_en_s;
    sc_signal<bool> read_en_s;
    sc_signal<sc_uint<32>> addr_bus_o_s;
    sc_signal<sc_uint<32>> data_bus_o_s;

    risc_v_model cpu("cpu");

    cpu.clk_i(clk_s);
    cpu.rst_i(rst_s);
    cpu.irq_timer_i(irq_timer_s);
    cpu.irq_ext_i(irq_ext_s);
    cpu.irq_sw_i(irq_sw_s);
    cpu.data_bus_i(data_bus_i_s);
    
    cpu.write_en_o(write_en_s);
    cpu.read_en_o(read_en_s);
    cpu.addr_bus_o(addr_bus_o_s);
    cpu.data_bus_o(data_bus_o_s);

    irq_timer_s.write(false);
    irq_ext_s.write(false);
    irq_sw_s.write(false);
    data_bus_i_s.write(0);
    
    cout << "@" << sc_time_stamp() << " Resetting CPU..." << endl;
    rst_s.write(true);

    sc_start(2, SC_NS); 

    cout << "@" << sc_time_stamp() << " Starting Execution..." << endl;
    rst_s.write(false);

    sc_start(10, SC_NS);

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}