#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(gpio_model) {
    // constants
    static const int WIDTH = 32;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> write_en_i;
    sc_in<bool> read_en_i;
    sc_in<sc_uint<WIDTH>> addr_bus_i;
    sc_in<sc_uint<WIDTH>> data_bus_i;

    // output ports
    sc_out<sc_uint<WIDTH>> data_bus_o;

    // local variables    
    sc_uint<WIDTH> input_reg;
    sc_uint<WIDTH> output_reg;

    // Main thread
    void mainThread() {
        // Reset/initial stage logic
        input_reg = 0;
        output_reg = 0;
        data_bus_o.write(0);

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            if (write_en_i.read()) {
                // Write to output register
                if (addr_bus_i.read() == 0x10000000) {
                    output_reg = data_bus_i.read();

                    // Turn LED ON/OFF based on input
                    if (output_reg == 1) {
                        cout << "@" << sc_time_stamp() << " GPIO: LED ON\n" << endl;
                    }
                    else if (output_reg == 0) {
                        cout << "@" << sc_time_stamp() << " GPIO: LED OFF\n" << endl;
                    }
                    else {
                        cout << "@" << sc_time_stamp() << " GPIO Write: 0x" << hex << output_reg << dec << " written to output register\n" << endl;
                    }
                }
                else {
                    cout << "@" << sc_time_stamp() << " GPIO Error: Write to invalid address 0x" << hex << addr_bus_i.read() << dec << endl << endl;
                }
            }
            else if (read_en_i.read()) {
                // Read from output register
                if (addr_bus_i.read() == 0x10000000) {
                    data_bus_o.write(output_reg);

                    cout << "@" << sc_time_stamp() << " GPIO Read: 0x" << hex << output_reg << dec << " read from output register\n" << endl;
                }
                // Read from input register
                else if (addr_bus_i.read() == 0x10000004) {
                    // Dummy input_reg
                    input_reg = 1;
                    data_bus_o.write(input_reg);

                    cout << "@" << sc_time_stamp() << " GPIO Read: 0x" << hex << input_reg << dec << " read from input register\n" << endl;
                }
                else {
                    cout << "@" << sc_time_stamp() << " GPIO Error: Read from invalid address 0x" << hex << addr_bus_i.read() << dec << endl << endl;
                }
            }
            else {
                data_bus_o.write(0);
            }

            wait();
        }
    }

    SC_CTOR(gpio_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};