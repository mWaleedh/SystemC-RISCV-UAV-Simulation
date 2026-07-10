### Why Interrupt Support is Needed
Currently, our CPU has to halt and sit in a while loop to wait for the timer to finish. Adding Interrupt support allow the CPU to run main application code continuously and only stop when the hardware forces it to handle a sudden event.

### How the Timer Notifies the CPU
When the timer's count reaches its target, it sets the irq_timer_i hardware wire High. The CPU's main thread will be updated to monitor this port before every instruction cycle.

### What Happens When an Interrupt Becomes Active
If the CPU detects that irq_timer_i is High, it temporarily suspends normal execution. It will not fetch the next sequential instruction. Instead the CPU will go to the hardware interrupt address and execute instructions stored at that address.

### How the CPU Saves the Current PC
Before jumping to handle the interrupt, the CPU must remember where it left off so it can return later without corrupting the main program. The CPU will copy the value of the next unexecuted instruction's address from the main PC register into a backup register.

### How the CPU Jumps to the Interrupt Handler
Once the PC is safely backed up, the CPU will force the main PC to load a memory address. This location is where we will store our Interrupt Service Routine (ISR) designed to handle the event.

### How the Interrupt Status is Cleared
Inside the ISR, the CPU will execute a SW instruction to write a 0 to the Timer Status Register (0x1000001C). This reset the irq_timer_i wire back to Low.
Finally, the ISR will end with a special return instruction, which tells the hardware to copy the instruction address from MEPC back into the main PC and resume execution.

### What is Kept Simple in This Educational Model
- The interrupt_enable wire is hardcoded to True. Meaning interrupts can't be disabled.
- Real RISC-V uses many Control and Status Registers. We implement only the absolute minimum. One to save the PC and one for the target address.
- All interrupts will simply jump to one handler address rather than having a complex lookup table.