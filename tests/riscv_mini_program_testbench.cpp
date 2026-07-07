/* ADDI x1, x0, 20
ADDI x2, x0, 20
BEQ x1, x2, +8
SUB x1, x1, x2
ADDI x3, x0, 100
SW x3, 0(x1)
LW x4, 0(x1)
BNE x1, x4, +8
ADD x4, x0, x0
ADD x5, x3, x4
SUB x6, x5, x1
JAL x7, +8
ADDI x6, x0, 0
ADDI x8, x0, 64
JALR x9, 0(x8)
ADDI x8, x0, 0
NOP */

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
    sc_trace_file *wf = sc_create_vcd_trace_file("./tests/waveform/riscv_mini_program_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, sys.cpu->pc, "pc");
    sc_trace(wf, sys.cpu->pc_next, "pc_next");
    sc_trace(wf, sys.cpu->cur_inst, "cur_inst");
    sc_trace(wf, sys.cpu->opcode, "opcode");
    sc_trace(wf, sys.cpu->rd, "rd");
    sc_trace(wf, sys.cpu->rs1, "rs1");
    sc_trace(wf, sys.cpu->rs2, "rs2");
    sc_trace(wf, sys.cpu->imm, "immediate");
    sc_trace(wf, sys.cpu->alu_res, "alu_rest");
    sc_trace(wf, sys.mem->data_bus_i, "mem_data_in");
    sc_trace(wf, sys.mem->data_bus_o, "mem_data_out");
    sc_trace(wf, sys.mem->read_en_i, "read_en");
    sc_trace(wf, sys.mem->write_en_i, "write_en");
    sc_trace(wf, sys.cpu->branch_taken, "branch_taken");

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
    sys.load_file("./tests/hex/riscv_mini_program.hex");

    // Run system
    sc_start(74, SC_NS);

    // Verify results
    cout << "x1 = 20: " << (sys.cpu->registers[1] == 20 ? "PASS" : "FAIL") << endl;

    cout << "x2 = 20: " << (sys.cpu->registers[2] == 20 ? "PASS" : "FAIL") << endl;

    cout << "x3 = 100: " << (sys.cpu->registers[3] == 100 ? "PASS" : "FAIL") << endl;

    cout << "x4 = 100: " << (sys.cpu->registers[4] == 100 ? "PASS" : "FAIL") << endl;

    cout << "x5 = 200: " << (sys.cpu->registers[5] == 200 ? "PASS" : "FAIL") << endl;

    cout << "x6 = 180: " << (sys.cpu->registers[6] == 180 ? "PASS" : "FAIL") << endl;

    cout << "x7 = 48: " << (sys.cpu->registers[7] == 48 ? "PASS" : "FAIL") << endl;

    cout << "x8 = 64: " << (sys.cpu->registers[8] == 64 ? "PASS" : "FAIL") << endl;

    cout << "x9 = 60: " << (sys.cpu->registers[9] == 60 ? "PASS" : "FAIL") << endl;

    cout << "mem[20] = 100: " << (sys.mem->memory[20] == 100 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}