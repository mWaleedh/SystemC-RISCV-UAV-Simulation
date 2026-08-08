/*
addi x1, x0, 5        x1 = 5
addi x2, x0, 5        x2 = 5

beq  x1, x2, +8       Taken (5 == 5, true)
addi x3, x0, 99       Skipped
addi x3, x0, 10       x3 = 10

bne  x1, x2, +8       Not taken (5 != 5, false)
addi x4, x0, 7        x4 = 7

add  x5, x3, x4       x5 = 17
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

    // Initialize system_top and connect input ports
    system_top sys("System_Top");
    sys.clk_i(clk_s);
    sys.rst_i(rst_s);
    sys.irq_ext_i(irq_ext_s);

    // VCD waveform trace
    sc_trace_file *wf = sc_create_vcd_trace_file("./waveforms/riscv_branch_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, sys.cpu->pc, "pc");
    sc_trace(wf, sys.cpu->if_id.inst, "cur_inst");
    sc_trace(wf, sys.cpu->id_ex.opcode, "opcode");
    sc_trace(wf, sys.cpu->id_ex.rs1, "rs1");
    sc_trace(wf, sys.cpu->id_ex.rs2, "rs2");
    sc_trace(wf, sys.cpu->id_ex.imm, "branch_immediate");
    sc_trace(wf, sys.cpu_data_read_en_s, "read_en");
    sc_trace(wf, sys.cpu_data_write_en_s, "write_en");

    // Clear input ports
    irq_ext_s.write(false);

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
    sc_start(16, SC_NS);

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