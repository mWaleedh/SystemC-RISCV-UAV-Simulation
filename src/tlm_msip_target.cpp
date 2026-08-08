#include "tlm_msip_target.h"
using namespace std;
using namespace tlm;

tlm_msip_target::tlm_msip_target(sc_module_name name, sc_time delay) : sc_module(name), target_socket("target_socket"), msip_delay(delay) { 
    // Connect to socket
    target_socket.register_b_transport(this, &tlm_msip_target::b_transport);
    
    // Initial state
    msip_reg = 0;
    irq_sw_o.initialize(false);

    SC_METHOD(reset_logic);
    sensitive << clk_i.pos();
}

void tlm_msip_target::reset_logic() {
    if (rst_i.read() == true) {
        msip_reg = 0;
    }
    
    // Trigger interrupt if msip_reg is 1
    irq_sw_o.write(msip_reg != 0);
}

// TLM function
void tlm_msip_target::b_transport(tlm_generic_payload& trans, sc_time& delay) {
    tlm_command cmd = trans.get_command();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();

    if (cmd == TLM_WRITE_COMMAND) {
        if (len == 4) {
            memcpy(&msip_reg, ptr, len);    // Read data sent by CPU
            msip_reg &= 0x1;                // Only keep the lowest bit
            
            cout << "@" << sc_time_stamp() << " MSIP: Software Interrupt bit written -> " << msip_reg << endl;
            
            // Simulate delay
            wait(msip_delay);
            trans.set_response_status(TLM_OK_RESPONSE);
        } 
        else {
            trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
        }
    } 
    else if (cmd == TLM_READ_COMMAND) {
        if (len == 4) {
            // Send the value of msip_reg to CPU
            memcpy(ptr, &msip_reg, len);
            
            // Simulate delay
            wait(msip_delay);
            trans.set_response_status(TLM_OK_RESPONSE);
        } 
        else {
            trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
        }
    } 
    else {
        trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
    }
}