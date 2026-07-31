#include "tlm_uart_target.h"
#include <iostream>
#include <cstring>
using namespace std;
using namespace tlm;

// Constructor
tlm_uart_target::tlm_uart_target(sc_module_name name, int tx_delay, sc_time b_delay) : sc_module(name), target_socket("target_socket"), tx_delay(tx_delay), bus_delay(bus_delay) {
    // Connect socket
    target_socket.register_b_transport(this, &tlm_uart_target::b_transport);
    
    SC_CTHREAD(uart_tick, clk_i.pos());
    reset_signal_is(rst_i, true);
}

// Clock to handle TX countdown
void tlm_uart_target::uart_tick() {
    // Reset/initial state
    busy_cycles = 0;
    tx_ready = true;
    
    // Wait marking end of reset
    wait();

    while (true) {
        // Transmission delay
        if (busy_cycles > 0) {
            busy_cycles--;

            if (busy_cycles == 0) {
                tx_ready = true;
            }
        }

        wait();
    }
}

// TLM function
void tlm_uart_target::b_transport(tlm_generic_payload& trans, sc_time& delay) {
    tlm_command cmd = trans.get_command();
    uint64_t addr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();

    // Check for number of bytes sent
    if (len != 4) {
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
        return;
    }

    uint32_t data = 0;
    // Read data coming from CPU
    if (cmd == TLM_WRITE_COMMAND) {
        memcpy(&data, ptr, 4);
    }

    // Check if its a write command
    if (addr == 0x00) { 
        if (cmd == TLM_WRITE_COMMAND) {
            // Only write if UART is ready
            if (tx_ready) {
                // Extract lowest byte and print to terminal
                char c = static_cast<char>(data & 0xFF);
                cout << "UART TX: " << c << endl << endl;
                
                // Set hardware delay counter
                tx_ready = false;
                busy_cycles = tx_delay;
            } 
            else {
                cout << "@" << sc_time_stamp() << " UART Warning: Dropped character. TX is busy" << endl;
            }
        } 
        else {
            trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
            return;
        }
    } 
    // Check if its a read command
    else if (addr == 0x04) { 
        if (cmd == TLM_READ_COMMAND) {
            // Read status (Lowest bit of tx_ready)
            uint32_t status_val = tx_ready ? 1 : 0;
            memcpy(ptr, &status_val, 4);
        } 
        else {
            trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
            return;
        }
    } 
    else {
        // Invalid address error
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    // Simulate bus delay
    wait(bus_delay);
    trans.set_response_status(TLM_OK_RESPONSE);
}