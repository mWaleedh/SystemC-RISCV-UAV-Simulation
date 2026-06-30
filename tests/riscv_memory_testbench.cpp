#include <systemc.h>
#include "../src/system_top.cpp"

int sc_main(int argc, char* argv[]) {
    // Create sc_clock object
    sc_clock clk_s("clk");

    // Create signals to connect input ports
    sc_signal<bool> rst_s;
    sc_signal<bool> irq_timer_s;
    sc_signal<bool> irq_ext_s;
    sc_signal<bool> irq_sw_s;

    // Initialize system_top and connect input ports
    system_top sys("System_Top");
        sys.clk_i(clk_s);
        sys.rst_i(rst_s);
        sys.irq_timer_i(irq_timer_s);
        sys.irq_ext_i(irq_ext_s);
        sys.irq_sw_i(irq_sw_s);

    // VCD waveform trace
    sc_trace_file *wf = sc_create_vcd_trace_file("riscv_memory_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, sys.read_en_s, "read_en");
    sc_trace(wf, sys.write_en_s, "write_en");
    sc_trace(wf, sys.addr_bus_s, "address_bus");
    sc_trace(wf, sys.mem_cpu_data_bus_s, "mem_to_cpu_bus");
    sc_trace(wf, sys.cpu_mem_data_bus_s, "cpu_to_mem_bus");

    // Clear input ports
    irq_timer_s.write(false);
    irq_ext_s.write(false);
    irq_sw_s.write(false);

    // Reset
    cout << "@" << sc_time_stamp() << " Applying Reset..." << endl;
    rst_s.write(true);
    sc_start(5, SC_NS);

    // Release Reset
    cout << "@" << sc_time_stamp() << " Releasing Reset...\n" << endl;
    rst_s.write(false);

    // Load instructions into memory
    sys.load_data(0, 0x00000013);
    sys.load_data(4, 0x00500093);
    sys.load_data(8, 0x00100113);

    // Verify instruction 1
    sc_start(6, SC_NS);
    cout << (sys.cpu->cur_inst == 0x00000013 ? "PASS" : "FAIL") << endl << endl;

    // Verify instruction 2
    sc_start(6, SC_NS);
    cout << (sys.cpu->cur_inst == 0x00500093 ? "PASS" : "FAIL") << endl << endl;

    // Verify instruction 3
    sc_start(6, SC_NS);
    cout << (sys.cpu->cur_inst == 0x00100113 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;
    
    return 0;
}