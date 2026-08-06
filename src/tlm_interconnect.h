#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
using namespace tlm;
using namespace tlm_utils;

SC_MODULE(tlm_interconnect) {
    simple_target_socket<tlm_interconnect> target_socket;

    // MMIO module initiator sockets
    simple_initiator_socket<tlm_interconnect> mem_socket;
    simple_initiator_socket<tlm_interconnect> gpio_socket;
    simple_initiator_socket<tlm_interconnect> timer_socket;
    simple_initiator_socket<tlm_interconnect> uart_socket;
    simple_initiator_socket<tlm_interconnect> msip_socket;

    // Variables to store peripheral base addresses
    uint32_t mem_base;
    uint32_t gpio_base;
    uint32_t timer_base;
    uint32_t uart_base;
    uint32_t msip_base;

    void b_transport(tlm_generic_payload& payload, sc_time& delay);

    SC_HAS_PROCESS(tlm_interconnect);
    tlm_interconnect(sc_module_name name, uint32_t mem_addr, uint32_t gpio_addr, uint32_t timer_addr, uint32_t uart_addr, uint32_t msip_addr);
};
