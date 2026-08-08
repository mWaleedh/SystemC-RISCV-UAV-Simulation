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

    // Clear input ports
    irq_ext_s.write(false);

    // Reset
    cout << "@" << sc_time_stamp() << " Applying Reset..." << endl;
    rst_s.write(true);
    sc_start(5, SC_NS);

    // Release Reset
    cout << "@" << sc_time_stamp() << " Releasing Reset...\n" << endl;
    rst_s.write(false);

    // Load JAL instructions
    sys.load_file("./hex/software_interrupt_program.hex");

    // Run system for JAL
    sc_start(25, SC_NS);

    // Verify results for JAL
    cout << "x29 = 0x80000003: " << (sys.cpu->registers[29] == 0x80000003 ? "PASS" : "FAIL") << endl;

    cout << "x28 = 99: " << (sys.cpu->registers[28] == 99 ? "PASS" : "FAIL") << endl;

    cout << "msip_reg = 0: " << (sys.tlm_msip->msip_reg == 0? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}