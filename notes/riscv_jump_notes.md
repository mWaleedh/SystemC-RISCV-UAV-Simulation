### What JAL Does
JAL is an unconditional jump instruction used mainly for calling functions. It calculates a target address by adding an immediate value to the current Program Counter (pc + imm) and jumps to it. It also saves the return address (PC + 4) into a destination register (rd), so the program knows where to resume after the function finishes.

### What JALR Does
JALR is also used for unconditional jumps, but instead of an adding an offset in the PC, it adds an immediate value to a base address stored in a register (rs1_data + imm), and then clear the lowest bit to ensure instruction alignment. Like JAL, it also saves the return address (PC + 4) into the destination register (rd).

### How the J-Type Immediate is Extracted
JAL uses a 20-bit immediate (bits 31, 19:12, 20, 10:1). In the Decode stage, we use bitwise shifts and masks to extract and reassemble these segments. Because it represents a signed offset, it requires sign extension. The sign bit is located at bit 20, so if it is 1 we extend it to all 31-bits.

### How the I-Type Immediate is Reused for JALR
Even though JALR is a jump instruction, it is classified as an I-Type instruction (opcode 0x67). This means its 12-bit immediate is not the same as JAL. We reused our existing ADDI and LW immediate extraction logic by simply adding case 0x67: to the I-Type switch.

### How the Return Address is Calculated
In the Execute (EX) stage, the return address is calculated simply as pc + 4. We temporarily store this return address inside the alu_res wire variable.

### How the Return Address is Written to rd
During the Write-Back (WB) stage, the CPU checks if the opcode belongs to JAL (0x6F) or JALR (0x67). If it does, and the destination register is not x0, it writes the alu_res which stores the return address to the destination register (rd).

### How pc_next is Calculated
- For JAL: pc_next = pc + imm;
- For JALR: pc_next = (rs1_data + imm) & ~1;
The ~1 is used in JALR to convert lsb bit to 0

### How Skipped Instructions Were Verified
We placed trap instructions right in the middle of jump paths for example ADDI x4, x0, 99. If the CPU failed to jump, it would execute the trap, overwriting the register values. By checking that the registers retained the correct results at the end of the simulation, we proved the jumps successfully worked.

### What Testbench Was Used
- We verified the JAL logic using an 5-instruction assembly program.
ADDI x1, x0, 5
JAL x5, +8
ADDI x2, x0, 99
ADDI x2, x0, 10
ADD x3, x1, x2

- We verified the JAL logic using an 4-instruction assembly program.
ADDI x1, x0, target_address
JALR x6, 0(x1)
ADDI x4, x0, 99
ADDI x4, x0, 7

### What Output Was Obtained
The testbench output perfectly matched the expected simulation behavior. The terminal logged the jumps and register updates. We also had automated checks that confirmed the final register values to verify the correctly execution of CPU.

### Problems Faced
There was a bug in the immediate generator which caused the immediate value for J-type instructions to be an extremely high value. The problem was in the sign extension part where it was checking the 0x4FFFF bits to be equal to 1 in order to sign extend where it should have been 0x100000 (the 20th bit).

12. Next Step
Write an in depth testbench that tests the implementation and of all the implemented operations which include Arithmetic, Memory Access, Branching, and Unconditional Jumps.