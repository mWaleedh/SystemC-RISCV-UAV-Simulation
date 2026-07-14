/*
addi x1, x0, 0x08     // x1 = 0x08
csrrw x0, mstatus, x1 // Initialize mstatus to 0x08
addi x2, x0, 0x04     // x2 = 0x04
csrrs x3, mstatus, x2 // Read old mstatus (0x08) to x3 and OR it with x2. New mstatus = 0x0C
csrrs x4, mstatus, x0 // rs1 = x0. Read old mstatus (0x0C) to x4 and mstatus stays 0x0C
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
    sys.load_file("./hex/riscv_csrrs_program.hex");

    // Run system
    sc_start(30, SC_NS);

    // Verify results
    cout << "x3 = 0x08: " << (sys.cpu->registers[3] == 0x08 ? "PASS" : "FAIL") << endl;

    cout << "x4 = 0x0C: " << (sys.cpu->registers[4] == 0x0C ? "PASS" : "FAIL") << endl;

    cout << "mstatus = 0x0C: " << (sys.cpu->mstatus == 0x0C ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}