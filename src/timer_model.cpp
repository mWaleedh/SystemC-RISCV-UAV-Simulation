#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(timer_model) {
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
    sc_out<bool> irq_timer_o;

    // local variables
    sc_uint<WIDTH> count_reg;
    sc_uint<WIDTH> compare_reg;
    sc_uint<WIDTH> control_reg;
    sc_uint<WIDTH> status_reg;

    // Main thread
    void mainThread() {
        // Reset/initial stage logic
        count_reg = 0;
        compare_reg = 0xFFFFFFFF;
        control_reg = 0;
        status_reg = 0;

        irq_timer_o.write(false);
        data_bus_o.write(0);

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            uint32_t addr = addr_bus_i.read();
            bool write_en = write_en_i.read();
            bool read_en = read_en_i.read();

            if (write_en) {
                switch(addr) {
                // Count register address
                case 0x10000010: 
                    count_reg = data_bus_i.read();
                    break;
                // Compare register address
                case 0x10000014: 
                    compare_reg = data_bus_i.read();
                    break;
                // Control register address
                case 0x10000018: 
                    control_reg = data_bus_i.read();
                    break;
                // Status register address
                case 0x1000001C: 
                    status_reg = data_bus_i.read();
                    
                    // Clear interrupt if CPU sends 0
                    if (status_reg == 0) {
                        irq_timer_o.write(false);
                    }
                    break;
                default:
                    cout << "@" << sc_time_stamp() << " Timer Error: Writing to invalid address 0x" << hex << addr << dec << endl << endl;
                    break;
                }

                cout << "@" << sc_time_stamp() << " Timer: Wrote 0x" << hex << data_bus_i.read() << " to address 0x" << addr << dec << endl << endl;
            }
            else if (read_en) {
                switch(addr) {
                // Count register address
                case 0x10000010: 
                    data_bus_o.write(count_reg);
                    break;
                // Compare register address
                case 0x10000014: 
                    data_bus_o.write(compare_reg);
                    break;
                // Control register address
                case 0x10000018: 
                    data_bus_o.write(control_reg);
                    break;
                // Status register address
                case 0x1000001C: 
                    data_bus_o.write(status_reg);
                    break;
                default:
                    data_bus_o.write(0);
                    cout << "@" << sc_time_stamp() << " Timer Error: Reading from invalid address 0x" << hex << addr << dec << endl << endl;
                    break;
                }

                cout << "@" << sc_time_stamp() << " Timer: Read from address 0x" << hex << addr << dec << endl << endl;
            }
            else {
                data_bus_o.write(0);
            }

            // If last bit of control_reg is 1 the timer is activated
            if ((control_reg & 0x1) == 1) {
                // Increment counter
                count_reg++;

                // Raise interrupt if count has reached threshold
                if (count_reg >= compare_reg) {
                    status_reg = 1;
                    irq_timer_o.write(true);
                }
            }

            wait();
        }
    }

    SC_CTOR(timer_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};