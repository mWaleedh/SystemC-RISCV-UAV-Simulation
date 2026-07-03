 #include <systemc.h>
#include "../src/system_top.cpp"
using namespace std;

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
    sc_trace_file *wf = sc_create_vcd_trace_file("riscv_branch_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, sys.cpu->pc, "pc");
    sc_trace(wf, sys.cpu->cur_inst, "cur_inst");
    sc_trace(wf, sys.cpu->opcode, "opcode");
    sc_trace(wf, sys.cpu->rs1, "rs1");
    sc_trace(wf, sys.cpu->rs2, "rs2");
    sc_trace(wf, sys.cpu->imm, "branch_immediate");
    sc_trace(wf, sys.cpu->branch_taken, "branch_taken");
    sc_trace(wf, sys.cpu->pc_next, "branch_target");
    sc_trace(wf, sys.read_en_s, "read_en");
    sc_trace(wf, sys.write_en_s, "write_en");

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

    // Load instructions
    sys.load_file("./hex/riscv_branch_program.hex");

    // Run system
    sc_start(42, SC_NS);

    // Verify results
    cout << "x1 = 5: " << (sys.cpu->registers[1] == 5 ? "PASS" : "FAIL") << endl;

    cout << "x2 = 5: " << (sys.cpu->registers[2] == 5 ? "PASS" : "FAIL") << endl;

    cout << "x3 = 10: " << (sys.cpu->registers[3] == 10 ? "PASS" : "FAIL") << endl;

    cout << "x4 = 7: " << (sys.cpu->registers[4] == 7 ? "PASS" : "FAIL") << endl;

    cout << "x5 = 17: " << (sys.cpu->registers[5] == 17 ? "PASS" : "FAIL") << endl;

    cout << "x0 = 0: " << (sys.cpu->registers[0] == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}