#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
using namespace tlm;
using namespace tlm_utils;

SC_MODULE(tlm_uart_target) {
    simple_target_socket<tlm_uart_target> target_socket;

    // Input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;

    // Local variables
    bool tx_ready;
    int busy_cycles;
    
    // Configurable delays
    int tx_delay;
    sc_time uart_delay;

    // Functions
    void b_transport(tlm_generic_payload& trans, sc_time& delay);
    void uart_tick();

    SC_HAS_PROCESS(tlm_uart_target);
    tlm_uart_target(sc_module_name name, int tx_delay, sc_time bus_delay);
};