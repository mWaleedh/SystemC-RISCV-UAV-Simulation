### What Was Added Today
Today, the CPU was successfully upgraded to interact with data memory. We implemented the LW and SW instructions. We also integrated a new testbench that verifies the CPU's ability to calculate memory addresses, store register data into the memory, and successfully load that data back into a different register. Finally, we added a function to load an entire hex program directly into memory.

### How Load Word Works
LW is an I-Type instruction (opcode = 0x03). It reads a 32-bit word from memory and saves it into a destination register. It requires a base address (stored in rs1), an offset (imm), and a target register (rd).

### How Store Word Works
SW is an S-Type instruction (opcode = 0x23). It takes a 32-bit word from a source register (rs2) and writes it into memory. It needs to read two registers (rs1 for the base address and rs2 for the data) and it uses the immediate value for offset. It does not write back to a register.

### How Address Calculation Works
In RISC-V, we take the base address and then add an offset to it.The base address is stored in rs1_data and the offset is stored in immediate.
The ALU simply adds rs1_data and imm and puts the result on the alu_res wire, and then it is sent over the addr_bus to memory.

### How the Memory Access Stage Works
- During the MEM stage, the CPU checks the opcode. 
- For SW, it sets write_en_o to true, pushes the calculated address (alu_res) onto the addr_bus_o, and pushes the data to be saved (rs2_data) onto the data_bus_o. It waits 2 cycles for the synchronous memory to write the data.
- For LW, it sets read_en_o to true and pushes alu_res to the addr_bus_o. It waits 1 cycle for the memory to see the request, drops the read enable, waits a 2nd cycle for the memory to push the data out, and captures data_bus_i into a temporary internal register called mem_read_data.
- For Math, it does nothing with the memory buses and just waits 1 cycle to keep pipeline timing consistent.

### Instruction Fetch vs. Data Memory Access
- Both stages interact with the exact same memory module.
- Instruction Fetch uses the program counter as the target address to retrieve 32-bit machine code.
- Data Memory uses alu_res as the target address to retrieve or store 32-bit variable data.

### How Write-Back Differs for LW, SW, and ALU
- The Write Back (WB) stage is the final step where the CPU updates its internal register file.
- For ALU Instructions the WB stage grabs the math result from the alu_res wire and saves it into registers[rd].
- For Load Instructions the WB stage ignores alu_res and instead grabs mem_read_data and saves it into registers[rd].
- For Store Instructions the WB stage does nothing. Data was sent to memory, so no internal registers need to be updated.

### What Test Program Was Used
We wrote a 6-instruction assembly program to verify the new pipeline paths:
ADDI x1, x0, 64
ADDI x2, x0, 25
SW x2, 0(x1)
LW x5, 0(x1)
ADD x6, x5, x2
NOP

### What Output Was Obtained
The testbench successfully simulated working of both LW and SW. Six automated checks confirmed that the registers updated correctly, the memory array accurately stored 25 at index 64, and the final ALU addition gave 50. All tests returned PASS.

### What Problems Were Faced
- Just like in the Fetch stage, the LW instruction required an additional cycle to ensure the CPU didn't read data_bus_i before the memory had time to push the data out.
- We had to add a new mem_read_data variable to safely store the memory data from the MEM stage over to the WB stage.

### What Will Be Added Next
With the CPU successfully doing math (ALU) and interacting with memory (Load/Store), the next step is adding Branching instructions (like BEQ, BNE) so the CPU can evaluate conditions, run loops, and change the Program Counter dynamically instead of just moving forward by 4 every cycle.