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

    // Load test instructions
    sys.load_file("./hex/interrupt_enable_program.hex");

    // Run system
    sc_start(92, SC_NS);

    // If x3 = 1, the CPU successfully ignored the interrupt during all disabled combinations
    cout << "x3 = 1: " << (sys.cpu->registers[3] == 1 ? "PASS" : "FAIL") << endl;

    // If x9 = 99, the CPU successfully jumped to ISR when BOTH were enabled
    cout << "x9 = 99: "<< (sys.cpu->registers[9] == 99 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}