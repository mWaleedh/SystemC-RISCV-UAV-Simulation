/*
lui x1, 0x10000         Timer base address
addi x2, x0, 60
csrrw x0, mtvec, x2     mtvec = 0x3C
addi x9, x0, 0          Set counter = x9 to 0
addi x3, x0, 10       
sw x3, 20(x1)           compare_reg = 10
addi x4, x0, 1        
sw x4, 24(x1)           Enable timer
addi x5, x0, 128      
csrrw x0, mie, x5       Enable 8th bit of mie
addi x6, x0, 8        
csrrw x0, mstatus, x6   Enable 3rd bit of mstatus

addi x7, x0, 2          Target count = 2
bne x9, x7, -4          Loop here until x9 = 2
addi x10, x0, 99        After 2 interrupts set x10 = 99

addi x9, x9, 1          Increment counter
sw x4, 28(x1)           Clear timer interrupt
mret                    Return from interrupt
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
    sys.load_file("./hex/periodic_interrupt_program.hex");

    // Run system
    sc_start(154, SC_NS);

    // Verify results
    cout << "x9 = 2: " << (sys.cpu->registers[9] == 2 ? "PASS" : "FAIL") << endl;

    cout << "x10 = 99: " << (sys.cpu->registers[10] == 99 ? "PASS" : "FAIL") << endl;

    cout << "x0 = 0: " << (sys.cpu->registers[0] == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}