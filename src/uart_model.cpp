#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(uart_model) {
    // constants
    static const int WIDTH = 32;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;

    // Bus ports and flags
    sc_in<sc_uint<WIDTH>> addr_bus_i;
    sc_in<sc_uint<WIDTH>> data_bus_i;
    sc_in<bool> write_en_i;
    sc_in<bool> read_en_i;

    // output ports
    sc_out<sc_uint<32>> data_bus_o;

    // local variables
    int busy_cycles;
    bool tx_ready;    

    void mainThread() {
        // Reset State
        busy_cycles = 0;
        tx_ready = true;
        data_bus_o.write(0);

        // Wait marking end of reset
        wait();

        while (true) {
            // Transmission delay
            if (busy_cycles > 0) {
                busy_cycles--;

                if (busy_cycles == 0) {
                    tx_ready = true;
                }
            }

            // Handle Memory Write
            if (write_en_i.read() == true) {
                sc_uint<WIDTH> addr = addr_bus_i.read();
                
                if (addr == 0x10000020) {
                    if (tx_ready) {
                        // Extract first 8 bits
                        char c = (char)(data_bus_i.read() & 0xFF);
                        
                        // Print the character to terminal
                        cout << "UART TX: " << c << endl << endl;
                        
                        // Simulate transmission time
                        tx_ready = false;
                        busy_cycles = 5;
                    } 
                    else {
                        cout << "@" << sc_time_stamp() << " UART Warning: Dropped character. TX is busy" << std::endl;
                    }
                }
            }

            // Handle Memory Read (Check status)
            if (read_en_i.read() == true) {
                sc_uint<WIDTH> addr = addr_bus_i.read();
                
                if (addr == 0x10000024) {
                    // Lsb of tx_ready tells whether UART is ready or not
                    data_bus_o.write(tx_ready ? 1 : 0);
                } 
                else {
                    data_bus_o.write(0);
                }
            } 
            else {
                // Clear output bus when not reading
                data_bus_o.write(0);
            }

            wait();
        }
    }

    SC_CTOR(uart_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};