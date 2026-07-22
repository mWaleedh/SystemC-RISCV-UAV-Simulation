/* 
0x00: addi x1, x0, 0
0x04: addi x2, x0, 1
0x08: addi x3, x0, 2

loop_start:
0x0C: addi x3, x3, -1

0x10: bne x3, x0, loop_start

0x14: beq x1, x2, error
0x18: blt x2, x1, error
0x1C: bne x1, x2, success

error:
0x20: addi x5, x0, 99

success:
0x24: addi x4, x0, 10
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
    sys.load_file("./hex/always_not_taken_program.hex");

    // Run system
    sc_start(23, SC_NS);

    // Verify results
    // Loop executed 2 times
    cout << "x3 = 0: " << (sys.cpu->registers[3] == 0 ? "PASS" : "FAIL") << endl;

    // Reached till last correct instruction
    cout << "x4 = 10: " << (sys.cpu->registers[4] == 10 ? "PASS" : "FAIL") << endl;

    // Skipped wrong instruction
    cout << "x5 = 0: " << (sys.cpu->registers[5] == 0 ? "PASS" : "FAIL") << endl;

    cout << "branches_executed = 5: " << (sys.cpu->branches_executed == 5 ? "PASS" : "FAIL") << endl;

    cout << "branches_taken = 2: " << (sys.cpu->branches_taken == 2 ? "PASS" : "FAIL") << endl;

    cout << "branch_mispredictions = 2: " << (sys.cpu->branch_mispredictions == 2 ? "PASS" : "FAIL") << endl;

    cout << "pipeline_flushes = 2: " << (sys.cpu->pipeline_flushes == 2 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}