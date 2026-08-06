#include "tlm_interconnect.h"
using namespace tlm;

tlm_interconnect::tlm_interconnect(sc_module_name name, uint32_t mem_addr, uint32_t gpio_addr, uint32_t timer_addr, uint32_t uart_addr, uint32_t msip_addr) : sc_module(name), target_socket("target_socket"), mem_socket("mem_socket"), gpio_socket("gpio_socket"), timer_socket("timer_socket"), uart_socket("uart_socket"), msip_socket("msip_socket"), mem_base(mem_addr), gpio_base(gpio_addr), timer_base(timer_addr), uart_base(uart_addr), msip_base(msip_addr) {                        
    // Receive payload from adapter
    target_socket.register_b_transport(this, &tlm_interconnect::b_transport);
}

void tlm_interconnect::b_transport(tlm_generic_payload& payload, sc_time& delay) {
    uint64_t addr = payload.get_address();

    // Memory address
    if (addr >= mem_base && addr <= (mem_base + 0x0FFF)) {
        payload.set_address(addr - mem_base);
        mem_socket->b_transport(payload, delay);
    }
    // GPIO address
    else if (addr >= gpio_base && addr <= (gpio_base + 0x000F)) {
        payload.set_address(addr - gpio_base);
        gpio_socket->b_transport(payload, delay);
    }
    // Timer address
    else if (addr >= timer_base && addr <= (timer_base + 0x000F)) {
        payload.set_address(addr - timer_base);
        timer_socket->b_transport(payload, delay);
    }
    // UART address
    else if (addr >= uart_base && addr <= (uart_base + 0x000F)) {
        payload.set_address(addr - uart_base);
        uart_socket->b_transport(payload, delay);
    }
    // MSIP address
    else if (addr >= msip_base && addr < (msip_base + 0x000F)) {
        payload.set_address(addr - msip_base);
        uart_socket->b_transport(payload, delay);
    }
    else {
        payload.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    payload.set_address(addr);
}