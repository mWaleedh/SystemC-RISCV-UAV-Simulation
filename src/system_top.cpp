#include <systemc.h>
#include "risc_v_model.cpp"
#include "memory_model.cpp"
#include "gpio_model.cpp"
#include "system_bus.cpp"
#include "timer_model.cpp"
using namespace std;

SC_MODULE(system_top) {
    // constants
    static const int WIDTH = 32;
    static const int SIZE = 256;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> irq_ext_i;
    sc_in<bool> irq_sw_i;

    // CPU instruction signals
    sc_signal<bool> cpu_inst_read_en_s;
    sc_signal<sc_uint<WIDTH>> cpu_inst_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> cpu_inst_bus_in_s;

    // CPU data signals
    sc_signal<bool> cpu_data_write_en_s;
    sc_signal<bool> cpu_data_read_en_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_bus_out_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_bus_in_s;  

    sc_signal<bool> irq_timer_s;

    // Memory instruction signals
    sc_signal<bool> mem_inst_read_en_s;
    sc_signal<sc_uint<WIDTH>> mem_inst_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> mem_inst_bus_out_s;

    // Memory data signals
    sc_signal<bool> mem_data_write_en_s;
    sc_signal<bool> mem_data_read_en_s;
    sc_signal<sc_uint<WIDTH>> mem_data_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> mem_data_bus_in_s;
    sc_signal<sc_uint<WIDTH>> mem_data_bus_out_s;

    // GPIO signals
    sc_signal<bool> gpio_write_en_s;
    sc_signal<bool> gpio_read_en_s;
    sc_signal<sc_uint<WIDTH>> gpio_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> gpio_data_in_s;
    sc_signal<sc_uint<WIDTH>> gpio_data_out_s;

    // Timer signals
    sc_signal<bool> timer_write_en_s;
    sc_signal<bool> timer_read_en_s;
    sc_signal<sc_uint<WIDTH>> timer_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> timer_data_in_s;
    sc_signal<sc_uint<WIDTH>> timer_data_out_s;

    // Module Pointers
    risc_v_model *cpu;
    memory_model *mem;
    gpio_model *gpio;
    system_bus *bus;
    timer_model *timer;

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
        timer = new timer_model("timer");

        // Connect CPU input/output ports
        cpu->clk_i(clk_i);
        cpu->rst_i(rst_i);
        cpu->irq_timer_i(irq_timer_s);
        cpu->irq_ext_i(irq_ext_i);
        cpu->irq_sw_i(irq_sw_i);

        // CPU instruction port
        cpu->inst_read_en_o(cpu_inst_read_en_s);
        cpu->inst_addr_bus_o(cpu_inst_addr_bus_s);
        cpu->inst_bus_i(cpu_inst_bus_in_s);

        // CPU data port
        cpu->data_write_en_o(cpu_data_write_en_s);
        cpu->data_read_en_o(cpu_data_read_en_s);
        cpu->data_addr_bus_o(cpu_data_addr_bus_s);
        cpu->data_bus_o(cpu_data_bus_out_s);
        cpu->data_bus_i(cpu_data_bus_in_s);

        // Connect Memory input/output ports
        mem->clk_i(clk_i);
        mem->rst_i(rst_i);

        // Memory instruction port
        mem->inst_read_en_i(mem_inst_read_en_s);
        mem->inst_addr_bus_i(mem_inst_addr_bus_s);
        mem->inst_bus_o(mem_inst_bus_out_s);

        // Memory data port
        mem->data_write_en_i(mem_data_write_en_s);
        mem->data_read_en_i(mem_data_read_en_s);
        mem->data_addr_bus_i(mem_data_addr_bus_s);
        mem->data_bus_i(mem_data_bus_in_s);
        mem->data_bus_o(mem_data_bus_out_s);

        // Connect GPIO input/output ports
        gpio->clk_i(clk_i);
        gpio->rst_i(rst_i);

        gpio->write_en_i(gpio_write_en_s);
        gpio->read_en_i(gpio_read_en_s);
        gpio->addr_bus_i(gpio_addr_bus_s);
        gpio->data_bus_i(gpio_data_in_s);
        gpio->data_bus_o(gpio_data_out_s);

        // Connect Timer input/output ports
        timer->clk_i(clk_i);
        timer->rst_i(rst_i);

        timer->write_en_i(timer_write_en_s);
        timer->read_en_i(timer_read_en_s);
        timer->addr_bus_i(timer_addr_bus_s);
        timer->data_bus_i(timer_data_in_s);
        timer->data_bus_o(timer_data_out_s);

        timer->irq_timer_o(irq_timer_s);
    
        // Connect System Bus input/output ports
        // CPU instruction ports
        bus->cpu_inst_read_en_i(cpu_inst_read_en_s);
        bus->cpu_inst_addr_bus_i(cpu_inst_addr_bus_s);
        bus->cpu_inst_bus_o(cpu_inst_bus_in_s);

        // CPU data ports
        bus->cpu_data_write_en_i(cpu_data_write_en_s);
        bus->cpu_data_read_en_i(cpu_data_read_en_s);
        bus->cpu_data_addr_bus_i(cpu_data_addr_bus_s);
        bus->cpu_data_bus_i(cpu_data_bus_out_s);
        bus->cpu_data_bus_o(cpu_data_bus_in_s);

        // Memory instruction ports
        bus->mem_inst_read_en_o(mem_inst_read_en_s);
        bus->mem_inst_addr_bus_o(mem_inst_addr_bus_s);
        bus->mem_inst_bus_i(mem_inst_bus_out_s);

        // Memory data ports
        bus->mem_data_write_en_o(mem_data_write_en_s);
        bus->mem_data_read_en_o(mem_data_read_en_s);
        bus->mem_data_addr_bus_o(mem_data_addr_bus_s);
        bus->mem_data_bus_o(mem_data_bus_in_s);
        bus->mem_data_bus_i(mem_data_bus_out_s);

        // GPIO
        bus->gpio_write_en_o(gpio_write_en_s);
        bus->gpio_read_en_o(gpio_read_en_s);
        bus->gpio_addr_bus_o(gpio_addr_bus_s);
        bus->gpio_data_bus_o(gpio_data_in_s);
        bus->gpio_data_bus_i(gpio_data_out_s);

        // Timer
        bus->timer_write_en_o(timer_write_en_s);
        bus->timer_read_en_o(timer_read_en_s);
        bus->timer_addr_bus_o(timer_addr_bus_s);
        bus->timer_data_bus_o(timer_data_in_s);
        bus->timer_data_bus_i(timer_data_out_s);
    }

    ~system_top() {
        delete cpu;
        delete mem;
        delete gpio;
        delete bus;
        delete timer;
    }
};