#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
using namespace tlm;
using namespace tlm_utils;

SC_MODULE(tlm_gpio_target) {
    simple_target_socket<tlm_gpio_target> target_socket;
    
    sc_in<bool> rst_i;

    // Local variables
    uint32_t input_reg;
    uint32_t output_reg;
    sc_time delay;

    // Functions
    void b_transport(tlm_generic_payload& trans, sc_time& delay);
    void reset_logic();

    SC_HAS_PROCESS(tlm_gpio_target);
    tlm_gpio_target(sc_module_name name, sc_time delay);
};