/*
lui x1, 0x10000       
addi x2, x0, 60       
csrrw x0, mtvec, x2     mtvec = 0x38
addi x3, x0, 10       
sw x3, 20(x1)           compare_reg = 10
addi x4, x0, 1        
sw x4, 24(x1)           Enable Timer
addi x5, x0, 128      
csrrw x0, mie, x5       Enable 8th bit of mie

addi x9, x0, 2          Delay = 2 iterations
addi x8, x8, 1          Increment loop counter
bne x8, x9, -4          Timer triggers interrupt but still waits 2 iterations

addi x6, x0, 8
csrrw x0, mstatus, x6   Enable global interrupts

jal x0, 0         Infinite loop

addi x10, x0, 99        x10 = 99 inside ISR
sw x4, 28(x1)           Clear timer interrupt
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

    // Load instructions
    sys.load_file("./hex/interrupt_masking_program.hex");

    // Run system
    sc_start(117, SC_NS);

    // Verify results
    // Delay counter
    cout << "x8 = 2: " << (sys.cpu->registers[8] == 2 ? "PASS" : "FAIL") << endl;

    // ISR instruction
    cout << "x10 = 99: " << (sys.cpu->registers[10] == 99 ? "PASS" : "FAIL") << endl;

    cout << "mstatus = Global interrupts enabled: " << (sys.cpu->mstatus == 0x08 ? "PASS" : "FAIL") << endl;

    cout << "mip = Interrupt cleared: " << (sys.cpu->mip == 0x00 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}