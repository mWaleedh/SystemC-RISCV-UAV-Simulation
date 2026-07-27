/*
addi x1, x0, 1
addi x2, x0, 2
beq x1, x2, fail
bge x1, x2, fail
blt x2, x1, fail
bne x1, x2, pass

fail:
addi x5, x0, 99
addi x5, x0, 99

pass:
addi x4, x0, 10
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
    sys.load_file("./hex/one_bit_mixed_branches_program.hex");

    // Run system
    sc_start(10, SC_NS);

    // Verify results
    cout << "branches_executed = 4: " << (sys.cpu->branches_executed == 4 ? "PASS" : "FAIL") << endl;

    cout << "branches_taken = 1: " << (sys.cpu->branches_taken == 1 ? "PASS" : "FAIL") << endl;

    cout << "branch_mispredictions = 1: " << (sys.cpu->branch_mispredictions == 1 ? "PASS" : "FAIL") << endl;

    cout << "pipeline_flushes = 1: " << (sys.cpu->pipeline_flushes == 1 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}