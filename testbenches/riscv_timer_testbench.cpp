/*
Compare value is set to 5 meaning after 5 cycles the interrupt is triggered
*/ 

#include <systemc.h>
#include "../src/system_top.cpp"
using namespace std;

int sc_main(int argc, char* argv[]) {
    // Create sc_clock object
    sc_clock clk_s("clk");

    // Create signals to connect input ports
    sc_signal<bool> rst_s;
    sc_signal<bool> irq_ext_s;
    sc_signal<bool> irq_sw_s;

    // Initialize system_top and connect input ports
    system_top sys("System_Top");
    sys.clk_i(clk_s);
    sys.rst_i(rst_s);
    sys.irq_ext_i(irq_ext_s);
    sys.irq_sw_i(irq_sw_s);

    // VCD waveform trace
    sc_trace_file *wf = sc_create_vcd_trace_file("./waveforms/riscv_timer_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, sys.cpu->pc, "pc");
    sc_trace(wf, sys.cpu->cur_inst, "cur_inst");
    sc_trace(wf, sys.cpu->addr_bus_o, "cpu_addr");
    sc_trace(wf, sys.cpu->read_en_o, "read_en");
    sc_trace(wf, sys.cpu->write_en_o, "write_en");
    sc_trace(wf, sys.timer->count_reg, "count_reg");
    sc_trace(wf, sys.timer->count_reg, "compare_reg");
    sc_trace(wf, sys.timer->count_reg, "control_reg");
    sc_trace(wf, sys.timer->count_reg, "status_reg");
    sc_trace(wf, sys.timer->irq_timer_o, "timer_interrupt");

    // Clear input ports
    irq_ext_s.write(false);
    irq_sw_s.write(false);

    // Reset
    cout << "@" << sc_time_stamp() << " Applying Reset..." << endl;
    rst_s.write(true);
    sc_start(5, SC_NS);

    // Release Reset
    cout << "@" << sc_time_stamp() << " Releasing Reset...\n" << endl;
    rst_s.write(false);

    // Load Timer test instructions
    sys.load_file("./hex/riscv_timer_program.hex");

    // Run system
    sc_start(36, SC_NS);

    // Verify results
    if (sys.irq_timer_s.read() == true) {
        cout << "PASS: Timer Interrupt Triggered Successfully" << endl << endl;
    }
    else {
        cout << "FAIL: Timer Interrupt not Triggered" << endl << endl;
    }

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}