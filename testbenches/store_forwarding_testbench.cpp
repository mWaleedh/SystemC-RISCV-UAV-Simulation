/* 
addi x1, x0, 50
addi x2, x0, 100
sw x1, 0(x2)
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
    sys.load_file("./hex/store_forwarding_program.hex");

    // Run system
    sc_start(9, SC_NS);

    // Verify results
    cout << "x1 = 50: " << (sys.cpu->registers[1] == 50 ? "PASS" : "FAIL") << endl;

    cout << "x2 = 100: " << (sys.cpu->registers[2] == 100 ? "PASS" : "FAIL") << endl;

    cout << "mem[100] = 50: " << (sys.mem->memory[100 / 4] == 50 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}