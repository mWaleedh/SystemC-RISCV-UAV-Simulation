# RISC-V Pipeline PC Redirect Priority
In a pipelined architecture, multiple stages can attempt to modify the Program Counter (PC) or modify the control flow simultaneously. To prevent undefined behavior, the Next PC logic and pipeline flush must be evaluated in a strict priority order. 

## Priority Hierarchy
### 1. Reset (Highest Priority)
- **Trigger:** rst_i goes High.
- **Action:** Overrides all other pipeline operations. Immediately forces PC = 0x00000000.
- **Pipeline State:** Completely flushes IF/ID, ID/EX, EX/MEM, and MEM/WB. Resets all CSRs, interrupts, and output ports. 

### 2. Synchronous Exception (Future Implementation)
- **Trigger:** Execution errors
- **Action:** Forces PC to the trap handler (mtvec). 
- **Pipeline State:** Flushes all instructions behind the faulting instruction. 

### 3. Accepted Interrupt
- **Trigger:** Asynchronous external event (irq_timer_i, irq_ext_i) if mstatus.MIE == 1.
- **Action:** Forces PC to the interrupt handler (mtvec), saves the current PC to mepc.
- **Pipeline State:** Flushes instructions currently in Fetch and Decode.

### 4. MRET (Machine Return)
- **Trigger:** Execution of the mret instruction in the EX stage.
- **Action:** Restores PC to the value stored in mepc. And restores global interrupts.
- **Pipeline State:** Flushes the IF and ID stages (the sequential instructions fetched directly after the mret).

### 5. Branch or Jump Correction
- **Trigger:** Taken branch (BEQ, BNE, etc), JAL, or JALR evaluated in the EX stage.
- **Action:** Modifies PC to the calculated target address.
- **Pipeline State:** Flushes the IF and ID stages (mispredicted path). 

### 6. Normal Sequential PC
- **Trigger:** Default state when no control hazards or traps exist.
- **Action:** PC = PC + 4.
- **Pipeline State:** Normal data flow. No flushes required.

### 7. Stall Behavior (Special Case)
- **Trigger:** Load-use hazard detected in Decode, or a multi-cycle memory latency.
- **Action:** Pauses the PC. Fetches the current PC again instead of updating PC to PC + 4.
- **Pipeline State:** Inserts bubbles (NOPs) into the stalled stage, and freezes previous registers.