#include "tlm_gpio_target.h"
#include <iostream>
#include <cstring>
using namespace std;
using namespace tlm;

// Constructor
tlm_gpio_target::tlm_gpio_target(sc_module_name name, sc_time delay) : sc_module(name), target_socket("target_socket"), delay(delay) {
    target_socket.register_b_transport(this, &tlm_gpio_target::b_transport);
    
    SC_METHOD(reset_logic);
    sensitive << rst_i.pos();
}

// Function to reset GPIO
void tlm_gpio_target::reset_logic() {
    input_reg = 0;
    output_reg = 0;
}

// TLM function
void tlm_gpio_target::b_transport(tlm_generic_payload& trans, sc_time& delay) {
    tlm_command cmd = trans.get_command();
    uint64_t addr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();

    if (len != 4) {
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);

        cout << "@" << sc_time_stamp() << " GPIO Error: Invalid access size" << endl << endl;
        return;
    }

    if (addr == 0x00) {
        if (cmd == TLM_WRITE_COMMAND) {
            memcpy(&output_reg, ptr, 4);
            
            if (output_reg == 1) {
                cout << "@" << sc_time_stamp() << " GPIO: LED ON\n" << endl;
            } 
            else if (output_reg == 0) {
                cout << "@" << sc_time_stamp() << " GPIO: LED OFF\n" << endl;
            } 
            else {
                cout << "@" << sc_time_stamp() << " GPIO Write: 0x" << hex << output_reg << dec << " written to output register\n" << endl;
            }
        } 
        else if (cmd == TLM_READ_COMMAND) {
            memcpy(ptr, &output_reg, 4);

            cout << "@" << sc_time_stamp() << " GPIO Read: 0x" << hex << output_reg << dec << " read from output register\n" << endl;
        }
    } 
    else if (addr == 0x04) {
        if (cmd == TLM_READ_COMMAND) {
            // Temporary placeholder
            input_reg = 1; 
            memcpy(ptr, &input_reg, 4);

            cout << "@" << sc_time_stamp() << " GPIO Read: 0x" << hex << input_reg << dec << " read from input register\n" << endl;
        } 
        else {
            trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
            
            cout << "@" << sc_time_stamp() << " GPIO Error: Cannot write to input register\n" << endl;
            return;
        }
    } 
    else {
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);

        cout << "@" << sc_time_stamp() << " GPIO Error: Invalid address 0x" << hex << addr << dec << "\n" << endl;
        return;
    }

    wait(delay);
    trans.set_response_status(TLM_OK_RESPONSE);
}