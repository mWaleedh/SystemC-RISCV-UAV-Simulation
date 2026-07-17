### Implemented Instructions
Currently the following instructions have been implemented:
- **I-type**: ADDI, ANDI, ORI, XORI, LW
- **R-type**: ADD, SUB, AND, OR, XOR
- **S-type**: SW
- **B-type**: BEQ, BNE
- **J-type**: JAL, JALR
- **U-type**: LUI

### Implemented CSR instructions
Currently the following instructions have been implemented:
- CSRRW (Atomic Read/Write CSR)
- CSRRS (Atomic Read and Set Bit CSR)

### Implemented machine CSRs
- mstatus at 0x300: Handles global interrupt enable MIE at bit 3 and previous interrupt enable at bit 7
- mie at 0x304: Masks specific interrupts, like Timer enable at bit 7
- mtvec at 0x305: Holds the hardcoded jump destination for the handler
- mepc at 0x341: Holds the return address
- mcause at 0x342: Holds the trap reason, such as Machine Timer Interrupt
- mip at 0x344: Flags an active, uncleared interrupt

### Timer Memory Map
**0x10000000:** Control Register. Used to Enable/Disable Timer
**0x10000014:** Compare Register. Sets the Cycle Threshold for Timer
**0x10000018:** Count Register. Holds the current Timer value.
**0x1000001C:** Status Register. LSB indicates whether the interrupt was triggered or not. It can be cleared to remove interrupt.

### Timer periodic behavior
- The timer count register increments independently every clock cycle.
- When the count register matches the compare register, the timer peripheral asserts a pending signal and sets the count register to 0.
- The timer stops counting until the pending signal is acknowledged by the CPU and set to 0.

### Write-one-to-clear status behavior
- The Status Register at 0x1000001C acts as a write-one-to-clear register.
- Software must execute a store instruction to write a 1 to the LSB of this address to acknowledge the interrupt.
- This action sets the pending bit low and clears the interrupt request starting the Timer again.

### Interrupt acceptance condition
A timer interrupt stops normal execution only if all three of the following conditions are met:
- Pending: The timer compare match has occurred, making the mip bit active.
- Unmasked: The timer interrupt is specifically enabled using the mie register.
- Global Enable: The CPU is accepting interrupts, meaning the mstatus bit is high.

### mepc behavior
- Saves the address of the next unexecuted instruction when an interrupt was triggered.
- Allows the CPU to return from the ISR and make sure no instructions are skipped or repeated upon returning.

### mcause behavior
- Automatically updates with the exception code when an interrupt is taken.
- Records the specific reason, identifying it as a Machine Timer Interrupt.

### mtvec behavior
Stores the hardcoded base address of the ISR.
The hardware forces the Program Counter to this exact address immediately upon accepting an interrupt.

### MRET behavior
- Hardware instruction used at the end of an interrupt handler.
- Restores the Program Counter directly from the mepc register.
- Enables global interrupts by copying the MPIE bit back to the MIE bit in the mstatus register.

### Completed testbenches
- periodic_interrupt_test: Validates timer configuration, interrupt firing, and safely returning to the main loop.
- interrupt_masking_test: Proves the timer can be ignored while global interrupts are disabled, then immediately trap once enabled.
- mepc_boundary_test: Verifies instruction boundary during a series of CSR and Store instructions.

### Current pipeline structure
- The datapath is logically divided into IF, ID, EX, MEM, and WB stages.
- Operates as a multi-cycle, non-pipelined architecture.
- Only one instruction is in the pipeline at a time.

### Known limitations
- Pipeline flush logic is not implemented because there are no overlapping instructions in the pipeline yet.
- Data and control hazard detection are currently unnecessary but will be required immediately after pipelining.
- Structural hazards are naturally bypassed by the multi-cycle nature of the current design.

### Next pipeline work
- Move from the current multi-cycle datapath into a 5-stage pipeline.
- Implement IF to ID, ID to EX, EX to MEM, and MEM to WB pipeline registers.
- Add forwarding logic to resolve RAW data hazards
- Design flush logic to remove instructions from the pipeline.