#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

SC_MODULE(tlm_adapter) {
    static const int WIDTH = 32;

    sc_in<bool> clk_i;

    // CPU Data Ports
    sc_in<bool> cpu_data_write_en_i;
    sc_in<bool> cpu_data_read_en_i;
    sc_in<sc_uint<WIDTH>> cpu_data_addr_bus_i;
    sc_in<sc_uint<WIDTH>> cpu_data_bus_i;
    sc_out<sc_uint<WIDTH>> cpu_data_bus_o;

    // CPU Instruction Ports
    sc_in<bool> cpu_inst_read_en_i;
    sc_in<sc_uint<WIDTH>> cpu_inst_addr_bus_i;
    sc_out<sc_uint<WIDTH>> cpu_inst_bus_o;

    // Memory Instruction ports
    sc_out<bool> mem_inst_read_en_o;
    sc_out<sc_uint<WIDTH>> mem_inst_addr_bus_o;
    sc_in<sc_uint<WIDTH>> mem_inst_bus_i;

    tlm_utils::simple_initiator_socket<tlm_adapter> socket;

    void processData();
    void passInst();

    SC_CTOR(tlm_adapter) : socket("socket") {
        SC_THREAD(processData);
        sensitive << clk_i.pos(); 

        SC_METHOD(passInst);
        sensitive << cpu_inst_read_en_i << cpu_inst_addr_bus_i << mem_inst_bus_i;
    }
};