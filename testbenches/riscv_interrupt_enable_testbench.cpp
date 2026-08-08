/*
csrrw x0, mie, x0       (Clear mie)
csrrw x0, mstatus, x0   (Clear mstatus)

lui x1, 0x10000         (Base timer address)
addi x2, x0, 1        
addi x6, x0, 5          (Load 5 for timer compare)
sw x6, 20(x1)           (compare_reg = 5)
sw x2, 24(x1)           (control_reg = 1)

addi x5, x0, 76         (mtvec = 0x4C)
csrrw x0, mtvec, x5   

addi x4, x0, 128      
csrrw x0, mie, x4       (Test 1: mie = 1, mstatus = 0 -> No Interrupt)

csrrw x0, mie, x0       (Clear mie back to 0)

addi x4, x0, 8        
csrrw x0, mstatus, x4   (Test 2: mie = 0, mstatus = 1 -> No Interrupt)

addi x3, x0, 1          (x3 = 1 -> Both tests passed)

addi x4, x0, 128      
csrrw x0, mie, x4       (Test 3: mie = 1, mstatus = 1 -> Interrupt)

addi x3, x0, 2          (Never executes)
j 0x48                  

addi x9, x0, 99         (x9 = 99 -> Proof we made it!)
sw x0, 28(x1)           (status_reg = 0 -> clear interrupt)
j 0x54                  
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

    // Load test instructions
    sys.load_file("./hex/interrupt_enable_program.hex");

    // Run system
    sc_start(37, SC_NS);

    // If x3 = 2, the CPU successfully ignored the interrupt during all disabled combinations
    cout << "x3 = 2: " << (sys.cpu->registers[3] == 2 ? "PASS" : "FAIL") << endl;

    // If x9 = 99, the CPU successfully jumped to ISR when BOTH were enabled
    cout << "x9 = 99: "<< (sys.cpu->registers[9] == 99 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}