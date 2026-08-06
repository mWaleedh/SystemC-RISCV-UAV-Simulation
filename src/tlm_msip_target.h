#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
using namespace tlm_utils;

SC_MODULE(tlm_msip_target) {
    simple_target_socket<tlm_msip_target> target_socket;
    
    // Input/Output ports
    sc_in<bool> rst_i;
    sc_out<bool> irq_sw_o;

    // Internal register
    uint32_t msip_reg;
    sc_time msip_delay;

    // Functions
    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
    void reset_logic();

    SC_HAS_PROCESS(tlm_msip_target);
    tlm_msip_target(sc_module_name name, sc_time delay);
};