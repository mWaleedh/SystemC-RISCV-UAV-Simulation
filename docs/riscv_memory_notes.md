### What Was Done Today
The RISC-V CPU model and the Synchronous Memory model were integrated into a single system. Instead of the testbench feeding instructions to the CPU, the CPU now autonomously requests and reads real instructions from the memory module.

### What Modules Were Connected
We created a top-level module called system_top. Inside it, we instantiated the risc_v_model and the memory_model, and their input/output ports were connected to each other using internal signals.

### What the CPU Does
The CPU acts as the brain of the system. It runs an infinite loop containing a 5-stage pipeline, executing 32-bit machine code. It generates memory addresses, decodes instructions, and controls the flow of data through the system.

### What the Memory Does
The Memory acts as a synchronous storage unit. It listens communicates with the CPU using control signals and system buses. When commanded, it either saves data into its internal array or sends the stored data to the CPU.

### How the CPU Fetches Instructions
During the Fetch stage, the CPU places the value of its Program Counter (PC) onto the addr_bus, and sets the read_en signal to true, and calls wait() to let the clock cycle tick, signaling the memory to respond.

### How the PC Controls the Memory Address
The Program Counter is an internal register that holds the exact memory address of the current instruction. By continuously placing this value onto the external address bus, the CPU tells the memory exactly which index to look at.

### How Memory Returns the Instruction
On the rising edge of the clock, the memory module detects read_en == true. It reads the requested address and sends the 32-bit value stored in that address over to the CPU using the data_bus.

### Why Synchronous Memory Needs One Clock Cycle to Respond
Because the memory uses a clocked thread (SC_CTHREAD), it can only evaluate inputs and update outputs exactly when the clock edge rises. Therefore, it takes 1 full clock cycle for the memory to see the CPU's request and push the data out. This requires the CPU to wait an extra cycle during the Fetch stage.

### Why the PC Increases by 4
The standard RISC-V architecture uses 32-bit instructions which is 4 bytes. Since memory is byte-addressed meaning each address points to a 1-byte slot, the CPU must jump forward by 4 addresses to reach the start of the next 32-bit instruction block.

### What Dummy Instructions Are Used
- 0x00000013 at Address 0
- 0x00500093 at Address 4
- 0x00100113 at Address 8

### What IF, ID, EX, MEM, WB Are Doing
- IF (Instruction Fetch): The CPU sends the PC to memory and reads the incoming instruction.
- ID (Instruction Decode): The 32-bit instruction is divided into the opcode, register targets, and immediate values.
- EX (Execute): The ALU performs mathematical and logical operations (currently acting as a placeholder).
- MEM (Memory Access): If the instruction is a Load or Store, the CPU interacts with memory.
- WB (Write Back): The final results are written back to the internal register file, and the PC is incremented.

### What Output Was Obtained
The terminal output provided a detailed, timestamped log of the CPU. It showed the CPU sending out addresses 0, 4, and 8, and accurately decoding the opcode, imm, and rd values for each. The testbench printed "PASS" three times, proving the CPU's internal cur_inst register perfectly matched the memory.

### What Problems Were Faced
- The CPU initially tried to read the data bus before the memory had time to update it. This was fixed by adding a second wait() in the Fetch stage to account for the memory's 1-cycle latency.
- We could not push the dummy instructions onto the data bus from the testbench because the system_top has no ports for the busses or control signals as they are for internal communication of modules. We solved this by making load_data() function in to load instructions into memory directly.
- The CPU was calling read on memory two times. This was because we added two waits in the fetch stage to avoid reading old data. To fix this we separated the two waits and moved the read_en = false in between them so that the memory doesn't see read_en = true for two cycles in a row.

### What Will Be Added Next
After CPU-memory instruction fetch works, we will add:
- Basic instruction decoding
- Simple ALU Operation
- Register Read/Write behavior
- Load/Store Instruction behavior
- Address Decoder