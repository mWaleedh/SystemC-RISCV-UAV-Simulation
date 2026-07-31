#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
using namespace tlm;
using namespace tlm_utils;

SC_MODULE(tlm_timer_target) {
    simple_target_socket<tlm_timer_target> target_socket;

    // Input/output ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_out<bool> irq_timer_o;

    // Internal registers
    uint32_t count_reg;
    uint32_t compare_reg;
    uint32_t control_reg;
    uint32_t status_reg;

    sc_time delay;

    // Functions
    void b_transport(tlm_generic_payload& trans, sc_time& delay);
    void timer_tick();

    SC_HAS_PROCESS(tlm_timer_target);
    tlm_timer_target(sc_module_name name, sc_time delay);
};