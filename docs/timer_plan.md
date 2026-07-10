### Purpose of the Timer
A hardware timer acts as a stopwatch which runs parallel to the CPU. Because a CPU executes instructions at varying speeds it cannot accurately keep track of time by just counting instructions. The timer module solves this by counting clock cycles. This allows the system to trigger events at specific intervals.

### Timer Memory Map
The timer acts as a Memory-Mapped I/O (MMIO) peripheral, meaning the CPU controls it by reading and writing to specific physical addresses.
- 0x10000010: Reserved for Timer Count Register.
- 0x10000014: Reserved for Timer Compare Register.
- 0x10000018: Reserved for TImer Control Register.
- 0x1000001C: Reserved for Timer Status Register.threshold.

### Register Functionality
- Count Register: The CPU can read this at any time to see how much time has passed. The CPU can also write to it to reset the stopwatch.
- Compare Register: The CPU writes a target value here. If the CPU writes 100, the timer is instructed to trigger an interrupt when the count reaches 100.
- Control Register: The CPU must write a 1 to this address to enable counting, and a 0 to pause it.
- Status Register: When the timer triggers the interrupt, it automatically sets this to 1. The CPU can clear the alarm by writing a 0 back to this register.

### How the Timer Increments (Background Logic)
The timer is driven by the main system clock. On every positive clock edge, the timer's internal hardware checks the Control Register. If Bit 0 is Enabled, the timer performs count_reg++.

### How the Compare Match & Interrupt works
On every clock cycle, after incrementing, a comparator checks if (count_reg >= compare_reg). If this condition is true, the Timer does two things:
1. Sets the Status Register's flag to 1.
2. Sets the timer interrupt (irq_timer_o) port to High.

### How the CPU Interrupt Input is used (Future Implementation)
The CPU has an irq_timer_i input pin, but its not currently connected. In the future, the CPU will be updated with Control and Status Registers and Trap Logic. At the end of every instruction, the CPU will check the irq_timer_i wire. If it sees that the wire is High it will:
1. Pause its normal program.
2. Save its current Program Counter (PC) into a register.
3. Jump to an ISR (Interrupt Service Routine) to handle the timer interrupt.
4. Once the ISR finishes the CPU will jump back to the saved PC and resume the normal program.