#include "cpu_tlm_adapter.h"
using namespace tlm;

void tlm_adapter::process_data() {
    // Initialize ready signal as false
    cpu_data_ready_o.write(false);
    cpu_data_error_o.write(false);

    while (true) {
        // Wait for next rising edge
        wait();

        // Reset output pins every cycle
        cpu_data_ready_o.write(false);
        cpu_data_error_o.write(false);

        bool read_en = cpu_data_read_en_i.read();
        bool write_en = cpu_data_write_en_i.read();

        if (read_en || write_en) {
            tlm_generic_payload payload;
            sc_time delay = SC_ZERO_TIME;

            uint32_t addr = cpu_data_addr_bus_i.read();
            uint32_t size = cpu_data_size_i.read();
            uint32_t data = 0;

            if (write_en) {
                data = cpu_data_bus_i.read();
                payload.set_command(TLM_WRITE_COMMAND);
            } 
            else {
                payload.set_command(TLM_READ_COMMAND);
            }

            // Set TLM generic payload values
            payload.set_address(addr);
            payload.set_data_ptr((unsigned char*)&data);
            payload.set_data_length(size);
            payload.set_streaming_width(size);
            payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

            // Send transaction
            initiator_socket->b_transport(payload, delay);

            // Check if interconnect returned error
            if (payload.is_response_error()) {
                // Alert CPU about error
                cpu_data_error_o.write(true);
            }

            wait(delay);

            // For read send data back to CPU
            if (read_en) {
                cpu_data_bus_o.write(data);
            }

            // Tell the CPU that data is ready
            cpu_data_ready_o.write(true);
        }
    }
}