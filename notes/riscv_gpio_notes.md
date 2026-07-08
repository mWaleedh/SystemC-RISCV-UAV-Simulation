### Why Memory-Mapped I/O is Used
RISC-V relies only on Memory_Mapped I/O and that's our biggest reason of using it. By wiring peripherals directly into the memory address space, the CPU doesn't need any special instructions to communicate with hardware. It simply sends the standard Load/Store instructions to GPIO through System Bus and it can drive an LED or read a button press.

### What GPIO Means
GPIO stands for General-Purpose Input/Output. It represents the physical metal pins on a microcontroller that can be programmed to either read external signals like from a button or send digital signals out like lighting an LED or turning a motor.

### How the CPU Accesses GPIO using LW/SW
- To control the GPIO, the CPU must calculate the physical address assigned to the peripheral.
- To write, the CPU loads a base address into a register, places the desired state into a source register, and executes SW x2, 0(x1).
- To read, the CPU executes LW x3, 4(x1), which pulls the state of the pins from the input address offset and saves it into destination register x3.

### What the Memory Map Is
The memory map dictates which addresses belong to which physical hardware modules. Our current 32-bit address space is mapped as follows:
- 0x00000000 - 0x00000FFF: Main Memory
- 0x10000000: GPIO Output Register
- 0x10000004: GPIO Input Register

### How the System Bus Routes Addresses
- If the address is < 0x00000FFF, the bus sets mem_write_en or mem_read_en to activate the Main Memory.
- If the address is 0x10000000 or 0x10000004, the bus routes the enable signals specifically to the GPIO module.
- Because only one module is enabled at a time, inactive modules output 0 on their data buses. The system bus merges the returning data using a bitwise OR (|) and sends the active data safely back to the CPU.

### How GPIO Output Works
When the System Bus forwards a write request to 0x10000000, the GPIO module receives it and saves the incoming 32-bit data into its internal output_reg. In our simulation, we added print GPIO: LED ON if the data is 1, and LED OFF if it is 0.

### How GPIO Input Works
When the System Bus forwards a read request to address 0x10000004, the GPIO module reads the input pins and forwards it to the System Bus. In our simulation, we hardcoded a dummy variable input_reg = 1 to represent a button being pressed. The module pushes this value onto its data_bus_o port, where it travels through the System Bus, into the CPU, and is saved in the destination register during the Write-Back stage.

### What Test Program Was Used
LW x1, 28(x0)
ADDI x2, x0, 1
SW x2, 0(x1)
ADDI x2, x0, 0
SW x2, 0(x1)
LW x3, 4(x1)
NOP

### What Result Was Obtained
The console log was as expected with printed "GPIO: LED ON", followed by "GPIO: LED OFF", and finally input register value was successfully stored in register x3. All tests were passing as confirmed by the PASS/FAIL checks at the end.

### Problems Faced
We couldn't wire both the Memory and GPIO directly into the CPU's input data port simultaneously. The System Bus had to be implemented to funnel the input signals using bitwise masking logic.

### Next Step
With the System Bus created and basic peripherals working, the next step involves creating a memory-mapped hardware timer module at address 0x10000010.