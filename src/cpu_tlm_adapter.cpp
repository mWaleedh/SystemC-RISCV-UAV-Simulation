#include "cpu_tlm_adapter.h"

// Pass instruction fetch request directly to memory
void tlm_adapter::passInst() {
    mem_inst_read_en_o.write(cpu_inst_read_en_i.read());
    mem_inst_addr_bus_o.write(cpu_inst_addr_bus_i.read());
    cpu_inst_bus_o.write(mem_inst_bus_i.read());
}

void tlm_adapter::processData() {
    // Initialize ready signal as false
    cpu_data_ready_o.write(false);

    while (true) {
        // Wait for next rising edge
        wait();

        // Set ready signal to false  before each iteration
        cpu_data_ready_o.write(false);

        bool read_en = cpu_data_read_en_i.read();
        bool write_en = cpu_data_write_en_i.read();

        if (read_en || write_en) {
            tlm::tlm_generic_payload payload;
            sc_time delay = SC_ZERO_TIME;

            uint32_t addr = cpu_data_addr_bus_i.read();
            uint32_t data = 0;

            if (write_en) {
                data = cpu_data_bus_i.read();
                payload.set_command(tlm::TLM_WRITE_COMMAND);
            } 
            else {
                payload.set_command(tlm::TLM_READ_COMMAND);
            }

            // Set TLM generic payload values
            payload.set_address(addr);
            payload.set_data_ptr((unsigned char*)&data);
            payload.set_data_length(4);
            payload.set_streaming_width(4);
            payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            // Send transaction
            socket->b_transport(payload, delay);

            // Check if interconnect returned error
            if (payload.is_response_error()) {
                SC_REPORT_ERROR("TLM_ADAPTER", "Transaction failed or invalid address");
            }

            // For read send data back to CPU
            if (read_en) {
                cpu_data_bus_o.write(data);
            }

            // Tell the CPU that data is ready
            cpu_data_ready_o.write(true);
        }
    }
}