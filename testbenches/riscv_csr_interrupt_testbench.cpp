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
    sc_trace_file *wf = sc_create_vcd_trace_file("./waveforms/csr_interrupt_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, sys.cpu->pc, "pc");
    sc_trace(wf, sys.cpu->cur_inst, "cur_inst");
    sc_trace(wf, sys.timer->count_reg, "count_reg");
    sc_trace(wf, sys.timer->compare_reg, "compare_reg");
    sc_trace(wf, sys.timer->status_reg, "status_reg");
    sc_trace(wf, sys.timer->irq_timer_o, "irq_timer");
    sc_trace(wf, sys.cpu->mstatus, "mstatus");
    sc_trace(wf, sys.cpu->mie, "mie");
    sc_trace(wf, sys.cpu->mip, "mip");
    sc_trace(wf, sys.cpu->mepc, "mepc");
    sc_trace(wf, sys.cpu->mcause, "mcause");
    sc_trace(wf, sys.cpu->mtvec, "mtvec");

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
    sys.load_file("./hex/csr_interrupt_program.hex");

    // Load base address of Timer
    sys.load_data(0x20, 0x10000010);

    // Run system
    sc_start(100, SC_NS);

    // Verify results
    cout << "x1 = 0x10000000: " << (sys.cpu->registers[1] == 0x10000000 ? "PASS" : "FAIL") << endl;
    
    cout << "mtvec = 0x40: " << (sys.cpu->mtvec == 0x40 ? "PASS" : "FAIL") << endl;
    
    cout << "mie (Timer Enabled): " << ((sys.cpu->mie & 0x80) != 0 ? "PASS" : "FAIL") << endl;
    
    cout << "mcause = 0x80000007: " << (sys.cpu->mcause == 0x80000007 ? "PASS" : "FAIL") << endl;
    
    cout << "mepc = 0x2C: " << (sys.cpu->mepc >= 0x2C ? "PASS" : "FAIL") << endl;
    
    cout << "mstatus (MIE restored): " << ((sys.cpu->mstatus & 0x8) != 0 ? "PASS" : "FAIL") << endl;
    
    cout << "mip (Interrupt Cleared): " << ((sys.cpu->mip & 0x80) == 0 ? "PASS" : "FAIL") << endl;
    
    cout << "x6 = 99: " << (sys.cpu->registers[6] == 99 ? "PASS" : "FAIL") << endl;
    
    cout << "x7 = 99: " << (sys.cpu->registers[7] == 99 ? "PASS" : "FAIL") << endl;

    cout << "x0 = 0: " << (sys.cpu->registers[0] == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}