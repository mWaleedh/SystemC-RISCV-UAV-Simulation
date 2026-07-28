/*
addi x1, x0, 64
csrrw x0, 0x305, x1
addi x2, x0, 0
lw x3, 0(x2)
addi x4, x3, 0
beq x0, x0, 12
jal x0, 0
jal x0, 0
jal x0, 12
jal x0, 0
jal x0, 0
addi x5, x0, 56
jalr x0, x5, 0
jal x0, 0
jal x0, 0
jal x0, 0
addi x6, x6, 1
mret
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
    sys.load_file("./hex/synchronous_fetch_pipeline_program.hex");

    sc_start(15, SC_NS); 

    sys.timer->irq_timer_o.write(true);
    sc_start(1, SC_NS);
    sys.timer->irq_timer_o.write(false);
    
    sc_start(10, SC_NS);

    // Verify results
    cout << "x4 = 0x05000093: " << (sys.cpu->registers[4] == 0x05000093 ? "PASS" : "FAIL") << endl;
    
    // Check if Interrupt executed
    cout << "x6 = 1: " << (sys.cpu->registers[6] == 1 ? "PASS" : "FAIL") << endl;
    
    cout << "branches_executed = 1: " << (sys.cpu->branches_executed == 1 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}