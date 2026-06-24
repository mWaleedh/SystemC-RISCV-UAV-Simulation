#include <iostream>
#include <systemc.h>

SC_MODULE(memory_model) {
    // constants
    static const int WIDTH = 32;
    static const int SIZE = 256;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_out<bool> write_en_i;
    sc_out<bool> read_en_i;
    sc_out<sc_uint<WIDTH>> addr_bus_i;
    sc_out<sc_uint<WIDTH>> data_bus_i;

    // output ports
    sc_out<sc_uint<WIDTH>> data_bus_o;

    // local variables
    sc_uint<WIDTH> memory[SIZE];

    void mainThread() {
        // Reset/initial stage logic
        for(int i = 0; i < SIZE; i++) {
            memory[i] = 0;
        }
        data_bus_o.write(0);

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            if (addr_bus_i.read() > SIZE - 1) {
                if (read_en_i.read() == true) {
                    data_bus_o.write(0);
                }
                cout << "@" << sc_time_stamp() << " Error: Trying to access invalid memory 0x" << hex << addr_bus_i.read() << dec << endl << endl;
            }
            else {
                if (write_en_i.read() == true) {
                    memory[addr_bus_i.read()] = data_bus_i.read();
                    cout << "@" << sc_time_stamp() << " Write: " << endl;
                    cout << "1. Address -> 0x" << hex << addr_bus_i.read() << endl;
                    cout << "2. Data -> 0x" << data_bus_i.read() << dec << endl << endl;
                }

                if (read_en_i.read() == true) {
                    data_bus_o.write(memory[addr_bus_i.read()]);
                    cout << "@" << sc_time_stamp() << " Read: " << endl;
                    cout << "1. Address -> 0x" << hex << addr_bus_i.read() << endl;
                    cout << "2. Data -> 0x" << data_bus_o.read() << dec << endl << endl;
                }
                else {
                    data_bus_o.write(0);
                }
            }

            wait();
        }
    }

    SC_CTOR(memory_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};