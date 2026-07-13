#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(system_bus) {
    // constants
    static const int WIDTH = 32;

    // CPU ports
    sc_in<bool> cpu_write_en_i;
    sc_in<bool> cpu_read_en_i;
    sc_in<sc_uint<WIDTH>> cpu_addr_bus_i;
    sc_in<sc_uint<WIDTH>> cpu_data_bus_i;
    sc_out<sc_uint<WIDTH>> cpu_data_bus_o;

    // Memory ports
    sc_out<bool> mem_write_en_o;
    sc_out<bool> mem_read_en_o;
    sc_out<sc_uint<WIDTH>> mem_addr_bus_o;
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
        bool write_en = cpu_write_en_i.read();
        bool read_en = cpu_read_en_i.read();
        uint32_t addr = cpu_addr_bus_i.read();
        uint32_t cpu_data = cpu_data_bus_i.read();

        mem_read_en_o.write(0);
        mem_write_en_o.write(0);
        gpio_read_en_o.write(0);
        gpio_write_en_o.write(0);
        timer_read_en_o.write(0);
        timer_write_en_o.write(0);
        
        // Route bus to memory
        if (addr >= 0 && addr <= 0xFFF) {
            if (write_en) {
                // Set flag
                mem_write_en_o.write(write_en);

                // Write data and address to memory
                mem_addr_bus_o.write(addr);
                mem_data_bus_o.write(cpu_data);
            }
            else if (read_en) {
                // Set flag
                mem_read_en_o.write(read_en);

                // Write address to memory
                mem_addr_bus_o.write(addr);
            }

            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_addr_bus_i.read() << dec << " routed to Main Memory" << endl << endl;
        }
        // Route bus to GPIO
        else if (addr == 0x10000000 || addr == 0x10000004) {
            if (write_en) {
                // Set flag
                gpio_write_en_o.write(write_en);

                // Write data and address to GPIO
                gpio_addr_bus_o.write(addr);
                gpio_data_bus_o.write(cpu_data);
            }
            else if (read_en) {
                // Set flag
                gpio_read_en_o.write(read_en);

                // Write address to GPIO
                gpio_addr_bus_o.write(addr);
            }

            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_addr_bus_i.read() << dec << " routed to GPIO" << endl << endl;
        }
        // Route bus to Timer
        else if (addr == 0x10000010 || addr == 0x10000014 || 0x10000018 || 0x1000001C) {
            if (write_en) {
                // Set flag
                timer_write_en_o.write(write_en);

                // Write data and address to Timer
                timer_addr_bus_o.write(addr);
                timer_data_bus_o.write(cpu_data);
            }
            else if (read_en) {
                // Set flag
                timer_read_en_o.write(read_en);

                // Write address to Timer
                timer_addr_bus_o.write(addr);
            }

            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_addr_bus_i.read() << dec << " routed to Timer" << endl << endl;
        }
        // If system_bus was called but address is invalid print error
        else if (write_en || read_en) {
            cout << "@" << sc_time_stamp() << " System Bus Error: Invalid address 0x" << hex << cpu_addr_bus_i.read() << dec << endl << endl;
        }

        // Write data to CPU. Only the sending module has their data_bus greater than 0
        cpu_data_bus_o.write(mem_data_bus_i.read() | gpio_data_bus_i.read() | timer_data_bus_i.read());
    }

    SC_CTOR(system_bus) {
        SC_METHOD(routeBus);
        sensitive << cpu_read_en_i << cpu_write_en_i << cpu_addr_bus_i << cpu_data_bus_i << mem_data_bus_i << gpio_data_bus_i << timer_data_bus_i;
    }
};