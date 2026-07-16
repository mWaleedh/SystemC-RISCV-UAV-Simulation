### CSRRW Execution Flow
CSRRW is used to completely overwrite a Control and Status Register. The CPU reads the old value from the specified CSR, saves it in the destination register (rd), and then writes the unmodified value from the source register (rs1) directly into the CSR.

### CSRRS Execution Flow
CSRRS is used to set specific bits in a CSR without altering the others. The CPU reads the old CSR value into rd, performs a bitwise OR operation between the old CSR value and the value in rs1, and writes the modified result back into the CSR.

### CSR Read in EX
During the Execute stage, the CPU reads the current value from the target CSR. This read operation must happen here so the pipeline has the correct value available for the ALU to process and ready to be passed down the pipeline for write-back.

### CSR Write in WB
The actual update to the CSR's registers is delayed until the Write-Back (WB) stage. Doing this synchronizes the CSR state updates with the rest of the CPU's register file writes and prevents unintended state changes if an instruction is flushed earlier in the pipeline.

### Old CSR Value Written to rd
According to RISC-V specifications the original value of the CSR is always written to the destination register (rd). This allows the software to safely check the previous state while simultaneously updating it to a new state.

### x0 Special Cases
The x0 register (which is hardwired to zero) has special architectural behaviors to avoid unnecessary operations. If the destination register rd is x0, the CPU simply ignores the register file write. And if the source register rs1 is x0 (for CSRRS), the write operation is completely ignored, meaning the CSR remains unchanged.

### What Test Program Was Used
We used csr_interrupt_program.hex. It loads the timer peripheral base address, sets mtvec to 0x40, sets the timer compare register, and enables interrupts in mie and mstatus. The program then loops during normal execution until the timer reaches the compare value. The CPU jumps to the interrupt handler, which modifies a register, clears the timer status by writing 1 to it, and executes MRET to resume the normal program. In the end we verify the correct execution by comparing the expected and actual value of that specific register.

### Testbench Issue Found
- We encountered an issue where the testbench was overwriting the instruction memory. Previous testbench logic was initializing data at addresses that overlapped with the .hex instructions. This corrupted the interrupt handler code, causing the CPU to fetch garbage instructions.
- Additionally, we faced a problem where if the timer's compare value was too low, the interrupt would re-trigger before the CPU could finish executing MRET.

### How the Issue Was Corrected
- To fix the memory overwrite, we added bound checks to load_program and load_data functions in the memory_model.cpp. This not only fixed our issue of overwriting but it also added the ability to load multiple .hex programs into memory without overwriting the previous ones.
- To fix the repeating interrupt problem, the timer compare value was increased (to 50 or 100) to account for pipeline overhead, and the timer clear command (SW) was placed immediately before MRET. Along with that an additional check was added that stopped the counter from incrementing when the status register was 1 (meaning an interrupt is already being handled).

### Final PASS Results
The console log was as expected. The CPU successfully entered the interrupt handler, modified the required registers, and returned safely using MRET. The final state confirmed x1, x7, and x8 held the correct values. The CSRs mtvec (0x40), mie, mcause (0x80000007), and mepc (0x30) all matched the expected values. Furthermore, mstatus successfully restored the MIE bit, and the mip interrupt pending bit was properly cleared. All tests were passing as confirmed by the PASS/FAIL checks at the end.