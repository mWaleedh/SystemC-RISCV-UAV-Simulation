### Why a Timer Peripheral is Needed
A CPU executes instructions at varying speeds depending on memory waits, pipeline stalls, and branching. Because of this, it cannot accurately track real-world time. A timer solves this by acting as a stopwatch that runs parallel to the CPU. This allows the system to trigger events at precise intervals.

### What is a Memory-Mapped Timer
The timer is implemented using Memory-Mapped I/O (MMIO). This means the timer's internal control registers are connected directly to the System Bus at specific physical addresses (0x10000010 to 0x1000001C). The CPU interacts with the timer using Load and Store instructions.

### How the Timer Count Works
The count_reg (mapped to 0x10000010) acts as a stopwatch. It's driven by the main system clock (clk_i), its internal hardware performs count_reg++ on every positive clock edge if enabled.

### How the Compare Register Works
The compare_reg (mapped to 0x10000014) acts as the limit or threshold. The CPU writes a target value to this register. On every clock cycle, the timer compares this with the count_reg (if count_reg >= compare_reg). When they are equal, it triggers the interrupt.

### How the Control Register Enables the Timer
The control_reg (mapped to 0x10000018) acts as a switch. To prevent the timer from running randomly, we check if ((control_reg & 0x1) == 1) before incrementing the counter. The CPU writes a 1 to this address to start the timer, and can write a 0 to pause it.

### How the Status Flag Works
The status_reg (mapped to 0x1000001C) is the alarm. When the count matches the compare value, the timer automatically sets this register to 1. The CPU can read this register to manually check if the timer has finished. The CPU can also write a 0 back to this register to disable the alarm.

### How the Interrupt Output is Generated
In addition to setting the status flag, The timer also sets irq_timer_o (Timer Interrupt) port to 1. This wire is routed through the System Bus directly into the CPU's irq_timer_i port.

### What Test Program Was Used
LW x1, 28(x0) -> Load the timer base address 0x10000010.
ADDI x2, x0, 5 -> Prepare the compare register value (5).
SW x2, 4(x1) -> Write 5 to the Compare Register.
ADDI x2, x0, 1 -> Prepare the enable bit (1).
SW x2, 8(x1) -> Write 1 to the Control Register to start timer.
LW x3, 0(x1)
BEQ x0, x0, 0 -> Run infinite loop.

### What Result Was Obtained
The testbench output perfectly matched the expected behavior. At 23ns, the System Bus sent the write request to the Compare Register. At 36ns, the Control Register was activated, starting the timer. While the CPU entered its infinite BEQ loop, the timer ran in the background. Exactly 5 clock cycles later at 41ns, the timer triggered the interrupt, and the testbench printed PASS.

### Waveform Explanation
- During the first few cycles, the CPU accesses low addresses (0x0, 0x4, etc) which are routed to Main Memory.
- At the SW instructions, the addr_bus holds addresses 0x10000014 and 0x10000018. The System Bus keeps the mem_write_en at 0 and sets the timer_write_en wire.
- Once the control register is written, the internal count_reg increments on every clock edge.
- When count_reg == 5, the irq_timer_s wire gets set to 1.

### Problems Faced
- In C++, bitwise & has a lower precedence to ==. Writing if (control_reg & 0x1 == 1) was evaluated wrongly by the compiler.

### Next Step
The hardware timer is successfully generating an interrupt signal, but the CPU is currently ignoring the irq_timer_i input pin. The next is implementing interrupt handling logic in the CPU where it pauses the current program, jumps to an Interrupt Service Routine (ISR), and resumes execution when finished.