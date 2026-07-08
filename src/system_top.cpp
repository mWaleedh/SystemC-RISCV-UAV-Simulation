#include <systemc.h>
#include "risc_v_model.cpp"
#include "memory_model.cpp"
#include "gpio_model.cpp"
#include "system_bus.cpp"
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

    // CPU signals
    sc_signal<bool> cpu_write_en_s;
    sc_signal<bool> cpu_read_en_s;
    sc_signal<sc_uint<WIDTH>> cpu_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_in_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_out_s;

    // Memory signals
    sc_signal<bool> mem_write_en_s;
    sc_signal<bool> mem_read_en_s;
    sc_signal<sc_uint<WIDTH>> mem_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> mem_data_in_s;
    sc_signal<sc_uint<WIDTH>> mem_data_out_s;

    // CPU signals
    sc_signal<bool> gpio_write_en_s;
    sc_signal<bool> gpio_read_en_s;
    sc_signal<sc_uint<WIDTH>> gpio_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> gpio_data_in_s;
    sc_signal<sc_uint<WIDTH>> gpio_data_out_s;

    // Module Pointers
    risc_v_model *cpu;
    memory_model *mem;
    gpio_model *gpio;
    system_bus *bus;

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
        gpio = new gpio_model("gpio");
        bus = new system_bus("bus");

        // Connect CPU input/output ports
        cpu->clk_i(clk_i);
        cpu->rst_i(rst_i);
        cpu->irq_timer_i(irq_timer_i);
        cpu->irq_ext_i(irq_ext_i);
        cpu->irq_sw_i(irq_sw_i);

        cpu->write_en_o(cpu_write_en_s);
        cpu->read_en_o(cpu_read_en_s);
        cpu->addr_bus_o(cpu_addr_bus_s);
        cpu->data_bus_i(cpu_data_in_s);
        cpu->data_bus_o(cpu_data_out_s);

        // Connect Memory input/output ports
        mem->clk_i(clk_i);
        mem->rst_i(rst_i);

        mem->write_en_i(mem_write_en_s);
        mem->read_en_i(mem_read_en_s);
        mem->addr_bus_i(mem_addr_bus_s);
        mem->data_bus_i(mem_data_in_s);
        mem->data_bus_o(mem_data_out_s);

        // Connect GPIO input/output ports
        gpio->clk_i(clk_i);
        gpio->rst_i(rst_i);

        gpio->write_en_i(gpio_write_en_s);
        gpio->read_en_i(gpio_read_en_s);
        gpio->addr_bus_i(gpio_addr_bus_s);
        gpio->data_bus_i(gpio_data_in_s);
        gpio->data_bus_o(gpio_data_out_s);
    
        // Connect System Bus input/output ports
        // CPU
        bus->cpu_write_en_i(cpu_write_en_s);
        bus->cpu_read_en_i(cpu_read_en_s);
        bus->cpu_addr_bus_i(cpu_addr_bus_s);
        bus->cpu_data_bus_i(cpu_data_out_s);
        bus->cpu_data_bus_o(cpu_data_in_s);
        // Memory
        bus->mem_write_en_o(mem_write_en_s);
        bus->mem_read_en_o(mem_read_en_s);
        bus->mem_addr_bus_o(mem_addr_bus_s);
        bus->mem_data_bus_o(mem_data_in_s);
        bus->mem_data_bus_i(mem_data_out_s);
        // GPIO
        bus->gpio_write_en_o(gpio_write_en_s);
        bus->gpio_read_en_o(gpio_read_en_s);
        bus->gpio_addr_bus_o(gpio_addr_bus_s);
        bus->gpio_data_bus_o(gpio_data_in_s);
        bus->gpio_data_bus_i(gpio_data_out_s);
    }

    ~system_top() {
        delete cpu;
        delete mem;
        delete gpio;
        delete bus;
    }
};