#include <systemc.h>
#include "../src/memory_model.cpp"

int ssc_main(int argc, char* argv[]) {
    // Create sc_clock object
    sc_clock clk_s("clk");

    // Create signals to connect input/output ports
    sc_signal<bool> rst_s;
    sc_signal<bool> write_en_s;
    sc_signal<bool> read_en_s;
    sc_signal<sc_uint<32>> addr_bus_s;
    sc_signal<sc_uint<32>> data_bus_in_s;

    sc_signal<sc_uint<32>> data_bus_out_s;

    // Initialize risc_v_model and connect input/output ports
    memory_model cpu("memory");
        cpu.clk_i(clk_s);
        cpu.rst_i(rst_s);
        cpu.write_en_i(write_en_s);
        cpu.read_en_i(read_en_s);
        cpu.addr_bus_i(addr_bus_s);
        cpu.data_bus_i(data_bus_in_s);

        cpu.data_bus_o(data_bus_out_s);

    // VCD waveform trace
    sc_trace_file *wf = sc_create_vcd_trace_file("memory_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, addr_bus_s, "address_bus");
    sc_trace(wf, data_bus_in_s, "data_bus_input");
    sc_trace(wf, data_bus_out_s, "data_bus_output");
    sc_trace(wf, read_en_s, "read_en");
    sc_trace(wf, write_en_s, "write_en");

    // Clear input ports
    write_en_s.write(0);
    read_en_s.write(0);
    addr_bus_s.write(0);
    data_bus_in_s.write(0);    

    cout << "@" << sc_time_stamp() << " Applying Reset...\n" << endl;
    rst_s.write(true);
    sc_start(5, SC_NS); 

    cout << "@" << sc_time_stamp() << " Releasing Reset...\n" << endl;
    rst_s.write(false);

    return 0;
}
