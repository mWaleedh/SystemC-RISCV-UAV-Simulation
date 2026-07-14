/*
addi x1, x0, 0xAA     // x1 = 0xAA
csrrw x0, mtvec, x1   // Write x1 to mtvec

addi x2, x0, 0x11     // x2 = 0x11
csrrw x0, mstatus, x2 // Set mstatus to 0x11

csrrs x3, mstatus, x0 // Read mstatus to x3 and don't write (rs1 = x0)
csrrs x0, mstatus, x1 // Read x0 and write (mstatus = 0x11 | 0xAA = 0xB)
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
    sys.load_file("./hex/csr_x0_program.hex");

    // Run system
    sc_start(36, SC_NS);

    // Verify results
    cout << "mtvec = 0xAA: " << (sys.cpu->mtvec == 0xAA ? "PASS" : "FAIL") << endl;

    cout << "x3 = 0x11: " << (sys.cpu->registers[3] == 0x11 ? "PASS" : "FAIL") << endl;

    cout << "mstatus = 0xBB: " << (sys.cpu->mstatus == 0xBB ? "PASS" : "FAIL") << endl;

    cout << "x0 = 0: " << (sys.cpu->registers[0] == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}
