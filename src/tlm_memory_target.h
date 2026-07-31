#include <systemc.h>
#include <string>
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
using namespace std;
using namespace tlm;
using namespace tlm_utils;

SC_MODULE(tlm_memory_target) {
    // Constants
    static const int WIDTH = 32;

    simple_target_socket<tlm_memory_target> target_socket;

    // Input/output ports (only for instructions)
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> inst_read_en_i;
    sc_in<sc_uint<WIDTH>> inst_addr_bus_i;
    sc_out<sc_uint<WIDTH>> inst_bus_o;

    // Configurable parameters
    uint32_t size_bytes;
    sc_time read_latency;
    sc_time write_latency;

    // 1-byte array pointer
    unsigned char* memory;
    uint32_t max_addr;

    // Functions
    void b_transport(tlm_generic_payload& trans, sc_time& delay);
    void inst_fetch_thread();

    // Initialization functions
    void load_data(uint32_t addr, uint32_t data);
    void load_file(const string& filename);

    // Constructor & Destructor
    SC_HAS_PROCESS(tlm_memory_target);
    tlm_memory_target(sc_module_name name, uint32_t size_bytes, sc_time read_lat, sc_time write_lat);
    ~tlm_memory_target();
};