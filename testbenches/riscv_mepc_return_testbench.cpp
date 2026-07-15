/*
lui x1, 0x10000       
addi x2, x0, 1        
sw x2, 20(x1)         (compare = 1)
sw x2, 24(x1)         (control = 1)
addi x5, x0, 48       (mtvec = 0x30)
csrrw x0, mtvec, x5   
addi x4, x0, 128      
csrrw x0, mie, x4     (Enable MTIE)

addi x6, x0, 1        (BEFORE: x6 = 1)
addi x4, x0, 8        
csrrw x0, mstatus, x4 (Trigger interrupt)
addi x7, x0, 1        (x7 = 1 -> mepc)

addi x8, x0, 1        (x8 = 1 -> ISR)
sw x0, 28(x1)         (Clear timer)
mret                  (Return to mepc instruction)             
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

    // Load test instructions
    sys.load_file("./hex/mepc_return_program.hex");

    // Run system
    sc_start(93, SC_NS);

    // Before interrupt
    cout << "x6 = 1: " << (sys.cpu->registers[6] == 1 ? "PASS" : "FAIL") << endl;

    // ISR instruction
    cout << "x8 = 1: "<< (sys.cpu->registers[8] == 1 ? "PASS" : "FAIL") << endl;

    // After interrupt
    cout << "x7 = 1: " << (sys.cpu->registers[7] == 1 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}