#include <systemc.h>

// CPU module
#include "risc_v_model.cpp"

// Legacy modules
#include "memory_model.cpp"
#include "gpio_model.cpp"
#include "timer_model.cpp"
#include "uart_model.cpp"
#include "system_bus.cpp"

// TLM modules
#include "cpu_tlm_adapter.cpp"
#include "tlm_interconnect.cpp"
#include "tlm_memory_target.cpp"
#include "tlm_gpio_target.cpp"
#include "tlm_timer_target.cpp"
#include "tlm_uart_target.cpp"
#include "tlm_msip_target.cpp"

using namespace std;

struct system_config {
    bool use_tlm = true;
    uint32_t mem_size_bytes = 4096;

    // Memory and peripheral latencies 
    sc_time mem_read_lat = SC_ZERO_TIME;
    sc_time mem_write_lat = SC_ZERO_TIME;
    sc_time gpio_lat = SC_ZERO_TIME;
    sc_time timer_lat = SC_ZERO_TIME;
    sc_time uart_lat = SC_ZERO_TIME;
    sc_time msip_lat = SC_ZERO_TIME;
    
    // Internal hardware delays
    uint32_t uart_tx_delay = 0;

    // Memory map base addresses
    uint32_t mem_base = 0x00000000;
    uint32_t gpio_base = 0x10000000;
    uint32_t timer_base = 0x10000010;
    uint32_t uart_base = 0x10000020;
    uint32_t msip_base = 0x10000028;
};

SC_MODULE(system_top) {
    // constants
    static const int WIDTH = 32;
    static const int SIZE = 256;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> irq_ext_i;
    
    // Config variable
    system_config config;

    // CPU instruction signals
    sc_signal<bool> cpu_inst_read_en_s;
    sc_signal<sc_uint<WIDTH>> cpu_inst_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> cpu_inst_bus_in_s;

    // CPU data signals
    sc_signal<bool> cpu_data_ready_s;
    sc_signal<bool> cpu_data_error_s;
    sc_signal<bool> cpu_data_write_en_s;
    sc_signal<bool> cpu_data_read_en_s;
    sc_signal<sc_uint<3>> cpu_data_size_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_bus_out_s;
    sc_signal<sc_uint<WIDTH>> cpu_data_bus_in_s;  

    // Interrupt signals
    sc_signal<bool> irq_timer_s;
    sc_signal<bool> irq_sw_s;

    // Legacy Memory instruction signals
    sc_signal<bool> mem_inst_read_en_s;
    sc_signal<sc_uint<WIDTH>> mem_inst_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> mem_inst_bus_out_s;

    // Legacy Memory data signals
    sc_signal<bool> mem_data_write_en_s;
    sc_signal<bool> mem_data_read_en_s;
    sc_signal<sc_uint<WIDTH>> mem_data_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> mem_data_bus_in_s;
    sc_signal<sc_uint<WIDTH>> mem_data_bus_out_s;

    // Legacy GPIO signals
    sc_signal<bool> gpio_write_en_s;
    sc_signal<bool> gpio_read_en_s;
    sc_signal<sc_uint<WIDTH>> gpio_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> gpio_data_in_s;
    sc_signal<sc_uint<WIDTH>> gpio_data_out_s;

    // Legacy Timer signals
    sc_signal<bool> timer_write_en_s;
    sc_signal<bool> timer_read_en_s;
    sc_signal<sc_uint<WIDTH>> timer_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> timer_data_in_s;
    sc_signal<sc_uint<WIDTH>> timer_data_out_s;

    // Legacy UART signals
    sc_signal<bool> uart_write_en_s;
    sc_signal<bool> uart_read_en_s;
    sc_signal<sc_uint<WIDTH>> uart_addr_bus_s;
    sc_signal<sc_uint<WIDTH>> uart_data_in_s;
    sc_signal<sc_uint<WIDTH>> uart_data_out_s;

    // CPU pointer
    risc_v_model *cpu;
    
    // TLM module pointers
    tlm_memory_target* tlm_mem;
    tlm_gpio_target* tlm_gpio;
    tlm_timer_target* tlm_timer;
    tlm_uart_target* tlm_uart;
    tlm_msip_target* tlm_msip;
    tlm_adapter *adapter;
    tlm_interconnect* interconnect;

    // Legacy module pointers
    memory_model *mem;
    gpio_model *gpio;
    timer_model *timer;
    uart_model *uart;
    system_bus *bus;

    // Function to load testbench data into memory
    void load_data(uint32_t addr, uint32_t data) {
        if (config.use_tlm) {
            tlm_mem->load_data(addr, data);
        } 
        else {
            mem->load_data(addr, data);
        }
    }
    
    // Function to load entire program 
    void load_file(const string& filename = "program.hex") {
        if (config.use_tlm) {
            tlm_mem->load_file(filename);
        } 
        else {
            mem->load_file(filename);
        }
    }

    // Constructor
    SC_HAS_PROCESS(system_top);
    system_top(sc_module_name name, const system_config& cfg = system_config()) : sc_module(name) {
        this->config = cfg;

        // CPU instantiation
        cpu = new risc_v_model("cpu");

        // Connect Clock and Reset to CPU
        cpu->clk_i(clk_i);
        cpu->rst_i(rst_i);

        // Connect CPU interrupt ports
        cpu->irq_timer_i(irq_timer_s);
        cpu->irq_sw_i(irq_sw_s);
        cpu->irq_ext_i(irq_ext_i);        

        // CPU instruction port
        cpu->inst_read_en_o(cpu_inst_read_en_s);
        cpu->inst_addr_bus_o(cpu_inst_addr_bus_s);
        cpu->inst_bus_i(cpu_inst_bus_in_s);

        // CPU data port
        cpu->data_write_en_o(cpu_data_write_en_s);
        cpu->data_read_en_o(cpu_data_read_en_s);
        cpu->data_addr_bus_o(cpu_data_addr_bus_s);
        cpu->data_size_o(cpu_data_size_s);
        cpu->data_bus_o(cpu_data_bus_out_s);
        cpu->data_ready_i(cpu_data_ready_s);
        cpu->data_bus_i(cpu_data_bus_in_s);
        cpu->data_error_i(cpu_data_error_s);
    
        // Use TLM modules
        if (config.use_tlm) {
            // TLM Peripheral instantiation 
            tlm_mem = new tlm_memory_target("tlm_mem", config.mem_size_bytes, config.mem_read_lat, config.mem_write_lat);
            tlm_gpio = new tlm_gpio_target("tlm_gpio", config.gpio_lat);
            tlm_timer = new tlm_timer_target("tlm_timer", config.timer_lat);
            tlm_uart = new tlm_uart_target("tlm_uart", config.uart_tx_delay, config.uart_lat);
            tlm_msip = new tlm_msip_target("tlm_msip", config.msip_lat);

            // TLM 
            adapter = new tlm_adapter("adapter");
            interconnect = new tlm_interconnect("interconnect", config.mem_base, config.gpio_base, config.timer_base, config.uart_base, config.msip_base);
            
            // Connect Clock/Reset of TLM modules
            tlm_mem->clk_i(clk_i);
            tlm_mem->rst_i(rst_i);
            tlm_gpio->rst_i(rst_i);
            tlm_timer->clk_i(clk_i);
            tlm_timer->rst_i(rst_i);
            tlm_uart->clk_i(clk_i);
            tlm_uart->rst_i(rst_i);
            tlm_msip->clk_i(clk_i);
            tlm_msip->rst_i(rst_i);
            adapter->clk_i(clk_i);

            // Connect CPU data ports to adapter
            adapter->cpu_data_write_en_i(cpu_data_write_en_s);
            adapter->cpu_data_read_en_i(cpu_data_read_en_s);
            adapter->cpu_data_addr_bus_i(cpu_data_addr_bus_s);
            adapter->cpu_data_bus_i(cpu_data_bus_out_s);
            adapter->cpu_data_ready_o(cpu_data_ready_s);
            adapter->cpu_data_bus_o(cpu_data_bus_in_s);
            adapter->cpu_data_size_i(cpu_data_size_s);
            adapter->cpu_data_error_o(cpu_data_error_s);

            // Connect CPU instruction ports to TLM Memory (Bypass TLM)
            tlm_mem->inst_read_en_i(cpu_inst_read_en_s);
            tlm_mem->inst_addr_bus_i(cpu_inst_addr_bus_s);
            tlm_mem->inst_bus_o(cpu_inst_bus_in_s);

            // Connect timer port to CPU
            tlm_timer->irq_timer_o(irq_timer_s);

            // Connect msip to CPU
            tlm_msip->irq_sw_o(irq_sw_s);

            // Connect adapter to interconnect
            adapter->initiator_socket.bind(interconnect->target_socket);

            // Connect interconnect to modules
            interconnect->mem_socket.bind(tlm_mem->target_socket);
            interconnect->gpio_socket.bind(tlm_gpio->target_socket);
            interconnect->timer_socket.bind(tlm_timer->target_socket);
            interconnect->uart_socket.bind(tlm_uart->target_socket);
            interconnect->msip_socket.bind(tlm_msip->target_socket);
        }
        // Use legacy modules
        else {
            mem = new memory_model("memory");
            gpio = new gpio_model("gpio");
            timer = new timer_model("timer");
            uart = new uart_model("uart");
            bus = new system_bus("bus");

            // Connect Clock/Reset to modules
            mem->clk_i(clk_i);
            mem->rst_i(rst_i);
            gpio->clk_i(clk_i);
            gpio->rst_i(rst_i);
            timer->clk_i(clk_i);
            timer->rst_i(rst_i);
            uart->clk_i(clk_i);
            uart->rst_i(rst_i);

            // Connect Instruction Memory input/output ports
            mem->inst_read_en_i(mem_inst_read_en_s);
            mem->inst_addr_bus_i(mem_inst_addr_bus_s);
            mem->inst_bus_o(mem_inst_bus_out_s);

            // Connect Data Memory input/output ports
            mem->data_write_en_i(mem_data_write_en_s);
            mem->data_read_en_i(mem_data_read_en_s);
            mem->data_addr_bus_i(mem_data_addr_bus_s);
            mem->data_bus_i(mem_data_bus_in_s);
            mem->data_bus_o(mem_data_bus_out_s);
            
            // Connect GPIO input/output ports
            gpio->write_en_i(gpio_write_en_s);
            gpio->read_en_i(gpio_read_en_s);
            gpio->addr_bus_i(gpio_addr_bus_s);
            gpio->data_bus_i(gpio_data_in_s);
            gpio->data_bus_o(gpio_data_out_s);

            // Connect Timer input/output ports
            timer->write_en_i(timer_write_en_s);
            timer->read_en_i(timer_read_en_s);
            timer->addr_bus_i(timer_addr_bus_s);
            timer->data_bus_i(timer_data_in_s);
            timer->data_bus_o(timer_data_out_s);

            timer->irq_timer_o(irq_timer_s);

            // Connect UART input/output ports
            uart->write_en_i(uart_write_en_s);
            uart->read_en_i(uart_read_en_s);
            uart->addr_bus_i(uart_addr_bus_s);
            uart->data_bus_i(uart_data_in_s);
            uart->data_bus_o(uart_data_out_s);

            // Connect components to System Bus
            // CPU instruction ports
            bus->cpu_inst_read_en_i(cpu_inst_read_en_s);
            bus->cpu_inst_addr_bus_i(cpu_inst_addr_bus_s);
            bus->cpu_inst_bus_o(cpu_inst_bus_in_s);

            // CPU data ports
            bus->cpu_data_write_en_i(cpu_data_write_en_s);
            bus->cpu_data_read_en_i(cpu_data_read_en_s);
            bus->cpu_data_size_i(cpu_data_size_s);
            bus->cpu_data_addr_bus_i(cpu_data_addr_bus_s);
            bus->cpu_data_bus_i(cpu_data_bus_out_s);
            bus->cpu_data_bus_o(cpu_data_bus_in_s);
            bus->cpu_data_ready_o(cpu_data_ready_s);
            bus->cpu_data_error_o(cpu_data_error_s);

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

            // GPIO data ports
            bus->gpio_write_en_o(gpio_write_en_s);
            bus->gpio_read_en_o(gpio_read_en_s);
            bus->gpio_addr_bus_o(gpio_addr_bus_s);
            bus->gpio_data_bus_o(gpio_data_in_s);
            bus->gpio_data_bus_i(gpio_data_out_s);

            // Timer data ports
            bus->timer_write_en_o(timer_write_en_s);
            bus->timer_read_en_o(timer_read_en_s);
            bus->timer_addr_bus_o(timer_addr_bus_s);
            bus->timer_data_bus_o(timer_data_in_s);
            bus->timer_data_bus_i(timer_data_out_s);

            // UART data ports
            bus->uart_write_en_o(uart_write_en_s);
            bus->uart_read_en_o(uart_read_en_s);
            bus->uart_addr_bus_o(uart_addr_bus_s);
            bus->uart_data_bus_o(uart_data_in_s);
            bus->uart_data_bus_i(uart_data_out_s);
        }
    }

    // Destructor
    ~system_top() {
        delete cpu;

        if (config.use_tlm) {
            delete tlm_mem;
            delete tlm_gpio;
            delete tlm_timer;
            delete tlm_uart;
            delete tlm_msip;
            delete adapter;
            delete interconnect;
        }
        else {
            delete mem;
            delete gpio;
            delete timer;
            delete uart;
            delete bus;
        }
    }
};