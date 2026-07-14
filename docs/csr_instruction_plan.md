### CSRRW (Control and Status Register Read and Write)
**Instruction Format:** CSRRW rd, csr, rs1
**Opcode:** 0x73 | **funct3:** 0x1

**Behavior:**
- Extract the 12-bit CSR address from the instruction (bits 31:20) and read the current value of the specified CSR.
- Route this old CSR value to the alu_res so it can be saved into the destination register (rd) during the WB stage.
- Take the data currently held in the source register (rs1) and write it into the selected CSR.
- If the destination register is x0, the CPU doesn't write to register file, as x0 is hardwired to zero in the RISC-V specification.
- If the destination register is x0 the old value is discarded.

### CSRRS (Control and Status Register Read and Set)
**Instruction Format:** CSRRS rd, csr, rs1
**Opcode:** 0x73 | **funct3:** 0x2

**Behavior:**
- Extract the 12-bit CSR address from the instruction (bits 31:20) and read the current value of the specified CSR.
- Route this old CSR value to the alu_res so it can be saved into the destination register (rd) during the WB stage.
- Perform a bitwise OR operation using the old CSR value and the value inside rs1. Write this newly computed result back into the CSR.
- If the source register (rs1) is x0, the instruction acts as a CSR read. The CPU ignores the write phase to avoid unintentionally modifying the CSR state.
- Similar to CSRRW, if the destination register is x0, the CPU prevents the old CSR value from writing to the register file.

### MRET (Machine Return from Trap)
**Instruction Format:** MRET
**Opcode:** 0x73 | **funct3:** 0x0 | **funct12:** 0x302

**Behavior:**
- Save the address stored in the Machine Exception Program Counter (mepc) into pc_next. This redirects the fetch stage to the exact instruction where the interrupt was triggered.
- The pending interrupt source must be cleared to prevent immediate re-triggering. In this implementation it is managed outside the CPU. The ISR writes 0 to the status_reg of the MMIO Timer which inturn clears the irq_timer_o pin.
- Re-enable global interrupts so the CPU can read future asynchronous events. This is achieved by setting the Machine Interrupt Enable (MIE) bit in the mstatus register.