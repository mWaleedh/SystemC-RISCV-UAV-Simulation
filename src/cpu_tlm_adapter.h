#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
using namespace tlm_utils;

SC_MODULE(tlm_adapter) {
    static const int WIDTH = 32;

    sc_in<bool> clk_i;

    // CPU Data input ports
    sc_in<bool> cpu_data_write_en_i;
    sc_in<bool> cpu_data_read_en_i;
    sc_in<sc_uint<3>> cpu_data_size_i;
    sc_in<sc_uint<WIDTH>> cpu_data_addr_bus_i;
    sc_in<sc_uint<WIDTH>> cpu_data_bus_i;

    // CPU Data output ports
    sc_out<bool> cpu_data_ready_o;
    sc_out<bool> cpu_data_error_o;
    sc_out<sc_uint<WIDTH>> cpu_data_bus_o;

    simple_initiator_socket<tlm_adapter> initiator_socket;

    void process_data();

    SC_CTOR(tlm_adapter) : initiator_socket("initiator_socket") {
        SC_THREAD(process_data);
        sensitive << clk_i.pos(); 
    }
};