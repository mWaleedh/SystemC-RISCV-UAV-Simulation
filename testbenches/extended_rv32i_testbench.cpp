/*
// --- LOGICAL INSTRUCTIONS ---
addi x1, x0, 15        // x1 = 0x0000000F
addi x2, x0, -16       // x2 = 0xFFFFFFF0
and x3, x1, x2         // x3 = 0x00000000
or x4, x1, x2          // x4 = 0xFFFFFFFF
xor x5, x1, x2         // x5 = 0xFFFFFFFF
andi x6, x1, 15        // x6 = 0x0000000F
ori x7, x2, 15         // x7 = 0xFFFFFFFF
xori x8, x2, 15        // x8 = 0xFFFFFFFF

// --- SHIFT INSTRUCTIONS ---
addi x9, x0, -8        // x9 = 0xFFFFFFF8
slli x10, x9, 1        // x10 = 0xFFFFFFF0
srli x11, x9, 1        // x11 = 0x7FFFFFFC (Logical)
srai x12, x9, 1        // x12 = 0xFFFFFFFC (Arithmetic)
addi x13, x0, 1        
slli x14, x13, 31      // x14 = 0x80000000 (Max Shift)
addi x15, x0, 33
sll x16, x9, x15       // x16 = 0xFFFFFFF0 
srl x17, x9, x15       // x17 = 0x7FFFFFFC 
sra x18, x9, x15       // x18 = 0xFFFFFFFC 

// --- BRANCH INSTRUCTIONS (Signed vs Unsigned) ---
addi x19, x0, -1       // 0xFFFFFFFF
addi x20, x0, 1        // 0x00000001
blt x19, x20, pass1    // -1 < 1 (Signed = True)
jal x0, fail
pass1:
bltu x19, x20, fail    // 0xFFFFFFFF < 1 (Unsigned = false)
bge x19, x20, fail     // -1 >= 1 (Signed = false)
bgeu x19, x20, pass2   // 0xFFFFFFFF >= 1 (Unsigned = True)
jal x0, fail
pass2:

// --- HAZARD and FORWARDING ---
sw x20, 0(x0)          // Store 1 at Mem[0]
lw x21, 0(x0)          // Load 1 from Mem[0]
add x22, x21, x21      // LOAD-USE HAZARD (x22 = 2)
slli x23, x22, 1       // EX-EX FORWARDING (x23 = 4)

// --- TERMINATION ---
addi x31, x0, 1
success_loop: jal x0, success_loop
fail: addi x31, x0, 0
fail_loop: jal x0, fail_loop
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
    sys.load_file("./hex/extended_rv32i_program.hex");

    // Run system
    sc_start(40, SC_NS);

    // Verify results
    // Logical Instructions Check
    cout << "AND  (x3 == 0x00000000): " << (sys.cpu->registers[3] == 0x00000000 ? "PASS" : "FAIL") << endl;
    cout << "OR   (x4 == 0xFFFFFFFF): " << (sys.cpu->registers[4] == 0xFFFFFFFF ? "PASS" : "FAIL") << endl;
    cout << "XOR  (x5 == 0xFFFFFFFF): " << (sys.cpu->registers[5] == 0xFFFFFFFF ? "PASS" : "FAIL") << endl;
    cout << "ANDI (x6 == 0x0000000F): " << (sys.cpu->registers[6] == 0x0000000F ? "PASS" : "FAIL") << endl;
    cout << "ORI  (x7 == 0xFFFFFFFF): " << (sys.cpu->registers[7] == 0xFFFFFFFF ? "PASS" : "FAIL") << endl;
    cout << "XORI (x8 == 0xFFFFFFFF): " << (sys.cpu->registers[8] == 0xFFFFFFFF ? "PASS" : "FAIL") << endl;

    // Shift Instructions Check
    cout << "SLLI (x10 == 0xFFFFFFF0): " << (sys.cpu->registers[10] == 0xFFFFFFF0 ? "PASS" : "FAIL") << endl;
    cout << "SRLI (x11 == 0x7FFFFFFC): " << (sys.cpu->registers[11] == 0x7FFFFFFC ? "PASS" : "FAIL") << endl;
    cout << "SRAI (x12 == 0xFFFFFFFC): " << (sys.cpu->registers[12] == 0xFFFFFFFC ? "PASS" : "FAIL") << endl;
    cout << "Max Shift (x14 == 0x80000000): " << (sys.cpu->registers[14] == 0x80000000 ? "PASS" : "FAIL") << endl;

    // Shift Masking Check
    cout << "SLL (x16 == 0xFFFFFFF0): " << (sys.cpu->registers[16] == 0xFFFFFFF0 ? "PASS" : "FAIL") << endl;
    cout << "SRL (x17 == 0x7FFFFFFC): " << (sys.cpu->registers[17] == 0x7FFFFFFC ? "PASS" : "FAIL") << endl;
    cout << "SRA (x18 == 0xFFFFFFFC): " << (sys.cpu->registers[18] == 0xFFFFFFFC ? "PASS" : "FAIL") << endl;

    // Hazard and Forwarding cCheck
    cout << "Load-Use stall (x22 == 2): " << (sys.cpu->registers[22] == 2 ? "PASS" : "FAIL") << endl;
    cout << "EX-EX Forward  (x23 == 4): " << (sys.cpu->registers[23] == 4 ? "PASS" : "FAIL") << endl;

    // Branch Signed/Unsigned Logic Check
    // If any of the branches did not work as intended)it would have jumped to x31 = 0
    cout << "Branch Logic (x31 == 1)  : " << (sys.cpu->registers[31] == 1 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}