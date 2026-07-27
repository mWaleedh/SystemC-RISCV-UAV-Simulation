/*
addi x1, x0, 1          Setup branch condition
addi x2, x0, 1          
lui x4, 0x10000         x4 = 0x10000000 (Base address of MMIO devices)
addi x6, x0, 64         x6 = 0x40 (Data address in memory)

beq x1, x2, 24          Jumps to 0x28

addi x3, x0, 99         Incorrect instructions start
sw x3, 0(x4)            
sw x3, 0(x6)            
csrrw x0, 0x340, x3     
jal x0, 0               Incorrect instructions end

addi x5, x0, 100        Correct instruction
jal x0, 0               
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
    sys.load_file("./hex/branch_prediction_safety_program.hex");

    // Run system
    sc_start(15, SC_NS);

    // Verify results
    // Incorrect update
    cout << "x3 = 0: " << (sys.cpu->registers[3] == 0 ? "PASS" : "FAIL") << endl;

    // Correct update
    cout << "x5 = 100: " << (sys.cpu->registers[5] == 100 ? "PASS" : "FAIL") << endl;

    cout << "branches_executed = 1: " << (sys.cpu->branches_executed == 1 ? "PASS" : "FAIL") << endl;

    cout << "pipeline_flushes = 2: " << (sys.cpu->pipeline_flushes == 2 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}