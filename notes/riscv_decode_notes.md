### How Instruction Fetch Works
During the Fetch stage, the CPU places the current Program Counter value onto the address bus and signals the memory to read by setting read_en to true. Because the memory is synchronous, the CPU waits one cycle for the memory to read the request, drops the read_en signal to prevent duplicate reads, and waits a second cycle for the memory to push the 32-bit instruction onto the data bus. The CPU then saves this value into its internal cur_inst register.

### How Instruction Decode Works
During the Decode stage, the CPU takes the 32-bit cur_inst and slices it into smaller, parts using bitwise shifts and masks. It identifies what operation to perform, which registers hold the input data, and where to save the result. It also reads the actual data from the source registers and calculates any immediate values.

### What the Instruction Parts Mean
- opcode (7 bits): The core identifier of the instruction category (for example 0x13 for I-type and 0x33 for R-type).
- rd (5 bits): The Destination Register index (0-31). This is where the final answer will be saved.
- rs1 (5 bits): The first Source Register index.
- rs2 (5 bits): The second Source Register index (only used in R-Type, S-Type, and B-Type instructions).
- funct3 (3 bits) & funct7 (7 bits): Extra identifier bits used alongside the opcode to specify the exact operation. For example, ADD and SUB share the same opcode, but funct7 tells the ALU which one to perform.

### How the Register File Works
The CPU contains an internal array of 32 elements, representing the registers x0 to x31. Each slot holds 32 bits of data. During Decode, the CPU reads from this array using the rs1 and rs2 indexes. During the Write Back stage, it saves the ALU's calculated result back into the array using the rd index.

### Why x0 Remains 0
The RISC-V specification specifies that register x0 is hardwired to 0. It cannot be overwritten. Our writeBack logic implements this by explicitly checking if (rd != 0) before saving data. This is a very useful feature. For example, a NOP is executed as ADDI x0, x0, 0.

### How ADDI Works
ADDI (Add Immediate) is an I-type instruction (opcode = 0x13, funct3 = 0x0). The ALU takes the data from rs1_data and adds it to the immediate value extracted from the instruction. The result is saved to rd.

### How ADD Works
ADD is an R-type instruction (opcode = 0x33, funct3 = 0x0, funct7 = 0x0). The ALU takes the data from rs1_data and adds it to rs2_data. The result is saved to rd.

### How SUB Works
SUB is an R-type instruction (opcode = 0x33, funct3 = 0x0, funct7 = 0x20). The ALU takes the data from rs1_data and subtracts it from rs2_data.The result is saved to rd.

### What Output Was Obtained
The terminal output provided a detailed, timestamped log of the CPU. It showed the CPU sending out addresses 0, 4, and 8, and accurately decoding the opcode, imm, and rd values for each. The testbench printed "PASS" three times, proving that the internal register values of the Register File are as expected.

### What Will Be Added Next
- Adding support for Load (LW) and Store (SW) instructions so the CPU can read and write data to the memory module using the MEM pipeline stage.
- Branching & Control Flow: Implementing instructions like BEQ or JAL so the CPU can make decisions, execute loops, and change the Program Counter dynamically instead of just adding 4 every cycle.