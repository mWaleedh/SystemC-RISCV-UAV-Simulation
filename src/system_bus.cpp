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
    sc_in<sc_uint<3>> cpu_data_size_i;
    sc_in<sc_uint<WIDTH>> cpu_data_addr_bus_i;
    sc_in<sc_uint<WIDTH>> cpu_data_bus_i;
    sc_out<sc_uint<WIDTH>> cpu_data_bus_o;
    sc_out<bool> cpu_data_ready_o;
    sc_out<bool> cpu_data_error_o;

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

    // UART ports
    sc_out<bool> uart_write_en_o;
    sc_out<bool> uart_read_en_o;
    sc_out<sc_uint<WIDTH>> uart_addr_bus_o;
    sc_out<sc_uint<WIDTH>> uart_data_bus_o;
    sc_in<sc_uint<WIDTH>> uart_data_bus_i;

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

        // Default values
        mem_data_read_en_o.write(false);
        mem_data_write_en_o.write(false);
        gpio_read_en_o.write(false);
        gpio_write_en_o.write(false);
        timer_read_en_o.write(false);
        timer_write_en_o.write(false);
        cpu_data_ready_o.write(false);

        sc_uint<WIDTH> return_data = 0;
        
        // Route bus to memory
        if (data_addr <= 0xFFF) {
            // Forward the control flags and address to memory
            mem_data_read_en_o.write(data_read_en);
            mem_data_write_en_o.write(data_write_en);
            mem_data_addr_bus_o.write(data_addr);
        
            if (data_write_en) {
                // Write data to memory
                mem_data_bus_o.write(cpu_data);
            }
            else if (data_read_en) {
            }

            return_data = mem_data_bus_i.read();
            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to Main Memory" << endl << endl;            
        }
        // Route bus to GPIO
        else if (data_addr == 0x10000000 || data_addr == 0x10000004) {
            // Forward the control flags and address to GPIO
            gpio_read_en_o.write(data_read_en);
            gpio_write_en_o.write(data_write_en);
            gpio_addr_bus_o.write(data_addr);

            if (data_write_en) {
                // Send data to GPIO
                gpio_data_bus_o.write(cpu_data);
            }
            return_data = gpio_data_bus_i.read();
            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to GPIO" << endl << endl;
        }
        // Route bus to Timer
        else if (data_addr == 0x10000010 || data_addr == 0x10000014 || data_addr == 0x10000018 || data_addr == 0x1000001C) {
            // Forward the control flags and address to Timer
            timer_read_en_o.write(data_read_en);
            timer_write_en_o.write(data_write_en);
            timer_data_bus_o.write(data_addr);

            if (data_write_en) {
                // Send data to Timer
                timer_data_bus_o.write(cpu_data);
            }

            return_data = timer_data_bus_i.read();
            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to Timer" << endl << endl;
        }
        // Route bus to UART
        else if (data_addr == 0x10000020 || data_addr == 0x10000024) {
            // Forward the control flags and address to Timer
            uart_read_en_o.write(data_read_en);
            uart_write_en_o.write(data_write_en);
            uart_data_bus_o.write(data_addr);

            if (data_write_en) {
                // Send data to UART
                uart_data_bus_o.write(cpu_data);
            }

            return_data = uart_data_bus_i.read();
            cout << "@" << sc_time_stamp() << " System Bus: Address 0x" << hex << cpu_data_addr_bus_i.read() << dec << " routed to UART" << endl << endl;
        }
        // If system_bus was called but address is invalid print error
        else if (data_write_en || data_read_en) {
            cout << "@" << sc_time_stamp() << " System Bus Error: Invalid address 0x" << hex << cpu_data_addr_bus_i.read() << dec << endl << endl;
        }

        // Return data to the CPU
        if (data_write_en || data_read_en) {
            cpu_data_bus_o.write(return_data);
            cpu_data_ready_o.write(true);
        } 
        else {
            cpu_data_ready_o.write(false);
        }
    }

    SC_CTOR(system_bus) {
        SC_METHOD(routeBus);
        sensitive << cpu_inst_read_en_i << cpu_inst_addr_bus_i << cpu_data_read_en_i << cpu_data_write_en_i << cpu_data_addr_bus_i << cpu_data_bus_i << mem_inst_bus_i << mem_data_bus_i << gpio_data_bus_i << timer_data_bus_i;
    }
};