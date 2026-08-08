#include "tlm_timer_target.h"
#include <iostream>
#include <cstring>
using namespace std;
using namespace tlm;

// Constructor
tlm_timer_target::tlm_timer_target(sc_module_name name, sc_time delay) : sc_module(name), target_socket("target_socket"), timer_delay(delay) { 
    // Connect to socket
    target_socket.register_b_transport(this, &tlm_timer_target::b_transport);
    
    // Initial state
    count_reg = 0;
    compare_reg = 0;
    control_reg = 0;
    status_reg = 0;
    irq_timer_o.initialize(false);

    SC_CTHREAD(timer_tick, clk_i.pos());
    reset_signal_is(rst_i, true);
}

// Clock thread to count ticks
void tlm_timer_target::timer_tick() {
    // Reset/initial state logic
    count_reg = 0;
    compare_reg = 0xFFFFFFFF;
    control_reg = 0;
    status_reg = 0;
    irq_timer_o.write(false);
    
    // Wait marking end of reset
    wait();

    while (true) {
        // Send timer status to CPU
        irq_timer_o.write(status_reg == 1);

        // Only increment counter register if timer is enabled and not currently in an interrupt
        if ((control_reg & 0x1) == 1 && status_reg == 0) {
            count_reg++;

            if (count_reg == compare_reg) {
                status_reg = 1; // Trigger interrupt
                count_reg = 0;  //Set to 0 to restart timer

                cout << "@" << sc_time_stamp() << " TIMER: Compare match" << endl;
                cout << "@" << sc_time_stamp() << " TIMER: Interrupt raised" << endl << endl;
            }
        }
        wait();
    }
}

// The TLM function
void tlm_timer_target::b_transport(tlm_generic_payload& trans, sc_time& delay) {
    tlm_command cmd = trans.get_command();
    uint64_t addr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();

    if (len != 4) {
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
        return;
    }

    uint32_t data = 0;
    if (cmd == TLM_WRITE_COMMAND) {
        memcpy(&data, ptr, 4);
    }

    // Write to register
    if (cmd == TLM_WRITE_COMMAND) {
        switch (addr) {
            case 0x00: 
                count_reg = data; 
                break;
            case 0x04: 
                compare_reg = data; 
                break;
            case 0x08: 
                control_reg = data;
                break;
            case 0x0C: 
                // Write 1 to clear
                if (data == 1) {
                    status_reg = 0;
                    cout << "@" << sc_time_stamp() << " Timer: Interrupt cleared. Restarting counter" << endl << endl;
                }
                break;
            default:
                trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);

                cout << "@" << sc_time_stamp() << " Timer Error: Writing to invalid address 0x" << hex << addr << dec << endl << endl;
                return;
        }
        cout << "@" << sc_time_stamp() << " Timer: Wrote 0x" << hex << data << " to address 0x" << addr << dec << endl << endl;
    } 
    // Read from register
    else if (cmd == TLM_READ_COMMAND) {
        switch (addr) {
            case 0x00: 
                memcpy(ptr, &count_reg, 4); 
                break;
            case 0x04: 
                memcpy(ptr, &compare_reg, 4); 
                break;
            case 0x08: 
                memcpy(ptr, &control_reg, 4); 
                break;
            case 0x0C: 
                memcpy(ptr, &status_reg, 4); 
                break;
            default:
                trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);

                cout << "@" << sc_time_stamp() << " Timer Error: Reading from invalid address 0x" << hex << addr << dec << endl << endl;
                return;
        }

        cout << "@" << sc_time_stamp() << " Timer: Read from address 0x" << hex << addr << dec << endl << endl;
    }

    wait(timer_delay);
    trans.set_response_status(TLM_OK_RESPONSE);
}