### What an Interrupt Means
An interrupt is a signal that forces the CPU to temporarily pause its normal sequential execution. It saves the current instruction and jumps to a specific set of instructions called ISR (Interrupt Service Routine) to handle the interrupt event immediately.

### Why a Timer Interrupt is Useful
A timer interrupt allows the system to keep track of real-world time and trigger events independent of the CPU's instruction flow. This removes the need for the CPU to waste cycles to count time for various reasons such as context switching.

### How the Timer Raises an Interrupt
Inside the memory-mapped timer module, a count_reg increments every clock cycle. When this count matches the compare_reg, the timer hardware automatically sets its internal status_reg to 1 and sets the external irq_timer_o wire High.

### How the CPU Detects an Interrupt
The CPU continuously evaluates its irq_timer_i input port. If this pin is High, global interrupts are enabled, and the CPU is not already processing another interrupt (!in_interrupt), it starts handling the interrupt.

### How the PC Changes During an Interrupt
When an interrupt is detected, the CPU saves the current Program Counter value into saved_pc. It then overwrites the Program Counter value with the memory address of the Interrupt Service Routine.

### How the Interrupt Handler Address is Selected
In the current implementation, the interrupt vector address is hardcoded to 0x80. When an interrupt occurs, the CPU jumps to this predefined memory location to execute the ISR.

### How the Interrupt is Cleared
Inside the ISR, the software executes a Store Word (SW) instruction which sets the Status Register (0x1000001C) to 0. This forces the timer to set the irq_timer_o wire to Low. Finally, the ISR executes a custom basic MRET (opcode = 0x73) instruction, which sets in_interrupt back to false and restores the Program Counter value by saving saved_pc to pc_next.

### What Test Program Was Used
- LW x1, 32(x0) -> Load the timer base address from memory
- ADDI x2, x0, 5 -> Prepare the compare register value (5)
- SW x2, 4(x1) -> Write 5 to the Compare Register
- ADDI x2, x0, 1 -> Prepare the enable bit (1)
- SW x2, 8(x1) -> Write 1 to the Control Register to start timer
- BEQ x6, x0, 0 -> Infinite wait loop
- ADDI x7, x0, 99 -> Main program executes this after the interrupt handler breaks the loop

*ISR Instructions*:
- SW x0, 12(x1) -> ISR writes 0 to the Status Register to clear the interrupt
- ADDI x6, x0, 99 -> ISR alters the wait loop condition (x6 = 99)
- MRET -> Return from interrupt

### What Result Was Obtained
The simulation successfully the timer interrupt being triggered and handled. The CPU entered the infinite loop, but when the timer matched at cycle 5, the CPU paused the loop and jumped to 0x80. The ISR cleared the timer logic and changed the register value to break the loop condition. After MRET was executed, the CPU returned to the main program, the loop condition became false, and executed the final instruction. The testbench verified both x6 and x7 equaled 99.

### What the Waveform Shows
- The count_reg increments the count every cycle.
- When count_reg == compare_reg, irq_timer port becomes 1.
- The CPU stops its normal execution and the PC changes to 0x80.
- The irq_timer_o wire drops back to 0 when the CPU executes the custom MRET function, and the PC returns to the main loop address.

### Current Limitations
This implementation relies on a simplified return logic using only bool flags like in_interrupt and variables like saved_pc instead of CSRs. The ISR address is hardcoded to 0x80, and it currently lacks an interrupt controller to handle multiple different interrupt sources or priorities.

### Next Step
The next step is to upgrade this simplified system to a more accurate implementation based on the RISC-V manual. This involves implementing Control and Status Registers (CSRs) such as mstatus, mie, mip, mepc, mcause, and mtvec to properly handle different modes, configurable handler addresses, and standard exception specifications.