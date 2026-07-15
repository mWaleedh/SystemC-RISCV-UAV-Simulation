/*
addi x1, x0, 128        (x1 = 128 -> Bit 7)
csrrw x0, mstatus, x1   (mstatus.MPIE = 1, MIE = 0)
addi x2, x0, 32         (x2 = 0x20)
csrrw x0, mepc, x2      (mepc = 0x20)

mret                    (PC -> 0x20, MIE <- MPIE (1), in_interrupt <- 0)

addi x3, x0, 99         (x3 = 99 -> CPU ignored MRET)
nop
nop

0x20: addi x4, x0, 1    (x4 = 1 -> MRET correctly jumped)
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
    sys.load_file("./hex/riscv_mret_program.hex");

    // Run system
    sc_start(36, SC_NS);

    // Incorrect sequential instruction doesn't execute
    cout << "x3 = 0: " << (sys.cpu->registers[3] == 0 ? "PASS" : "FAIL") << endl;

    // PC changes to mepc and instruction executes
    cout << "x4 = 1: " << (sys.cpu->registers[4] == 1 ? "PASS" : "FAIL") << endl;

    // After MRET global interrupts must be enabled (bit 3 of mstatus)
    cout << "mstatus & 0x8 = 1: " << ((sys.cpu->mstatus & 0x8) != 0 ? "PASS" : "FAIL") << endl;

    // in_interrupt is cleared
    cout << "in_interrupt = 0: " << (sys.cpu->in_interrupt == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}