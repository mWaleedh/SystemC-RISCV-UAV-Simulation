### What the Memory Model Does
This module is a 4-byte memory (RAM), having 256 slots. It acts as the external data storage for a CPU, allowing the processor to save and retrieve 32-bit data at specific addresses.

### Why it is Clock-Based
This project is clocked based so memory also needs to work with a synchronous design. This also aligns with modern DDR RAM design which only reads or writes on clock cycles.

### What Each Port Means
- clk_i: The main clock of the entire system. The whole memory module works on the rising edge of this clock.
- rst_i: Initializes the module to the initial state and clears memory when active.
- write_en_i: Tells the memory to save the incoming data into the required memory address.
- read_en_i: Tells the memory to output the stored data at a specific memory address.
- addr_bus_i: Specifies which index of the memory array to target for reading or writing.
- data_bus_i: Carries the 32-bit data that needs to be written into memory.
- data_bus_o: Carries the 32-bit data out of memory to the CPU.

### How Reset Works
When the reset signal (rst_i) is set HIGH, the module enters its reset/initial state logic block. It loops through the entire internal memory array and sets it to 0. It clears the data_bus_o port.

### How the Write Operation Works
When the write_en_i signal is 1, the memory reads the address sitting on the addr_bus_i. Assuming the address is within the valid 256-word range, it takes the 32-bit data currently in the data_bus_i and stores it directly into the internal array at that specific address index.

### How the Read Operation Works
When the read_en_i signal is 1, the memory looks up the array index specified by the addr_bus_i. It retrieves the 32-bit value currently stored at that location and writes it onto the data_bus_o wire so the CPU receive it.

### What Happens in the Idle State
If both read_en_i and write_en_i are false, the memory enters an idle state. The internal loop still triggers every clock cycle, but because neither operational condition is met, no data is written to the array, and no new data is pushed out. It holds its current state, keeping the last read value on the output bus.

### What Happens if Read and Write are Both Active
This causes a read/write conflict. The model detects this edge case and prints a Warning to the terminal. To solve the conflict, the Write operation is prioritized, saving the new data to memory instead of executing the read request.

### How Invalid Addresses are Handled
If the CPU requests an address that is greater than or equal to the SIZE of the memory array , the memory catches it to prevent a C++ segmentation fault. It prints an Error to the terminal. If it was specifically a read request, it sends 0 to the output bus.

### What Test Cases Were Performed
- Reset Test: Verified memory and output cleared to 0.
- Basic Read/Write 1: Wrote 0xABCD1234 to Address 0 and successfully read it back.
- Basic Read/Write 2: Wrote 0x12345678 to Address 4 and successfully read it back.
- Basic Read/Write 3: Wrote 0xDEADBEEF to Address 8 and successfully read it back.
- Idle Verification: Disabled read/write flags to confirm the memory state did not change.
- Conflict Test: Activated Read and Write simultaneously to verify the warning message and write-priority.
- Invalid Address: Sent a request to address 300 (out of bounds) to verify the error handling boundary checks.

11. What Output Was Obtained
Every valid operation generated a timestamped message confirming the operation type (Read/Write/Warning/Error), the targeted hexadecimal address, and the hexadecimal data transferred. Each test case has a PASS check, confirming that the actual simulated hardware behavior perfectly matched the expected behavior.

12. How this Memory Connects with the RISC-V Model Later
In a top-level file, the Memory Module will be wired directly to the RISC-V CPU. The CPU's output pins (addr_bus_o, data_bus_o, write_en_o, read_en_o) will connect directly into the Memory's input pins (addr_bus_i, data_bus_i, write_en_i, read_en_i). Finally, the Memory's data_bus_o will plug back into the CPU's data_bus_i. When the CPU executes a sw (Store) or lw (Load) instruction during its MEM stage, it will flip these control wires and communicate with this module.