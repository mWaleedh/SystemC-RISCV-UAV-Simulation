/*
LW  x3, 0(x0)  
ADD x4, x3, x3 
NOP
NOP
NOP
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
    sys.load_file("./hex/load_use_hazard_program.hex");

    sys.load_data(32, 100);

    // Run system
    sc_start(10, SC_NS);

    // Verify results
    cout << "x3 = 100: " << (sys.cpu->registers[3] == 100 ? "PASS" : "FAIL") << endl;

    cout << "x4 = 200: " << (sys.cpu->registers[4] == 200 ? "PASS" : "FAIL") << endl;

    cout << "x0 = 0: " << (sys.cpu->registers[0] == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}