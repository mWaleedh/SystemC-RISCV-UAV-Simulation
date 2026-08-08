#include "tlm_memory_target.h"
#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;
using namespace tlm;

// Constructor
tlm_memory_target::tlm_memory_target(sc_module_name name, uint32_t size_b, sc_time read_lat, sc_time write_lat) : sc_module(name), target_socket("target_socket"), size_bytes(size_b), read_latency(read_lat), write_latency(write_lat) {
    // Allocate byte array and initialize to zero
    memory = new unsigned char[size_bytes]();
    max_addr = 0;

    // Connect socket to TLM function
    target_socket.register_b_transport(this, &tlm_memory_target::b_transport);

    // Thread for instruction access
    SC_CTHREAD(inst_fetch_thread, clk_i.pos());
    reset_signal_is(rst_i, true);
    
    // Reset logic
    SC_METHOD(reset_logic);
    sensitive << rst_i;
}

// Destructor
tlm_memory_target::~tlm_memory_target() {
    delete[] memory;
}

// Reset function
void tlm_memory_target::reset_logic() {
    if (rst_i.read() == true) {
        max_addr = 0;
        for (int i = 0; i < size_bytes; i++) {
            memory[i] = 0;
        }
    }
}


// Load data at specific address
void tlm_memory_target::load_data(uint32_t addr, uint32_t data) {
    if (addr + 3 < size_bytes) {
        if (addr >= max_addr) {
            memcpy(&memory[addr], &data, 4);
        } 
        else {
            cout << "Memory Error: Trying to access already occupied memory 0x" << hex << addr << dec << endl << endl;
        }
    } 
    else {
        cout << "Memory Error: Trying to access invalid memory 0x" << hex << addr << dec << endl << endl;
    }
}

// Load entire instruction file into memory
void tlm_memory_target::load_file(const string& filename) {
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        cout << "Error: Unable to open file " << filename << endl << endl;
        return;
    }

    string temp;
    while (getline(file, temp)) {
        if (temp.empty()) {
            continue;
        }
        uint32_t data = stoul(temp, nullptr, 16);
        
        if (max_addr + 3 < size_bytes) {
            memcpy(&memory[max_addr], &data, 4);
            max_addr += 4;
        }
    }
    file.close();

    cout << "TLM Memory: Loaded data from file " << filename << endl << endl;
}

// Non-TLM way to access instructions
void tlm_memory_target::inst_fetch_thread() {
    // Reset/initial state
    inst_bus_o.write(0);

    // Wait marking end of reset
    wait();

    while (true) {
        if (inst_read_en_i.read() == true) {
            uint32_t addr = inst_addr_bus_i.read();

            // Check if address is within range
            if (addr + 3 < size_bytes) {
                // Read 4 bytes from memory
                uint32_t fetched_inst = 0;
                memcpy(&fetched_inst, &memory[addr], 4);

                // Send instruction to CPU
                inst_bus_o.write(fetched_inst);
            } 
            else {
                inst_bus_o.write(0);
                cout << "@" << sc_time_stamp() << " Memory Error (IF): Trying to access invalid memory 0x" << hex << addr << dec << endl << endl;
            }
        } 
        else {
            inst_bus_o.write(0);
        }

        wait();
    }
}

// TLM function
void tlm_memory_target::b_transport(tlm_generic_payload& trans, sc_time& delay) {
    tlm_command cmd = trans.get_command();
    uint64_t addr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();

    // Check if address is within bounds
    if (addr >= size_bytes || (addr + len) > size_bytes) {
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);

        cout << "@" << sc_time_stamp() << " TLM Memory Error: Invalid access at 0x" << hex << addr << dec << endl << endl;
        return;
    }

    // Handle read
    if (cmd == TLM_READ_COMMAND) {
        memcpy(ptr, &memory[addr], len);
        wait(read_latency);
    } 
    // Handle write
    else if (cmd == TLM_WRITE_COMMAND) {
        memcpy(&memory[addr], ptr, len);
        wait(write_latency);
    }

    trans.set_response_status(TLM_OK_RESPONSE);
}