#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

SC_MODULE(tlm_interconnect) {
    tlm_utils::simple_target_socket<tlm_interconnect> target_socket;

    // MMIO module initiator sockets
    tlm_utils::simple_initiator_socket<tlm_interconnect> mem_socket;
    tlm_utils::simple_initiator_socket<tlm_interconnect> gpio_socket;
    tlm_utils::simple_initiator_socket<tlm_interconnect> timer_socket;
    tlm_utils::simple_initiator_socket<tlm_interconnect> uart_socket;

    void b_transport(tlm::tlm_generic_payload& payload, sc_time& delay);

    SC_CTOR(tlm_interconnect) : target_socket("target_socket"), mem_socket("mem_socket"), gpio_socket("gpio_socket"), timer_socket("timer_socket"), uart_socket("uart_socket") {                        
        // Receive payload from adapter
        target_socket.register_b_transport(this, &tlm_interconnect::b_transport);
    }
};