### What are Branch Instructions
Branch instructions allow the CPU to make decisions like if-else statements and perform loops. Instead of executing instructions in a sequential line, branch instructions tell the CPU to dynamically jump to a different instruction in memory based on a condition.

### How BEQ Works
Branch if Equal is a B-Type instruction. During the Execute stage, the ALU compares the values inside two source registers (rs1 and rs2). If rs1_data == rs2_data, the branch is taken.

### How BNE Works
Branch if Not Equal is also a B-Type instruction. The ALU compares the two source registers, but this time, if rs1_data != rs2_data, the branch is taken. If they are equal, the branch is not taken.

### How the B-Type Immediate is Extracted
The RISC-V specification has the immediate bits scattered across the 32-bit instruction placing them at bits 31, 7, 25:30, and 8:11.
In our immediateGenerator(), we are using multiple bitwise shifts and masks to extract these pieces, and join them back together in the correct order, and then we sign-extend the 12th bit up to 32 bits.

### How PC is Updated for Branch Taken
If the ALU determines the branch condition is met, the CPU calculates a new target address by adding the extracted immediate value to the current Program Counter. The CPU then jumps to this new address.

### How PC is Updated for Branch Not Taken
If the condition is not met, the CPU ignores the immediate value. It updates the Program Counter normally to the next instruction in memory.

### Why Branch Instructions Do Not Use Write Back
Branch instructions do not produce a numerical answer that needs to be saved into a destination register. Their purpose is to just modify the Program Counter. Therefore, the Write Back stage does nothing to the register file during a branch instruction, it only updates the pc variable.

### What Test Program Was Used
We verified the branch logic using an 8-instruction assembly program.
ADDI x1, x0, 5
ADDI x2, x0, 5
BEQ x1, x2, +8
ADDI x3, x0, 99
ADDI x3, x0, 10
BNE x1, x2, +8
ADDI x4, x0, 7
ADD x5, x3, x4

### What Output Was Obtained
The terminal output showed BEQ comparing 5 and 5, deciding to take the branch, and updating the PC. As a result, the ADDI x3, x0, 99 instruction was completely bypassed, and x3 was correctly set to 10. The BNE evaluated as false, allowing the remaining sequential math to execute. The final result successfully evaluated to x5 = 17.

### What Problems Were Faced
Deciding where to calculate the new PC address. Initially I had it in the Write Back stage, but since in a real hardware implementation the calculation requires the ALU or dedicated adder. We solved this by using a pc_next variable and only assigning the next instruction address to the pc in Write Back stage.

### What Will Be Added Next
With basic Arithmetic (I/R-Type), Memory (Load/Store), and Conditional Branching (B-Type) complete, the final thing left to complete the basic processor is Unconditional Jumps. Implementing JAL and JALR will allow the CPU to perform function calls and returns!