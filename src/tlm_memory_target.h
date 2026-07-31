#include <systemc.h>
#include <string>
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
using namespace std;
using namespace tlm;
using namespace tlm_utils;

SC_MODULE(tlm_memory_target) {
    simple_target_socket<tlm_memory_target> target_socket;

    // Configurable parameters
    uint32_t size_bytes;
    sc_time read_latency;
    sc_time write_latency;

    // 1-byte array pointer
    unsigned char* memory;
    uint32_t max_addr;

    // TLM function
    void b_transport(tlm_generic_payload& trans, sc_time& delay);

    // Initialization functions
    void load_data(uint32_t addr, uint32_t data);
    void load_file(const string& filename);

    // Constructor & Destructor
    SC_HAS_PROCESS(tlm_memory_target);
    tlm_memory_target(sc_module_name name, uint32_t size_bytes, sc_time read_lat, sc_time write_lat);
    ~tlm_memory_target();
};