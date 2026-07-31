#include "tlm_interconnect.h"
using namespace tlm;

void tlm_interconnect::b_transport(tlm_generic_payload& payload, sc_time& delay) {
    uint64_t addr = payload.get_address();

    if (addr >= 0x00000000 && addr <= 0x00000FFF) {
        mem_socket->b_transport(payload, delay);
    }
    else if (addr >= 0x10000000 && addr <= 0x1000000F) {
        payload.set_address(addr - 0x10000000);
        gpio_socket->b_transport(payload, delay);
    }
    else if (addr >= 0x10000010 && addr <= 0x1000001F) {
        payload.set_address(addr - 0x10000010);
        timer_socket->b_transport(payload, delay);
    }
    else if (addr >= 0x10000020 && addr <= 0x1000002F) {
        payload.set_address(addr - 0x10000020);
        uart_socket->b_transport(payload, delay);
    }
    else {
        payload.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    payload.set_address(addr);
}