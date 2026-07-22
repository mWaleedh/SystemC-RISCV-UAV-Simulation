#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(system_bus) {
    // constants
    static const int WIDTH = 32;

    // CPU instruction ports
    sc_in<bool> cpu_inst_read_en_i;
    sc_in<sc_uint<WIDTH>> cpu_inst_addr_bus_i;
    sc_out<sc_uint<WIDTH>> cpu_inst_bus_o;

    // CPU data ports
    sc_in<bool> cpu_data_write_en_i;
    sc_in<bool> cpu_data_read_en_i;
    sc_in<sc_uint<WIDTH>> cpu_data_addr_bus_i;
    sc_in<sc_uint<WIDTH>> cpu_data_bus_i;
    sc_out<sc_uint<WIDTH>> cpu_data_bus_o;

    // Memory instruction ports
    sc_out<bool> mem_inst_read_en_o;
    sc_out<sc_uint<WIDTH>> mem_inst_addr_bus_o;
    sc_in<sc_uint<WIDTH>> mem_inst_bus_i;

    // Memory data ports
    sc_out<bool> mem_data_write_en_o;
    sc_out<bool> mem_data_read_en_o;
    sc_out<sc_uint<WIDTH>> mem_data_addr_bus_o;
    sc_out<sc_uint<WIDTH>> mem_data_bus_o;
    sc_in<sc_uint<WIDTH>> mem_data_bus_i;

    // GPIO ports
    sc_out<bool> gpio_write_en_o;
    sc_out<bool> gpio_read_en_o;
    sc_out<sc_uint<WIDTH>> gpio_addr_bus_o;
    sc_out<sc_uint<WIDTH>> gpio_data_bus_o;
    sc_in<sc_uint<WIDTH>> gpio_data_bus_i;

    // Timer ports
    sc_out<bool> timer_write_en_o;
    sc_out<bool> timer_read_en_o;
    sc_out<sc_uint<WIDTH>> timer_addr_bus_o;
    sc_out<sc_uint<WIDTH>> timer_data_bus_o;
    sc_in<sc_uint<WIDTH>> timer_data_bus_i;

    // Main thread
    void routeBus() {
        // Instruction memory route
        bool inst_read_en = cpu_inst_read_en_i.read();
        uint32_t inst_addr = cpu_inst_addr_bus_i.read();

        mem_inst_read_en_o.write(false);

        if (inst_read_en) {
            if (inst_addr <= 0xFFF) {
                mem_inst_read_en_o.write(true);
                mem_inst_addr_bus_o.write(inst_addr);
                cout << "@" << sc_time_stamp() << " System Bus (IF): Address 0x" << hex << inst_addr << dec << " routed to Main Memory" << endl << endl;
            }
            else {
                cout << "@" << sc_time_stamp() << " System Bus Error (IF): Invalid address 0x" << hex << inst_addr << dec << endl << endl;
            }
        }

        cpu_inst_bus_o.write(mem_inst_bus_i.read());

        // Data memory route
        bool data_read_en = cpu_data_read_en_i.read();
        bool data_write_en = cpu_data_write_en_i.read();
        uint32_t data_addr = cpu_data_addr_bus_i.read();
        uint32_t cpu_data = cpu_data_bus_i.read();

        mem_data_read_en_o.write(0);
        mem_data_write_en_o.write(0);
        gpio_read_en_o.write(0);
        gpio_write_en_o.write(0);
        timer_read_en_o.write(0);
        timer_write_en_o.write(0);

        sc_uint<WIDTH> return_data = 0;
        
        // Route bus to memory
        if (data_addr <= 0xFFF) {
            if (data_write_en) {
                // Set flag
                mem_data_write_en_o.write(data_write_en);

                // Write data and address to memory
                mem_data_addr_bus_o.write(data_addr);
                mem_data_bus_o.write(cpu_data);

                cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to Main Memory" << endl << endl;
            }
            else if (data_read_en) {
                // Set flag
                mem_data_read_en_o.write(data_read_en);

                // Write address to memory
                mem_data_addr_bus_o.write(data_addr);

                cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to Main Memory" << endl << endl;
            }
            return_data = mem_data_bus_i.read();
        }
        // Route bus to GPIO
        else if (data_addr == 0x10000000 || data_addr == 0x10000004) {
            if (data_write_en) {
                // Set flag
                gpio_write_en_o.write(data_write_en);

                // Write data and address to GPIO
                gpio_addr_bus_o.write(data_addr);
                gpio_data_bus_o.write(cpu_data);
            }
            else if (data_read_en) {
                // Set flag
                gpio_read_en_o.write(data_read_en);

                // Write address to GPIO
                gpio_addr_bus_o.write(data_addr);
            }
            return_data = gpio_data_bus_i.read();

            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to GPIO" << endl << endl;
        }
        // Route bus to Timer
        else if (data_addr == 0x10000010 || data_addr == 0x10000014 || data_addr == 0x10000018 || data_addr == 0x1000001C) {
            if (data_write_en) {
                // Set flag
                timer_write_en_o.write(data_write_en);

                // Write data and address to Timer
                timer_addr_bus_o.write(data_addr);
                timer_data_bus_o.write(cpu_data);
            }
            else if (data_read_en) {
                // Set flag
                timer_read_en_o.write(data_read_en);

                // Write address to Timer
                timer_addr_bus_o.write(data_addr);
            }
            return_data = timer_data_bus_i.read();

            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to Timer" << endl << endl;
        }
        // If system_bus was called but address is invalid print error
        else if (data_write_en || data_read_en) {
            cout << "@" << sc_time_stamp() << " System Bus Error: Invalid address 0x" << hex << cpu_data_addr_bus_i.read() << dec << endl << endl;
        }

        // Write data to CPU. Only the sending module has their data_bus greater than 0
        cpu_data_bus_o.write(return_data);
    }

    SC_CTOR(system_bus) {
        SC_METHOD(routeBus);
        sensitive << cpu_inst_read_en_i << cpu_inst_addr_bus_i << cpu_data_read_en_i << cpu_data_write_en_i << cpu_data_addr_bus_i << cpu_data_bus_i << mem_inst_bus_i << mem_data_bus_i << gpio_data_bus_i << timer_data_bus_i;
    }
};