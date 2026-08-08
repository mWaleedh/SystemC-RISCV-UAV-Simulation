/*

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
    sys.load_file("./hex/tlm_memory_program.hex");

    // Run system
    sc_start(50, SC_NS);

    // Verify results
    // Test SW and LW
    cout << "x5 = 0xDEADBEEF: " << (sys.cpu->registers[5] == 0xDEADBEEF ? "PASS" : "FAIL") << endl;

    // Test SB and LB
    cout << "x6 = 0xFFFFFFAA: " << (sys.cpu->registers[6] == 0xFFFFFFAA ? "PASS" : "FAIL") << endl;

    // Test LBU
    cout << "x7 = 0x000000AA: " << (sys.cpu->registers[7] == 0x000000AA ? "PASS" : "FAIL") << endl;

    // Test SH and LH
    cout << "x8 = 0xFFFFBEEF: " << (sys.cpu->registers[8] == 0xFFFFBEEF ? "PASS" : "FAIL") << endl;

    // Test LHU
    cout << "x9 = 0x0000BEEF: " << (sys.cpu->registers[9] == 0x0000BEEF ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}