/*
addi x1, x0, 0x55    // x1 = 0x55
csrrw x2, mtvec, x1  // Write x1 to mtvec and read old mtvec (0x0) into x2
csrrw x3, mtvec, x0  // Write x0 to mtvec and read old mtvec (0x55) into x3
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

    // Load instructions
    sys.load_file("./hex/riscv_csrrw_program.hex");

    // Run system
    sc_start(18, SC_NS);

    // Verify results
    cout << "x2 = 0: " << (sys.cpu->registers[2] == 0 ? "PASS" : "FAIL") << endl;

    cout << "x3 = 0x55: " << (sys.cpu->registers[3] == 0x55 ? "PASS" : "FAIL") << endl;

    cout << "x0 = 0: " << (sys.cpu->registers[0] == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}