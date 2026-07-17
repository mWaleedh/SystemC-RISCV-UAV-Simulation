### Current IF stage
- Fetches the next 32-bit instruction from instruction memory using the current value of the Program Counter.
- In the current multi-cycle design, this stage completes its operation and passes the instruction forward before any other stage activates.

### Current ID stage
- Decodes the fetched 32-bit instruction into opcode, funct3, funct7, etc to determine the exact operation.
- Accesses the register file to read the values of up to two source registers simultaneously.
- Routes the instruction through the immediate generation unit to sign-extend immediate values based on the specific instruction type.
- Generates all necessary control signals needed for MRET and CSRs.

### Current EX stage
- Uses the Arithmetic Logic Unit to perform mathematical computations and bitwise logical operations.
- Calculates the actual target address for branch and jump instructions using the Program Counter and the generated immediate offset.
- Evaluates branch conditions by comparing the two source register values to determine if a branch should be taken.
- Handles the initial read phase for Control and Status Register instructions to extract values from machine registers like mtvec or mstatus.

### Current MEM stage
- Communicates with the main data memory and the memory-mapped peripheral space, including the timer base address at 0x10000000 through the System Bus.
- Performs load operations to retrieve data from memory to store into Register File.
- Performs store operations to write data from the register file out to memory or to trigger write-one-to-clear behaviors in the timer status register.

### Current WB stage
- Writes the final computed results back into the destination register within the main register file.
- Selects between data loaded from memory, results computed by the ALU, or upper immediate values depending on the instruction type.
- Writes to the Control and Status Register instructions like mstatus or mie.
- Ensures that register x0 remains zero, discarding any write attempts.

### Pipeline registers between stages
- IF to ID register will be addedfor the fetched instruction and the Program Counter for decoding.
- ID to EX register will be added for decoded control signals, read register data, sign-extended immediate, and the Program Counter.
- EX to MEM register will be added for the computed ALU result, the store data, the branch target, and memory control signals.
- MEM to WB register will be added to hold loaded memory data, the ALU result, and the final writeback control signals.

### Current valid bits
- No valid bits are currently implemented anywhere in the processor datapath.
- Because the design is currently strictly multi-cycle, every instruction operates sequentially and completes its five stages.
- Valid bits will need to be added to distinguish between valid instructions and bubbles.

### Current flushing logic
- There is no flushing logic present in the current architecture.
- Without overlapping pipeline stages, there are never any newer, mistakenly fetched instructions sitting in the datapath behind a branch or a trap.

### Current stall logic
- Stall logic is currently unneeded and completely unimplemented.
- The multi-cycle waits for memory operations and execution steps to finish before allowing the Program Counter to move to the next instruction.
- Moving to a 5-stage pipeline will force the implementation of a Hazard Detection Unit to stall the IF and ID stages.

### Current forwarding logic
- Since the writeback stage finishes updating the register file before the next instruction is even fetched, there is no need for forwarding logic.
- The pipeline upgrade will require a Forwarding Unit to route data backward from the EX to EX stage and MEM to EX stage.

### Planned branch-prediction point
- Basic branch prediction logic will be implemented in the Decode stage.
- Once the decode logic identifies the instruction as a branch, the processor can make an early assumption to either continue fetching sequentially or assume the branch is taken.
- Implementing static prediction, such as assuming all backward branches are taken for loops and forward branches are not taken, will help minimize the performance penalty of control hazards.

### Planned branch-resolution point
- Actual branch resolution and verification will occur in the Execute stage.
- The ALU will physically compute the target address and evaluate the exact comparison condition.
- If the Execute stage determines that the early prediction was incorrect, it will flush the IF and ID stages of the pipeline.