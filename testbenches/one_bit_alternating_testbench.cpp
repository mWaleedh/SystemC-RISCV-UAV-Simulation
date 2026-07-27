/*
addi x1, x0, 2
addi x2, x0, 1
addi x3, x0, 0

loop:
beq x3, x0, flip1

flip0:
addi x3, x0, 0
jal x0, dec_loop

flip1:
addi x3, x0, 1

dec_loop:
addi x1, x1, -1
bne x1, x0, loop

success:
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
    sys.load_file("./hex/one_bit_alternating_program.hex");

    // Run system
    sc_start(28, SC_NS);

    // Verify results
    cout << "branches_executed = 4: " << (sys.cpu->branches_executed == 4 ? "PASS" : "FAIL") << endl;

    cout << "branches_taken = 2: " << (sys.cpu->branches_taken == 2 ? "PASS" : "FAIL") << endl;

    cout << "branch_mispredictions = 4: " << (sys.cpu->branch_mispredictions == 4 ? "PASS" : "FAIL") << endl;

    cout << "pipeline_flushes = 4: " << (sys.cpu->pipeline_flushes == 5 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}