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
    sys.load_file("./hex/branch_interrupt_overlap_program.hex");

    sc_start(15, SC_NS); 

    // Trigger interrupt after 15 cycles
    sys.timer->irq_timer_o.write(true);
    sc_start(1, SC_NS);
    sys.timer->irq_timer_o.write(false);

    // Run system
    sc_start(6, SC_NS);

    // Verify results
    cout << "x3 = 0: " << (sys.cpu->registers[3] == 0 ? "PASS" : "FAIL") << endl;

    cout << "x4 = 88: " << (sys.cpu->registers[4] == 88 ? "PASS" : "FAIL") << endl;

    cout << "x5 = 1: " << (sys.cpu->registers[5] == 1 ? "PASS" : "FAIL") << endl;
    
    cout << "branches_executed = 1: " << (sys.cpu->branches_executed == 1 ? "PASS" : "FAIL") << endl;

    cout << "timer_interrupts = 1: " << (sys.cpu->timer_interrupts == 1 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}