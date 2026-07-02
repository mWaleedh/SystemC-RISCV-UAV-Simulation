#include <systemc.h>
#include "risc_v_model.cpp"
#include "memory_model.cpp"
using namespace std;

SC_MODULE(system_top) {
    // constants
    static const int WIDTH = 32;
    static const int SIZE = 256;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> irq_timer_i;
    sc_in<bool> irq_ext_i;
    sc_in<bool> irq_sw_i;

    // signals
    sc_signal<bool> write_en_s;
    sc_signal<bool> read_en_s;
    sc_signal<sc_uint<WIDTH>> addr_bus_s;
    sc_signal<sc_uint<WIDTH>> cpu_mem_data_bus_s;
    sc_signal<sc_uint<WIDTH>> mem_cpu_data_bus_s;

    // Module Pointers
    risc_v_model *cpu;
    memory_model *mem;

    // Function to load testbench data into memory
    void load_data(uint32_t addr, uint32_t data) {
        mem->load_data(addr, data);
    }
    
    // Function to load entire program 
    void load_file(const string& filename = "program.hex") {
        mem->load_file(filename);
    }

    SC_CTOR(system_top) {
        // Module instantiations
        cpu = new risc_v_model("cpu");
        mem = new memory_model("memory");

        // Connect input/output ports of CPU
        cpu->clk_i(clk_i);
        cpu->rst_i(rst_i);
        cpu->irq_timer_i(irq_timer_i);
        cpu->irq_ext_i(irq_ext_i);
        cpu->irq_sw_i(irq_sw_i);
        cpu->data_bus_i(mem_cpu_data_bus_s);
        
        cpu->write_en_o(write_en_s);
        cpu->read_en_o(read_en_s);
        cpu->addr_bus_o(addr_bus_s);
        cpu->data_bus_o(cpu_mem_data_bus_s);

        // Connect input/output ports of Memory
        mem->clk_i(clk_i);
        mem->rst_i(rst_i);
        mem->write_en_i(write_en_s);
        mem->read_en_i(read_en_s);
        mem->addr_bus_i(addr_bus_s);
        mem->data_bus_i(cpu_mem_data_bus_s);

        mem->data_bus_o(mem_cpu_data_bus_s);
    }

    ~system_top() {
        delete cpu;
        delete mem;
    }
};